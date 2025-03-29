#ifndef KERNEL_INTERFACES_NATIVE_FS_HPP
#define KERNEL_INTERFACES_NATIVE_FS_HPP

#include "interfaces/native/rw.hpp"
#include "libstd/string.hpp"
#include "util/vec.hpp"
namespace interfaces {
class File;
class Directory;
class Path {
public:
    virtual ~Path() {};

    virtual String getPath() =0;
    virtual bool isDir() =0;
    // open a file if it exists, undefined if not exists
    // TODO: after i impl exception support, make this throw instead of UB on non exists
    virtual File openFile() =0;
    virtual Directory openDirectory() =0;
    virtual bool exists() =0;
};

// TODO: extended attrs, perms etc

class Directory {
public:
    virtual ~Directory() {};
    virtual util::Vec<Path*> listDir() =0;
};

class File {
public:
    virtual ~File() {};
    // virtual uint8_t* read(uint64_t offset, uint64_t size) =0;
    // virtual String readString(uint64_t offset, uint64_t size) =0;
    // virtual void readInto(uint64_t offset, uint64_t size, uint8_t* output) =0;
    virtual bool readable() =0;
    virtual Reader* getReader() =0;
    virtual bool writeable() =0;
    virtual Writer* getWriter() =0;

    virtual uint64_t size() =0;
    // TODO: exceptions and writing
};

class FileSystem {
public:
    virtual ~FileSystem() {};
    // returns the actual name of the FS, e.g. "squashfs"
    virtual String getFsName() =0;
    // gets the root dir
    virtual Directory* getRootDir() =0;
    // TODO: unmount
};

struct FileSystemProvider {
    bool (*check)(File* source);
    FileSystem* (*construct)(File* source);
};
}

#endif // !KERNEL_INTERFACES_NATIVE_FS_HPP
