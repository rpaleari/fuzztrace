.globl	_start

_test:
  mov $1, %rdi
  ret

_start:
  jmp _do_start // only required not to loose the first instruction

_do_start:
  movq  $2, %rax
  cmpq  $1, %rax
  movq  $0, %rdi
  jle   .L2
  nop
  nop
  nop
  call	_test
  nop
  nop
  nop
.L2:
  mov $0x3c, %rax
  syscall
  // useless, but useful to terminate cleanly our graph :-)
  ret
