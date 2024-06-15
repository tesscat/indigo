#include <logs/logs.hpp>
#include <io/iostream.hpp>
void v(uint64_t i);
extern "C" void module_load() {
    // logs::info << "hi" << (int64_t)18;
    // f();
    void* a = (void*)&logs::info;
    v((uint64_t)a);
    logs::info << "hello there\n";
}
