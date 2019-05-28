.global idt__flush
.type idt__flush, @function
idt__flush:
	mov 4(%esp), %eax // load idt ptr from the stack
	lidt (%eax)
	ret

.global idt__sti
.type idt__sti, @function
idt__sti:
	sti
	ret

