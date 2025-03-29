#ifndef LOADER_NEWDEL_HPP
#define LOADER_NEWDEL_HPP

#include <uefi.h>
void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *p);
void operator delete[](void *p);

#endif // !LOADER_NEWDEL_HPP
