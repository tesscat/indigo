default rel

global LongMode
[BITS 64]
section .low_text
LongMode:
    ; just for the linker
    mov rsp, stack_top
    push 0x08                 ; Push code segment to stack, 0x08 is a stand-in for your code segment
    mov rax, TrueLongMode ; Load address of .reload_CS into RAX
    push rax                  ; Push this value to the stack
    retfq
stack_bot:
resb 32
stack_top:

%define DATA_SEG     0x0010
[BITS 64]
[default rel]
default rel
section .text
default rel
[default rel]
[BITS 64]
extern other_stack_top
extern entry_other
[BITS 64]
TrueLongMode:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ; do the stack
    mov rax, other_stack_top
    mov rsp, [rax]
    jmp entry_other
