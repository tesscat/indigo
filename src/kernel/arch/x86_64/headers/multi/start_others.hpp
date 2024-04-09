#ifndef KERNEL_MULTI_START_OTHERS_HPP
#define KERNEL_MULTI_START_OTHERS_HPP

#include "sync/spinlock.hpp"
namespace multi {
extern sync::Spinlock startupLock;

void startOthers();
}

#endif // !KERNEL_MULTI_START_OTHERS_HPP
