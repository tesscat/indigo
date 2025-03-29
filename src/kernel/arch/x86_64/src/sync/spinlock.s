[BITS 64]
global __lockSpinlock
__lockSpinlock:
    lock bts dword [rdi], 0
    jc spinPause
    ret
spinPause:
    pause
    test dword [rdi], 1
    jnz spinPause
    jmp __lockSpinlock

global __awaitSpinlockRelease
__awaitSpinlockRelease:
    test dword [rdi], 1
    jnz while
    ret
while:
    pause
    test dword [rdi], 1
    jnz while
    ret

global __releaseSpinlock
__releaseSpinlock:
    mov dword [rdi], 0
    ret
