#include <uefi.h>
#include <vec_efi/vec_efi.hpp>

namespace indigo {
template <typename T>
Vec<T>::Vec() {
  allocated = VEC_ALLOC_BLOCK_SIZE;
  data = malloc(VEC_ALLOC_BLOCK_SIZE * sizeof(T));
  len = 0;
}
template <typename T>
Vec<T>::Vec(size_t len) {
  allocated = (len - (len % VEC_ALLOC_BLOCK_SIZE)) + VEC_ALLOC_BLOCK_SIZE;
  data = malloc(allocated * sizeof(T));
  len = len;
}
template <typename T>
void Vec<T>::Trim() {
  allocated = len;
  data = realloc(data, allocated * sizeof(T));
}
template <typename T>
Vec<T>::~Vec() {
  // TODO: if data has destructor call it
  if (!takeMyData)
    free(data);
}
template <typename T>
T& Vec<T>::operator[] (size_t index) {
  return data[index];
}
template <typename T>
void Vec<T>::Append(T value) {
  data[len] = value;
  len++;
  if (len >= allocated) {
    allocated += VEC_ALLOC_BLOCK_SIZE;
    data = realloc(data, allocated * sizeof(T));
  }
}

}
