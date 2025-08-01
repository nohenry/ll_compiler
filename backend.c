
#include "backend.h"
#include "ast.h"

/* #include "backends/linux-x64_64-gas.c" */
#include "backends/linux-x64_64-elf.c"
#include "backends/ir.c"
#include "backends/c.c"
#include "backends/qbe.c"
#include "backends/x86_64_common.c"

LL_Backend ll_backend_init(Compiler_Context* cc, LL_Backend_Kind kind) {
	size_t backend_size;
	switch (kind) {
	case LL_BACKEND_IR: backend_size = sizeof(LL_Backend_Ir); break;
	/* case LL_BACKEND_LINUX_X86_64_GAS: backend_size = sizeof(Linux_x86_64_Gas_Backend); break; */
	case LL_BACKEND_LINUX_X86_64_ELF: backend_size = sizeof(Linux_x86_64_Elf_Backend); break;
	case LL_BACKEND_C: backend_size = sizeof(LL_Backend_C); break;
	case LL_BACKEND_QBE: backend_size = sizeof(LL_Backend_Qbe); break;
	}

	LL_Backend backend = {
		.kind = kind,
		.backend = arena_alloc(&cc->arena, backend_size),
	};

	switch (kind) {
	case LL_BACKEND_IR: ir_init(cc, backend.backend); break;
	/* case LL_BACKEND_LINUX_X86_64_GAS: linux_x86_64_gas_init(cc, backend.backend); break; */
	case LL_BACKEND_LINUX_X86_64_ELF:
		linux_x86_64_elf_init(cc, backend.backend);
		backend.get_layout = linux_x86_64_elf_get_layout;
	   	break;
	case LL_BACKEND_C:
		backend_c_init(cc, backend.backend); break;
	case LL_BACKEND_QBE:
		backend_qbe_init(cc, backend.backend); break;
		break;
	}

	return backend;
}

void ll_backend_generate_statement(Compiler_Context* cc, LL_Backend* b, Ast_Base* stmt) {
	switch (b->kind) {
	case LL_BACKEND_IR: ir_generate_statement(cc, b->backend, stmt); break;
	case LL_BACKEND_C: backend_c_generate_root(cc, b->backend, stmt); break;
	case LL_BACKEND_QBE: backend_c_generate_root(cc, b->backend, stmt); break;
	}
}

void ll_backend_generate_statement_from_ir(Compiler_Context* cc, LL_Backend* b, LL_Backend_Ir* bir) {
	switch (b->kind) {
	/* case LL_BACKEND_LINUX_X86_64_GAS: linux_x86_64_gas_generate(cc, b->backend, bir); break; */
	case LL_BACKEND_LINUX_X86_64_ELF: linux_x86_64_elf_generate(cc, b->backend, bir); break;
	}
}

bool ll_backend_write_to_file(Compiler_Context* cc, LL_Backend* b, char* filepath) {
	switch (b->kind) {
	case LL_BACKEND_IR: return ir_write_to_file(cc, b->backend, filepath); break;
	case LL_BACKEND_C: return backend_c_write_to_file(cc, b->backend, filepath); break;
	case LL_BACKEND_QBE: return backend_qbe_write_to_file(cc, b->backend, filepath); break;
	/* case LL_BACKEND_LINUX_X86_64_GAS: return linux_x86_64_gas_write_to_file(cc, b->backend, filepath); break; */
	case LL_BACKEND_LINUX_X86_64_ELF: return linux_x86_64_elf_write_to_file(cc, b->backend, filepath); break;
	}
}

