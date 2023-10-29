#include "vec_efi/vec_efi.hpp"
#include <uefi.h>
#include <file.hpp>
#include <ion_efi/ion_efi.hpp>

void error(size_t loc, const char* message) {
  printf("Error: %lu: %s", loc, message);
  while(1);
}

extern "C" int main(int argc, char** argv) {
  char* data = readFile("root.ion");
  if (data != NULL) {
    printf(data);
    ion::Input input;
    size_t pos = 0;
    input.pos = &pos;
    input.str = data;
    // input.error = error;
    ion::RootNode* root = ion::parse(input);
    size_t len = root->getLen();
    indigo::Vec<const char*> pathpath;
    pathpath.Append("kernel");
    pathpath.Append("path");
    ion::Node* kpath = root->getFrom(&pathpath);
    if (kpath) {
      if (kpath->getType() == ion::NodeType::StrNode) {
        class ion::StrNode* snode = static_cast<class ion::StrNode*>(kpath);
        printf(snode->getStr());
      } else {printf("invalid non-string node type at kernel.path");}
    } else {printf("no node at kernel.path");}
  }

  

  while (1);
  return 0;
}
