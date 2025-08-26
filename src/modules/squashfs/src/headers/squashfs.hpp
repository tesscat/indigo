#ifndef MODULES_SQUASHFS_SQUASHFS_HPP
#define MODULES_SQUASHFS_SQUASHFS_HPP

#include "interfaces/native/fs.hpp"
namespace filesystems {
    class SquashFS : public interfaces::FileSystem {
        virtual String getFsName() override;
        virtual interfaces::Directory* getRootDir() override;
    };
}

#endif // !MODULES_SQUASHFS_SQUASHFS_HPP
