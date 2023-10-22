.global outB

/* send a byte to an IO port
; stack: [esp + 8] is the data byte
;        [esp + 4] is the port
;        [esp] is the return addr
*/
outB:
  mov 8(%esp), %al // [esp + 8], %al
  mov 4(%esp), %dx //[esp + 4], %dx
  out %al, %dx
  ret

.global inB

/* read a byte from an IO port
 stack: [esp + 4] is the addr of io port
        [esp] is the return addr
*/
inB:
  mov 4(%esp), %dx
  in %dx, %al
  ret
