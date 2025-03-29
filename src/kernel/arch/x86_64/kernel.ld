ENTRY (_start)

SECTIONS
{
    /* link the very low stuff */
    . = 0x8000;
    .low_text ALIGN (4K) : {
        *(.low_text.start)
        *(.low_text)
    }

    . = 0x03000000;
    /* The kernel will live at -2GiB + 48MiB in the virtual address space, */
    /* which will be mapped to 48MiB in the physical address space. */
    /* Hopefully we dodge all the EFI stuff. */
    /* Note that we page-align the sections. */
    /* Due to x86_64 sign-extension style addressing, this becomes 0xffffffff80000000 */


    . += 0xffffffff80000000;
    _kernel_virt_start = .;
    /* Add a symbol that indicates the start address of the kernel. */
    .text ALIGN (4K): AT (ADDR (.text) - 0xffffffff80000000)
    {
        *(.text)
    }
    .rodata ALIGN (4K): AT (ADDR (.rodata) - 0xffffffff80000000)
    {
        *(.rodata)
        *(.rodata.*)
    }
    /* the constructor shenanigansen */
    .init_array ALIGN (4K) : AT (ADDR (.init_array) - 0xffffffff80000000) {
        _init_array_start = .;
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array .ctors))
        _init_array_end = .;
    }
    .data ALIGN (4K): AT (ADDR (.data) - 0xffffffff80000000)
    {
        *(.data)
    }
    .bss ALIGN (4K): AT (ADDR (.bss) - 0xffffffff80000000)
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    }
    /* Add a symbol that indicates the end address of the kernel. */
    _kernel_virt_end = .;
}
