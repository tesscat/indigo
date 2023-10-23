#include <uefi.h>
#include <file.hpp>

char* readFile(const char* name) {
  FILE* f = fopen(name, "r");
  if (f == NULL) {
    printf("Unable to open file %s\n", name);
    return nullptr;
  }
  // get size
  fseek(f, 0, SEEK_END);
  size_t size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char* data = (char*) malloc(sizeof(char) * (size + 1));
  if (data == NULL) {
    printf("Unable to allocate memory for file %s\n", name);
    fclose(f);
    return nullptr;
  }
  fread(data, size, 1, f);
  data[size] = '\0';
  fclose(f);

  return data;
}
