// #include <efi.h>
// #include <efilib.h>


#include <uefi.h>

int main(int argc, char **argv)
{
    printf("Hello World!\n");
    printf("oi mate\n");
    while (1);
    return 0;
}

// EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
// {
//     EFI_STATUS Status;
//     EFI_INPUT_KEY Key;
//
//     /* Store the system table for future use in other functions */
//     ST = SystemTable;
//
//     // Disable the watchdog from resetting us after five minutes
//     ST->BootServices->SetWatchdogTimer(0, 0, 0, NULL);
//
//     /* Say hi */
//     Status = ST->ConOut->OutputString(ST->ConOut, L"Hello World\r\n"); // EFI Applications use Unicode and CRLF, a la Windows
//     if (EFI_ERROR(Status))
//         return Status;
//
//     /* Now wait for a keystroke before continuing, otherwise your
//        message will flash off the screen before you see it.
//
//        First, we need to empty the console input buffer to flush
//        out any keystrokes entered before this point */
//     Status = ST->ConIn->Reset(ST->ConIn, FALSE);
//     if (EFI_ERROR(Status))
//         return Status;
//
//     /* Now wait until a key becomes available.  This is a simple
//        polling implementation.  You could try and use the WaitForKey
//        event instead if you like */
//     while ((Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY) ;
//
//     return Status;
// }
