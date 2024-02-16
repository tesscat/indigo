#ifndef COMMON_FILESYSTEM_HPP
#define COMMON_FILESYSTEM_HPP

namespace filesystem {
struct File {
    bool isRegularFile;
    char* path;
};
struct FilesystemDriver {

};
}

#endif // !COMMON_FILESYSTEM_HPP
