#include <graphics/psf.hpp>
#include <graphics/screen.hpp>
#include <loader/kernel_args.hpp>

#define loop_forever {__asm__ __volatile__ ("hlt"); while(1);}

KernelArgs* kargs;

extern "C" void _start(KernelArgs* args) {
    kargs = args;
    graphics::psf::initPSF();
    graphics::screen::initScreen();
    for (unsigned int x = 0; x < kargs->fbWidth; x++) {
        for (unsigned int y = 0; y < kargs->fbHeight; y++) {
            unsigned int pixel = ((255*x)/kargs->fbWidth & 0xFF) | 0xFF | ((((255*y)/kargs->fbHeight) << 8) & 0xFF00) | 0x00FF0000;
            graphics::screen::plotPixel(x, y, pixel);
        }
    }
    if (!args) {
        graphics::psf::print("Please give me arguments as kernel params!!");
        loop_forever;
    }



    // for (int i = 0; i < 50; i++) {
    // graphics::psf::putchar('A' + i, 0x00FFFFFF, 0x00000000);
    // graphics::psf::putchar('f', 200, 0);
    // graphics::psf::print("abcdefg", 0x00FFFFFF, 0x00);
    // }

    // graphics::psf::putchar('f', 200, 0);
    // graphics::psf::print("Lorem ipsum dolor sit amet, officia excepteur ex fugiat reprehenderit enim labore culpa sint ad nisi Lorem pariatur mollit ex esse exercitation amet. Nisi anim cupidatat excepteur officia. Reprehenderit nostrud nostrud ipsum Lorem est aliquip amet voluptate voluptate dolor minim nulla est proident. Nostrud officia pariatur ut officia. Sit irure elit esse ea nulla sunt ex occaecat reprehenderit commodo officia dolor Lorem duis laboris cupidatat officia voluptate. Culpa proident adipisicing id nulla nisi laboris ex in Lorem sunt duis officia eiusmod. Aliqua reprehenderit commodo ex non excepteur duis sunt velit enim. Voluptate laboris sint cupidatat ullamco ut ea consectetur et est culpa et culpa duis.", 0x00FFFFFF, 0x00);
    graphics::psf::print("Kernel arguments:");
    for (int i = 0; i < args->argc; i++) {
        graphics::psf::putchar(' ');
        graphics::psf::print(args->argv[i]);
    }
    
    loop_forever;
}
