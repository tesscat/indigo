#include <stdint.h>
#define _STDINT_H
#include <uefi.h>
#include <elf.hpp>
#include <file.hpp>
#include <ion_efi/ion_efi.hpp>
#include <kernel.hpp>
#include <loader/kernel_args.hpp>
#include <graphics.hpp>

static inline void plotPixel(int x, int y, uint32_t* fb, uint64_t width, uint64_t pitch, uint32_t pixel)
{
   fb[(width)*y + x] = pixel;
}

Kernel::Kernel(const char* path, ion::RootNode* root) {
  // load us up
  BinaryFile file = readBinaryFile(path);
  if (file.data == nullptr) {
    printf("Kernel not found or unreadable");
    return;
  }
  // are we ELF?
  elf::ExeHeader* header = (elf::ExeHeader*) file.data;
  if (!elf::isUsableElfExeHeader(header)) {
    printf("unusable kernel, invalid ELF header");
    return;
  }
  // printf("phsz: %i\n", header->progEntrySize);

  // load segments
  // elf::ProgHeader* prog_header = (elf::ProgHeader*)((uint8_t*)file.data + header->progHeaderPos);
  for (int i = 0; i < header->progHeaderTableLength; i++) {
    elf::ProgHeader* prog_header = (elf::ProgHeader*)((uint8_t*)file.data + header->progHeaderPos + i*header->progEntrySize);
    // printf("iterint point %i\n", i);
    // printf("pht: %i\n", prog_header->type);
    // printf("o: %x v: %x\n", prog_header->fileOffset, prog_header->virtAddr);
    if (prog_header->type == PH_LOAD) {
      // printf("Setting %i in memory\n", i);
      memcpy((void*)prog_header->virtAddr, (uint8_t*)file.data + prog_header->fileOffset, prog_header->fileSize);
      memset(((uint8_t*)prog_header->virtAddr + prog_header->fileSize), 0, prog_header->memSize - prog_header->fileSize);
    }
  }


  void* entry_loc = (void*)header->entryPointPos;
  entry = (void (* __attribute__((sysv_abi)))(KernelArgs*)) entry_loc;

  // printf("entry pt: %x\n", entry_loc);
  // while(1);

  delete (uint8_t*)file.data;
}

void Kernel::Run(size_t argc, char** argv) {
  // calculate the size of KernelArgs
  size_t size = sizeof(KernelArgs);
  // add on the bit for the char*-s
  size += argc*sizeof(char*);
  // add on for each of the strings themselves
  for (size_t i = 0; i < argc; i++) {
    // plus null-termi
    size += strlen(argv[i]) + 1;
  }
  // ok allocate it
  KernelArgs* args = (KernelArgs*) malloc(size);
  args->totalSize = size;


  // do the graphics
  FramebufferDescriptor fb = gopSetup();
  // do the struct-filling
  args->framebuffer = fb.framebuffer;
  args->fbWidth = fb.width;
  args->fbHeight = fb.height;
  args->fbPitch = fb.pitch;

  // printf("w: %i, h: %i, p: %i fb: %x\n", args->fbWidth, args->fbHeight, args->fbPitch, args->framebuffer);
  // while(1);

  // memset(args->framebuffer, -1, fb.height * (fb.width + fb.pitch));

  // string wrangling, fun times
  char** end = (char**) ((uint8_t*)args + size);
  args->argv = end;
  // copy over the char*s
  memcpy(end, argv, sizeof(char*) * argc);
  char* currentStr = (char*)(end + sizeof(char*)*argc);
  for (size_t i = 0; i < argc; i++) {
    // TODO: maybe deduplicate the strlen calls
    size_t len = strlen(argv[i]);
    memcpy(currentStr, argv[i], len);
    currentStr[len] = '\0';
    currentStr += (len + 1);
  }

  // printf("p: %i w: %i h: %i\n", args->fbPitch, args->fbWidth, args->fbHeight);

  
  // escape from the bootservices!!
  if (exit_bs() != 0) {
    printf("Failed to exit bootservices. Truly, we are trapped.\n");
    while(1);
  }
  // for (uint64_t i = 0; i < (args->fbPitch * args->fbHeight); i++) {
    // args->framebuffer[i] = 0xFFFFFFFF;
  // }
  // while(1);
  // okay we should be good to go
  entry(args);
  while(1);
}
