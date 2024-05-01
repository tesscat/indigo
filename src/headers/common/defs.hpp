#ifndef COMMON_DEFS_HPP
#define COMMON_DEFS_HPP

// two gigabytes down from the very top
// 2**64 - (2*(1024**3)) = 0xffffffff80000000
#define KERNEL_OFFSET (0xffffffff80000000)

// 1GiB up from KERNEL_OFFSET
#define HEAP_VIRTUAL_BASE (KERNEL_OFFSET + (1*1024*1024*1024))

#endif // !COMMON_DEFS_HPP
