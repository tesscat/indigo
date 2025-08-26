#include "graphics/psf.hpp"
#include "util/util.hpp"
#include <graphics/screen.hpp>
#include <libmem/mem.hpp>

namespace graphics::screen {
    uint32_t* backBuffer;
    uint64_t backBufferEnd;

    // TODO: rewrite when we have kernel malloc()
    void initScreen() {
        // pick a location for the backBuffer
        // we know that EFI is kind of wasteful, and the kernel itself starts at
        // 32MB we need width*height*sizeof(uint32_t) that doesn't clash with
        // framebuffer, or kargs, or kernel (so less than 32MB) we use width not
        // pitch because we'll copy row-by-row anyway

        uint64_t startKargs = (uint64_t)kargs;
        uint64_t endKargs   = startKargs + kargs->totalSize;

        uint64_t startFb    = (uint64_t)kargs->framebuffer;
        uint64_t endFb =
            startFb + (sizeof(uint32_t) * kargs->fbPitch * kargs->fbHeight);

        uint64_t candidates[] = {endKargs + 8};
        uint64_t i            = 0;
        bool works            = true;

        for (i = 0; i < 1; i++) {
            uint64_t startPtr = candidates[i];
            uint64_t endPtr   = startPtr + (sizeof(uint32_t) * kargs->fbWidth *
                                          kargs->fbHeight);
            works             = true;
            // does it clash with Kargs?
            if ((startPtr >= startKargs && startPtr <= endKargs) ||
                (endPtr >= startKargs && endPtr <= endKargs))
                // borked
                works = false;
            // does it clash with fb?
            if ((startPtr >= startFb && startPtr <= endFb) ||
                (endPtr >= startFb && endPtr <= endFb))
                works = false;

            if (works) break;
        }

        if (works) {
            backBuffer    = (uint32_t*)candidates[i];
            backBufferEnd = candidates[i] +
                            sizeof(uint32_t) * kargs->fbWidth * kargs->fbHeight;
        } else {
            // better hope it doesn't scroll
            graphics::psf::putchar('X', 10, 10, 0x00FFFFFF, 0x00000000);
            loop_forever;
        }
    }

    // such that pixel is RGBA
    void plotPixel(unsigned int x, unsigned int y, unsigned int pixel) {
        if (kargs->fbIsBGR) {
            // output format is for some reason 0x00rrggbb
            pixel = pixel >> 8;
        } else {
            // who knows
            // TODO: what is this??
        }
        kargs->framebuffer[x + y * kargs->fbPitch] = pixel;
        backBuffer[x + y * kargs->fbWidth]         = pixel;
    }

    void scrollDown(unsigned int lines, uint32_t fill) {
        if (lines > kargs->fbHeight) lines = kargs->fbHeight;
        // copy n + lines-th line to framebuffer
        for (unsigned int i = 0; i < (kargs->fbHeight - lines); i++) {
            memcpy(
                &kargs->framebuffer[kargs->fbPitch * (i)],
                &backBuffer[(i + lines) * kargs->fbWidth],
                kargs->fbWidth * sizeof(unsigned int)
            );
            // update backBuffer accordingly
            memcpy(
                &backBuffer[kargs->fbWidth * (i)],
                &backBuffer[(i + lines) * kargs->fbWidth],
                kargs->fbWidth * sizeof(unsigned int)
            );
        }
        // fill the rest
        for (unsigned int i = kargs->fbHeight - lines; i < kargs->fbHeight;
             i++) {
            for (unsigned int x = 0; x < kargs->fbWidth; x++) {
                plotPixel(x, i, fill);
            }
        }
    }
}
