#include "multi/cpu.hpp"
#include <io/iostream.hpp>
#include <logs/logs.hpp>
extern "C" void module_load() {
    logs::info << "hello there from fsmod on CPU#" << multi::getCpuIdx()
               << "\n";
}
