#include <file.h>

#include <efi.h>
#include <efilib.h>
#include <x86_64/efibind.h>

UINT64 FileSize(EFI_FILE_HANDLE fileHandle) {
  UINT64 ret;
  EFI_FILE_INFO* fileInfo;
  fileInfo = LibFileInfo(fileHandle);
  ret = fileInfo->FileSize;

  FreePool(fileInfo);
  return ret;
}

UINT8* ReadFile(CHAR16* fileName, EFI_FILE_HANDLE volume) {
  EFI_FILE_HANDLE fileHandle;
  // open it
  if (uefi_call_wrapper(volume->Open, 5, volume, &fileHandle, fileName, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM)
    != EFI_SUCCESS) {
    ST->ConOut->OutputString(ST->ConOut, L"Failed to open file: ");
    ST->ConOut->OutputString(ST->ConOut, fileName);
    return NULL;
  }

  UINT64 readSize = FileSize(fileHandle);
  UINT8* buffer = (UINT8*)AllocatePool(readSize);

  uefi_call_wrapper(fileHandle->Read, 3, fileHandle, &readSize, buffer);

  // close the file
  uefi_call_wrapper(fileHandle->Close, 1, fileHandle);

  return buffer;
}
