#ifndef KERNEL_MODULES_SPINE_HPP
#define KERNEL_MODULES_SPINE_HPP

namespace modules {
bool spineContains(const char* name);
void* spineGet(const char* name);
void spineSet(const char* name, void* fn);

void initSpine();
}

#endif // !KERNEL_MODULES_SPINE_HPP
