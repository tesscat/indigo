#include "elf.hpp"
#include "libmem/string.hpp"
#include "logs/logs.hpp"
#include "memory/heap.hpp"
#include "sync/spinlock.hpp"
#include "util/map.hpp"
#include "util/vec.hpp"
#include <cstdint>
#include <libmem/mem.hpp>
#include <modules/module.hpp>
#include <modules/spine.hpp>
#include <util/util.hpp>

struct LoadedSectInfo {
    uint64_t size;
    uint8_t* loc;
};

namespace modules {

    sync::Spinlock moduleLock;
    // TODO: switch to a hashset once we have those
    util::Vec<String> loadedComdats;

    Module::Module(uint8_t* loaded_elf) {
        moduleLock.lock();
        // TODO: do checks
        const elf::ElfHeader* hdr = (elf::ElfHeader*)loaded_elf;

        // elf::SymbolTableEntry* symtab;
        // uint64_t symtab_length;
        uint8_t* strtab;
        // Find the symbol table and strtab {{{
        for (uint64_t i = 0; i < hdr->sectHeaderTableLength; i++) {
            elf::SectHeader* shdr =
                (elf::SectHeader*)(loaded_elf + hdr->sectHeaderPos +
                                   hdr->sectEntrySize * i);
            if (shdr->type == elf::SectHeaderType::SHT_SYMTAB) {
                // symtab = (elf::SymbolTableEntry*)(loaded_elf +
                // shdr->fileOffset); symtab_length = shdr->fileSize /
                // shdr->entrySize;
            }
            if (shdr->type == elf::SectHeaderType::SHT_STRTAB) {
                strtab = loaded_elf + shdr->fileOffset;
            }
        }
        // }}}

        // we need to keep track of which ones we skip
        // TODO: this should be a set not a vect
        util::Vec<uint64_t> skippedSectIdxs;
        // First we should inspect COMDAT sections for deduplication
        for (uint64_t i = 0; i < hdr->sectHeaderTableLength; i++) {
            elf::SectHeader* shdr =
                (elf::SectHeader*)(loaded_elf + hdr->sectHeaderPos +
                                   hdr->sectEntrySize * i);
            if (shdr->type == elf::SectHeaderType::SHT_GROUP) {
                uint32_t* group = (uint32_t*)(loaded_elf + shdr->fileOffset);
                // is this a COMDAT group?
                if (group[0] & elf::SectionGroupFlags::GRP_COMDAT) {
                    elf::SectHeader* symtab_hdr =
                        (elf::SectHeader*)(loaded_elf + hdr->sectHeaderPos +
                                           hdr->sectEntrySize * shdr->link);
                    elf::SymbolTableEntry* symtab_local =
                        (elf::SymbolTableEntry*)(loaded_elf +
                                                 symtab_hdr->fileOffset);
                    elf::SymbolTableEntry* signature =
                        &symtab_local[shdr->info];
                    char* sig_name = (char*)(strtab + signature->nameOffset);
                    String sig(sig_name);
                    // it's also possible that the kernel itself has defs for
                    // these
                    if (loadedComdats.Contains(sig) || spine.hasKey(sig)) {
                        // skip it
                        for (uint8_t* curr = shdr->entrySize + (uint8_t*)group;
                             curr <
                             loaded_elf + shdr->fileOffset + shdr->fileSize;
                             curr += shdr->entrySize) {
                            uint32_t* currEnt = (uint32_t*)curr;
                            skippedSectIdxs.Append(*currEnt);
                            logs::info << "skipped section "
                                       << (uint64_t)*currEnt << "\n";
                        }
                    } else {
                        // register it
                        loadedComdats.Append(sig);
                    }
                }
            }
        }

        // load sections
        util::Map<uint64_t, LoadedSectInfo> sectLocations;
        for (uint64_t i = 0; i < hdr->sectHeaderTableLength; i++) {
            if (skippedSectIdxs.Contains(i)) continue;
            elf::SectHeader* shdr =
                (elf::SectHeader*)(loaded_elf + hdr->sectHeaderPos +
                                   hdr->sectEntrySize * i);
            if (shdr->flags & elf::SectFlags::SHF_ALLOC) {
                // Make sure we follow alignment
                uint8_t* sect =
                    (uint8_t*)kmalloc(shdr->fileSize + shdr->alignment - 1);
                sect += (-(int64_t)((uint64_t)sect % shdr->alignment)) %
                        shdr->alignment;
                // load it!
                if (shdr->type != elf::SectHeaderType::SHT_NOBITS) {
                    memcpy(sect, loaded_elf + shdr->fileOffset, shdr->fileSize);
                }
                // add it to the map
                LoadedSectInfo loadedSect{.size = shdr->fileSize, .loc = sect};
                sectLocations.set(i, loadedSect);
            }
        }

        void* load_addr   = nullptr;
        void* unload_addr = nullptr;

        // util::Map<String, void*> local_bindings;

        // register things in symtab
        for (uint64_t i = 0; i < hdr->sectHeaderTableLength; i++) {
            if (skippedSectIdxs.Contains(i)) continue;
            elf::SectHeader* shdr =
                (elf::SectHeader*)(loaded_elf + hdr->sectHeaderPos +
                                   hdr->sectEntrySize * i);
            if (shdr->type == elf::SectHeaderType::SHT_SYMTAB) {
                elf::SymbolTableEntry* ste =
                    (elf::SymbolTableEntry*)(((uint64_t)loaded_elf) +
                                             shdr->fileOffset);
                for (; (uint64_t)ste <
                       shdr->fileSize + shdr->fileOffset + (uint64_t)loaded_elf;
                     *((uint64_t*)&ste) += shdr->entrySize) {
                    // if the value isn't zero, it's visible and globally/weakly
                    // linked, and the section isn't 0 (null sect)
                    if (ste->visibility == elf::SymbolVisibility::STV_DEFAULT &&
                        (ste->binding == elf::SymbolBinding::STB_WEAK ||
                         ste->binding == elf::SymbolBinding::STB_GLOBAL) &&
                        (ste->sectionIndex != 0)) {
                        // if we haven't skipped the section
                        uint64_t sIdx = ste->sectionIndex;
                        if (!skippedSectIdxs.Contains(sIdx)) {
                            // Find the name
                            char* name = (char*)(strtab + ste->nameOffset);
                            // and add it! if not already
                            String s(name);
                            uint8_t* sectLoc = sectLocations.get(sIdx).loc;
                            // unless it's module_load or _unload
                            if (s == "module_load") {
                                load_addr = ste->value + sectLoc;
                            } else if (s == "module_unload") {
                                unload_addr = ste->value + sectLoc;
                            } else { // if (!spine.hasKey(s)) {
                                void* symbolVal = (ste->value + sectLoc);
                                spine.set(s, symbolVal);
                                logs::info << "added " << s << "\n";
                            } // else {
                            // logs::info << "duplicate symbol `" << s << "`,
                            // ignoring newer def\n";
                            // }
                        }
                    }
                    // } else if (ste->binding == elf::SymbolBinding::STB_LOCAL
                    // && ste->type != elf::SymbolType::STT_FILE &&
                    // ste->sectionIndex != 0) {
                    //     // if we haven't skipped the section
                    //     uint64_t sIdx = ste->sectionIndex;
                    //     if (!skippedSectIdxs.Contains(sIdx)) {
                    //         // Find the name
                    //         char* name = (char*)(strtab + ste->nameOffset);
                    //         // and add it!
                    //         String s(name);
                    //         uint8_t* sectLoc = sectLocations.get(sIdx).loc;
                    //         local_bindings.set(s, (void*)(ste->value +
                    //         sectLoc)); logs::info << "local added " << s <<
                    //         "\n";
                    //     }
                    // }
                }
            }
        }

        auto resolve_sym = [&](String s) {
            // if (local_bindings.hasKey(s)) return local_bindings.get(s);
            if (spine.hasKey(s)) return spine.get(s);
            return (void*)nullptr;
        };

        // After everything is loaded, handle relocations
        for (uint64_t i = 0; i < hdr->sectHeaderTableLength; i++) {
            if (skippedSectIdxs.Contains(i)) continue;
            elf::SectHeader* shdr =
                (elf::SectHeader*)(loaded_elf + hdr->sectHeaderPos +
                                   hdr->sectEntrySize * i);
            if (shdr->type == elf::SectHeaderType::SHT_RELA) {
                elf::Rela* rela = (elf::Rela*)(loaded_elf + shdr->fileOffset);
                // find relevant symbol table
                elf::SectHeader* sthdr =
                    (elf::SectHeader*)(loaded_elf + hdr->sectHeaderPos +
                                       hdr->sectEntrySize * shdr->link);
                elf::SymbolTableEntry* symtab =
                    (elf::SymbolTableEntry*)(loaded_elf + sthdr->fileOffset);
                // find relevant section position
                uint64_t idx = shdr->info;
                // If it's garbage, skip
                if (!sectLocations.hasKey(idx)) continue;
                LoadedSectInfo lsi = sectLocations.get(idx);
                uint8_t* sect      = lsi.loc;
                bool valid         = true;
                // TODO: make this loop better
                while ((uint64_t)rela -
                           ((uint64_t)loaded_elf + shdr->fileOffset) <
                       shdr->fileSize) {
                    // handle the relocation
                    // If it's too big, skip
                    if (rela->offset >= lsi.size) continue;
                    uint8_t* loc = sect + rela->offset;
                    switch (rela->type) {
                        // We can handle PLT32 as PC32:
                        // https://stackoverflow.com/questions/64424692/how-does-the-address-of-r-x86-64-plt32-computed
                        // It's just function + addend - loc
                        case elf::RelocType::R_X86_64_PC32:
                        case elf::RelocType::R_X86_64_PLT32: {
                            uint32_t* l                 = (uint32_t*)loc;
                            elf::SymbolTableEntry symTE = symtab[rela->sym];
                            char* name = (char*)strtab + symTE.nameOffset;
                            String s(name);

                            void* sym = resolve_sym(s);
                            if (!sym) {
                                logs::info
                                    << "Spine loading could not find symbol: "
                                    << name << '\n';
                                valid = false;
                            }
                            // actually write it
                            *l = ((uint64_t)sym) - (uint64_t)loc + rela->addend;
                            break;
                        }
                        case elf::RelocType::R_X86_64_REX_GOTPCRELX: {
                            panic("Unhandled REX reloc");
                            break;
                        }
                        case elf::RelocType::R_X86_64_64: {
                            uint64_t* l                 = (uint64_t*)loc;
                            elf::SymbolTableEntry symTE = symtab[rela->sym];
                            if (symTE.type == elf::SymbolType::STT_SECTION) {
                                // has it been loaded?
                                uint64_t sIdx = symTE.sectionIndex;
                                if (!sectLocations.hasKey(sIdx))
                                    panic("Did not load sect");
                                LoadedSectInfo& lsi = sectLocations.get(sIdx);
                                *l = ((uint64_t)lsi.loc) + rela->addend;
                            } else {
                                char* name = (char*)strtab + symTE.nameOffset;
                                String s(name);

                                void* sym = resolve_sym(s);
                                if (!sym) {
                                    logs::info << "Spine loading could not "
                                                  "find symbol: "
                                               << name << '\n';
                                    valid = false;
                                }
                                // TODO: switch t for actual function called
                                *l = ((uint64_t)sym) + rela->addend;
                            }
                            break;
                        }
                        case elf::RelocType::R_X86_64_32S: {
                            uint32_t* l                 = (uint32_t*)loc;
                            elf::SymbolTableEntry symTE = symtab[rela->sym];
                            if (symTE.type == elf::SymbolType::STT_SECTION) {
                                // has it been loaded?
                                uint64_t sIdx = symTE.sectionIndex;
                                if (!sectLocations.hasKey(sIdx))
                                    panic("Did not load sect");
                                LoadedSectInfo& lsi = sectLocations.get(sIdx);
                                *l = ((uint64_t)lsi.loc) + rela->addend;
                            } else {
                                char* name = (char*)strtab + symTE.nameOffset;
                                String s(name);

                                void* sym = resolve_sym(s);
                                if (!sym) {
                                    logs::info << "Spine loading could not "
                                                  "find symbol: "
                                               << name << '\n';
                                    valid = false;
                                }
                                // TODO: switch t for actual function called
                                *l = ((uint64_t)sym) + rela->addend;
                            }
                            break;
                        }
                        default: {
                            panic("Unknown relocation") break;
                        }
                    }
                    rela = (elf::Rela*)(((uint64_t)rela) + shdr->entrySize);
                }
                if (!valid) panic("Unknown sym");
            }
        }
        // Find the load + unload function addrs
        // for (uint64_t i = 0; i < symtab_length; i++) {
        //     elf::SymbolTableEntry sym = symtab[i];
        //     uint8_t* name = strtab + sym.nameOffset;
        //     if (libmem::strcmp("module_load", (char*)name)) {
        //         uint64_t idx = sym.sectionIndex;
        //         if (!sectLocations.hasKey(idx)) continue;
        //         LoadedSectInfo lsi = sectLocations.get(idx);
        //         uint8_t* sect = lsi.loc;
        //         load_addr = sect + sym.value;
        //     } else if (libmem::strcmp("module_unload", (char*)name)) {
        //         uint64_t idx = sym.sectionIndex;
        //         if (!sectLocations.hasKey(idx)) continue;
        //         LoadedSectInfo lsi = sectLocations.get(idx);
        //         uint8_t* sect = lsi.loc;
        //         unload_addr = sect + sym.value;
        //     }
        // }
        load     = (void (*)())load_addr;
        unload   = (void (*)())unload_addr;
        inmemory = true;
        loaded   = false;

        moduleLock.release();
    }

    void Module::Load() {
        if (!loaded) {
            loaded = true;
            if (load) load();
        }
    }
    void Module::Unload() {
        if (loaded) {
            loaded = false;
            if (unload) unload();
        }
    }
}
