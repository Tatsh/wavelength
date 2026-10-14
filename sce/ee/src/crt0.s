# Start-up code of the Emotion Engine executable.
#
# The kernel enters _start with interrupts disabled. The routine clears .bss, requests the main
# thread's stack and the heap from the kernel, initialises the kernel library, and calls main.
# main's result ends the program through Exit.

	.set	noreorder
	.set	noat

	.text
	.align	3
	.globl	_start
	.type	_start, @function
	.ent	_start
_start:
	# Clear every general register but the kernel's $26 and $27, the multiply registers, the shift
	# amount, the floating-point registers and accumulator, and the floating-point control word.
	.irp	reg, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 28, 29, 30, 31
	padduw	$\reg, $0, $0
	.endr
	mthi	$0
	mthi1	$0
	mtlo	$0
	mtlo1	$0
	mtsah	$0, 0
	.irp	reg, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
	mtc1	$0, $f\reg
	.endr
	adda.s	$f0, $f1
	sync.p
	ctc1	$0, $31

	la	$2, _fbss
	la	$3, _end
	# The empty instructions lengthen the loop past the six instructions of the R5900 short-loop erratum.
1:
	sq	$0, 0($2)
	nop
	sltu	$1, $2, $3
	nop
	nop
	bne	$1, $0, 1b
	addiu	$2, $2, 16

	# SetupThread(_gp, _stack, _stack_size, _args, _root) returns the stack pointer.
	la	$4, _gp
	la	$5, _stack
	la	$6, _stack_size
	la	$7, _args
	la	$8, _root
	move	$28, $4
	addiu	$3, $0, 60
	syscall
	move	$29, $2

	# SetupHeap(_end, _heap_size).
	la	$4, _end
	la	$5, _heap_size
	addiu	$3, $0, 61
	syscall

	jal	_InitSys
	nop
	jal	FlushCache
	move	$4, $0
	ei

	# The compiler runtime runs the static constructors from _init. The original compiler ran
	# them from main instead.
	jal	_init
	nop

	la	$2, _args
	lw	$4, 0($2)
	jal	main
	addiu	$5, $2, 4
	j	Exit
	move	$4, $2
	.end	_start
	.size	_start, . - _start

	.align	3
	.globl	_root
	.type	_root, @function
	.ent	_root
_root:
	# The kernel runs _root when the main thread returns. _root calls ExitThread.
	addiu	$3, $0, 35
	syscall
	.end	_root
	.size	_root, . - _root

	.bss
	.align	6
	.globl	_args
	.type	_args, @object
# The kernel writes the argument count, sixteen argument pointers, and the argument strings here
# for SetupThread.
_args:
	.space	4 + 16 * 4 + 256
	.size	_args, 4 + 16 * 4 + 256
