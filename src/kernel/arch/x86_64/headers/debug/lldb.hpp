#ifndef KERNEL_DEBUG_LLDB_HPP
#define KERNEL_DEBUG_LLDB_HPP

#include <cstddef>

namespace debug {
    // Sets the name for the next module loaded via the Module::Module ctor.
    // Provide *just* the name (e.g. squashfs)
    void setNextModuleLoadName(const char* name);
    // Inform that you have loaded a module's section (defined by file offset)
    // into memory at virt_addr
    void loadedModuleSection(const size_t file_offs, const void* virt_addr);
}

#endif // !KERNEL_DEBUG_LLDB_HPP
