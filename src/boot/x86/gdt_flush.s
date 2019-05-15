.global gdt__flush
.type gdt__flush, @function
gdt__flush:
	mov 4(%esp), %eax	// retrieve the GDT pointer from the stack
	lgdt (%eax)			// load the new GDT pointer

	// set all segment values EXCEPT %cs to the segment 3 (0x10)
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	// far jump to code segment to update %cs
	ljmp $0x08, $flush
flush:
	ret

	
