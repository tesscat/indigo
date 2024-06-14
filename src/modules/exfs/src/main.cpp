#include <logs/logs.hpp>
#include <io/iostream.hpp>
extern "C" void module_load() {
    logs::info << "hi" << 18;
    // f();
}
