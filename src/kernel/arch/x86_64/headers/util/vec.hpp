#ifndef KERNEL_UTIL_VEC_HPP
#define KERNEL_UTIL_VEC_HPP

#include "memory/heap.hpp"
#ifndef VEC_ALLOC_BLOCK_SIZE
#define VEC_ALLOC_BLOCK_SIZE 16
#endif

#include <stdint.h>

namespace util {
template <typename T>
class Vec {
private:
    uint64_t allocated;
public:
    T* data;
    uint64_t len;
    /**
   * @brief set to true to stop Vec from deallocating the data on exit
   */
    bool takeMyData;
    Vec();
    // call if this thing isn't default constructed right
    void init();
    Vec(uint64_t len);
    void Trim();
    ~Vec();
    T& operator[] (uint64_t index);
    void Append(T value);

    Vec(Vec& other) = delete;
    Vec(Vec&& other) = delete;
};
}

namespace util {
template <typename T>
Vec<T>::Vec() {
    init();
}
template <typename T>
void Vec<T>::init() {
    allocated = VEC_ALLOC_BLOCK_SIZE;
    data = (T*) kmalloc(VEC_ALLOC_BLOCK_SIZE * sizeof(T));
    len = 0;
}
template <typename T>
Vec<T>::Vec(uint64_t len_) : len{len_} {
    allocated = (len - (len % VEC_ALLOC_BLOCK_SIZE)) + VEC_ALLOC_BLOCK_SIZE;
    data = (T*) kmalloc(allocated * sizeof(T));
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
        kfree(data);
}
template <typename T>
T& Vec<T>::operator[] (uint64_t index) {
    return data[index];
}
template <typename T>
void Vec<T>::Append(T value) {
    data[len] = value;
    len++;
    if (len >= allocated) {
        allocated += VEC_ALLOC_BLOCK_SIZE;
        data = (T*) krealloc(data, allocated * sizeof(T));
    }
}

}

#endif // !KERNEL_UTIL_VEC_HPP
