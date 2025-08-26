#ifndef COMMON_ELF_HPP
#define COMMON_ELF_HPP

#define ELF_MAGIC "\177ELF"
// 2's complement little-endian
#define ELF_DATA_2_LSB 1
// Executable file
#define ET_EXEC 2
// Loadable program segment
#define PT_LOAD 1
// AMD x86_64 architecture
#define EM_MACH 62
// program header load
#define PH_LOAD 1

#include <stdint.h>

namespace elf {
    struct ElfHeader {
        // Magic number thingy
        uint8_t ident[4];
        // 1 for 32bit, 2 for 64
        uint8_t archType;
        // 1 for little, 2 for big
        uint8_t endianness;
        // ELF header version
        uint8_t headerVersion;
        // OS ABI (whatever that means)
        uint8_t ABI;
        uint8_t _padding[8];
        // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
        uint16_t objType;
        // instruction set
        uint16_t instructionSet;
        // elf version
        uint32_t elfVersion;
        // program entry position
        uint64_t entryPointPos;
        // program header table position
        uint64_t progHeaderPos;
        // section header table position
        uint64_t sectHeaderPos;
        // arch-specific flags
        uint32_t flags;
        // header size in bytes
        uint16_t headerSize;
        // program header table entry size
        uint16_t progEntrySize;
        // program header table entry count
        uint16_t progHeaderTableLength;
        // section header table entry size
        uint16_t sectEntrySize;
        // section header table entry count
        uint16_t sectHeaderTableLength;
        // index of section names in the section header
        uint16_t sectHeaderNamesIndex;
    } __attribute__((packed));
    struct ProgHeader {
        // segment type:
        // 0 = null entry
        // 1 = load: clear memSize bytes at virtAddr, then copy fileSize bytes
        // into it from fileOffset 2 = dynamic: requires dynamic linking (TODO)
        // 3 = interp: contains a file path to use as the interpreter for the
        // following segment 4 = note 5 = reserved 6 = segment containing the
        // program header itself 7 = thread-local storage template
        //
        // there are more, but those are the most relevant
        uint32_t type;
        // 1 = executable, 2 = writable, 4 = readable
        uint32_t flags;
        // where in the file can the data be found?
        uint64_t fileOffset;
        // where in memory should we load this segment to?
        uint64_t virtAddr;
        // physical address of the segment
        uint64_t physAddr;
        // how big is it in the file
        uint64_t fileSize;
        // how big should it be in memory
        uint64_t memSize;
        // required alignment, should be a power of 2
        uint64_t alignment;
    } __attribute__((packed));
    namespace SectHeaderType {
        enum SectHeaderType {
            // Section header table entry unused
            SHT_NULL = 0x0,
            // Program data
            SHT_PROGBITS = 0x1,
            // Symbol table
            SHT_SYMTAB = 0x2,
            // String table
            SHT_STRTAB = 0x3,
            // Relocation entries with addends
            SHT_RELA = 0x4,
            // Symbol hash table
            SHT_HASH = 0x5,
            // Dynamic linking information
            SHT_DYNAMIC = 0x6,
            // Notes
            SHT_NOTE = 0x7,
            // Program space with no data (bss)
            SHT_NOBITS = 0x8,
            // Relocation entries, no addends
            SHT_REL = 0x9,
            // Reserved
            SHT_SHLIB = 0x0A,
            // Dynamic linker symbol table
            SHT_DYNSYM = 0x0B,
            // Array of constructors
            SHT_INIT_ARRAY = 0x0E,
            // Array of destructors
            SHT_FINI_ARRAY = 0x0F,
            // Array of pre-constructors
            SHT_PREINIT_ARRAY = 0x10,
            // Section group
            SHT_GROUP = 0x11,
            // Extended section indices
            SHT_SYMTAB_SHNDX = 0x12,
            // Number of defined types.
            SHT_NUM = 0x13,
        };
    }
    namespace SectionGroupFlags {
        enum SectionGroupFlags {
            GRP_COMDAT   = 0x1,
            GRP_MASKOS   = 0x0ff00000,
            GRP_MASKPROC = 0xf0000000,
        };
    }

    namespace SectFlags {
        enum SectFlags : uint64_t {
            // Writable
            SHF_WRITE = 0x1,
            // Occupies memory during execution
            SHF_ALLOC = 0x2,
            // Executable
            SHF_EXECINSTR = 0x4,
            // Might be merged
            SHF_MERGE = 0x10,
            // Contains null-terminated strings
            SHF_STRINGS = 0x20,
            // 'sh_info' contains SHT index
            SHF_INFO_LINK = 0x40,
            // Preserve order after combining
            SHF_LINK_ORDER = 0x80,
            // Non-standard OS specific handling required
            SHF_OS_NONCONFORMING = 0x100,
            // Section is member of a group
            SHF_GROUP = 0x200,
            // Section hold thread-local data
            SHF_TLS = 0x400,
            // OS-specific
            SHF_MASKOS = 0x0FF00000,
            // Processor-specific
            SHF_MASKPROC = 0xF0000000,
            // Special ordering requirement (Solaris)
            SHF_ORDERED = 0x4000000,
            // Section is excluded unless referenced or allocated (Solaris)
            SHF_EXCLUDE = 0x8000000,
        };
    }
    struct SectHeader {
        // offset into string section that represents the name of this section
        uint32_t nameOffset;
        // The type of this header
        SectHeaderType::SectHeaderType type : 32;
        // Flags
        SectFlags::SectFlags flags : 64;
        // Virtual addr of section in memory, if it's loaded
        uint64_t virtAddr;
        // Location of section in file image
        uint64_t fileOffset;
        // size of section in file image
        uint64_t fileSize;
        // section idx of associated section
        uint32_t link;
        // extra info
        uint32_t info;
        // alignment, must be a power of 2
        uint64_t alignment;
        // size of each entry in this section
        // if this section contains fixed-size entries, else zero
        uint64_t entrySize;
    } __attribute__((packed));
    enum SymbolBinding : uint8_t {
        STB_LOCAL  = 0,
        STB_GLOBAL = 1,
        STB_WEAK   = 2,
        STB_LOOS   = 10,
        STB_HIOS   = 12,
        STB_LOPROC = 13,
        STB_HIPROC = 15,
    };
    enum SymbolType : uint8_t {
        STT_NOTYPE  = 0,
        STT_OBJECT  = 1,
        STT_FUNC    = 2,
        STT_SECTION = 3,
        STT_FILE    = 4,
        STT_COMMON  = 5,
        STT_TLS     = 6,
        STT_LOOS    = 10,
        STT_HIOS    = 12,
        STT_LOPROC  = 13,
        STT_HIPROC  = 15,
    };
    enum SymbolVisibility : uint8_t {
        STV_DEFAULT   = 0,
        STV_INTERNAL  = 1,
        STV_HIDDEN    = 2,
        STV_PROTECTED = 3,
    };
    struct SymbolTableEntry {
        // Offset into string table of this symbol's name
        uint32_t nameOffset;
        // Extra info (type and binding attributes)
        SymbolType type       : 4;
        SymbolBinding binding : 4;
        // Other (seems to define visibility type)
        SymbolVisibility visibility : 3;
        uint8_t padding             : 5;
        // Section index
        uint16_t sectionIndex;
        // Value
        uint64_t value;
        // Size (0 if no/unknown size)
        uint64_t size;
    } __attribute__((packed));
    static_assert(sizeof(SymbolTableEntry) == 0x18);
    // for definitions of variables as:
    // A Represents the addend used to compute the value of the relocatable
    // field. B Represents the base address at which a shared object has been
    // loaded into memory during execution. Generally, a shared object is built
    // with a 0 base virtual address, but the execution address will be
    // different. G Represents the offset into the global offset table at which
    // the relocation entry’s symbol will reside during execution. GOT
    // Represents the address of the global offset table. L Represents the place
    // (section offset or address) of the Procedure Linkage Table entry for a
    // symbol. P Represents the place (section offset or address) of the storage
    // unit being relocated (computed using r_offset). S Represents the value of
    // the symbol whose index resides in the relocation entry. Z Represents the
    // size of the symbol whose index resides in the relocation entry. The AMD64
    // ABI architectures uses only Elf64_Rela relocation entries with explicit
    // addends. The r_addend member serves as the relocation addend.
    enum RelocType {
        // none, none
        R_X86_64_NONE = 0,
        // word64, S + A
        R_X86_64_64 = 1,
        // word32, S + A - P
        R_X86_64_PC32 = 2,
        // word32, G + A
        R_X86_64_GOT32 = 3,
        // word32, L + A - P
        R_X86_64_PLT32 = 4,
        // none, none
        R_X86_64_COPY = 5,
        // word64, S
        R_X86_64_GLOB_DAT = 6,
        // word64, S
        R_X86_64_JUMP_SLOT = 7,
        // word64, B + A
        R_X86_64_RELATIVE = 8,
        // word32, G + GOT + A - P
        R_X86_64_GOTPCREL = 9,
        // word32, S + A
        R_X86_64_32 = 10,
        // word32, S + A
        R_X86_64_32S = 11,
        // word16, S + A
        R_X86_64_16 = 12,
        // word16, S + A - P
        R_X86_64_PC16 = 13,
        // word8, S + A
        R_X86_64_8 = 14,
        // word8, S + A - P
        R_X86_64_PC8 = 15,
        // word64, undef
        R_X86_64_DTPMOD64 = 16,
        // word64, undef
        R_X86_64_DTPOFF64 = 17,
        // word64, undef
        R_X86_64_TPOFF64 = 18,
        // word32, undef
        R_X86_64_TLSGD = 19,
        // word32, undef
        R_X86_64_TLSLD = 20,
        // word32, undef
        R_X86_64_DTPOFF32 = 21,
        // word32, undef
        R_X86_64_GOTTPOFF = 22,
        // word32, undef
        R_X86_64_TPOFF32 = 23,
        // word64, S + A - P
        R_X86_64_PC64 = 24,
        // word64, S + A - GOT
        R_X86_64_GOTOFF64 = 25,
        // word32, GOT + A - P
        R_X86_64_GOTPC32 = 26,
        // word32, Z + A
        R_X86_64_SIZE32 = 32,
        // word64, Z + A
        R_X86_64_SIZE64 = 33,
        // word32, undef
        R_X86_64_GOTPC32_TLSDESC = 34,
        // none, undef
        R_X86_64_TLSDESC_CALL = 35,
        // word64×2, undef
        R_X86_64_TLSDESC = 36,
        // word64, indirect (B + A)
        R_X86_64_IRELATIVE     = 37,
        R_X86_64_REX_GOTPCRELX = 42,
    };
    struct Rela {
        // Offset (for a relocatable, from beginning of section)
        uint64_t offset;
        // Relocation type and symbol index
        // uint64_t info;
        RelocType type : 32;
        uint32_t sym;
        int64_t addend;
    } __attribute__((packed));
    inline bool isUsableElfExeHeader(ElfHeader* header) {
        return (
                   // does magic match?
                   header->ident[0] == ELF_MAGIC[0] &&
                   header->ident[1] == ELF_MAGIC[1] &&
                   header->ident[2] == ELF_MAGIC[2] &&
                   header->ident[3] == ELF_MAGIC[3]
               ) &&
               // is it 64-bit?
               header->archType == 2 &&
               // is it little-endian?
               header->endianness == 1 &&
               // is it executable?
               header->objType == 2 &&
               // is it the right architecture?
               header->instructionSet == EM_MACH &&
               // do we have program headers?
               header->progHeaderTableLength > 0;
    }
}

#endif // !COMMON_ELF_HPP
