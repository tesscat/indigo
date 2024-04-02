[BITS 64]

extern kernel_start

; space for the stack
; since the old one created by trampoline will be unmapped with the new pagemap
section .bss
align 16
stack_bottom:
resb 2097152 ; 2 MiB
stack_top:

section .text

global _start
_start:
    ; make _sure_ HW interrupts are disabled
    cli

    ; remap the New Stack

    mov rsp, stack_top

    call kernel_start
