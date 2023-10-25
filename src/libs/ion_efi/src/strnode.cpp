#include <ion_efi/ion_efi.hpp>
#include <uefi.h>

namespace ion {
#define MALLOC_STR_CHUNK_SIZE 32
StrNode::StrNode(Input input) {
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

  // add how far we've gone
  // accounting for the trailing '"'
  *input.pos += (idx + 1);
}

char* StrNode::getStr() {return data;}
StrNode::~StrNode() {
  free(data);
  free(path);
}

}
