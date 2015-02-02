        .global _start
        .text

foo:
	inc %eax
	ret
	
_start:
	xor %eax, %eax
	jmp label1

label1:
	call foo
	nop
	jmp exit
	nop

exit:
        mov $0x3c, %rax
        mov $0x00, %rdi
        syscall
