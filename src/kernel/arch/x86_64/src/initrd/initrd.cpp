#include "graphics/screen.hpp"
#include "inator/inator.hpp"
#include "interfaces/native/fs.hpp"
#include "memory/file.hpp"
#include "providers/fs.hpp"
#include <initrd/initrd.hpp>

namespace initrd {
int initrd() {
    // we need to:
    // - mount it
    // - read it
    // - do stuff with it
    // - load all relevant modules/configs/whatever from it
    // - unmount it (maybe?)
    //   we _should_ unmount it since it's taking up space we don't need it to
    memory::MemoryFile* initrdFile = new memory::MemoryFile(kargs->initrd, kargs->initrdLen, true, false);
    interfaces::FileSystem* initrdFs = providers::tryMount(initrdFile);

    return 0;
}

void submitInitrd() {
    inator::Target t;
    t.name = String("initrd");
    t.preference = 1;
    t.load = initrd;

    inator::graph->addTarget(t);
}
}
