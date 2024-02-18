ENTRY (_start)

SECTIONS
{
    /* 32MB */
    . = 0x02000000;

    _tramp_start = .;

   .text ALIGN (4K):
   {
       *(.text)
    }
    .rodata ALIGN (4K):
    {
        *(.rodata)
    }
    .data ALIGN (4K):
    {
        *(.data)
    }
    .bss ALIGN (4K):
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    }

    _tramp_end = .;
}
