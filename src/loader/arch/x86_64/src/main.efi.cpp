#include <uefi.h>
#include <file.hpp>
#include <ionefi/ionefi.hpp>

extern "C" int main(int argc, char** argv) {
  char* data = readFile("root.ion");
  if (data != NULL)
    printf(data);

  const char* test_str = "hello! \\\" weird.";
  ion::StrNode s(test_str);
  printf(s.getStr());

  while (1);
  return 0;
}
