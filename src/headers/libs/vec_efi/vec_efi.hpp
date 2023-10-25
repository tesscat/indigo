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

#endif // !LIBS_VEC_EFI_HPP
