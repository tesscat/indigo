#include <kernel.hpp>
#define _STDINT_H
#include <vec_efi/vec_efi.hpp>
#include <uefi.h>
#include <file.hpp>
#include <acpi.hpp>
#include <ion_efi/ion_efi.hpp>

void error(size_t loc, const char* message) {
    printf("Error: %lu: %s", loc, message);
    while(1);
}

extern "C" int main(int argc, char** argv) {
    // find the ACPI tables
    acpi::initAcpiTables();
    

    char* data = readFile("root.ion");
    if (data == NULL) {
        printf("Unable to load root.ion");
        while(1);
    }

    ion::Input input;
    size_t pos = 0;
    input.pos = &pos;
    input.str = data;

    ion::RootNode* root = ion::parse(input);

    size_t len = root->getLen();

    indigo::Vec<const char*> pathpath;
    pathpath.Append("kernel");
    pathpath.Append("path");
    ion::Node* kpath = root->getFrom(&pathpath);

    if (!kpath) {
        printf("no node at kernel.path");
        while(1);
    }
    if (kpath->getType() != ion::NodeType::StrNode) {
        printf("invalid non-string node type at kernel.path");
        while(1);
    }

    indigo::Vec<const char*> tramppath;
    tramppath.Append("trampoline");
    ion::Node* tpath = root->getFrom(&tramppath);

    if (!tpath) {
        printf("no node at trampoline");
        while(1);
    }
    if (tpath->getType() != ion::NodeType::StrNode) {
        printf("invalid non-string node type at trampoline");
        while(1);
    }

    indigo::Vec<const char*> fspath;
    fspath.Append("fsdriver");
    ion::Node* fsdriverpath = root->getFrom(&fspath);

    if (!fsdriverpath) {
        printf("no node at fsdriver");
        while(1);
    }
    if (fsdriverpath->getType() != ion::NodeType::StrNode) {
        printf("invalid non-string node type at fsdriver");
        while(1);
    }

    indigo::Vec<const char*> initrdpathpath;
    initrdpathpath.Append("initrd");
    ion::Node* initrdpath = root->getFrom(&initrdpathpath);

    if (!initrdpath) {
        printf("no node at initrd");
        while(1);
    }
    if (initrdpath->getType() != ion::NodeType::StrNode) {
        printf("invalid non-string node type at initrd");
        while(1);
    }

    class ion::StrNode* snode = static_cast<class ion::StrNode*>(kpath);
    const char* path = snode->getStr();
    class ion::StrNode* tnode = static_cast<class ion::StrNode*>(tpath);
    const char* tpathn = tnode->getStr();
    class ion::StrNode* fnode = static_cast<class ion::StrNode*>(fsdriverpath);
    const char* fpathn = fnode->getStr();
    class ion::StrNode* initrdnode = static_cast<class ion::StrNode*>(initrdpath);
    const char* initrdpathn = initrdnode->getStr();
    Kernel kernel(path, root, tpathn, fpathn, initrdpathn);

    indigo::Vec<const char*> argpath;
    argpath.Append("kernel");
    argpath.Append("args");
    ion::Node* argnode = root->getFrom(&argpath);
    if (argnode) {
        if (argnode->getType() != ion::NodeType::ArrayNode) {
            printf("Invalid type for kernel.args");
            while(1);
        }
        indigo::Vec<char*> args;
        class ion::ArrayNode* arrnode = static_cast<class ion::ArrayNode*>(argnode);
        size_t len = arrnode->getLen();
        for (int i = 0; i < len; i++) {
            ion::Node* node = arrnode->getArr()[i];
            if (node->getType() != ion::NodeType::StrNode) {
                printf("Argument at %i is not a string\n", i);
                while(1);
            }
            args.Append(static_cast<class ion::StrNode*>(node)->getStr());
        }

        args.takeMyData = true;
        kernel.Run(len, args.data);
    }

    kernel.Run(0, nullptr);

    while (1);
    return 0;
}
