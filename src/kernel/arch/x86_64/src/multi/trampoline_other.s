default rel
[default rel]
; Low entry code for other processors
; serves only to set up a stack, call lgdt + load page tables, and then jump to higher entry
; we start off in Real Mode (16 bits)
extern LongMode
[BITS 16]
.code16
section .low_text
global trampoline_start
trampoline_start:
    cli
    jmp SwitchToLongMode
; The kernel should set us up as such:
; 0x8f00 is a 64 bit pointer to the page table

; code taken from https://wiki.osdev.org/Entering_Long_Mode_Directly
%define PAGE_PRESENT    (1 << 0)
%define PAGE_WRITE      (1 << 1)
%define CODE_SEG     0x0008
ALIGN 4
IDT:
    .Length       dw 0
    .Base         dd 0
SwitchToLongMode:
    ; Disable IRQs
    mov al, 0xFF                      ; Out 0xFF to 0xA1 and 0x21 to disable all IRQs.
    out 0xA1, al
    out 0x21, al
    nop
    nop
    lidt [IDT]                        ; Load a zero length IDT so that any NMI causes a triple fault.
    ; Enter long mode.
    mov eax, 10100000b                ; Set the PAE and PGE bit.
    mov cr4, eax
    mov ecx, 0x8f00
    mov edx, [ecx]                    ; Point CR3 at the PML4.
    mov cr3, edx
    mov ecx, 0xC0000080               ; Read from the EFER MSR. 
    rdmsr    
    or eax, 0x00000100                ; Set the LME bit.
    wrmsr
    mov ebx, cr0                      ; Activate long mode -
    or ebx,0x80000001                 ; - by enabling paging and protection simultaneously.
    mov cr0, ebx                    
    lgdt [GDT.Pointer]                ; Load GDT.Pointer defined below.
    jmp CODE_SEG:LongMode             ; Load CS with 64 bit segment and flush the instruction cache
ALIGN 4
    ; Global Descriptor Table
GDT:
.Null:
    dq 0x0000000000000000             ; Null Descriptor - should be present.
.Code:
    dq 0x00209A0000000000             ; 64-bit code descriptor (exec/read).
    dq 0x0000920000000000             ; 64-bit data descriptor (read/write).
ALIGN 4
    dw 0                              ; Padding to make the "address of the GDT" field aligned on a 4-byte boundary
.Pointer:
    dw $ - GDT - 1                    ; 16-bit Size (Limit) of GDT.
    dd GDT                            ; 32-bit Base Address of GDT. (CPU will zero extend to 64-bit)
