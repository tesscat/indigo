#include <ion_efi/ion_efi.hpp>
#include <uefi.h>
#include <ion_efi/parse.hpp>

namespace ion {
bool isWhitespace(char c) {
  return c == ' ' ||
    c == '\t' ||
    c == '\r' ||
    c == '\n';
}

bool isAlphaNumeric(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9');
}

void skipWhitespace(Input input) {
  char c = input[0];
  while (c != '\0' && isWhitespace(c) || c == '#') {
    if (c == '#') {
      while (c != '\n' && c != '\0') {
        *input.pos += 1;
        c = input[0];
      }
      continue;
    }
    *input.pos += 1;
    c = input[0];
  }
}

char* takeName(Input input) {
  size_t pos = 0;
  while (isAlphaNumeric(input[pos]) && input[pos] != '\0') {
    pos++;
  }
  char* name = (char*) malloc(pos + 1);
  memcpy(name, &input[0], pos);
  name[pos] = '\0';
  
  *input.pos += pos;
  return name;
}

}
