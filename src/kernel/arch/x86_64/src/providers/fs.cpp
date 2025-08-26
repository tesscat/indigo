#include <providers/fs.hpp>

namespace providers {
    util::Vec<interfaces::FileSystemProvider> fileSystems;

    interfaces::FileSystem* tryMount(interfaces::File* file) {
        for (uint64_t i = 0; i < fileSystems.len; i++) {
            if (fileSystems[i].check(file)) {
                return fileSystems[i].construct(file);
            }
        }
        return nullptr;
    }
}
