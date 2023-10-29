#ifndef LIBS_VEC_EFI_HPP
#define LIBS_VEC_EFI_HPP

#ifndef VEC_ALLOC_BLOCK_SIZE
#define VEC_ALLOC_BLOCK_SIZE 16
#endif

namespace indigo {
template <typename T>
class Vec {
private:
  size_t allocated;
public:
  T* data;
  size_t len;
  /**
   * @brief set to true to stop Vec from deallocating the data on exit
   */
  bool takeMyData;
  Vec();
  Vec(size_t len);
  void Trim();
  ~Vec();
  T& operator[] (size_t index);
  void Append(T value);

  Vec(Vec& other) = delete;
  Vec(Vec&& other) = delete;
};
}

namespace indigo {
template <typename T>
Vec<T>::Vec() {
  allocated = VEC_ALLOC_BLOCK_SIZE;
  data = (T*) malloc(VEC_ALLOC_BLOCK_SIZE * sizeof(T));
  len = 0;
}
template <typename T>
Vec<T>::Vec(size_t len) {
  allocated = (len - (len % VEC_ALLOC_BLOCK_SIZE)) + VEC_ALLOC_BLOCK_SIZE;
  data = (T*) malloc(allocated * sizeof(T));
  len = len;
}
template <typename T>
void Vec<T>::Trim() {
  allocated = len;
  data = (T*) realloc(data, allocated * sizeof(T));
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
    data = (T*) realloc(data, allocated * sizeof(T));
  }
}

}

#endif // !LIBS_VEC_EFI_HPP
