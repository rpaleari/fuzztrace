	.global _start
	.text

_start:
	xor %eax, %eax
	xor %ecx, %ecx
	mov $0x10, %eax

loop1:
	cmp $0x03, %eax
	jne tail
	inc %ecx
	inc %ecx
tail:
	dec %eax
	jnz loop1

exit:
	mov $0x3c, %rax
	mov $0x00, %rdi
	syscall
