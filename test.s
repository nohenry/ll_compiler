	.text
	.intel_syntax noprefix
	.file	"test.c"
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	push	rbp
	.cfi_def_cfa_offset 16
	.cfi_offset rbp, -16
	mov	rbp, rsp
	.cfi_def_cfa_register rbp
	sal ax, 1
	rol ax, 1

	mov	byte ptr [rbp], 0x7
	mov	word ptr [rbp], 0x7
	mov	dword ptr [rbp], 0x7
	mov	qword ptr [rbp], 0x7
	mov	byte ptr [r8], 0x7
	mov	word ptr [r8], 0x7
	mov	dword ptr [r8], 0x7
	mov	qword ptr [r8], 0x7

	mov	dword ptr [rbp - 4], 0
	mov	dword ptr [rbp - 8], edi
	mov	qword ptr [rbp - 16], rsi
	mov	dword ptr [rbp - 20], 12
	mov	eax, dword ptr [rbp - 20]
	mov	dword ptr [rbp - 24], eax
	xor	eax, eax
	pop	rbp
	.cfi_def_cfa rsp, 8
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc
                                        # -- End function
	.ident	"clang version 14.0.6"
	.section	".note.GNU-stack","",@progbits
