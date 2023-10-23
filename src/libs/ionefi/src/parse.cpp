#include <ionefi/ionefi.hpp>
#include <uefi.h>

namespace ion {
// Node* parseValue(const char* input) {
//   if (input[0] == '{') {
//     return new ObjectNode(input);
//   } else if (input[0] == '[') {
//     return new ArrayNode(input);
//   } else if (input[0] == '"') {
//     return new StrNode(input);
//   } else {
//     return nullptr;
//   }
// }

#define MALLOC_STR_CHUNK_SIZE 32
StrNode::StrNode(const char* input) {
  // just assume input[0] is '"'
  size_t idx = 1;
  bool escaped = false;
  size_t size = MALLOC_STR_CHUNK_SIZE;
  size_t data_idx = 0;
  data = (char*) malloc(size);
  while (escaped || input[idx] != '"') {
    if (escaped || input[idx] != '\\') {
      data[data_idx] = input[idx];
      data_idx++;
      if (data_idx == size) {
        // exceeded current buffer, reallocate it
        size += MALLOC_STR_CHUNK_SIZE;
        data = (char*) realloc(data, size);
      }
      escaped = false;
    } else {
      escaped = true;
    }
    idx++;
  }

  // trim and add null-terminator
  data = (char*) realloc(data, data_idx + 1);
  data[data_idx] = '\0';
}

char* StrNode::getStr() {return data;}
//
// Node* parse(char* input);
}
