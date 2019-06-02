/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

/*
Declare a multiboot header that marks the program as a kernel. These are magic
values that are documented in the multiboot standard. The bootloader will
search for this signature in the first 8 KiB of the kernel file, aligned at a
32-bit boundary. The signature is in its own section so the header can be
forced to be within the first 8 KiB of the kernel file.
*/
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
The multiboot standard does not define the value of the stack pointer register
(esp) and it is up to the kernel to provide a stack. This allocates room for a
small stack by creating a symbol at the bottom of it, then allocating 16384
bytes for it, and finally creating a symbol at the top. The stack grows
downwards on x86. The stack is in its own section so it can be marked nobits,
which means the kernel file is smaller because it does not contain an
uninitialized stack. The stack on x86 must be 16-byte aligned according to the
System V ABI standard and de-facto extensions. The compiler will assume the
stack is properly aligned and failure to align the stack will result in
undefined behavior.
*/
.section .bootstrap_stack, "aw", @nobits
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .bootstrap_heap, "aw", @nobits
.global bootstrap_heap_start
bootstrap_heap_start:
.skip 262144 # 256 KiB
.global bootstrap_heap_end
bootstrap_heap_end:

/* Preallocate pages used for paging without using hard-code addresses. */
.section .bss, "aw", @nobits
.align 0x1000 // align on pages
boot_page_directory:
.skip 0x1000 // 4 * 1024 entries
boot_page_table1:
.skip 0x1000 // 4 * 1024 entries


/* 
WARNING : We only allocate the first 4MiB of physical memory, while the fist
1MiB is reserved for BIOS/Multiboot/Hardware.
ANother page table will be required if the kernel grows byond 3 MiB.
*/

/*
The linker script specifies _start as the entry point to the kernel and the
bootloader will jump to this position once the kernel has been loaded. It
doesn't make sense to return from this function as the bootloader is gone.
*/

.section .text
.global _start
.type _start, @function
_start:
	/*
	The bootloader has loaded us into 32-bit protected mode on a x86
	machine. Interrupts are disabled. Paging is disabled. The processor
	state is as defined in the multiboot standard. The kernel has full
	control of the CPU. The kernel can only make use of hardware features
	and any code it provides as part of itself. There's no printf
	function, unless the kernel provides its own <stdio.h> header and a
	printf implementation. There are no security restrictions, no
	safeguards, no debugging mechanisms, only what the kernel provides
	itself. It has absolute and complete power over the machine.
	*/
	
	/* 
	We need to set up paging so that we can load the Virtualy addressed
	kernel.
	0x0 - 0x3FFFFF (P) <----> 0xC0000000 - 0xC003FFFFF (V)
	To avoid faults after enabling paging, we also need to setup a
	temporary identity mapping : 
	0x0 - 0x3FFFFF (P) <----> 0x0 - 0x3FFFFF (V)
	*/

	/* Physical address of boot_page_table1 in %edi */
	movl $(boot_page_table1 - 0xC0000000), %edi
	/* First address to map is 0x0 */
	movl $0, %esi
	/* Map the first 256 pages (bios+grub zone) RW */
	movl $256, %ecx 
	
loop_first_256:
	/* %edx contains a PTE */
	movl %esi, %edx
	/* set PTE flags : PRESENT + WRITABLE */
	orl $0x3, %edx
	/* write PTE in Page Table */
	movl %edx, (%edi)
	
	/* size of pages is 0x1000 */
	addl $0x1000, %esi
	/* size of PTE is 4 bytes */
	addl $4, %edi
	/* decrements %ecx unless %ecx becomes 0 */
	loop loop_first_256
	

	/* Map kernel read only pages (.text and .rodata) */
	movl $(_kernel_read_only_end), %ecx
	subl $(_kernel_start), %ecx

	/* If the end of read only section is not aligned with 0x1000
	 * add one more read only page */
	movl %ecx, %eax
	shrl $12, %ecx
	andl $0xfff, %eax
	cmpl $0, %eax
	jz loop_read_only_kernel
	addl $1, %ecx

loop_read_only_kernel:
	/* %edx contains a PTE */
	movl %esi, %edx
	/* set PTE flags : PRESENT */
	orl $0x1, %edx
	/* write PTE in page table */
	movl %edx, (%edi)
	
	/* size of pages is 0x1000 */
	addl $0x1000, %esi
	/* size of PTE is 4 bytes */
	addl $4, %edi
	/* decrements %ecx unless %ecx becomes 0 */
	loop loop_read_only_kernel

	/* Map the remaining pages of the pages table */
	movl $(_kernel_end), %ecx
	subl $(_kernel_read_only_end), %ecx
	/* If the end of section is not aligned with 0x1000
	 * add one more read only page */
	movl %ecx, %eax
	shrl $12, %ecx
	andl $0xfff, %eax
	cmpl $0, %eax
	jz loop_writable_kernel
	addl $1, %ecx

loop_writable_kernel:
	/* %edx contains a PTE */
	movl %esi, %edx
	/* set PTE flags : PRESENT + WRITABLE */
	orl $0x3, %edx
	/* write PTE in page table */
	movl %edx, (%edi)
	
	/* size of pages is 0x1000 */
	addl $0x1000, %esi
	/* size of PTE is 4 bytes */
	addl $4, %edi
	/* decrements %ecx unless %ecx becomes 0 */
	loop loop_writable_kernel

	/* Identity mapping : 0x0 - 0x3FFFFF
	0x3 is the flag of the PDE : PRESENT + WRITABLE */
	movl $(boot_page_table1 - 0xC0000000 + 0x3), boot_page_directory - 0xC0000000 + 0
	/* Mapping : 0x0 - 0x3FFFFF <----> 0xC0000000 - 0xC03FFFFF
	768 is the index of the PDE in the PD which represents the 0xC0000000 - 0xC0400000 
	Each PDE is 4 bytes long */
	movl $(boot_page_table1 - 0xC0000000 + 0x3), boot_page_directory - 0xC0000000 + 768 * 4
	
	/* Map the last PDE to the page directory itself to access PTE from
	 * virtual memory 
     * PD entries can be accessed at 0xfffff000 (from 0xfffff000 to 0xffffffff)
     * For one PD entry (let's say 768), PT entries can be accessed
     * at 0xffc00000 + (768 << 12) (so from 0xfff00000 to 0xfff00fff)
     */
	movl $(boot_page_directory - 0xC0000000 + 0x3), boot_page_directory - 0xC0000000 + 1023 * 4
	
	/* Put Page Directory physical address in %cr3 */
	movl $(boot_page_directory - 0xC0000000), %ecx
	movl %ecx, %cr3

	/* Enable paging and the kernel write-protect bit */
	movl %cr0, %ecx
	orl $0x80010000, %ecx
	movl %ecx, %cr0

	/* Now paging is enabled! */

	/* Make an absolute jump to higher half */
	lea higher_half_kernel, %ecx
	jmp *%ecx

higher_half_kernel:
	/* Now we are in higher half */
	
	/* Unmap identity mapping */
	movl $0, boot_page_directory + 0
	
	/* Reload %cr3 to flush the TLB */
	movl %cr3, %ecx
	movl %ecx, %cr3


	/*
	To set up a stack, we set the esp register to point to the top of the
	stack (as it grows downwards on x86 systems). This is necessarily done
	in assembly as languages such as C cannot function without a stack.
	*/
	mov $stack_top, %esp

	/*
	This is a good place to initialize crucial processor state before the
	high-level kernel is entered. It's best to minimize the early
	environment where crucial features are offline. Note that the
	processor is not fully initialized yet: Features such as floating
	point instructions and instruction set extensions are not initialized
	yet. The GDT should be loaded here. Paging should be enabled here.
	C++ features such as global constructors and exceptions will require
	runtime support to work as well.
	*/

	/*
	Enter the high-level kernel. The ABI requires the stack is 16-byte
	aligned at the time of the call instruction (which afterwards pushes
	the return pointer of size 4 bytes). The stack was originally 16-byte
	aligned above and we've since pushed a multiple of 16 bytes to the
	stack since (pushed 0 bytes so far) and the alignment is thus
	preserved and the call is well defined.
	*/

	pushl %ecx // contains the physical addr of the current Page Directory
	pushl %ebx  // contains the multiboot info address provided by GRUB
	
	call kernel_main

	/*
	If the system has nothing more to do, put the computer into an
	infinite loop. To do that:
	1) Disable interrupts with cli (clear interrupt enable in eflags).
	   They are already disabled by the bootloader, so this is not needed.
	   Mind that you might later enable interrupts and return from
	   kernel_main (which is sort of nonsensical to do).
	2) Wait for the next interrupt to arrive with hlt (halt instruction).
	   Since they are disabled, this will lock up the computer.
	3) Jump to the hlt instruction if it ever wakes up due to a
	   non-maskable interrupt occurring or due to system management mode.
	*/
	cli
1:	hlt
	jmp 1b

/*
Set the size of the _start symbol to the current location '.' minus its start.
This is useful when debugging or when you implement call tracing.
*/
.size _start, . - _start
