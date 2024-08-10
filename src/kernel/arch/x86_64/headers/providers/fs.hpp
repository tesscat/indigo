#ifndef KERNEL_PROVIDERS_FS_HPP
#define KERNEL_PROVIDERS_FS_HPP

#include "interfaces/native/fs.hpp"
#include "util/vec.hpp"
namespace providers {
extern util::Vec<interfaces::FileSystemProvider> fileSystems;

// TODO: make this throw instead of nullptring
interfaces::FileSystem* tryMount(interfaces::File* file);
}

#endif // !KERNEL_PROVIDERS_FS_HPP
