#ifndef LOADER_FILE_HPP
#define LOADER_FILE_HPP

#include <efi.h>

UINT8* ReadFile(CHAR16* fileName, EFI_FILE_HANDLE volume);

#endif
