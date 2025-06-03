#pragma once

#include <stddef.h>
#include "common.h"
#include "ast.h"
#include "backends/ir.h"

#define BACKEND_INDENT "    "

typedef struct {
	size_t count;
	size_t capacity;
	char* items;
} String_Builder;

typedef enum {
	LL_BACKEND_IR,
	LL_BACKEND_LINUX_X86_64_GAS,
	LL_BACKEND_LINUX_X86_64_ELF,
} LL_Backend_Kind;

typedef struct {
	LL_Backend_Kind kind;
	void* backend;
} LL_Backend;

LL_Backend ll_backend_init(Compiler_Context* cc, LL_Backend_Kind kind);
bool ll_backend_write_to_file(Compiler_Context* cc, LL_Backend* b, char* filepath);

void ll_backend_generate_statement(Compiler_Context* cc, LL_Backend* b, Ast_Base* stmt);
void ll_backend_generate_statement_from_ir(Compiler_Context* cc, LL_Backend* b, LL_Backend_Ir* bir);

static inline uint64_t align_forward(uint64_t offset, uint64_t alignment) {
      return ((offset + alignment - 1) & (~(alignment - 1)));
}

#define AS_LITTLE_ENDIAN_U8(x) (x)
#define AS_LITTLE_ENDIAN_U16(x) (x)
#define AS_LITTLE_ENDIAN_U32(x) (x)
#define AS_LITTLE_ENDIAN_U64(x) (x)

#define PUN(x, type) ({ __typeof__(x) val = (x); (*((type*)&val)); })

