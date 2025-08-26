#include <graphics.hpp>
#include <uefi.h>

FramebufferDescriptor gopSetup() {
    FramebufferDescriptor out;
    efi_guid_t gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t* gop      = nullptr;
    efi_status_t status = BS->LocateProtocol(&gop_guid, NULL, (void**)&gop);
    if (EFI_ERROR(status) || !gop) {
        printf("Failed to initialize GOP\n");
        return out;
    }
    status = gop->SetMode(gop, 0);
    ST->ConOut->Reset(ST->ConOut, 0);
    ST->StdErr->Reset(ST->StdErr, 0);
    if (EFI_ERROR(status)) {
        printf("failed to set video mode\n");
        return out;
    }
    out.framebuffer = (unsigned int*)gop->Mode->FrameBufferBase;
    out.width       = gop->Mode->Information->HorizontalResolution;
    out.height      = gop->Mode->Information->VerticalResolution;
    out.pitch       = /* (sizeof(unsigned int)) *  */ gop->Mode->Information
                    ->PixelsPerScanLine;
    out.format = gop->Mode->Information->PixelFormat ==
                         PixelRedGreenBlueReserved8BitPerColor
                     ? RGBR
                     : BGRR;

    return out;
}
