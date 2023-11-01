#include <stdint.h>
#define _STDINT_H
#include <vec_efi/vec_efi.hpp>
#include <ion_efi/ion_efi.hpp>
#include <uefi.h>

namespace ion {
StrNode::StrNode(Input input) {
  // just assume input[0] is '"'
  *input.pos += 1;
  indigo::Vec<char> str;
  bool escaped = false;
  while ((escaped || input[0] != '"') && input[0] != '\0') {
    if (escaped || input[0] != '\\') {
      str.Append(input[0]);
      escaped = false;
    } else {
      escaped = true;
    }
    *input.pos += 1;
  }

  if (input[0] != '"') {
    input.is_err = true;
    input.error(*input.pos, "Expected '\"' to finish string");
  } else {
    *input.pos += 1;
  }

  // trim and add null-terminator
  str.Append('\0');
  str.Trim();
  str.takeMyData = true;
  data = str.data;
}

char* StrNode::getStr() {return data;}
StrNode::~StrNode() {
  free(data);
}

Node* StrNode::get(indigo::Vec<const char*>* path, size_t offs) {
  if (path->len - offs != 0) return nullptr;
  return this;
}

}
