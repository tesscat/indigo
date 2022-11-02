/ * Multiboot header constants */
.set ALIGN, 1<<0 /* align loaded modules on page boundaries */
.set MEMINFO, 1<<1 /* provide memory map */
.set FLAGS, ALIGN | MEMINFO /* flags */
.set MAGIC, 0x1BADB002 /* ew magic number */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum for validation */

/* Declare the multiboot section. This marks the program as a mb-compliant kernel.
   These are "magic" values from the MB spec. This will be searched for in the first
   8 KiB of the file, at 32-bit alignment.
   We make a separate section to force alignment. */
.section .multiboot
.align 4 /* align to the 32-bits we mentioned earlier */
.long MAGIC /* just write the data out */
.long FLAGS
.long CHECKSUM

/* set-up the stack, in an unititialised data (bss) section. */
.section .bss
.align 16 /* Demanded by System V ABI Standard, needed for compiler good behaviour */
stack_bottom:
.skip 16384 /* 16 KiB */
stack_top:

/* woo the start bit */
.section .text
.global _start
.type _start, @function
_start:
  /* Bootloader gives us:
    - 32-bit protected mode
    - Interrupts and paging disabled
    - Multiboot-compliant environment
  */

  /* make a stack */
  mov $stack_top, %esp

  /*
  TODO: GDT and Paging before we jump into fancy code
  */

  call kernel_start

  /* the kernel_start has returned */
  /* clear/disable interrupts, halt, and jump to halt if we escape. */

  cli
1:hlt
  jmp 1b

/* call tracing things */
.size _start, . - _start
