#include "headers/squashfs.hpp"
#include "interfaces/native/fs.hpp"
#include "logs/logs.hpp"
#include "providers/fs.hpp"
#include "util/util.hpp"

bool checkForValidSquashfs(interfaces::File* source) {
    return true;
    if (!source->readable()) return false;
    return false;
}

interfaces::FileSystem* constructSquashfs(interfaces::File* source) {
    logs::info << "ctoring squashfs\n";
    logs::info << "fs is " << source->size();
    panic("um");
    return nullptr;
}

extern "C" void module_load() {
    logs::info << "hello from module_load squashfs\n";
    // panic("uh");
    interfaces::FileSystemProvider fsp {.check = checkForValidSquashfs, .construct = constructSquashfs};
    providers::fileSystems.Append(fsp);
}


