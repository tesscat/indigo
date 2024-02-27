ENTRY (_start)

SECTIONS
{
    . = 0x03000000;
    /* The kernel will live at 0xe00000000000 + 48MB in the virtual address space, */
    /* which will be mapped to 48MB in the physical address space. */
    /* Hopefully we dodge all the EFI stuff. */
    /* Note that we page-align the sections. */
    /* Due to x86_64 sign-extension style addressing, this becomes 0xffffe00000000000 */

    _kernel_phys_start = .;

    . += 0xffffe00000000000;
    /* Add a symbol that indicates the start address of the kernel. */
    .text ALIGN (4K): AT (ADDR (.text) - 0xffffe00000000000)
    {
        *(.text)
    }
    .rodata ALIGN (4K): AT (ADDR (.rodata) - 0xffffe00000000000)
    {
        *(.rodata)
        *(.rodata.*)
    }
    .data ALIGN (4K): AT (ADDR (.data) - 0xffffe00000000000)
    {
        *(.data)
    }
    .bss ALIGN (4K): AT (ADDR (.bss) - 0xffffe00000000000)
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    }
    /* Add a symbol that indicates the end address of the kernel. */
    _kernel_phys_end = . - 0xffffe00000000000;
}
