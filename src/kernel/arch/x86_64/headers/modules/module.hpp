#ifndef KERNEL_MODULES_MODULE_HPP
#define KERNEL_MODULES_MODULE_HPP

#include <stdint.h>

namespace modules {
    class Module {
        void (*unload)();
        void (*load)();
        bool inmemory;
        bool loaded;

    public:
        Module(uint8_t* loaded_elf);
        void Load();
        void Unload();
    };
}

#endif // !KERNEL_MODULES_MODULE_HPP
