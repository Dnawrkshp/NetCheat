# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: crt0.s 1066 2005-04-29 10:25:23Z pixel $
# Standard startup file.


.set noat
.set noreorder

.global _start
.global	_exit

	# Support for _init() and _fini().
	.global _init
	.global _fini
	.type	_init, @function
	.type	_fini, @function

	# The .weak keyword ensures there's no error if
	# _init/_fini aren't defined.
	.weak	_init
	.weak	_fini

	.extern	_heap_size
	.extern	_stack
	.extern _stack_size

	.text

	nop
	nop

	.ent _start
_start:

# Clear bss elf segment (static uninitalised data)
zerobss:
	la	$2, _fbss
	la	$3, _end
loop:
	nop
	nop
	nop
	sq	$0,($2)
	sltu	$1,$2,$3
	bne	$1,$0,loop
	addiu	$2,$2,16

# Some program loaders (such as Pukklink) execute programs as a thread, but
# support passing argc and argv values via a0.  This also indicates that when
# the program terminates, control should be returned to the program loader
# instead of the PS2 browser.
	la	$2, _args_ptr
    	sw	$4,($2)

# Setup a thread to use
	la	$4, _gp
	la	$5, _stack
	la	$6, _stack_size
	la	$7, _args
	la	$8, _root
	move	$28,$4
	addiu	$3,$0,60
	syscall			# RFU060(gp, stack, stack_size, args, root_func)
	move	$29, $2

# Heap
	addiu	$3,$0,61
	la	$4, _end
	la	$5, _heap_size
	syscall			# RFU061(heap_start, heap_size)
	nop

# Flush the data cache (no need to preserve regs for this call)
	li	$3, 0x64
	move	$4,$0
	syscall			# FlushCache(0) - Writeback data cache

# Let's try to set up the initial cwd; have to pass on argc and argv.
# That may be used for other purposes as well, such as tweaking the command
# line on the fly, while providing gnu-style "generic option system".

# Check for arguments pased via ExecPS2 or LoadExecPS2
	la	$2, _args
	lw	$3, ($2)
	bnez	$3, 1f
	nop

# Otherwise check for arguments passed by a loader via a0 (_arg_ptr)
	la	$2, _args_ptr
	lw	$3, ($2)
	beqzl	$3, 2f
	addu	$4, $0, 0

	addiu	$2, $3, 4
1:
	lw	$4, ($2)
	addiu	$5, $2, 4
2:
# just get rid of it - we're not sending any args, but executing through ExecPS2, so a0 != 0, here will most likely tlbmiss occur
  jal	_ps2sdk_args_parse
	nop
	nop

	# Call ps2sdk's libc initialisation.
	jal	_ps2sdk_libc_init
	nop
	
	# Call global constructors through _init().
	la	$8, _init
	beqz	$8, 1f		# does _init() exist?
	nop

	jalr	$8
	nop
1:


# Jump main, now that environment and args are setup
	ei

# Check for arguments pased via ExecPS2 or LoadExecPS2
	la	$2, _args
	lw	$3, ($2)
	bnez	$3, 1f
	nop

# Otherwise check for arguments passed by a loader via a0 (_arg_ptr)
	la	$2, _args_ptr
	lw	$3, ($2)
	beqzl	$3, 2f
	addu	$4, $0, 0

	addiu	$2, $3, 4
1:
	lw	$4, ($2)
	addiu	$5, $2, 4
2:
	jal	main
	nop
	.end	_start

	.ent	_exit
_exit:
	# Call global deconstructors through _fini().
	la	$8, _fini
	beqz	$8, 3f		# does _fini() exist?
	nop

	jalr	$8
	nop
3:
	# Call ps2sdk's libc deinitialisation.
	jal     _ps2sdk_libc_deinit
	nop
	
# If we received our program arguments in a0, then we were executed by a
# loader, and we don't want to return to the browser.
	la	$4, _args_ptr
	lw	$5, ($4)
	beqz	$5, 1f
	move	$4, $2		# main()'s return code

	lw	$6, ($5)
	sw	$0, ($6)
	addiu	$3, $0, 36
	syscall			# ExitDeleteThread(void)

# Return to the browser via Exit()
1:
	addiu	$3, $0, 4
	syscall			# Exit(void)
	.end	_exit

# Call ExitThread()
	.ent	_root
_root:
	addiu	$3, $0, 35
	syscall
	.end	_root

	.bss
	.align	6
_args:
	.space	256+16*4+4
_args_ptr:
	.space	4
