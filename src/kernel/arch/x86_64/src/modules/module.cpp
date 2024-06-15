#include "elf.hpp"
#include "libmem/string.hpp"
#include "logs/logs.hpp"
#include "memory/heap.hpp"
#include "util/map.hpp"
#include <cstdint>
#include <modules/module.hpp>
#include <util/util.hpp>
#include <libmem/mem.hpp>
#include <modules/spine.hpp>

struct LoadedSectInfo {
    uint64_t size;
    uint8_t* loc;
};

namespace modules {
Module::Module(uint8_t* loaded_elf) {
    // TODO: do checks
    const elf::ElfHeader* hdr = (elf::ElfHeader*) loaded_elf;
    // First, load sections
    util::Map<uint64_t, LoadedSectInfo> sectLocations;
    elf::SymbolTableEntry* symtab;
    uint64_t symtab_length;
    uint8_t* strtab;
    for (uint64_t i = 0; i < hdr->sectHeaderTableLength; i++) {
        elf::SectHeader* shdr = (elf::SectHeader*)(loaded_elf + hdr->sectHeaderPos + hdr->sectEntrySize * i);
        if (shdr->flags & elf::SectFlags::SHF_ALLOC) {
            // Make sure we follow alignment
            uint8_t* sect = (uint8_t*)kmalloc(shdr->fileSize + shdr->alignment - 1);
            sect += (-(int64_t)((uint64_t)sect % shdr->alignment)) % shdr->alignment;
            // load it!
            if (shdr->type != elf::SectHeaderType::SHT_NOBITS) {
                memcpy(sect, loaded_elf + shdr->fileOffset, shdr->fileSize);
            }
            // add it to the map
            sectLocations.set(i, (LoadedSectInfo){.size = shdr->fileSize, .loc = sect});
        }
        if (shdr->type == elf::SectHeaderType::SHT_SYMTAB) {
            symtab = (elf::SymbolTableEntry*)(loaded_elf + shdr->fileOffset);
            symtab_length = shdr->fileSize / shdr->entrySize;
        }
        if (shdr->type == elf::SectHeaderType::SHT_STRTAB) {
            strtab = loaded_elf + shdr->fileOffset;
        }
    }
    // After everything is loaded, handle relocations
    for (uint64_t i = 0; i < hdr->sectHeaderTableLength; i++) {
        elf::SectHeader* shdr = (elf::SectHeader*)(loaded_elf + hdr->sectHeaderPos + hdr->sectEntrySize * i);
        if (shdr->type == elf::SectHeaderType::SHT_RELA) {
            elf::Rela* rela = (elf::Rela*)(loaded_elf + shdr->fileOffset);
            // find relevant symbol table
            elf::SectHeader* sthdr = (elf::SectHeader*)(loaded_elf + hdr->sectHeaderPos + hdr->sectEntrySize * shdr->link);
            elf::SymbolTableEntry* symtab = (elf::SymbolTableEntry*)(loaded_elf + sthdr->fileOffset);
            // find relevant section position
            uint64_t idx = shdr->info;
            // If it's garbage, skip
            if (!sectLocations.hasKey(idx)) continue; 
            LoadedSectInfo lsi = sectLocations.get(idx);
            uint8_t* sect = lsi.loc;
            bool valid = true;
            // TODO: make this loop better
            while ((uint64_t)rela - ((uint64_t)loaded_elf + shdr->fileOffset) < shdr->fileSize) {
                // handle the relocation
                // If it's too big, skip
                if (rela->offset >= lsi.size) continue;
                uint8_t* loc = sect + rela->offset;
                switch (rela->type) {
                    // We can handle PLT32 as PC32: https://stackoverflow.com/questions/64424692/how-does-the-address-of-r-x86-64-plt32-computed
                    // It's just function + addend - loc
                    case elf::RelocType::R_X86_64_PC32 :
                    case elf::RelocType::R_X86_64_PLT32 : {
                        uint32_t* l = (uint32_t*) loc;
                        elf::SymbolTableEntry symTE = symtab[rela->sym];
                        char* name = (char*)strtab + symTE.nameOffset;
                        if (!spineContains(name)) {
                            logs::info << "Spine loading could not find symbol: " << name << '\n';
                            valid = false;
                        }
                        void* sym = spineGet(name);
                        // TODO: switch t for actual function called
                        *l = ((uint64_t)sym) - (uint64_t)loc + rela->addend;
                        break;
                    }
                    case elf::RelocType::R_X86_64_REX_GOTPCRELX : {
                        panic("Unhandled REX reloc");
                        break;
                    }
                    case elf::RelocType::R_X86_64_64 : {
                        uint64_t* l = (uint64_t*) loc;
                        elf::SymbolTableEntry symTE = symtab[rela->sym];
                        if (symTE.type == elf::SymbolType::STT_SECTION) {
                            // has it been loaded?
                            uint64_t sIdx = symTE.sectionIndex;
                            if (!sectLocations.hasKey(sIdx)) panic("Did not load sect");
                            LoadedSectInfo& lsi = sectLocations.get(sIdx);
                            *l = ((uint64_t)lsi.loc) + rela->addend;
                        } else {
                            char* name = (char*)strtab + symTE.nameOffset;
                            if (!spineContains(name)) {
                                logs::info << "Spine loading could not find symbol: " << name << '\n';
                                valid = false;
                            }
                            void* sym = spineGet(name);
                            // TODO: switch t for actual function called
                            *l = ((uint64_t)sym) + rela->addend;
                        }
                        break;

                    }
                    default : {
                        break;
                        // panic("Unknown relocation")
                    }
                }
                rela = (elf::Rela*)(((uint64_t)rela) + shdr->entrySize);
            }
            if (!valid) panic("Unknown sym");
        }
    }
    void* load_addr = nullptr;
    void* unload_addr = nullptr;
    // Find the load + unload function addrs
    for (uint64_t i = 0; i < symtab_length; i++) {
        elf::SymbolTableEntry sym = symtab[i];
        uint8_t* name = strtab + sym.nameOffset;
        if (libmem::strcmp("module_load", (char*)name)) {
            uint64_t idx = sym.sectionIndex;
            if (!sectLocations.hasKey(idx)) continue; 
            LoadedSectInfo lsi = sectLocations.get(idx);
            uint8_t* sect = lsi.loc;
            load_addr = sect + sym.value;
        } else if (libmem::strcmp("module_unload", (char*)name)) {
            uint64_t idx = sym.sectionIndex;
            if (!sectLocations.hasKey(idx)) continue; 
            LoadedSectInfo lsi = sectLocations.get(idx);
            uint8_t* sect = lsi.loc;
            unload_addr = sect + sym.value;
        }
    }
    load = (void(*)())load_addr;
    unload = (void(*)())unload_addr;
    inmemory = true;
    loaded = false;
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
