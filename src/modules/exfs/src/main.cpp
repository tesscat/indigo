#include "multi/cpu.hpp"
#include <logs/logs.hpp>
#include <io/iostream.hpp>
extern "C" void module_load() {
    logs::info << "hello there from fsmod on CPU#" << multi::getCpuIdx() << "\n";
}
