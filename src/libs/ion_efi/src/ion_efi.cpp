#include <ion_efi/ion_efi.hpp>
#include <uefi.h>

namespace ion {
Node* parseValue(const char* input) {
  if (input[0] == '{') {
    return new ObjectNode(input);
  } else if (input[0] == '[') {
    return new ArrayNode(input);
  } else if (input[0] == '"') {
    return new StrNode(input);
  } else {
    return nullptr;
  }
}


//
// Node* parse(char* input);
}
