#include "util/util.hpp"
#include <debug/lldb.hpp>

namespace debug {
    void setNextModuleLoadName(const char* name) {
        UNUSED(name);
    }
    void loadedModuleSection(const size_t file_offs, const void* virt_addr) {
        UNUSED(file_offs);
        UNUSED(virt_addr);
    }
}
