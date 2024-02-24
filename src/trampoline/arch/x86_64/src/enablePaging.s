[BITS 64]

extern returnTo

; space for the stack

section .bss
align 16
stack_bottom:
resb 16384 ; 16 KiB
stack_top:

section .text

global enablePaging
enablePaging:
    mov rax, rdi 
    mov cr3, rax

    ; make sure HW interrupts are disabled
    cli

    ; we like to not map the stack
    ; since the stack is a remnant of the Who The Hell Knows Times, this is not very good
    ; so we shall remap it here
    ; good luck returning past this point

    mov rsp, stack_top

    call returnTo
