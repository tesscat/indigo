#ifndef KERNEL_UTIL_VEC_HPP
#define KERNEL_UTIL_VEC_HPP

#include "libstd/merge_sort.hpp"
#include "memory/heap.hpp"
#include <libmem/mem.hpp>
#ifndef VEC_ALLOC_BLOCK_SIZE
#define VEC_ALLOC_BLOCK_SIZE 16
#endif

#include <stdint.h>
#include <new>

namespace util {
// // manual vec {{{
// template <typename T>
// class ManualVec {
// private:
//     bool hasInit;
//     uint64_t allocated;
// public:
//     T* data;
//     uint64_t len;
//     /**
//    * @brief set to true to stop Vec from deallocating the data on exit
//    */
//     bool takeMyData;
//     ManualVec();
//     // call if this thing isn't default constructed right
//     void init();
//     ManualVec(uint64_t len);
//     void Trim();
//     void ApproxTrim();
//     ~ManualVec();
//     T& operator[] (uint64_t index);
//     void Append(T value);
//     bool Contains(T& value);
//     void Sort();

//     ManualVec(const ManualVec& other);
//     ManualVec(ManualVec&& other) = delete;
//     ManualVec<T>& operator=(ManualVec<T>& other);
// };
// }

// namespace util {
// template<typename T>
// ManualVec<T>::ManualVec(const ManualVec& other) {
//     allocated = other.allocated;
//     len = other.len;
//     data = (T*)kmalloc(allocated * sizeof(T));
//     // data = new T[allocated];
//     for (int i = 0; i < len; i++) {
//         // messed up placement new stuff
//         new (&data[i]) T((const T)other.data[i]);
//     }
//     // memcpy(data, other.data, allocated*sizeof(T));
//     takeMyData = false;
// }
// template<typename T>
// bool ManualVec<T>::Contains(T& value) {
//     for (uint64_t i = 0; i < len; i++) {
//         if (data[i] == value) return true;
//     }
//     return false;
// }
// template <typename T>
// ManualVec<T>::ManualVec() {
//     init();
// }
// template <typename T>
// void ManualVec<T>::init() {
//     if (hasInit) return;
//     hasInit = true;
//     allocated = VEC_ALLOC_BLOCK_SIZE;
//     data = (T*) kmalloc(VEC_ALLOC_BLOCK_SIZE * sizeof(T));
//     len = 0;
// }
// template <typename T>
// ManualVec<T>::ManualVec(uint64_t len_) : len{len_} {
//     allocated = (len - (len % VEC_ALLOC_BLOCK_SIZE)) + VEC_ALLOC_BLOCK_SIZE;
//     data = (T*) kmalloc(allocated * sizeof(T));
// }

// template <typename T>
// void ManualVec<T>::Trim() {
//     allocated = len;
//     if (allocated == 0) allocated = 1;
//     data = (T*) krealloc(data, allocated * sizeof(T));
// }
// template <typename T>
// void ManualVec<T>::ApproxTrim() {
//     allocated = (len - (len % VEC_ALLOC_BLOCK_SIZE)) + VEC_ALLOC_BLOCK_SIZE;
//     if (allocated == 0) allocated = VEC_ALLOC_BLOCK_SIZE;
//     data = (T*) krealloc(data, allocated * sizeof(T));
// }
// template <typename T>
// ManualVec<T>::~ManualVec() {
//     // TODO: if data has destructor call it
//     for (int i = 0; i < len; i++) {
//         (&data[i])->~T();
//     }
//     if (!takeMyData)
//         kfree(data);
// }
// template <typename T>
// T& ManualVec<T>::operator[] (uint64_t index) {
//     return data[index];
// }
// template <typename T>
// void ManualVec<T>::Append(T value) {
//     data[len] = value;
//     len++;
//     if (len >= allocated) {
//         allocated += VEC_ALLOC_BLOCK_SIZE;
//         data = (T*) krealloc(data, allocated * sizeof(T));
//     }
// }
// template<typename T>
// void ManualVec<T>::Sort() {
//     imsort(data, 0, len);
// }
// // }}}

// vec {{{
template <typename T>
class Vec {
private:
    uint64_t allocated = 0;
public:
    T* data = nullptr;
    uint64_t len = 0;
    /**
   * @brief set to true to stop Vec from deallocating the data on exit
   */
    bool takeMyData;
    Vec();
    // call if this thing isn't default constructed right
    Vec(uint64_t len);
    void Trim();
    void ApproxTrim();
    ~Vec();
    T& operator[] (uint64_t index);
    void Append(T value);
    bool Contains(T& value);
    void Sort();

    Vec(const Vec& other);
    Vec(Vec&& other) = delete;
    Vec<T>& operator=(Vec<T>& other) = default;
    void init() {}
};
}

namespace util {
template<typename T>
Vec<T>::Vec(const Vec& other) {
    allocated = other.allocated;
    len = other.len;
    if (allocated != 0) {
        data = (T*)kmalloc(allocated * sizeof(T));
        for (int i = 0; i < len; i++) {
            new (&data[i]) T((const T)other.data[i]);
        }
    }
    takeMyData = false;
}
template<typename T>
bool Vec<T>::Contains(T& value) {
    for (uint64_t i = 0; i < len; i++) {
        if (data[i] == value) return true;
    }
    return false;
}
template <typename T>
Vec<T>::Vec() {
    allocated = 0; // VEC_ALLOC_BLOCK_SIZE;
    data = nullptr; // (T*) kmalloc(VEC_ALLOC_BLOCK_SIZE * sizeof(T));
    len = 0;
}
template <typename T>
Vec<T>::Vec(uint64_t len_) : len{len_} {
    if (len != 0) {
        allocated = (len - (len % VEC_ALLOC_BLOCK_SIZE)) + VEC_ALLOC_BLOCK_SIZE;
        data = (T*) kmalloc(allocated * sizeof(T));
    } else {
        allocated = 0;
        data = nullptr;
    }
}

template <typename T>
void Vec<T>::Trim() {
    allocated = len;
    if (allocated == 0) {
        if (data) {kfree(data);}
        data = nullptr;
    }
    else {data = (T*) krealloc(data, allocated * sizeof(T));}
}
template <typename T>
void Vec<T>::ApproxTrim() {
    allocated = (len - (len % VEC_ALLOC_BLOCK_SIZE)) + VEC_ALLOC_BLOCK_SIZE;
    if (allocated == 0) allocated = VEC_ALLOC_BLOCK_SIZE;
    data = (T*) krealloc(data, allocated * sizeof(T));
}
template <typename T>
Vec<T>::~Vec() {
    // TODO: if data has destructor call it
    for (int i = 0; i < len; i++) (&data[i])->~T();
    if (!takeMyData && data)
        kfree(data);
    data == nullptr;
    len = 0;
}
template <typename T>
T& Vec<T>::operator[] (uint64_t index) {
    return data[index];
}
template <typename T>
void Vec<T>::Append(T value) {
    if (len >= allocated) {
        allocated += VEC_ALLOC_BLOCK_SIZE;
        if (data) {
            data = (T*) krealloc(data, allocated * sizeof(T));
        } else {
            data = (T*)kmalloc(allocated * sizeof(T));
        }
    }
    data[len] = value;
    len++;
}
template<typename T>
void Vec<T>::Sort() {
    imsort(data, 0, len);
}
// }}}
}

#endif // !KERNEL_UTIL_VEC_HPP
