
#include <stdio.h>
#include <stdlib.h>

#include "../elf.h"
#include "../arena.h"
#include "../backend.h"
#include "../common.h"
#include "../arena.h"
#include "../ast.h"
#include "../typer.h"

#include "x86_64_common.h"

typedef struct {
	size_t count, capacity;
	uint8_t* items;
} Linux_x86_64_Elf_Ops_List;

typedef struct {
	size_t count, capacity;
	uint64_t* items;
} Linux_x86_64_Elf_Local_List;

typedef struct {
	String_View name;
	size_t file_offset;
	size_t count, capacity;
	uint8_t* items; // data/instructions
	Elf64_Shdr shdr;
	int index;
} Linux_x86_64_Elf_Section;

typedef struct {
	size_t count, capacity;
	Elf64_Sym* items; // data/instructions
} Linux_x86_64_Elf_Symbols;

typedef struct {
	X86_64_Machine_Code_Writer w;
	Linux_x86_64_Elf_Ops_List ops;

	LL_Ir_Function* fn;
	Linux_x86_64_Elf_Local_List locals;

	Linux_x86_64_Elf_Section* current_section;
	Linux_x86_64_Elf_Section section_text;
	Linux_x86_64_Elf_Section section_data;
	Linux_x86_64_Elf_Section section_strtab;
	Linux_x86_64_Elf_Symbols symbols;
} Linux_x86_64_Elf_Backend;

typedef struct {
	size_t size, alignment;
} Linux_x86_64_Elf_Layout;

void linux_x86_64_elf_append_op_segment_u8(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint8_t segment) {
	segment = AS_LITTLE_ENDIAN_U8(segment);
	arena_da_append(&cc->arena, b->current_section, segment);
}

void linux_x86_64_elf_append_op_segment_u16(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint16_t segment) {
	segment = AS_LITTLE_ENDIAN_U16(segment);
	arena_da_append_many(&cc->arena, b->current_section, (uint8_t*)&segment, 2);
}

void linux_x86_64_elf_append_op_segment_u32(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint32_t segment) {
	segment = AS_LITTLE_ENDIAN_U32(segment);
	arena_da_append_many(&cc->arena, b->current_section, (uint8_t*)&segment, 4);
}

void linux_x86_64_elf_append_op_segment_u64(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint64_t segment) {
	segment = AS_LITTLE_ENDIAN_U64(segment);
	arena_da_append_many(&cc->arena, b->current_section, (uint8_t*)&segment, 8);
}

static Linux_x86_64_Elf_Layout linux_x86_64_elf_get_layout(LL_Type* ty) {
	switch (ty->kind) {
	case LL_TYPE_INT: return (Linux_x86_64_Elf_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_UINT: return (Linux_x86_64_Elf_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_FLOAT: return (Linux_x86_64_Elf_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_POINTER: return (Linux_x86_64_Elf_Layout) { .size = 8, .alignment = 8 };
	default: return (Linux_x86_64_Elf_Layout) { .size = 0, .alignment = 0 };
	}
}

static void linux_x86_64_elf_write_operand(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand operand) {
	switch (operand & LL_IR_OPERAND_TYPE_MASK) {
	case LL_IR_OPERAND_IMMEDIATE_BIT:
		/* arena_sb_sprintf(&cc->arena, &b->output, "%d", operand & LL_IR_OPERAND_VALUE_MASK); */
	   	break;
	case LL_IR_OPERAND_REGISTER_BIT: {
		/* LL_Type* type = b->fn->registers.items[operand & LL_IR_OPERAND_VALUE_MASK].type; */
		/* switch (type->kind) { */
		/* case LL_TYPE_INT: */
		/* case LL_TYPE_UINT: */
		/* 	switch (type->width) { */
		/* 	case 8: arena_sb_append_cstr(&cc->arena, &b->output, "al"); break; */
		/* 	case 16: arena_sb_append_cstr(&cc->arena, &b->output, "ax"); break; */
		/* 	case 32: arena_sb_append_cstr(&cc->arena, &b->output, "eax"); break; */
		/* 	case 64: arena_sb_append_cstr(&cc->arena, &b->output, "rax"); break; */
		/* 	default: assert(false); */
		/* 	} */
		/* 	break; */
		/* default: assert(false); */
		/* } */
		break;
	}
	case LL_IR_OPERAND_LOCAL_BIT: {
		LL_Type* type = b->fn->locals.items[operand & LL_IR_OPERAND_VALUE_MASK].ident->base.type;
		uint64_t offset = b->locals.items[operand & LL_IR_OPERAND_VALUE_MASK];
		switch (type->kind) {
		case LL_TYPE_INT:
		case LL_TYPE_UINT:
			/* switch (type->width) { */
			/* case 8: arena_sb_append_cstr(&cc->arena, &b->output, "byte"); break; */
			/* case 16: arena_sb_append_cstr(&cc->arena, &b->output, "word"); break; */
			/* case 32: arena_sb_append_cstr(&cc->arena, &b->output, "dword"); break; */
			/* case 64: arena_sb_append_cstr(&cc->arena, &b->output, "qword"); break; */
			/* default: assert(false); */
			/* } */
			/* arena_sb_sprintf(&cc->arena, &b->output, " ptr [rbp - %lu]", offset); */
			break;
		case LL_TYPE_POINTER:
			/* arena_sb_sprintf(&cc->arena, &b->output, "qword ptr [rbp - %lu]", offset); */
			break;
		default: assert(false);
		}
		break;
	}
	}
}

void linux_x86_64_elf_init(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
	b->w.append_u8 = (__typeof__(b->w.append_u8))linux_x86_64_elf_append_op_segment_u8;
	b->w.append_u16 = (__typeof__(b->w.append_u16))linux_x86_64_elf_append_op_segment_u16;
	b->w.append_u32 = (__typeof__(b->w.append_u32))linux_x86_64_elf_append_op_segment_u32;
	b->w.append_u64 = (__typeof__(b->w.append_u64))linux_x86_64_elf_append_op_segment_u64;
	memset(&b->ops, 0, sizeof(b->ops));

	memset(&b->section_text, 0, sizeof(b->section_text));
	b->section_text.name = str_lit(".text");
	b->section_text.shdr.sh_type = SHT_PROGBITS;
	b->section_text.shdr.sh_flags = SHF_ALLOC | SHF_EXECINSTR;
	b->section_text.shdr.sh_addralign = 16;
	b->section_text.index = 1;

	memset(&b->section_data, 0, sizeof(b->section_data));
	b->section_data.name = str_lit(".data");
	b->section_data.shdr.sh_type = SHT_PROGBITS;
	b->section_data.shdr.sh_flags = SHF_ALLOC | SHF_WRITE;
	b->section_data.shdr.sh_addralign = 1;
	b->section_data.index = 2;

	memset(&b->section_strtab, 0, sizeof(b->section_strtab));
	b->section_strtab.name = str_lit(".strtab");
	b->section_strtab.shdr.sh_type = SHT_STRTAB;
	b->section_strtab.shdr.sh_addralign = 1;
	b->section_strtab.index = 3;

	memset(&b->symbols, 0, sizeof(b->symbols));

	b->current_section = &b->section_text;
	arena_da_append(&cc->arena, &b->section_strtab, 0);
	arena_da_append(&cc->arena, &b->symbols, ((Elf64_Sym){}));


	Elf64_Sym sym;
	{
		sym.st_name = b->section_strtab.count;
		arena_da_append_many(&cc->arena, &b->section_strtab, str_lit("text.c").ptr, str_lit("text.c").len);
		arena_da_append(&cc->arena, &b->section_strtab, 0);

		sym.st_info = ELF32_ST_INFO(STB_LOCAL, STT_FILE);
		sym.st_other = STV_DEFAULT;
		sym.st_shndx = SHN_ABS;
		sym.st_value = 0;
		sym.st_size = 0;
		arena_da_append(&cc->arena, &b->symbols, sym);
	}
	{
		sym.st_name = b->section_strtab.count;
		arena_da_append_many(&cc->arena, &b->section_strtab, str_lit(".text").ptr, str_lit(".text").len);
		arena_da_append(&cc->arena, &b->section_strtab, 0);

		sym.st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
		sym.st_other = STV_DEFAULT;
		sym.st_shndx = b->section_text.index;
		sym.st_value = 0;
		sym.st_size = 0;
		arena_da_append(&cc->arena, &b->symbols, sym);
	}
}

bool linux_x86_64_elf_write_to_file(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, char* filepath) {
	int i;
	FILE* fptr;
	Elf64_Shdr shdr, *shdr_ptr;
	if (!(fptr = fopen(filepath, "w"))) {
		fprintf(stderr, "Unable to open output file: %s\n", filepath);
		return false;
	}

	Elf64_Ehdr header = {
		.e_ident = { ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3, ELFCLASS64, ELFDATA2LSB, EV_CURRENT, ELFOSABI_SYSV, 0 },
		.e_type = ET_REL,
		.e_machine = EM_X86_64,
		.e_version = EV_CURRENT,
		.e_flags = 0,
		.e_ehsize = sizeof(header),
		/* .e_phentsize = sizeof(Elf64_Phdr), */
		.e_shentsize = sizeof(Elf64_Shdr),
   	};
	arena_da_append_many(&cc->arena, &b->ops, &header, sizeof(header));
	Elf64_Ehdr* hdrptr = (Elf64_Ehdr*)b->ops.items;
	/* fwrite(&header, 1, sizeof(header), fptr); */


	Linux_x86_64_Elf_Section sections[] = {
		{ 0 },
	   	b->section_text, b->section_data,
	   	{
			.name = str_lit(".symtab"),
		   	.items = (uint8_t*)b->symbols.items,
		   	.count = b->symbols.count * sizeof(Elf64_Sym),
			.shdr = {
				.sh_type = SHT_SYMTAB,
				.sh_info = 3,
				.sh_link = 4,
				.sh_addralign = 8,
				.sh_entsize = sizeof(Elf64_Sym),
			}
		},
		b->section_strtab
	};
	for (i = 0; i < LEN(sections); ++i) {

		if (sections[i].shdr.sh_addralign > 1) {
			uint64_t aligned_to = align_forward(b->ops.count, sections[i].shdr.sh_addralign);
			printf("alskfjldsf %x\n", aligned_to);
			arena_da_reserve(&cc->arena, &b->ops, aligned_to);
		}

		sections[i].file_offset = b->ops.count;
		arena_da_append_many(&cc->arena, &b->ops, sections[i].items, sections[i].count);
	}

	Linux_x86_64_Elf_Section shstrtab = { 0 };
	arena_da_append(&cc->arena, &shstrtab, 0);

	size_t shstrtab_offset = b->ops.count;
	for (i = 0; i < LEN(sections); ++i) {
		if (i != 0) {
			sections[i].shdr.sh_name = shstrtab.count;
			arena_da_append_many(&cc->arena, &shstrtab, sections[i].name.ptr, sections[i].name.len);
			arena_da_append(&cc->arena, &shstrtab, 0);
		} else {
			sections[i].shdr.sh_name = 0;
		}
	}

	size_t shstrtab_name = shstrtab.count;
	arena_da_append_many(&cc->arena, &shstrtab, str_lit(".shstrtab").ptr, str_lit(".shstrtab").len);
	arena_da_append(&cc->arena, &shstrtab, 0);

	arena_da_append_many(&cc->arena, &b->ops, shstrtab.items, shstrtab.count);

	uint64_t needed_padding = align_forward(b->ops.count, 16) - b->ops.count;
	if (needed_padding) {
		arena_da_reserve(&cc->arena, &b->ops, b->ops.count + needed_padding);
	}

	hdrptr = (Elf64_Ehdr*)b->ops.items;
	hdrptr->e_shoff = b->ops.count;
	hdrptr->e_shnum = LEN(sections) + 1 /* plus one for shstrtab */;
	hdrptr->e_shstrndx = LEN(sections);

	for (i = 0; i < LEN(sections); ++i) {
		shdr.sh_name = sections[i].shdr.sh_name;
		shdr.sh_type = sections[i].shdr.sh_type;
		shdr.sh_flags = sections[i].shdr.sh_flags;
		shdr.sh_addr = 0x00000;
		shdr.sh_offset = i == 0 ? 0 : sections[i].file_offset;
		shdr.sh_size = sections[i].count;
		shdr.sh_link = sections[i].shdr.sh_link;
		shdr.sh_info = sections[i].shdr.sh_info;
		shdr.sh_addralign = sections[i].shdr.sh_addralign;
		shdr.sh_entsize = sections[i].shdr.sh_entsize;

		arena_da_append_many(&cc->arena, &b->ops, &shdr, sizeof(shdr));
	}

	size_t shdr_offset = b->ops.count;
	arena_da_append_many(&cc->arena, &b->ops, &shdr, sizeof(shdr));
	shdr_ptr = (Elf64_Shdr*)(b->ops.items + shdr_offset);

	shdr_ptr->sh_name = shstrtab_name;
	shdr_ptr->sh_offset = shstrtab_offset;
	shdr_ptr->sh_type = SHT_STRTAB;
	shdr_ptr->sh_flags = 0;
	shdr_ptr->sh_addr = 0;
	shdr_ptr->sh_size = shstrtab.count;
	shdr_ptr->sh_link = 0;
	shdr_ptr->sh_info = 0;
	shdr_ptr->sh_addralign = 1;
	shdr_ptr->sh_entsize = 0;

	return fwrite(b->ops.items, 1, b->ops.count, fptr) == b->ops.count;
	/* return fwrite(&header, 1, sizeof(header), fptr) == sizeof(header); */
	/* return fwrite(&, 1, sizeof(header), fptr) == sizeof(header); */
	/* return true; */
}

static void linux_x86_64_elf_generate_block(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
	size_t i;
	for (i = 0; i < block->ops.count; ++i) {
		LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->ops.items[i];
		LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->ops.items[i + 1];
		printf("%zu\n", x86_64_instructions_table_size);

		switch (opcode) {
		case LL_IR_OPCODE_RET: break;
		case LL_IR_OPCODE_STORE:

			/* c7 c0 04 00 00 00 */
			/* 48 c7 c0 04 00 00 00 */

			/* X86_64_OPERAND_MI_MEM(X86_64_OPCODE_MOV, word, rbp, 0xA0, 100); */
			/* X86_64_OPERAND_MI_MEM(X86_64_OPCODE_MOV, dword, rbp, -0x90, 100); */
			/* X86_64_OPERAND_MI(X86_64_OPCODE_MOV, byte, rbp, 0x10, 100); */
			/* arena_sb_append_cstr(&cc->arena, &b->output, BACKEND_INDENT "mov "); */
			/* write_operand(cc, b, bir, operands[0]); */
			/* arena_sb_append_cstr(&cc->arena, &b->output, ", "); */
			/* write_operand(cc, b, bir, operands[1]); */
			/* arena_sb_append_cstr(&cc->arena, &b->output, "\n"); */
		   	i += 2;
		   	break;
		case LL_IR_OPCODE_LOAD: i += 2; break;
		case LL_IR_OPCODE_INVOKE: {
			uint32_t count = operands[2];
			for (uint32_t j = 0; j < count; ++j) {
			}

			i += 3 + count;
		   	break;
		}
		}
	}
}

void linux_x86_64_elf_generate(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir) {
	Elf64_Sym sym;
	int fi;
	for (fi = 0; fi < bir->fns.count; ++fi) {
		LL_Ir_Function* fn = &bir->fns.items[fi];
		LL_Ir_Block* block = fn->entry;
		b->fn = fn;

		arena_da_reserve(&cc->tmp_arena, &b->locals, fn->locals.count);

		uint64_t offset = 0;
		for (int li = 0; li < fn->locals.count; ++li) {
			Linux_x86_64_Elf_Layout l = linux_x86_64_elf_get_layout(fn->locals.items[li].ident->base.type);
			offset = align_forward(offset + l.size, l.alignment);
			b->locals.items[li] = offset;
		}

		/* printf("function " FMT_SV_FMT ":\n", FMT_SV_ARG(fn->ident->str)); */

		size_t function_offset = b->current_section->count;
		/* X86_64_WRITE_INSTRUCTION(OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp })); */
		/* X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rsp })); */

		int bi = 0;
		while (block) {
			linux_x86_64_elf_generate_block(cc, b, bir, block);
			bi++;
			block = block->next;
		}

		/* X86_64_WRITE_INSTRUCTION(OPCODE_POP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp })); */
		X86_64_WRITE_INSTRUCTION(OPCODE_RET, noarg, ((X86_64_Instruction_Parameters){ }));

		sym.st_name = b->section_strtab.count;
		arena_da_append_many(&cc->arena, &b->section_strtab, fn->ident->str.ptr, fn->ident->str.len);
		arena_da_append(&cc->arena, &b->section_strtab, 0);

		sym.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_FUNC);
		sym.st_other = STV_DEFAULT;
		sym.st_shndx = b->current_section->index;
		sym.st_value = function_offset;
		sym.st_size = b->current_section->count - function_offset;
		arena_da_append(&cc->arena, &b->symbols, sym);
	}
}

