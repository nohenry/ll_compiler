
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
	size_t count, capacity, starti;
	uint8_t* items;
	uint32_t free;
} Linux_x86_64_Register_Queue;

typedef struct {
	size_t text_rel_byte_offset;
	uint32_t data_item;
} Linux_x86_64_Internal_Relocation;

typedef struct {
	size_t count, capacity;
	Linux_x86_64_Internal_Relocation* items;
} Linux_x86_64_Internal_Relocation_List;

typedef struct {
	size_t count, capacity;
	Elf64_Rela* items;
} Linux_x86_64_Relocation_List;

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
	Linux_x86_64_Elf_Local_List registers;

	/* Linux_x86_64_Internal_Relocation_List internal_relocations; */
	Linux_x86_64_Relocation_List relocations;
	Linux_x86_64_Internal_Relocation_List internal_relocations;

	Linux_x86_64_Elf_Section* current_section;
	Linux_x86_64_Elf_Section section_text;
	Linux_x86_64_Elf_Section section_data;
	Linux_x86_64_Elf_Section section_strtab;
	Linux_x86_64_Elf_Symbols symbols;

	Linux_x86_64_Register_Queue register_queue;
	uint8_t next_reg;

	uint32_t stack_used;
} Linux_x86_64_Elf_Backend;

typedef struct {
	size_t size, alignment;
} Linux_x86_64_Elf_Layout;

typedef struct {
    const X86_64_Operand_Register* registers;
    uint8_t register_count, register_next;
	int32_t stack_offset;
} Linux_x86_64_Elf_Call_Convention;

const static X86_64_Operand_Register call_convention_registers_systemv[] = {
    X86_64_OPERAND_REGISTER_rdi,
    X86_64_OPERAND_REGISTER_rsi,
    /* X86_64_OPERAND_REGISTER_rdx, */
    /* X86_64_OPERAND_REGISTER_rcx, */
    /* X86_64_OPERAND_REGISTER_r8, */
    /* X86_64_OPERAND_REGISTER_r9, */
};

Linux_x86_64_Elf_Call_Convention linux_x64_64_call_convention_systemv(Linux_x86_64_Elf_Backend* b) {
    Linux_x86_64_Elf_Call_Convention result;
    result.registers = call_convention_registers_systemv;
    result.register_count = LEN(call_convention_registers_systemv);
    result.register_next = 0;
	result.stack_offset = 0;
    return result;
}

X86_64_Operand_Register linux_x64_64_call_convention_next_reg(Linux_x86_64_Elf_Backend* b, Linux_x86_64_Elf_Call_Convention* cc) {
    if (cc->register_next >= cc->register_count) {
        return X86_64_OPERAND_REGISTER_invalid;
    } else {
        return cc->registers[cc->register_next++];
    }
}

uint32_t linux_x64_64_call_convention_next_mem(Linux_x86_64_Elf_Backend* b, Linux_x86_64_Elf_Call_Convention* cc) {
	uint32_t stack_offset = cc->stack_offset;
	cc->stack_offset += 8;
	b->stack_used += 8;
	return stack_offset;
}

void linux_x64_64_register_queue_reset(Compiler_Context* cc, Linux_x86_64_Register_Queue* queue) {
	queue->starti = 0;
	queue->count = 0;
	queue->free = 0xFFCFu;

	for (size_t i = 0; i < x86_64_usable_gp_registers_count; ++i) {
		arena_da_append(&cc->arena, queue, x86_64_usable_gp_registers[i]);
	}
}

void linux_x64_64_register_queue_enqueue(Compiler_Context* cc, Linux_x86_64_Register_Queue* queue, uint8_t reg) {
	if (queue->starti >= queue->count) {
		queue->starti = 0;
		queue->count = 0;
	}
	queue->free |= (uint32_t)(1u << reg);
	if (queue->starti > 0) {
		queue->items[--queue->starti] = reg;
	} else {
		arena_da_append(&cc->arena, queue, reg);
	}
}

uint8_t linux_x64_64_register_queue_dequeue(Compiler_Context* cc, Linux_x86_64_Register_Queue* queue) {
	uint8_t result;
	while (queue->starti < queue->count) {
		result = queue->items[queue->starti++];
		if (queue->free & (uint32_t)(1u << result)) {
			queue->free &= ~(uint32_t)(1u << result);
			return result;
		}
	}
	return (uint8_t)-1;
}


static inline bool linux_x64_64_register_queue_is_free(Compiler_Context* cc, Linux_x86_64_Register_Queue* queue, uint8_t reg) {
	return queue->free & (uint32_t)(1u << reg);
}

static inline void linux_x64_64_register_queue_use_reg(Compiler_Context* cc, Linux_x86_64_Register_Queue* queue, uint8_t reg) {
	queue->free |= (uint32_t)(1u << reg);
}

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

void linux_x86_64_elf_append_op_many(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint8_t* bytes, uint64_t count) {
	arena_da_append_many(&cc->arena, b->current_section, (uint8_t*)bytes, count);
}

static Linux_x86_64_Elf_Layout linux_x86_64_elf_get_layout(LL_Type* ty) {
	switch (ty->kind) {
	case LL_TYPE_INT: return (Linux_x86_64_Elf_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_UINT: return (Linux_x86_64_Elf_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_FLOAT: return (Linux_x86_64_Elf_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
	case LL_TYPE_POINTER: return (Linux_x86_64_Elf_Layout) { .size = 8, .alignment = 8 };
	case LL_TYPE_STRING: return (Linux_x86_64_Elf_Layout) { .size = 8, .alignment = 8 };
	default: return (Linux_x86_64_Elf_Layout) { .size = 0, .alignment = 1 };
	}
}

static X86_64_Operand_Register linux_x86_64_elf_get_next_reg(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b) {
	X86_64_Operand_Register result;
	result = x86_64_usable_gp_registers[b->next_reg++];
	if (b->next_reg >= x86_64_usable_gp_registers_count) {
		/* TODO: handle this case */
		assert(false);
	}
	return result;
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
	b->w.append_many = (__typeof__(b->w.append_many))linux_x86_64_elf_append_op_many;
	memset(&b->ops, 0, sizeof(b->ops));
	memset(&b->internal_relocations, 0, sizeof(b->internal_relocations));
	memset(&b->relocations, 0, sizeof(b->relocations));

	memset(&b->section_text, 0, sizeof(b->section_text));
	b->section_text.name = str_lit(".text");
	b->section_text.shdr.sh_type = SHT_PROGBITS;
	b->section_text.shdr.sh_flags = SHF_ALLOC | SHF_EXECINSTR;
	b->section_text.shdr.sh_addralign = 16;
	b->section_text.shdr.sh_addr = 0;
	b->section_text.index = 1;

	memset(&b->section_data, 0, sizeof(b->section_data));
	b->section_data.name = str_lit(".data");
	b->section_data.shdr.sh_type = SHT_PROGBITS;
	b->section_data.shdr.sh_flags = SHF_ALLOC | SHF_WRITE;
	b->section_data.shdr.sh_addralign = 1;
	b->section_data.shdr.sh_addr = 0;
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
	{
		sym.st_name = b->section_strtab.count;
		arena_da_append_many(&cc->arena, &b->section_strtab, str_lit(".data").ptr, str_lit(".data").len);
		arena_da_append(&cc->arena, &b->section_strtab, 0);

		sym.st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
		sym.st_other = STV_DEFAULT;
		sym.st_shndx = b->section_data.index;
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



	Linux_x86_64_Elf_Section sections[] = {
		{ 0 },
	   	b->section_text, b->section_data,
	   	{
			.name = str_lit(".symtab"),
		   	.items = (uint8_t*)b->symbols.items,
		   	.count = b->symbols.count * sizeof(Elf64_Sym),
			.shdr = {
				.sh_type = SHT_SYMTAB,
				.sh_info = 4,
				.sh_link = 5,
				.sh_addralign = 8,
				.sh_entsize = sizeof(Elf64_Sym),
			}
		},
	   	{
			.name = str_lit(".rela.text"),
		   	.items = (uint8_t*)b->relocations.items,
		   	.count = b->relocations.count * sizeof(Elf64_Rela),
			.shdr = {
				.sh_type = SHT_RELA,
				.sh_info = 1,
				.sh_link = 3,
				.sh_addralign = 8,
				.sh_entsize = sizeof(Elf64_Rela),
			}
		},
		b->section_strtab
	};
	for (i = 0; i < LEN(sections); ++i) {

		if (sections[i].shdr.sh_addralign > 1) {
			uint64_t aligned_to = align_forward(b->ops.count, sections[i].shdr.sh_addralign);
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
		shdr.sh_addr = sections[i].shdr.sh_addr;
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
}

#define OPD_VALUE(operand) (operand & LL_IR_OPERAND_VALUE_MASK)
#define OPD_TYPE(operand) (operand & LL_IR_OPERAND_TYPE_MASK)

static void x86_64_allocate_physical_registers(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
	size_t i;
    uint32_t offset = 0;
	for (i = 0; i < block->rops.count; ) {
		LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->rops.items[i];
		LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->rops.items[i + 1];

		/* ir_print_op(cc, bir, block->rops.items, i); */
		/* printf("\n"); */

		switch (opcode) {
		case LL_IR_OPCODE_RET: break;
		case LL_IR_OPCODE_RETVALUE: {
			if (linux_x64_64_register_queue_is_free(cc, &b->register_queue, X86_64_OPERAND_REGISTER_rax)) {
				linux_x64_64_register_queue_use_reg(cc, &b->register_queue, X86_64_OPERAND_REGISTER_rax);

				if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
					b->registers.items[OPD_VALUE(operands[1])] = X86_64_OPERAND_REGISTER_rax;
				}
			} else assert(false);
			break;
		}
		case LL_IR_OPCODE_STORE:
			if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
				/* b->registers.items[OPD_VALUE(operands[1])] = linux_x86_64_elf_get_next_reg(cc, b); */
				b->registers.items[OPD_VALUE(operands[1])] = linux_x64_64_register_queue_dequeue(cc, &b->register_queue);
			}
			break;
		case LL_IR_OPCODE_CAST:
		case LL_IR_OPCODE_LOAD:
			if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
				b->registers.items[OPD_VALUE(operands[1])] = linux_x64_64_register_queue_dequeue(cc, &b->register_queue);
			}
			linux_x64_64_register_queue_enqueue(cc, &b->register_queue, b->registers.items[OPD_VALUE(operands[0])]);
			break;
		case LL_IR_OPCODE_LEA:
			linux_x64_64_register_queue_enqueue(cc, &b->register_queue, b->registers.items[OPD_VALUE(operands[0])]);
		   	break;
		case LL_IR_OPCODE_AND:
		case LL_IR_OPCODE_OR:
		case LL_IR_OPCODE_XOR:
		case LL_IR_OPCODE_SUB:
		case LL_IR_OPCODE_ADD:
			b->registers.items[OPD_VALUE(operands[0])] = linux_x64_64_register_queue_dequeue(cc, &b->register_queue);
			if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
				/* result and first operand can be the same */
				b->registers.items[OPD_VALUE(operands[1])] = b->registers.items[OPD_VALUE(operands[0])];
			}

			if (OPD_TYPE(operands[2]) == LL_IR_OPERAND_REGISTER_BIT) {
				if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
					/* result and second operand can be the same, if first operand isn't register */
					b->registers.items[OPD_VALUE(operands[2])] = linux_x64_64_register_queue_dequeue(cc, &b->register_queue);
				} else {
					b->registers.items[OPD_VALUE(operands[2])] = b->registers.items[OPD_VALUE(operands[0])];
				}
			}

			linux_x64_64_register_queue_enqueue(cc, &b->register_queue, b->registers.items[OPD_VALUE(operands[0])]);

			break;
        case LL_IR_OPCODE_MUL: {
            if (linux_x64_64_register_queue_is_free(cc, &b->register_queue, X86_64_OPERAND_REGISTER_rax)) {
                linux_x64_64_register_queue_use_reg(cc, &b->register_queue, X86_64_OPERAND_REGISTER_rax);

                b->registers.items[OPD_VALUE(operands[0])] = X86_64_OPERAND_REGISTER_rax;
            }

			if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
				/* result and first operand can be the same */
				b->registers.items[OPD_VALUE(operands[1])] = b->registers.items[OPD_VALUE(operands[0])];
			} else assert(false);

			if (OPD_TYPE(operands[2]) == LL_IR_OPERAND_REGISTER_BIT) {
				if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
					/* result and second operand can be the same, if first operand isn't register */
					b->registers.items[OPD_VALUE(operands[2])] = linux_x64_64_register_queue_dequeue(cc, &b->register_queue);
				} else {
					b->registers.items[OPD_VALUE(operands[2])] = b->registers.items[OPD_VALUE(operands[0])];
				}
			}

			linux_x64_64_register_queue_enqueue(cc, &b->register_queue, b->registers.items[OPD_VALUE(operands[0])]);
            break;
        }
		case LL_IR_OPCODE_INVOKEVALUE:
            offset = 1;
		case LL_IR_OPCODE_INVOKE: {
			uint32_t count = operands[1 + offset];
            Linux_x86_64_Elf_Call_Convention callconv = linux_x64_64_call_convention_systemv(b);

            for (uint32_t pi = 0; pi < count; ++pi) {
                X86_64_Operand_Register reg = linux_x64_64_call_convention_next_reg(b, &callconv);

				if (reg == X86_64_OPERAND_REGISTER_invalid) {
					// we must use memory
					b->registers.items[OPD_VALUE(operands[2 + pi + offset])] = linux_x64_64_register_queue_dequeue(cc, &b->register_queue);
				} else {
					if (linux_x64_64_register_queue_is_free(cc, &b->register_queue, reg)) {
						linux_x64_64_register_queue_use_reg(cc, &b->register_queue, reg);

						switch (OPD_TYPE(operands[2 + pi + offset])) {
							case LL_IR_OPERAND_REGISTER_BIT: 
								b->registers.items[OPD_VALUE(operands[2 + pi + offset])] = reg;
								break;
							case LL_IR_OPERAND_IMMEDIATE_BIT:
								break;
							default: printf("TODO: unhadnled argument operand\n");
						}
					} else {
						assert(false);
					}
				}
            }

			break;
		}
		}

		size_t count = ir_get_op_count(cc, bir, block->rops.items, i);
		i += count;
	}
}

typedef struct {
	bool immediate, mem_right, single;
} X86_64_Get_Variant_Params;

static X86_64_Variant_Kind linux_x86_64_get_variant(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Type* type, X86_64_Get_Variant_Params params) {
	assert(!params.immediate || !params.mem_right);
	switch (type->kind) {
	case LL_TYPE_STRING:
	case LL_TYPE_POINTER:
		if (params.immediate) return X86_64_VARIANT_KIND_rm64_i32;
		else if (params.mem_right) return X86_64_VARIANT_KIND_r64_rm64;
		else return X86_64_VARIANT_KIND_rm64_r64;
	case LL_TYPE_UINT:
	case LL_TYPE_INT:
		if (type->width <= 8u) {
			if (params.immediate) return X86_64_VARIANT_KIND_rm8_i8;
			else if (params.mem_right) return X86_64_VARIANT_KIND_r8_rm8;
			else if (params.single) return X86_64_VARIANT_KIND_rm8;
			else return X86_64_VARIANT_KIND_rm8_r8;
		} else if (type->width <= 16u) {
			if (params.immediate) return X86_64_VARIANT_KIND_rm16_i16;
			else if (params.mem_right) return X86_64_VARIANT_KIND_r16_rm16;
			else if (params.single) return X86_64_VARIANT_KIND_rm16;
			else return X86_64_VARIANT_KIND_rm16_r16;
		} else if (type->width <= 32u) {
			if (params.immediate) return X86_64_VARIANT_KIND_rm32_i32;
			else if (params.mem_right) return X86_64_VARIANT_KIND_r32_rm32;
			else if (params.single) return X86_64_VARIANT_KIND_rm32;
			else return X86_64_VARIANT_KIND_rm32_r32;
		} else if (type->width <= 64u) {
			if (params.immediate) return X86_64_VARIANT_KIND_rm64_i32;
			else if (params.mem_right) return X86_64_VARIANT_KIND_r64_rm64;
			else if (params.single) return X86_64_VARIANT_KIND_rm64;
			else return X86_64_VARIANT_KIND_rm64_r64;
		}
	default: break;
	}
	abort();
	return (X86_64_Variant_Kind)-1;
}

static void linux_x86_64_elf_generate_mov(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand result, LL_Ir_Operand src) {
	X86_64_Instruction_Parameters params = { 0 };
	LL_Type* type = ir_get_operand_type(b->fn, result);
	switch (OPD_TYPE(result)) {
	case LL_IR_OPERAND_LOCAL_BIT: {
		params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
		params.displacement = -b->locals.items[OPD_VALUE(result)];

		switch (type->kind) {
		case LL_TYPE_POINTER:
		case LL_TYPE_STRING:
		case LL_TYPE_UINT:
		case LL_TYPE_INT: {
			/* TODO: max immeidiate is 28 bits */
			switch (OPD_TYPE(src)) {
			case LL_IR_OPERAND_IMMEDIATE_BIT:
				params.immediate = OPD_VALUE(src);
				X86_64_WRITE_INSTRUCTION_DYN(OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
				break;
			case LL_IR_OPERAND_REGISTER_BIT:
				params.reg1 = b->registers.items[OPD_VALUE(src)];
				X86_64_WRITE_INSTRUCTION_DYN(OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) {}), params);
				break;
			case LL_IR_OPERAND_DATA_BIT: {
				/* arena_da_append(&cc->tmp_arena, &b->internal_relocations, ((Linux_x86_64_Internal_Relocation) {  })); */
				/* X86_64_WRITE_INSTRUCTION(OPCODE_LEA, r64_rm64, params); */
				/* X86_64_WRITE_INSTRUCTION(OPCODE_MOV, , params); */
				break;
			}
			case LL_IR_OPERAND_LOCAL_BIT:
				break;
			case LL_IR_OPERAND_PARMAETER_BIT: {
				break;
			}
			default: assert(false);
			}
			break;
		}
		default: assert(false);
		}

		break;
	}
	case LL_IR_OPERAND_REGISTER_BIT: {
		params.reg0 = b->registers.items[OPD_VALUE(result)];

		switch (type->kind) {
		case LL_TYPE_STRING:
		case LL_TYPE_ANYINT:
		case LL_TYPE_UINT:
		case LL_TYPE_INT: {
			/* TODO: max immeidiate is 28 bits */
			switch (OPD_TYPE(src)) {
			case LL_IR_OPERAND_IMMEDIATE_BIT:
				params.immediate = OPD_VALUE(src);

				X86_64_WRITE_INSTRUCTION_DYN(OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
				break;
			case LL_IR_OPERAND_REGISTER_BIT:
				params.reg1 = b->registers.items[OPD_VALUE(src)];
				X86_64_WRITE_INSTRUCTION_DYN(OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) {}), params);
				break;
			case LL_IR_OPERAND_LOCAL_BIT:
				break;
			case LL_IR_OPERAND_PARMAETER_BIT: {
				break;
			}
			default: assert(false);
			}
			break;
		}
		default: assert(false);
		}
		break;
	}
	}
}

static void linux_x86_64_elf_generate_load_cast(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand result, LL_Ir_Operand src, bool load) {
    X86_64_Instruction_Parameters params;
    uint32_t opcode;
    X86_64_Variant_Kind kind;
    LL_Type* from_type;
    LL_Type* to_type = ir_get_operand_type(b->fn, result);

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_LOCAL_BIT: {
        from_type = b->fn->locals.items[OPD_VALUE(src)].ident->base.type;
        break;
    }
    case LL_IR_OPERAND_REGISTER_BIT: {
        from_type = ir_get_operand_type(b->fn, src);
        break;
    }
    case LL_IR_OPERAND_IMMEDIATE_BIT: {
        from_type = to_type;
        break;
    }
    default: printf("todo: add load operands\n"); break;
    }

    params.reg0 = b->registers.items[OPD_VALUE(result)];

    if (to_type != from_type) {
        switch (from_type->kind) {
        case LL_TYPE_UINT:
            switch (to_type->kind) {
                case LL_TYPE_INT:
                case LL_TYPE_UINT:
                    if (from_type->width < to_type->width) {
                        if (from_type->width <= 8) {
                            opcode = OPCODE_MOVZX;
                            if (to_type->width <= 16) {
                                kind = X86_64_VARIANT_KIND_r16_rm8;
                                break;
                            } else if (to_type->width <= 32) {
                                kind = X86_64_VARIANT_KIND_r32_rm8;
                                break;
                            } else if (to_type->width <= 64) {
                                kind = X86_64_VARIANT_KIND_r64_rm8;
                                break;
                            }
                        } else if (from_type->width <= 16) {
                            opcode = OPCODE_MOVZX;
                            if (to_type->width <= 32) {
                                kind = X86_64_VARIANT_KIND_r32_rm16;
                                break;
                            } else if (to_type->width <= 64) {
                                kind = X86_64_VARIANT_KIND_r64_rm16;
                                break;
                            }
                        }
                    }
                    opcode = OPCODE_MOV;
                    kind = linux_x86_64_get_variant(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .mem_right = true });
                    break;
                default: assert(false);
            }
            break;
        case LL_TYPE_INT:
            switch (to_type->kind) {
            case LL_TYPE_UINT:
            case LL_TYPE_INT:
                if (from_type->width < to_type->width) {
                    if (from_type->width <= 8) {
                        opcode = OPCODE_MOVSX;
                        if (to_type->width <= 16) {
                            kind = X86_64_VARIANT_KIND_r16_rm8;
                            break;
                        } else if (to_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm8;
                            break;
                        } else if (to_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm8;
                            break;
                        }
                    } else if (from_type->width <= 16) {
                        opcode = OPCODE_MOVSX;
                        if (to_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm16;
                            break;
                        } else if (to_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm16;
                            break;
                        }
                    } else if (from_type->width <= 32) {
                        opcode = OPCODE_MOVSXD;
                        if (to_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm32;
                            break;
                        }
                    }
                }
                opcode = OPCODE_MOV;
                kind = linux_x86_64_get_variant(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .mem_right = true });
                break;
                default: assert(false);
            }

            break;
        default: assert(false);
        }
    } else {
        opcode = OPCODE_MOV;
        kind = linux_x86_64_get_variant(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .mem_right = true });
    }

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_LOCAL_BIT: {
        params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
        params.displacement = -b->locals.items[OPD_VALUE(src)];
        X86_64_WRITE_INSTRUCTION_DYN(opcode, kind, params);
        break;
    }
    case LL_IR_OPERAND_REGISTER_BIT: {
        if (load) {
            params.reg1 = b->registers.items[OPD_VALUE(src)] | X86_64_REG_BASE;
        } else {
            params.reg1 = b->registers.items[OPD_VALUE(src)];
        }
        X86_64_WRITE_INSTRUCTION_DYN(opcode, kind, params);
        break;
    }
    case LL_IR_OPERAND_IMMEDIATE_BIT: {
        params.immediate = OPD_VALUE(src);
        X86_64_WRITE_INSTRUCTION_DYN(opcode, linux_x86_64_get_variant(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
        break;
    }
    default: printf("todo: add load operands\n"); break;
    }
}

#define LINUX_X86_64_REGISTERS_EQL(_a, _b) (b->registers.items[OPD_VALUE(_a)] == b->registers.items[OPD_VALUE(_b)])

static void linux_x86_64_elf_generate_block(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
	size_t i;
    int32_t opcode1, opcode2;

	linux_x64_64_register_queue_reset(cc, &b->register_queue);
	x86_64_allocate_physical_registers(cc, b, bir, block);
	block->generated_offset = (int64_t)b->section_text.count;

	if (block->ref1) {
		printf("fixup block ref -> %u\n", block->ref1);
		int32_t* dst_offset = (int32_t*)&b->section_text.items[bir->blocks.items[block->ref1].fixup_offset];
		*dst_offset = (int32_t)(block->generated_offset - bir->blocks.items[block->ref1].fixup_offset - 4);
	}

	for (i = 0; i < block->ops.count; ) {
		LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->ops.items[i];
		LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->ops.items[i + 1];
		X86_64_Instruction_Parameters params = { 0 };

		switch (opcode) {
		case LL_IR_OPCODE_BRANCH:
			if (bir->blocks.items[operands[0]].generated_offset != -1) {
				params.relative = bir->blocks.items[operands[0]].generated_offset - (int64_t)b->section_text.count;
				if (params.relative >= INT8_MIN && params.relative <= INT8_MAX) {
					params.relative -= 2;
					X86_64_WRITE_INSTRUCTION(OPCODE_JMP, rel8, params);
				} else {
					params.relative -= 5;
					X86_64_WRITE_INSTRUCTION(OPCODE_JMP, rel32, params);
				}
			} else {
				params.relative = 0;
				X86_64_WRITE_INSTRUCTION(OPCODE_JMP, rel32, params);
				block->fixup_offset = (int64_t)b->section_text.count - 4u;
			}
			break;
		case LL_IR_OPCODE_BRANCH_COND:
			params.relative = 0;
			if (operands[1] == block->next) {
				// then block is next

				X86_64_WRITE_INSTRUCTION(x86_64_get_inverse_compare(b->registers.items[OPD_VALUE(operands[0])]), rel32, params);
				bir->blocks.items[operands[1]].ref1 = 0;
				bir->blocks.items[operands[2]].ref1 = bir->blocks.items[operands[1]].prev;
				printf("block ref -> %u\n", bir->blocks.items[operands[2]].ref1);
			} else if (operands[2] == block->next) {
				// else block is next
				X86_64_WRITE_INSTRUCTION(b->registers.items[OPD_VALUE(operands[0])], rel32, params);
				bir->blocks.items[operands[2]].ref1 = 0;
				bir->blocks.items[operands[1]].ref1 = bir->blocks.items[operands[2]].prev;
			} else assert(false);
			block->fixup_offset = (int64_t)b->section_text.count - 4u;
			break;
		case LL_IR_OPCODE_RET:
			params.relative = 0;
			X86_64_WRITE_INSTRUCTION(OPCODE_JMP, rel32, params);
		   	break;
		case LL_IR_OPCODE_RETVALUE: {
			LL_Type* type = ir_get_operand_type(b->fn, operands[0]);
			switch (OPD_TYPE(operands[0])) {
			case LL_IR_OPERAND_IMMEDIATE_BIT:
				params.reg0 = X86_64_OPERAND_REGISTER_rax;
				params.immediate = OPD_VALUE(operands[0]);
				X86_64_WRITE_INSTRUCTION_DYN(OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
				break;
			case LL_IR_OPERAND_REGISTER_BIT:
				if (b->registers.items[OPD_VALUE(operands[0])] != X86_64_OPERAND_REGISTER_rax) {
					params.reg0 = X86_64_OPERAND_REGISTER_rax;
					params.reg1 = b->registers.items[OPD_VALUE(operands[0])];
					X86_64_WRITE_INSTRUCTION_DYN(OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) {}), params);
				}
				break;
			default: printf("TODO: handle return value\n"); break;
			}
			params.relative = 0;
			X86_64_WRITE_INSTRUCTION(OPCODE_JMP, rel32, params);
			break;
		}
		case LL_IR_OPCODE_STORE: {
			linux_x86_64_elf_generate_mov(cc, b, bir, operands[0], operands[1]);
		   	break;
		}
		case LL_IR_OPCODE_EQ:
			b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JE;
			goto DO_OPCODE_COMPARE;
		case LL_IR_OPCODE_NEQ:
			b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JNE;
			goto DO_OPCODE_COMPARE;
		case LL_IR_OPCODE_GTE:
			b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JGE;
			goto DO_OPCODE_COMPARE;
		case LL_IR_OPCODE_GT:
			b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JG;
			goto DO_OPCODE_COMPARE;
		case LL_IR_OPCODE_LTE:
			b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JLE;
			goto DO_OPCODE_COMPARE;
		case LL_IR_OPCODE_LT: {
			b->registers.items[OPD_VALUE(operands[0])] = OPCODE_JL;
			LL_Type* type;
DO_OPCODE_COMPARE:
			type = ir_get_operand_type(b->fn, operands[0]);
			switch (type->kind) {
			case LL_TYPE_ANYINT:
			case LL_TYPE_INT: {
				/* TODO: max immeidiate is 28 bits */
				switch (OPD_TYPE(operands[1])) {
				case LL_IR_OPERAND_REGISTER_BIT: {
					switch (OPD_TYPE(operands[2])) {
					case LL_IR_OPERAND_IMMEDIATE_BIT:
						params.reg0 = b->registers.items[OPD_VALUE(operands[1])];
						params.immediate = OPD_VALUE(operands[2]);
						X86_64_WRITE_INSTRUCTION_DYN(OPCODE_CMP, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
						break;
					case LL_IR_OPERAND_REGISTER_BIT:
						params.reg0 = b->registers.items[OPD_VALUE(operands[1])];
						params.reg1 = b->registers.items[OPD_VALUE(operands[2])];
						X86_64_WRITE_INSTRUCTION_DYN(OPCODE_CMP, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) {}), params);
						break;
					default: printf("TODO: handle add other register rhs"); break;
					}
					break;
				}
				case LL_IR_OPERAND_IMMEDIATE_BIT: {
					assert(false);
				}
				}
				break;
			}
			default: printf("TODO: handoe other type\n"); assert(false);
			}
			break;
		}
        case LL_IR_OPCODE_SUB:
            opcode1 = OPCODE_SUB;
            opcode2 = OPCODE_DEC;
            goto DO_OPCODE_ARITHMETIC;
		case LL_IR_OPCODE_ADD: {
            LL_Type* type;
            opcode1 = OPCODE_ADD;
            opcode2 = OPCODE_INC;
DO_OPCODE_ARITHMETIC:
			type = ir_get_operand_type(b->fn, operands[0]);
			switch (type->kind) {
			case LL_TYPE_ANYINT:
			case LL_TYPE_INT: {
				/* TODO: max immeidiate is 28 bits */
				switch (OPD_TYPE(operands[1])) {
				case LL_IR_OPERAND_REGISTER_BIT: {
					switch (OPD_TYPE(operands[2])) {
					case LL_IR_OPERAND_IMMEDIATE_BIT:
						if (LINUX_X86_64_REGISTERS_EQL(operands[1], operands[0])) {
							params.reg0 = b->registers.items[OPD_VALUE(operands[0])];
							if (OPD_VALUE(operands[2]) == 1) {
								X86_64_WRITE_INSTRUCTION_DYN(opcode2, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params);
							} else {
								params.immediate = OPD_VALUE(operands[2]);
								X86_64_WRITE_INSTRUCTION_DYN(opcode1, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
							}
						}
						break;
					case LL_IR_OPERAND_REGISTER_BIT:
						if (LINUX_X86_64_REGISTERS_EQL(operands[1], operands[0])) {
							params.reg0 = b->registers.items[OPD_VALUE(operands[0])];
							params.reg1 = b->registers.items[OPD_VALUE(operands[2])];
							X86_64_WRITE_INSTRUCTION_DYN(opcode1, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) {}), params);
						}
						break;
					default: printf("TODO: handle add other register rhs"); break;
					}
					break;
				}
				case LL_IR_OPERAND_IMMEDIATE_BIT: {
					switch (OPD_TYPE(operands[2])) {
					case LL_IR_OPERAND_IMMEDIATE_BIT:
						linux_x86_64_elf_generate_mov(cc, b, bir, operands[0], operands[1]);
						params.reg0 = b->registers.items[OPD_VALUE(operands[0])];
						params.immediate = OPD_VALUE(operands[2]);
						X86_64_WRITE_INSTRUCTION_DYN(opcode1, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
						break;
					case LL_IR_OPERAND_REGISTER_BIT:
						if (OPD_VALUE(operands[2]) == 1) {
							params.reg0 = b->registers.items[OPD_VALUE(operands[0])];
							X86_64_WRITE_INSTRUCTION_DYN(opcode2, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params);
						} else {
							params.immediate = OPD_VALUE(operands[2]);
							if (LINUX_X86_64_REGISTERS_EQL(operands[1], operands[0])) {
								params.reg0 = b->registers.items[OPD_VALUE(operands[0])];
								X86_64_WRITE_INSTRUCTION_DYN(opcode1, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
							}
						}
						break;
					}
					break;
				}
				}
				break;
			}
			default: printf("TODO: handoe other type\n"); assert(false);
			}
			break;
		}
        case LL_IR_OPCODE_MUL: {
			LL_Type* type = ir_get_operand_type(b->fn, operands[0]);
            params.reg0 = b->registers.items[OPD_VALUE(operands[2])];

            switch (type->kind) {
            case LL_TYPE_INT:
                X86_64_WRITE_INSTRUCTION_DYN(OPCODE_IMUL, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params);
                break;
            case LL_TYPE_UINT:
                X86_64_WRITE_INSTRUCTION_DYN(OPCODE_MUL, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params);
                break;
            }

            break;
        }
        case LL_IR_OPCODE_DIV: {
			LL_Type* type = ir_get_operand_type(b->fn, operands[0]);
            params.reg0 = b->registers.items[OPD_VALUE(operands[2])];

            switch (type->kind) {
            case LL_TYPE_INT:
                X86_64_WRITE_INSTRUCTION_DYN(OPCODE_IDIV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params);
                break;
            case LL_TYPE_UINT:
                X86_64_WRITE_INSTRUCTION_DYN(OPCODE_DIV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params);
                break;
            }

            break;
        }
		case LL_IR_OPCODE_CAST: {
            linux_x86_64_elf_generate_load_cast(cc, b, bir, operands[0], operands[1], false);
            break;
		}
		case LL_IR_OPCODE_LOAD: {
            linux_x86_64_elf_generate_load_cast(cc, b, bir, operands[0], operands[1], true);
            break;
		}
		case LL_IR_OPCODE_LEA: {
			LL_Type* type = ir_get_operand_type(b->fn, operands[0]);
			switch (type->kind) {
			case LL_TYPE_STRING:
			case LL_TYPE_ANYINT:
			case LL_TYPE_INT: {
				/* TODO: max immeidiate is 28 bits */
				params.reg0 = b->registers.items[OPD_VALUE(operands[0])];
				switch (OPD_TYPE(operands[1])) {
				case LL_IR_OPERAND_LOCAL_BIT: {
					params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
					params.displacement = -b->locals.items[OPD_VALUE(operands[1])];
					X86_64_WRITE_INSTRUCTION(OPCODE_LEA, r64_rm64, params);
					break;
				}
				case LL_IR_OPERAND_DATA_BIT: {
					params.reg0 = b->registers.items[OPD_VALUE(operands[0])];
					params.immediate = 0;
					X86_64_WRITE_INSTRUCTION(OPCODE_MOV, r64_i64, params);
					/* arena_da_append(&cc->tmp_arena, &b->internal_relocations, ((Linux_x86_64_Internal_Relocation) { .data_item = OPD_VALUE(operands[1]), .text_rel_byte_offset = b->current_section->count - 4 /1* sizeof displacement *1/ })); */
					arena_da_append(&cc->tmp_arena, &b->relocations, ((Elf64_Rela) { .r_offset = b->current_section->count - 8, .r_info = ELF64_R_INFO(3, R_X86_64_64), .r_addend = bir->data_items.items[OPD_VALUE(operands[1])].binary_offset }));
					break;
				}
				default: printf("todo: add lea operands\n"); break;
				}
				break;
			}
			default: printf("todo: add lea types\n"); break;
			}

			break;
		}
		case LL_IR_OPCODE_INVOKE: {
			uint8_t reg;
			uint32_t invokee = operands[0];
			uint32_t count = operands[1];
            Linux_x86_64_Elf_Call_Convention callconv = linux_x64_64_call_convention_systemv(b);

			for (uint32_t j = 0; j < count; ++j) {
                switch (OPD_TYPE(operands[2 + j])) {
                    case LL_IR_OPERAND_REGISTER_BIT: 
                        reg = linux_x64_64_call_convention_next_reg(b, &callconv);

						if (reg == X86_64_OPERAND_REGISTER_invalid) {
							int32_t offset = linux_x64_64_call_convention_next_mem(b, &callconv);
							params.reg0 = X86_64_OPERAND_REGISTER_rsp | X86_64_REG_BASE;
							params.displacement = offset;
							params.reg1 = OPD_VALUE(operands[2 + j]);
							X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm64_r64, params);
						}
                        break;
                    case LL_IR_OPERAND_IMMEDIATE_BIT:
						reg = linux_x64_64_call_convention_next_reg(b, &callconv);
						if (reg == X86_64_OPERAND_REGISTER_invalid) {
							int32_t offset = -linux_x64_64_call_convention_next_mem(b, &callconv);
							params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
							params.displacement = offset;
							params.immediate = OPD_VALUE(operands[2 + j]);
							X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm64_i32, params);
						} else {
							params.reg0 = reg;
							params.immediate = OPD_VALUE(operands[2 + j]);
							X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm64_i32, params);
						}
                        break;
                    default: printf("TODO: unhadnled argument operand\n");
                }
			}

			switch (OPD_TYPE(invokee)) {
			case LL_IR_OPERAND_FUNCTION_BIT: {
				LL_Ir_Function* fn = &bir->fns.items[OPD_VALUE(invokee)];
				Ast_Ident* ident = fn->ident;

				if (fn->flags & LL_IR_FUNCTION_FLAG_EXTERN || fn->generated_offset == LL_IR_FUNCTION_OFFSET_INVALID) {
					params.relative = 0;

					X86_64_WRITE_INSTRUCTION(OPCODE_CALL, rel32, params);

					if (fn->flags & LL_IR_FUNCTION_FLAG_EXTERN) {
						arena_da_append(
							&cc->tmp_arena,
							&b->relocations,
							((Elf64_Rela) { .r_offset = b->current_section->count - 4, .r_info = ELF64_R_INFO(b->symbols.count, R_X86_64_PLT32), .r_addend = -4 })
						);
					} else {
						arena_da_append(
							&cc->tmp_arena,
							&b->internal_relocations,
							((Linux_x86_64_Internal_Relocation) { b->current_section->count - 4, OPD_VALUE(invokee) })
						);
						ident->symbol_index = (int32_t)b->symbols.count;
					}

					Elf64_Sym sym;
					sym.st_name = b->section_strtab.count;
					arena_da_append_many(&cc->arena, &b->section_strtab, ident->str.ptr, ident->str.len);
					arena_da_append(&cc->arena, &b->section_strtab, 0);

					sym.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE);
					sym.st_other = STV_DEFAULT;
					sym.st_shndx = 0;
					sym.st_value = 0;
					sym.st_size = 0;
					arena_da_append(&cc->arena, &b->symbols, sym);
				} else {
					params.relative = (int32_t)(fn->generated_offset - (int64_t)b->current_section->count);
					X86_64_WRITE_INSTRUCTION(OPCODE_CALL, rel32, params);
				}
				break;
			}
			default: printf("handle inveok type"); break;
			}

		   	break;
		}
		/* case LL_IR_OPCODE_INVOKE: { */
		/* 	uint32_t count = operands[2]; */
		/* 	for (uint32_t j = 0; j < count; ++j) { */
		/* 	} */

		/*    	break; */
		/* } */
		default: printf("handle other op\n"); break;
		}

		size_t count = ir_get_op_count(cc, bir, block->ops.items, i);
		i += count;
	}
}

void linux_x86_64_elf_generate(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir) {
	Elf64_Sym sym, *psym;
	int fi;

	for (fi = 0; fi < bir->data_items.count; ++fi) {
		bir->data_items.items[fi].binary_offset = b->section_data.count;
		arena_da_append_many(&cc->arena, &b->section_data, bir->data_items.items[fi].ptr, bir->data_items.items[fi].len);
		arena_da_append(&cc->arena, &b->section_data, 0);
	}

	for (fi = 0; fi < bir->fns.count; ++fi) {
		LL_Ir_Function* fn = &bir->fns.items[fi];
		LL_Ir_Block_Ref block = fn->entry;
		if (fn->flags & LL_IR_FUNCTION_FLAG_EXTERN) continue;
		b->fn = fn;
		b->next_reg = 0;

		arena_da_reserve(&cc->tmp_arena, &b->locals, fn->locals.count);
		arena_da_reserve(&cc->tmp_arena, &b->registers, fn->registers.count);

		uint64_t offset = 0;
		for (int li = 0; li < fn->locals.count; ++li) {
			Linux_x86_64_Elf_Layout l = linux_x86_64_elf_get_layout(fn->locals.items[li].ident->base.type);
			offset = align_forward(offset + l.size, l.alignment);
			b->locals.items[li] = offset;
		}
		b->stack_used = (uint32_t)offset;

		/* printf("function " FMT_SV_FMT ":\n", FMT_SV_ARG(fn->ident->str)); */

		size_t function_offset = b->current_section->count;
		X86_64_WRITE_INSTRUCTION(OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp }));
		X86_64_WRITE_INSTRUCTION(OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rsp }));
		X86_64_WRITE_INSTRUCTION(OPCODE_SUB, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = 0 }));
		size_t stack_size_offset = b->current_section->count - 4;
		fn->generated_offset = (int64_t)function_offset;

		// symbol could be valid in the case that it's used before defined
		if (fn->ident->symbol_index == AST_IDENT_SYMBOL_INVALID) {
			fn->ident->symbol_index = (int32_t)b->symbols.count;
			arena_da_append(&cc->arena, &b->symbols, sym);
		}

		int bi = 0;
		while (block) {
			linux_x86_64_elf_generate_block(cc, b, bir, &bir->blocks.items[block]);
			bi++;
			block = bir->blocks.items[block].next;
		}

		/* for (bi = 0; bi < b->registers.count; ++bi) { */
		/* 	printf("reg r%d -> %u\n", bi, b->registers.items[bi]); */
		/* } */

		uint32_t stack_used = align_forward(b->stack_used, 16);
		int32_t* pstack_size = (int32_t*)&b->current_section->items[stack_size_offset];
		*pstack_size = stack_used;
		X86_64_WRITE_INSTRUCTION(OPCODE_ADD, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = stack_used }));
		X86_64_WRITE_INSTRUCTION(OPCODE_POP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp }));
		X86_64_WRITE_INSTRUCTION(OPCODE_RET, noarg, ((X86_64_Instruction_Parameters){ }));

		psym = &b->symbols.items[fn->ident->symbol_index]; // must lookup again in case of realloc
														   
		psym->st_name = b->section_strtab.count;
		arena_da_append_many(&cc->arena, &b->section_strtab, fn->ident->str.ptr, fn->ident->str.len);
		arena_da_append(&cc->arena, &b->section_strtab, 0);

		psym->st_info = ELF32_ST_INFO(STB_GLOBAL, STT_FUNC);
		psym->st_other = STV_DEFAULT;
		psym->st_shndx = b->current_section->index;
		psym->st_value = function_offset;
		psym->st_size = b->current_section->count - function_offset;
	}

	/* ---- process internal relocations ---- */

	for (fi = 0; fi < b->internal_relocations.count; ++fi) {
		Linux_x86_64_Internal_Relocation relocation = b->internal_relocations.items[fi];

		LL_Ir_Function* fn = &bir->fns.items[relocation.data_item];
		if (fn->generated_offset != LL_IR_FUNCTION_OFFSET_INVALID) {
			int64_t rel_offset = fn->generated_offset - (int64_t)relocation.text_rel_byte_offset - 4;
			int32_t* dst_offset = (int32_t*)&b->section_text.items[relocation.text_rel_byte_offset];
			*dst_offset = (int32_t)rel_offset;
		}
	}

	/* for (fi = 0; fi < b->relocations.count; ++fi) { */
	/* 	Elf64_Rela relocation = b->relocations.items[fi]; */
	/* 	if (fi + 1 < b->relocations.count) { */
	/* 		b->relocations.items[fi] = b->relocations.items[fi + 1]; */
	/* 	} */
	/* 	uint32_t* dst_offset = (uint32_t*)&b->section_text.items[relocation.r_address]; */
	/* 	*dst_offset = rel_offset; */
	/* } */

	/* x86_64_run_tests(cc, b); */
}

#undef OPD_VALUE
#undef OPD_TYPE

