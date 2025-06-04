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

	mov eax, [rax]
	mov eax, [rcx]
	mov eax, [rdx]
	mov eax, [rbx]
	mov eax, [rsp]
	mov eax, [rbp]
	mov eax, [rsi]
	mov eax, [rdi]

	mov eax, [rax + rcx]
	mov eax, [rcx + rcx]
	mov eax, [rdx + rcx]
	mov eax, [rbx + rcx]
	mov eax, [rsp + rcx]
	mov eax, [rbp + rcx]
	mov eax, [rsi + rcx]
	mov eax, [rdi + rcx]

	mov eax, [rax * 4]
	mov eax, [rcx * 4]
	mov eax, [rdx * 4]
	mov eax, [rbx * 4]
	# mov eax, [rsp * 4]
	mov eax, [rbp * 4]
	mov eax, [rsi * 4]
	mov eax, [rdi * 4]

	mov eax, [rax + 7]
	mov eax, [rcx + 7]
	mov eax, [rdx + 7]
	mov eax, [rbx + 7]
	mov eax, [rsp + 7]
	mov eax, [rbp + 7]
	mov eax, [rsi + 7]
	mov eax, [rdi + 7]
	mov eax, [rsp * 4]
	mov eax, [r12 * 4]

	mov eax, [rax * 4 + rcx]
	mov eax, [rcx * 4 + rcx]
	mov eax, [rdx * 4 + rcx]
	mov eax, [rbx * 4 + rcx]
	# mov eax, [rsp * 4 + rcx]
	mov eax, [rbp * 4 + rcx]
	mov eax, [rsi * 4 + rcx]
	mov eax, [rdi * 4 + rcx]

	mov eax, [rax + rcx + 7]
	mov eax, [rcx + rcx + 7]
	mov eax, [rdx + rcx + 7]
	mov eax, [rbx + rcx + 7]
	mov eax, [rsp + rcx + 7]
	mov eax, [rbp + rcx + 7]
	mov eax, [rsi + rcx + 7]
	mov eax, [rdi + rcx + 7]

	mov eax, [rax * 4 + rcx + 7]
	mov eax, [rcx * 4 + rcx + 7]
	mov eax, [rdx * 4 + rcx + 7]
	mov eax, [rbx * 4 + rcx + 7]
	# mov eax, [rsp * 4 + rcx + 7]
	mov eax, [rbp * 4 + rcx + 7]
	mov eax, [rsi * 4 + rcx + 7]
	mov eax, [rdi * 4 + rcx + 700]
	mov ecx, [rcx + rbx]

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
