.macro ISR_NOERRCODE index
	.global isr\index
	.type isr\index, @function
	isr\index:
		cli // disable interruts
		pushl $0 // pushaa a dummy error code
		pushl $\index // push the interrupt index
		jmp isr_common_stub
.endm

.macro ISR_ERRCODE index
	.global isr\index
	.type isr\index, @function
	isr\index:
		cli // disable interrupts
		pushl $\index // push the interrupt index
		jmp isr_common_stub
.endm

// isr_handler declared in isr.c
.extern isr__handler

// This is our common ISR stub. It saves the processor state, sets
// upt the kernel mode segments, calls the C-level fault handler
// and finally restores the stack frame
isr_common_stub:
	pusha // pushes edi,esi,ebp,esp,ebx,adx,ecx,eax

	movw %ds, %ax
	pushl %eax // save %ds

	// load the kernel data segment descriptor
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	call isr__handler

	popl %eax // reload %ds
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	popa // pop all registers
	addl $8, %esp // clean error code and interrupt number
	sti // enable interrupt
	iret

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE 8
ISR_NOERRCODE 9
ISR_ERRCODE 10
ISR_ERRCODE 11
ISR_ERRCODE 12
ISR_ERRCODE 13
ISR_ERRCODE 14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31
