#include <ion_efi/parse.hpp>

namespace ion {
void skipWhitespace(Input input) {
  size_t count = 0;
  char c = input[count];
  while (c != '\0' && (
    c == ' ' ||
    c == '\t' ||
    c == '\r' ||
    c == '\n'
  )) {
    count++;
    c = input[count];
  }
  *input.pos += count;
}
}
