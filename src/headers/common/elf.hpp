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
struct ExeHeader {
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
};
struct ProgHeader {
  // segment type:
  // 0 = null entry
  // 1 = load: clear memSize bytes at virtAddr, then copy fileSize bytes into it from fileOffset
  // 2 = dynamic: requires dynamic linking (TODO)
  // 3 = interp: contains a file path to use as the interpreter for the following segment
  // 4 = note
  //
  // there are more, but those are the most relevant
  uint32_t type;
  // 1 = executable, 2 = writable, 4 = readable
  uint32_t flags;
  // where in the file can the data be found?
  uint64_t fileOffset;
  // where in memory should we load this segment to?
  uint64_t virtAddr;
  // undefined for SYSV ABI, posix-uefi claims it's physical addr?
  uint64_t padding;
  // how big is it in the file
  uint64_t fileSize;
  // how big should it be in memory
  uint64_t memSize;
  // required alignment, should be a power of 2
  uint64_t alignment;
};

inline bool isUsableElfExeHeader(ExeHeader* header) {
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
