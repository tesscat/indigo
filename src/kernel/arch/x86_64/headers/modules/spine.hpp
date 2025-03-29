#ifndef KERNEL_MODULES_SPINE_HPP
#define KERNEL_MODULES_SPINE_HPP

#include "libstd/string.hpp"
#include "util/map.hpp"
#include <stdint.h>

namespace modules {
// bool spineContains(const char* name);
// void* spineGet(const char* name);
// void spineSet(const char* name, void* fn, uint64_t);

void initSpine();
extern util::Map<String, void*> spine;
}

#endif // !KERNEL_MODULES_SPINE_HPP
