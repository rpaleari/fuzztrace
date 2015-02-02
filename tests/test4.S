/* Test for fallback branches */
.global _start
.text

_start:
  jmp tgt                       // just for the sake of getting tgt as a block

tgt:  
  mov $0x0, %rax
  test %rax,%rax
  jz exit
  xor %rax,%rax
  xor %rax,%rax
  xor %rax,%rax
exit:
  mov $0x3c, %rax
  mov $0x00, %rdi
  syscall
  // Useless ret to cleanly terminate bb after syscall
  ret
