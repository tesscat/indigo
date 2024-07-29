// #include "acpi/acpi.hpp"
// #include "inator/inator.hpp"
#include "elf.hpp"
#include "graphics/screen.hpp"
#include "libmem/string.hpp"
#include "libstd/string.hpp"
#include "memory/page_alloc.hpp"
#include "util/map.hpp"
#include "util/util.hpp"
#include <memory/phys_alloc.hpp>
namespace modules {

util::Map<String, void*> spine;

bool spineContains(const char* name) {
    String s (name);
    return spine.hasKey(s);
}

void* spineGet(const char* name) {
    String s (name);
    return spine.get(s);
}

void spineSet(const char* name, void* fn) {
    String s (name);
    spine.set(s, fn);
}

void initSpine() {
    // Map the kernel elf thingy
    memory::kernelIdentityMapBlock((uint64_t)kargs->kElf, kargs->kElfLen);
    // do the ELF
    elf::ElfHeader* kHeader = (elf::ElfHeader*)kargs->kElf;
    elf::SectHeader* sectTable = (elf::SectHeader*)(((uint64_t)kHeader) + kHeader->sectHeaderPos);
    elf::SectHeader* strTabHeader = (elf::SectHeader*)((uint64_t(sectTable)) + kHeader->sectHeaderNamesIndex*kHeader->sectEntrySize);
    char* shStrTab = (char*)(((uint64_t)kHeader) + strTabHeader->fileOffset);
    char* strTab = nullptr;
    // First, find the Actual strtab
    elf::SectHeader* curr = sectTable;
    for (int i = 0; i < kHeader->sectHeaderTableLength; i++, *((uint64_t*)&curr) += kHeader->sectEntrySize) {
        // is this the correct type?
        if (curr->type == elf::SectHeaderType::SHT_STRTAB) {
            // Is this ".strtab"?
            char* name = shStrTab + curr->nameOffset;
            if(libmem::strcmp(name, ".strtab")) {
                // yippee!
                strTab = (char*)kHeader + curr->fileOffset;
                break;
            }
        }
    }
    if (strTab == nullptr) {
        panic("Unable to find strtab in kernel elf");
    }
    // for all symbol tables
    curr = sectTable;
    for (int i = 0; i < kHeader->sectHeaderTableLength; i++, *((uint64_t*)&curr) += kHeader->sectEntrySize) {
        // is this a symbol table?
        if (curr->type == elf::SectHeaderType::SHT_SYMTAB) {
            // okay then, find it
            elf::SymbolTableEntry* ste = (elf::SymbolTableEntry*)(((uint64_t)kHeader) + curr->fileOffset);
            for (; (uint64_t)ste < curr->fileSize + curr->fileOffset + (uint64_t)kHeader; *((uint64_t*)&ste) += curr->entrySize) {
                // if the value isn't zero, it's visible and globally linked;
                if (ste->value != 0 && ste->visibility == elf::SymbolVisibility::STV_DEFAULT) {
                    // Find the name
                    char* name = strTab + ste->nameOffset;
                    // and add it!
                    spineSet(name, (void*)ste->value);
                }
            }
        }
    }
    // free + unmap block
    memory::kernelIdentityUnmapBlock((uint64_t)kargs->kElf, kargs->kElfLen);
    memory::freeBlock((uint64_t)kargs->kElf, kargs->kElfLen);
}
}
