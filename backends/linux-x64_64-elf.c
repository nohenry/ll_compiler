
// #include <stdio.h>
// #include <stdlib.h>
#include <limits.h>

#include "../src/elf.h"
#include "../src/backend.h"
#include "../src/common.h"
#include "../src/ast.h"
#include "../src/typer.h"

#include "../core/machine_code.h"

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
    string name;
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

typedef enum {
    COLLAPSE_KIND_MEM = (1u << 0u),
    COLLAPSE_KIND_REG = (1u << 1u),
    COLLAPSE_KIND_IMM = (1u << 2u),
} Linux_x86_64_Elf_Collapse_Kind;

typedef struct {
    uint8_t op_collapse;
    Linux_x86_64_Elf_Collapse_Kind op_did_collapse;
    int32_t op_parameter;
} Linux_x86_64_Elf_Collapse;

typedef struct {
    uint32_t count, capacity;
    Linux_x86_64_Elf_Collapse* items;
} Linux_x86_64_Elf_Collapse_List;

typedef struct {
    OC_Machine_Code_Writer w;
    Linux_x86_64_Elf_Ops_List ops;

    LL_Ir_Function* fn;
    Linux_x86_64_Elf_Local_List locals;
    Linux_x86_64_Elf_Local_List registers;
    /* Linux_x86_64_Elf_Local_List register_collapses; */
    Linux_x86_64_Elf_Collapse_List register_collapses;

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

    Code_Ident memcpy_ident;
} Linux_x86_64_Elf_Backend;

typedef struct {
    bool is_reg;
    union {
        X86_64_Operand_Register reg;
        int32_t stack_offset;
    };
} Linux_Call_Convention_Parameter;

typedef struct {
    const X86_64_Operand_Register* registers;
    uint8_t register_count, register_next;
    int32_t stack_offset;
} Linux_x86_64_Elf_Call_Convention;

const static X86_64_Operand_Register call_convention_registers_systemv[] = {
    X86_64_OPERAND_REGISTER_rdi,
    X86_64_OPERAND_REGISTER_rsi,
    X86_64_OPERAND_REGISTER_rdx,
    X86_64_OPERAND_REGISTER_rcx,
    X86_64_OPERAND_REGISTER_r8,
    X86_64_OPERAND_REGISTER_r9,
};

Linux_x86_64_Elf_Call_Convention linux_x86_64_call_convention_systemv(Linux_x86_64_Elf_Backend* b) {
    Linux_x86_64_Elf_Call_Convention result;
    result.registers = call_convention_registers_systemv;
    result.register_count = oc_len(call_convention_registers_systemv);
    result.register_next = 0;
    result.stack_offset = 0;
    return result;
}

X86_64_Operand_Register linux_x86_64_call_convention_next_reg(Linux_x86_64_Elf_Backend* b, Linux_x86_64_Elf_Call_Convention* cc) {
    if (cc->register_next >= cc->register_count) {
        return X86_64_OPERAND_REGISTER_invalid;
    } else {
        return cc->registers[cc->register_next++];
    }
}

uint32_t linux_x86_64_call_convention_next_mem(Linux_x86_64_Elf_Backend* b, Linux_x86_64_Elf_Call_Convention* cc) {
    uint32_t stack_offset = cc->stack_offset;
    cc->stack_offset += 8;
    b->stack_used += 8;
    return stack_offset;
}

Linux_Call_Convention_Parameter linux_x86_64_call_convention_next(Linux_x86_64_Elf_Backend* b, Linux_x86_64_Elf_Call_Convention* cc) {
    Linux_Call_Convention_Parameter result;
    result.reg = linux_x86_64_call_convention_next_reg(b, cc);
    if (result.reg != X86_64_OPERAND_REGISTER_invalid) {
        result.is_reg = true;
    } else {
        result.is_reg = false;
        result.stack_offset = linux_x86_64_call_convention_next_mem (b, cc);
    }
    return result;
}

Linux_Call_Convention_Parameter linux_x86_64_call_convention_nth_parameter(Linux_x86_64_Elf_Backend* b, Linux_x86_64_Elf_Call_Convention* cc, uint32_t n) {
    while (n > 0) {
        linux_x86_64_call_convention_next(b, cc);
        n--;
    }
    return linux_x86_64_call_convention_next(b, cc);
}

void linux_x86_64_register_queue_reset(Compiler_Context* cc, Linux_x86_64_Register_Queue* queue) {
    queue->starti = 0;
    queue->count = 0;
    queue->free = 0xFFCFu;

    for (size_t i = 0; i < x86_64_usable_gp_registers_count; ++i) {
        oc_array_append(&cc->arena, queue, x86_64_usable_gp_registers[i]);
    }
}

void linux_x86_64_register_queue_enqueue(Compiler_Context* cc, Linux_x86_64_Register_Queue* queue, uint8_t reg) {
    if (queue->starti >= queue->count) {
        queue->starti = 0;
        queue->count = 0;
    }
    queue->free |= (uint32_t)(1u << reg);
    if (queue->starti > 0) {
        queue->items[--queue->starti] = reg;
    } else {
        oc_array_append(&cc->arena, queue, reg);
    }
}

uint8_t linux_x86_64_register_queue_dequeue(Compiler_Context* cc, Linux_x86_64_Register_Queue* queue) {
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


static inline bool linux_x86_64_register_queue_is_free(Compiler_Context* cc, Linux_x86_64_Register_Queue* queue, uint8_t reg) {
    return queue->free & (uint32_t)(1u << reg);
}

static inline void linux_x86_64_register_queue_use_reg(Compiler_Context* cc, Linux_x86_64_Register_Queue* queue, uint8_t reg) {
    queue->free |= (uint32_t)(1u << reg);
}

void linux_x86_64_elf_append_op_segment_u8(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint8_t segment) {
    segment = AS_LITTLE_ENDIAN_U8(segment);
    oc_array_append(&cc->arena, b->current_section, segment);
}

void linux_x86_64_elf_append_op_segment_u16(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint16_t segment) {
    segment = AS_LITTLE_ENDIAN_U16(segment);
    oc_array_append_many(&cc->arena, b->current_section, (uint8_t*)&segment, 2);
}

void linux_x86_64_elf_append_op_segment_u32(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint32_t segment) {
    segment = AS_LITTLE_ENDIAN_U32(segment);
    oc_array_append_many(&cc->arena, b->current_section, (uint8_t*)&segment, 4);
}

void linux_x86_64_elf_append_op_segment_u64(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint64_t segment) {
    segment = AS_LITTLE_ENDIAN_U64(segment);
    oc_array_append_many(&cc->arena, b->current_section, (uint8_t*)&segment, 8);
}

void linux_x86_64_elf_append_op_many(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, uint8_t* bytes, uint64_t count) {
    oc_array_append_many(&cc->arena, b->current_section, (uint8_t*)bytes, count);
}

LL_Backend_Layout linux_x86_64_elf_get_layout(LL_Type* ty) {
    LL_Backend_Layout sub_layout;
    switch (ty->kind) {
    case LL_TYPE_INT: return (LL_Backend_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
    case LL_TYPE_UINT: return (LL_Backend_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
    case LL_TYPE_FLOAT: return (LL_Backend_Layout) { .size = ty->width / 8, .alignment = ty->width / 8 };
    case LL_TYPE_POINTER: return (LL_Backend_Layout) { .size = 8, .alignment = 8 };
    case LL_TYPE_STRING: return (LL_Backend_Layout) { .size = 8, .alignment = 8 };
    case LL_TYPE_ARRAY: {
        sub_layout = linux_x86_64_elf_get_layout(((LL_Type_Array*)ty)->element_type);
        return (LL_Backend_Layout) { .size = max(sub_layout.size, sub_layout.alignment) * ty->width, .alignment = sub_layout.alignment };
    }
    default: return (LL_Backend_Layout) { .size = 0, .alignment = 1 };
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
    b->section_text.name = lit(".text");
    b->section_text.shdr.sh_type = SHT_PROGBITS;
    b->section_text.shdr.sh_flags = SHF_ALLOC | SHF_EXECINSTR;
    b->section_text.shdr.sh_addralign = 16;
    b->section_text.shdr.sh_addr = 0;
    b->section_text.index = 1;

    memset(&b->section_data, 0, sizeof(b->section_data));
    b->section_data.name = lit(".data");
    b->section_data.shdr.sh_type = SHT_PROGBITS;
    b->section_data.shdr.sh_flags = SHF_ALLOC | SHF_WRITE;
    b->section_data.shdr.sh_addralign = 1;
    b->section_data.shdr.sh_addr = 0;
    b->section_data.index = 2;

    memset(&b->section_strtab, 0, sizeof(b->section_strtab));
    b->section_strtab.name = lit(".strtab");
    b->section_strtab.shdr.sh_type = SHT_STRTAB;
    b->section_strtab.shdr.sh_addralign = 1;
    b->section_strtab.index = 3;

    memset(&b->symbols, 0, sizeof(b->symbols));

    b->current_section = &b->section_text;
    oc_array_append(&cc->arena, &b->section_strtab, 0);
    oc_array_append(&cc->arena, &b->symbols, ((Elf64_Sym){}));


    Elf64_Sym sym;
    {
        sym.st_name = b->section_strtab.count;
        oc_array_append_many(&cc->arena, &b->section_strtab, lit("text.c").ptr, lit("text.c").len);
        oc_array_append(&cc->arena, &b->section_strtab, 0);

        sym.st_info = ELF32_ST_INFO(STB_LOCAL, STT_FILE);
        sym.st_other = STV_DEFAULT;
        sym.st_shndx = SHN_ABS;
        sym.st_value = 0;
        sym.st_size = 0;
        oc_array_append(&cc->arena, &b->symbols, sym);
    }
    {
        sym.st_name = b->section_strtab.count;
        oc_array_append_many(&cc->arena, &b->section_strtab, lit(".text").ptr, lit(".text").len);
        oc_array_append(&cc->arena, &b->section_strtab, 0);

        sym.st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
        sym.st_other = STV_DEFAULT;
        sym.st_shndx = b->section_text.index;
        sym.st_value = 0;
        sym.st_size = 0;
        oc_array_append(&cc->arena, &b->symbols, sym);
    }
    {
        sym.st_name = b->section_strtab.count;
        oc_array_append_many(&cc->arena, &b->section_strtab, lit(".data").ptr, lit(".data").len);
        oc_array_append(&cc->arena, &b->section_strtab, 0);

        sym.st_info = ELF32_ST_INFO(STB_LOCAL, STT_SECTION);
        sym.st_other = STV_DEFAULT;
        sym.st_shndx = b->section_data.index;
        sym.st_value = 0;
        sym.st_size = 0;
        oc_array_append(&cc->arena, &b->symbols, sym);
    }

    {
        memset(&b->memcpy_ident, 0, sizeof(b->memcpy_ident));
        b->memcpy_ident.str = lit("memcpy");
        b->memcpy_ident.symbol_index = b->symbols.count;

        Elf64_Sym sym;
        sym.st_name = b->section_strtab.count;
        oc_array_append_many(&cc->arena, &b->section_strtab, b->memcpy_ident.str.ptr, b->memcpy_ident.str.len);
        oc_array_append(&cc->arena, &b->section_strtab, 0);

        sym.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE);
        sym.st_other = STV_DEFAULT;
        sym.st_shndx = 0;
        sym.st_value = 0;
        sym.st_size = 0;
        oc_array_append(&cc->arena, &b->symbols, sym);
    }
}

bool linux_x86_64_elf_write_to_file(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, char* filepath) {
    int i;
    FILE* fptr;
    Elf64_Shdr shdr, *shdr_ptr;
    if (!fopen_s(&fptr, filepath, "w")) {
        eprint("Unable to open output file: %s\n", filepath);
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
    oc_array_append_many(&cc->arena, &b->ops, &header, sizeof(header));
    Elf64_Ehdr* hdrptr = (Elf64_Ehdr*)b->ops.items;



    Linux_x86_64_Elf_Section sections[] = {
        { 0 },
           b->section_text, b->section_data,
           {
            .name = lit(".symtab"),
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
            .name = lit(".rela.text"),
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
    for (i = 0; i < oc_len(sections); ++i) {

        if (sections[i].shdr.sh_addralign > 1) {
            uint64_t aligned_to = oc_align_forward(b->ops.count, sections[i].shdr.sh_addralign);
            oc_array_reserve(&cc->arena, &b->ops, aligned_to);
        }

        sections[i].file_offset = b->ops.count;
        oc_array_append_many(&cc->arena, &b->ops, sections[i].items, sections[i].count);
    }

    Linux_x86_64_Elf_Section shstrtab = { 0 };
    oc_array_append(&cc->arena, &shstrtab, 0);

    size_t shstrtab_offset = b->ops.count;
    for (i = 0; i < oc_len(sections); ++i) {
        if (i != 0) {
            sections[i].shdr.sh_name = shstrtab.count;
            oc_array_append_many(&cc->arena, &shstrtab, sections[i].name.ptr, sections[i].name.len);
            oc_array_append(&cc->arena, &shstrtab, 0);
        } else {
            sections[i].shdr.sh_name = 0;
        }
    }

    size_t shstrtab_name = shstrtab.count;
    oc_array_append_many(&cc->arena, &shstrtab, lit(".shstrtab").ptr, lit(".shstrtab").len);
    oc_array_append(&cc->arena, &shstrtab, 0);

    oc_array_append_many(&cc->arena, &b->ops, shstrtab.items, shstrtab.count);

    uint64_t needed_padding = oc_align_forward(b->ops.count, 16) - b->ops.count;
    if (needed_padding) {
        oc_array_reserve(&cc->arena, &b->ops, b->ops.count + needed_padding);
    }

    hdrptr = (Elf64_Ehdr*)b->ops.items;
    hdrptr->e_shoff = b->ops.count;
    hdrptr->e_shnum = oc_len(sections) + 1 /* plus one for shstrtab */;
    hdrptr->e_shstrndx = oc_len(sections);

    for (i = 0; i < oc_len(sections); ++i) {
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

        oc_array_append_many(&cc->arena, &b->ops, &shdr, sizeof(shdr));
    }

    size_t shdr_offset = b->ops.count;
    oc_array_append_many(&cc->arena, &b->ops, &shdr, sizeof(shdr));
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

static void x86_64_reset_collapse(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, uint32_t op, uint32_t reg) {
    b->register_collapses.items[reg].op_collapse = 0;
    b->register_collapses.items[reg].op_did_collapse = 0;
}

static void x86_64_set_collapse(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, uint32_t op, uint32_t reg, uint8_t op_collapse) {
    b->register_collapses.items[reg].op_collapse |= op_collapse;
}

Linux_x86_64_Elf_Collapse* x86_64_get_collapse(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, uint32_t reg) {
    return &b->register_collapses.items[reg];
}

static void x86_64_allocate_physical_registers(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
    size_t i;
    uint32_t offset = 0;

    for (i = 0; i < block->rops.count; ) {
        LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->rops.items[i];
        LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->rops.items[i + 1];

        /* ir_print_op(cc, bir, block->rops.items, i); */
        /* printf("\n"); */

        switch (opcode) {
        case LL_IR_OPCODE_BRANCH: break;
        case LL_IR_OPCODE_BRANCH_COND:
            b->registers.items[OPD_VALUE(operands[0])] = linux_x86_64_register_queue_dequeue(cc, &b->register_queue);
            break;
        case LL_IR_OPCODE_RET: break;
        case LL_IR_OPCODE_RETVALUE: {
            x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[1]));
            if (linux_x86_64_register_queue_is_free(cc, &b->register_queue, X86_64_OPERAND_REGISTER_rax)) {
                linux_x86_64_register_queue_use_reg(cc, &b->register_queue, X86_64_OPERAND_REGISTER_rax);

                if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
                    b->registers.items[OPD_VALUE(operands[1])] = X86_64_OPERAND_REGISTER_rax;
                }
            } else oc_assert(false);

            break;
        }
        case LL_IR_OPCODE_STORE:
            if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
                /* b->registers.items[OPD_VALUE(operands[1])] = linux_x86_64_elf_get_next_reg(cc, b); */
                b->registers.items[OPD_VALUE(operands[1])] = linux_x86_64_register_queue_dequeue(cc, &b->register_queue);
                x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[1]));
                x86_64_set_collapse(cc, b, bir, i, OPD_VALUE(operands[1]), COLLAPSE_KIND_MEM);
            }
            break;
        case LL_IR_OPCODE_CAST:
        case LL_IR_OPCODE_LOAD:
            if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
                b->registers.items[OPD_VALUE(operands[1])] = linux_x86_64_register_queue_dequeue(cc, &b->register_queue);
                x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[1]));
                if (opcode == LL_IR_OPCODE_CAST) x86_64_set_collapse(cc, b, bir, i, OPD_VALUE(operands[1]), COLLAPSE_KIND_MEM);
            }
            linux_x86_64_register_queue_enqueue(cc, &b->register_queue, b->registers.items[OPD_VALUE(operands[0])]);
            break;
        case LL_IR_OPCODE_LEA:
            linux_x86_64_register_queue_enqueue(cc, &b->register_queue, b->registers.items[OPD_VALUE(operands[0])]);
               break;
        case LL_IR_OPCODE_LEA_INDEX:
            linux_x86_64_register_queue_enqueue(cc, &b->register_queue, b->registers.items[OPD_VALUE(operands[0])]);

            if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
                b->registers.items[OPD_VALUE(operands[1])] = linux_x86_64_register_queue_dequeue(cc, &b->register_queue);
                x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[1]));
            }

            if (OPD_TYPE(operands[2]) == LL_IR_OPERAND_REGISTER_BIT) {
                b->registers.items[OPD_VALUE(operands[2])] = linux_x86_64_register_queue_dequeue(cc, &b->register_queue);
                x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[2]));
            }
               break;
        case LL_IR_OPCODE_AND:
        case LL_IR_OPCODE_OR:
        case LL_IR_OPCODE_XOR:
        case LL_IR_OPCODE_SUB:
        case LL_IR_OPCODE_ADD:
            x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[1]));
            x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[2]));

            b->registers.items[OPD_VALUE(operands[0])] = linux_x86_64_register_queue_dequeue(cc, &b->register_queue);
            if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
                /* result and first operand can be the same */
                b->registers.items[OPD_VALUE(operands[1])] = b->registers.items[OPD_VALUE(operands[0])];
                x86_64_set_collapse(cc, b, bir, i, OPD_VALUE(operands[1]), COLLAPSE_KIND_MEM | COLLAPSE_KIND_REG);
            }

            if (OPD_TYPE(operands[2]) == LL_IR_OPERAND_REGISTER_BIT) {
                x86_64_set_collapse(cc, b, bir, i, OPD_VALUE(operands[2]), COLLAPSE_KIND_REG | COLLAPSE_KIND_IMM);
                if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
                    /* result and second operand can be the same, if first operand isn't register */
                    b->registers.items[OPD_VALUE(operands[2])] = linux_x86_64_register_queue_dequeue(cc, &b->register_queue);
                } else {
                    b->registers.items[OPD_VALUE(operands[2])] = b->registers.items[OPD_VALUE(operands[0])];
                }
            }

            linux_x86_64_register_queue_enqueue(cc, &b->register_queue, b->registers.items[OPD_VALUE(operands[0])]);

            break;

        case LL_IR_OPCODE_LT:
        case LL_IR_OPCODE_LTE:
        case LL_IR_OPCODE_GT:
        case LL_IR_OPCODE_GTE:
        case LL_IR_OPCODE_EQ:
        case LL_IR_OPCODE_NEQ:
            x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[1]));
            x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[2]));
            x86_64_set_collapse(cc, b, bir, i, OPD_VALUE(operands[1]), COLLAPSE_KIND_REG | COLLAPSE_KIND_MEM);
            x86_64_set_collapse(cc, b, bir, i, OPD_VALUE(operands[2]), COLLAPSE_KIND_REG | COLLAPSE_KIND_IMM);
            b->registers.items[OPD_VALUE(operands[1])] = linux_x86_64_register_queue_dequeue(cc, &b->register_queue);
            b->registers.items[OPD_VALUE(operands[2])] = linux_x86_64_register_queue_dequeue(cc, &b->register_queue);
            break;
        case LL_IR_OPCODE_DIV:
        case LL_IR_OPCODE_MUL: {
            if (linux_x86_64_register_queue_is_free(cc, &b->register_queue, X86_64_OPERAND_REGISTER_rax)) {
                linux_x86_64_register_queue_use_reg(cc, &b->register_queue, X86_64_OPERAND_REGISTER_rax);

                b->registers.items[OPD_VALUE(operands[0])] = X86_64_OPERAND_REGISTER_rax;
            }

            if (OPD_TYPE(operands[2]) == LL_IR_OPERAND_REGISTER_BIT) {
                /* result and first operand can be the same */
                b->registers.items[OPD_VALUE(operands[2])] = b->registers.items[OPD_VALUE(operands[0])];
                x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[2]));
            } else oc_assert(false);

            if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
                x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[1]));
                x86_64_set_collapse(cc, b, bir, i, OPD_VALUE(operands[1]), COLLAPSE_KIND_REG);
                if (OPD_TYPE(operands[2]) == LL_IR_OPERAND_REGISTER_BIT) {
                    /* result and second operand can be the same, if first operand isn't register */
                    b->registers.items[OPD_VALUE(operands[1])] = linux_x86_64_register_queue_dequeue(cc, &b->register_queue);
                } else {
                    b->registers.items[OPD_VALUE(operands[1])] = b->registers.items[OPD_VALUE(operands[0])];
                }
            }

            linux_x86_64_register_queue_enqueue(cc, &b->register_queue, b->registers.items[OPD_VALUE(operands[0])]);
            break;
        }
        case LL_IR_OPCODE_INVOKEVALUE:
            offset = 1;
        case LL_IR_OPCODE_INVOKE: {
            uint32_t count = operands[1 + offset];
            Linux_x86_64_Elf_Call_Convention callconv = linux_x86_64_call_convention_systemv(b);

            for (uint32_t pi = 0; pi < count; ++pi) {
                X86_64_Operand_Register reg = linux_x86_64_call_convention_next_reg(b, &callconv);

                if (reg == X86_64_OPERAND_REGISTER_invalid) {
                    // we must use memory
                    b->registers.items[OPD_VALUE(operands[2 + pi + offset])] = linux_x86_64_register_queue_dequeue(cc, &b->register_queue);
                } else {
                    if (linux_x86_64_register_queue_is_free(cc, &b->register_queue, reg)) {
                        linux_x86_64_register_queue_use_reg(cc, &b->register_queue, reg);

                        switch (OPD_TYPE(operands[2 + pi + offset])) {
                            case LL_IR_OPERAND_REGISTER_BIT: 
                                x86_64_reset_collapse(cc, b, bir, i, OPD_VALUE(operands[2 + pi + offset]));
                                b->registers.items[OPD_VALUE(operands[2 + pi + offset])] = reg;
                                break;
                            case LL_IR_OPERAND_IMMEDIATE_BIT:
                                break;
                            default: oc_todo("unhadnled argument operand");
                        }
                    } else {
                        oc_assert(false);
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
    oc_assert(!params.immediate || !params.mem_right);
    switch (type->kind) {
    case LL_TYPE_STRING:
    case LL_TYPE_POINTER:
        if (params.immediate) return X86_64_VARIANT_KIND_rm64_i32;
        else if (params.mem_right) return X86_64_VARIANT_KIND_r64_rm64;
        else return X86_64_VARIANT_KIND_rm64_r64;
    case LL_TYPE_BOOL:
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
    oc_exit(-1);
    return (X86_64_Variant_Kind)-1;
}

static void linux_x86_64_elf_generate_mov_to_register(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Type* type, X86_64_Operand_Register result, LL_Ir_Operand src, bool store) {
    X86_64_Instruction_Parameters params = { 0 };
    params.reg0 = result;
    if (store) {
        params.reg0 |= X86_64_REG_BASE;
    }

    switch (type->kind) {
    case LL_TYPE_STRING:
    case LL_TYPE_ANYINT:
    case LL_TYPE_UINT:
    case LL_TYPE_INT: {
        /* oc_todo: max immeidiate is 28 bits */
        switch (OPD_TYPE(src)) {
        case LL_IR_OPERAND_IMMEDIATE_BIT:
            params.immediate = OPD_VALUE(src);

            OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
            break;
        case LL_IR_OPERAND_REGISTER_BIT:
            params.reg1 = b->registers.items[OPD_VALUE(src)];
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { 0 }), params);
            break;
        case LL_IR_OPERAND_LOCAL_BIT:
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -b->locals.items[OPD_VALUE(src)];
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_LEA, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .mem_right = true }), params);
            break;
        case LL_IR_OPERAND_DATA_BIT:
            params.immediate = 0;
            OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_MOV, r64_i64, params);
            oc_array_append(&cc->tmp_arena, &b->relocations, ((Elf64_Rela) { .r_offset = b->current_section->count - 8, .r_info = ELF64_R_INFO(3, R_X86_64_64), .r_addend = bir->data_items.items[OPD_VALUE(src)].binary_offset }));
            break;
        default: oc_todo("handle other operands"); break;
        }
        break;
    }
    default: oc_todo("handle other types"); break;
    }
}

static void linux_x86_64_elf_generate_mov(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand result, LL_Ir_Operand src, bool store) {
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
            /* oc_todo: max immeidiate is 28 bits */
            switch (OPD_TYPE(src)) {
            case LL_IR_OPERAND_IMMEDIATE_BIT:
                params.immediate = OPD_VALUE(src);
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
                break;
            case LL_IR_OPERAND_REGISTER_BIT:
                params.reg1 = b->registers.items[OPD_VALUE(src)];
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { 0 }), params);
                break;
            case LL_IR_OPERAND_DATA_BIT: {
                /* oc_array_append(&cc->tmp_arena, &b->internal_relocations, ((Linux_x86_64_Internal_Relocation) {  })); */
                /* OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_LEA, r64_rm64, params); */
                /* OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_MOV, , params); */
                break;
            }
            case LL_IR_OPERAND_LOCAL_BIT:
                break;
            case LL_IR_OPERAND_PARMAETER_BIT: {
                break;
            }
            default: oc_todo("handle other operands"); break;
            }
            break;
        }
        default: oc_todo("handle other types"); break;
        }

        break;
    }
    case LL_IR_OPERAND_REGISTER_BIT:
        linux_x86_64_elf_generate_mov_to_register(cc, b, bir, type, b->registers.items[OPD_VALUE(result)], src, store);
        break;
    }
}

static void linux_x86_64_elf_generate_load_cast(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Operand result, LL_Ir_Operand src, bool load) {
    uint32_t opcode;
    X86_64_Variant_Kind kind;
    LL_Type* from_type;
    Linux_x86_64_Elf_Collapse* collapse;
    X86_64_Instruction_Parameters params = { 0 };
    LL_Type* to_type = ir_get_operand_type(b->fn, result);

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
        from_type = to_type;
        break;
    default:
        from_type = ir_get_operand_type(b->fn, src);
        break;
    }

    params.reg0 = b->registers.items[OPD_VALUE(result)];

    if (to_type != from_type) {
        switch (from_type->kind) {
        case LL_TYPE_UINT:
            switch (to_type->kind) {
            case LL_TYPE_BOOL:
            case LL_TYPE_INT:
            case LL_TYPE_UINT:
                if (from_type->width < to_type->width) {
                    if (from_type->width <= 8) {
                        opcode = X86_64_OPCODE_MOVZX;
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
                        opcode = X86_64_OPCODE_MOVZX;
                        if (to_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm16;
                            break;
                        } else if (to_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm16;
                            break;
                        }
                    }
                }
                opcode = X86_64_OPCODE_MOV;
                kind = linux_x86_64_get_variant(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .mem_right = true });
                break;
            default: oc_assert(false);
            }
            break;
        case LL_TYPE_INT:
            switch (to_type->kind) {
            case LL_TYPE_BOOL:
            case LL_TYPE_UINT:
            case LL_TYPE_INT:
                if (from_type->width < to_type->width) {
                    if (from_type->width <= 8) {
                        opcode = X86_64_OPCODE_MOVSX;
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
                        opcode = X86_64_OPCODE_MOVSX;
                        if (to_type->width <= 32) {
                            kind = X86_64_VARIANT_KIND_r32_rm16;
                            break;
                        } else if (to_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm16;
                            break;
                        }
                    } else if (from_type->width <= 32) {
                        opcode = X86_64_OPCODE_MOVSXD;
                        if (to_type->width <= 64) {
                            kind = X86_64_VARIANT_KIND_r64_rm32;
                            break;
                        }
                    }
                }
                opcode = X86_64_OPCODE_MOV;
                kind = linux_x86_64_get_variant(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .mem_right = true });
                break;
            default: oc_assert(false);
            }

            break;
        default: oc_assert(false);
        }
    } else {
        opcode = X86_64_OPCODE_MOV;
        kind = linux_x86_64_get_variant(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .mem_right = true });
    }

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_LOCAL_BIT: {
        // @collapse_gen
        collapse = x86_64_get_collapse(cc, b, bir, OPD_VALUE(result));

        if (collapse && collapse->op_collapse & COLLAPSE_KIND_MEM) {
            collapse->op_did_collapse = COLLAPSE_KIND_MEM;
            collapse->op_parameter = b->locals.items[OPD_VALUE(src)];
        } else {
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = -b->locals.items[OPD_VALUE(src)];
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, kind, params);
        }

        break;
    }
    case LL_IR_OPERAND_REGISTER_BIT: {
        if (load) {
            params.reg1 = b->registers.items[OPD_VALUE(src)] | X86_64_REG_BASE;
        } else {
            // @collapse_use
            collapse = x86_64_get_collapse(cc, b, bir, OPD_VALUE(src));
            if (collapse->op_did_collapse) {
                if (collapse->op_did_collapse == COLLAPSE_KIND_MEM) {
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -b->locals.items[OPD_VALUE(src)];
                }
            } else {
                params.reg1 = b->registers.items[OPD_VALUE(src)];
            }
            
        }
        OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, kind, params);
        break;
    }
    case LL_IR_OPERAND_PARMAETER_BIT: {
        Linux_x86_64_Elf_Call_Convention conv = linux_x86_64_call_convention_systemv(b);
        Linux_Call_Convention_Parameter parameter = linux_x86_64_call_convention_nth_parameter(b, &conv, OPD_VALUE(src));

        if (parameter.is_reg) {
            // @collapse_gen
            collapse = x86_64_get_collapse(cc, b, bir, OPD_VALUE(result));
            if (collapse && collapse->op_collapse & COLLAPSE_KIND_REG) {
                collapse->op_did_collapse = COLLAPSE_KIND_REG;
                collapse->op_parameter = parameter.reg;
            } else {
                params.reg1 = parameter.reg;
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, kind, params);
            }
        } else {
            params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
            params.displacement = parameter.stack_offset + 16;
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, kind, params);
        }
        break;
    }
    case LL_IR_OPERAND_IMMEDIATE_BIT: {
        collapse = x86_64_get_collapse(cc, b, bir, OPD_VALUE(result));
        if (collapse && collapse->op_collapse & COLLAPSE_KIND_IMM) {
            collapse->op_did_collapse = COLLAPSE_KIND_IMM;
            collapse->op_parameter = OPD_VALUE(src);
        } else {
            params.immediate = OPD_VALUE(src);
            OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode, linux_x86_64_get_variant(cc, b, bir, to_type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
        }
        break;
    }
    default: oc_todo("add load operands"); break;
    }
}

#define LINUX_X86_64_REGISTERS_EQL(_a, _b) (b->registers.items[OPD_VALUE(_a)] == b->registers.items[OPD_VALUE(_b)])

static void linux_x86_64_elf_generate_block(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
    size_t i;
    int32_t opcode1, opcode2;
    Linux_x86_64_Elf_Collapse *collapse1, *collapse2;
    X86_64_Get_Variant_Params get_variant = { 0 };

    linux_x86_64_register_queue_reset(cc, &b->register_queue);
    x86_64_allocate_physical_registers(cc, b, bir, block);
    block->generated_offset = (int64_t)b->section_text.count;

    if (block->ref1) {
        int32_t* dst_offset = (int32_t*)&b->section_text.items[bir->blocks.items[block->ref1].fixup_offset];
        *dst_offset = (int32_t)(block->generated_offset - bir->blocks.items[block->ref1].fixup_offset - 4);
    }
    if (block->ref2) {
        int32_t* dst_offset = (int32_t*)&b->section_text.items[bir->blocks.items[block->ref2].fixup_offset];
        *dst_offset = (int32_t)(block->generated_offset - bir->blocks.items[block->ref2].fixup_offset - 4);
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
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_JMP, rel8, params);
                } else {
                    params.relative -= 5;
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_JMP, rel32, params);
                }
            } else {
                params.relative = 0;
                OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_JMP, rel32, params);
                block->fixup_offset = (int64_t)b->section_text.count - 4u;
            }
            break;
        case LL_IR_OPCODE_BRANCH_COND:
            params.relative = 0;
            if (operands[1] == block->next) {
                // then block is next

                OC_X86_64_WRITE_INSTRUCTION(b, oc_x86_64_get_inverse_compare(b->registers.items[OPD_VALUE(operands[0])]), rel32, params);
                bir->blocks.items[operands[1]].ref1 = 0;
                bir->blocks.items[operands[2]].ref1 = bir->blocks.items[operands[1]].prev;
            } else if (operands[2] == block->next) {
                // else block is next
                OC_X86_64_WRITE_INSTRUCTION(b, b->registers.items[OPD_VALUE(operands[0])], rel32, params);
                bir->blocks.items[operands[2]].ref1 = 0;
                bir->blocks.items[operands[1]].ref1 = bir->blocks.items[operands[2]].prev;
            } else oc_assert(false);
            block->fixup_offset = (int64_t)b->section_text.count - 4u;
            break;
        case LL_IR_OPCODE_RET:
            params.relative = 0;
            OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_JMP, rel32, params);
            break;
        case LL_IR_OPCODE_RETVALUE: {
            LL_Type* type = ir_get_operand_type(b->fn, operands[0]);
            switch (OPD_TYPE(operands[0])) {
            case LL_IR_OPERAND_IMMEDIATE_BIT:
                params.reg0 = X86_64_OPERAND_REGISTER_rax;
                params.immediate = OPD_VALUE(operands[0]);
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params);
                break;
            case LL_IR_OPERAND_REGISTER_BIT:
                if (b->registers.items[OPD_VALUE(operands[0])] != X86_64_OPERAND_REGISTER_rax) {
                    params.reg0 = X86_64_OPERAND_REGISTER_rax;
                    params.reg1 = b->registers.items[OPD_VALUE(operands[0])];
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_MOV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { 0 }), params);
                }
                break;
            default: oc_todo("handle return value"); break;
            }
            params.relative = 0;
            OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_JMP, rel32, params);
            break;
        }
        case LL_IR_OPCODE_STORE: {
            if (OPD_TYPE(operands[1]) == LL_IR_OPERAND_REGISTER_BIT) {
                // @collapse_use
                collapse1 = x86_64_get_collapse(cc, b, bir, OPD_VALUE(operands[1]));
                if ((collapse1->op_collapse & COLLAPSE_KIND_MEM) && collapse1->op_did_collapse) {
                } else {
                    linux_x86_64_elf_generate_mov(cc, b, bir, operands[0], operands[1], true);
                }
            } else {
                linux_x86_64_elf_generate_mov(cc, b, bir, operands[0], operands[1], true);
            }
               break;
        }

        case LL_IR_OPCODE_MEMCOPY: {
            LL_Type* type = ir_get_operand_type(b->fn, operands[0]);
            switch (type->kind) {
            case LL_TYPE_ARRAY: {
                LL_Type_Array* arr_type = (LL_Type_Array*)type;
                LL_Backend_Layout elem_layout = linux_x86_64_elf_get_layout(arr_type->element_type);
                size_t size = max(elem_layout.alignment, elem_layout.size);

                OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdi }) );
                OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rsi }) );

                linux_x86_64_elf_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rdi, operands[0], false);
                linux_x86_64_elf_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rsi, operands[1], false);

                switch (size) {
                case 1:
                case 2:
                case 4:
                case 8: {
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx }) );

                    LL_Ir_Operand size_value;
                    switch (OPD_TYPE(operands[2])) {
                    case LL_IR_OPERAND_IMMEDIATE_BIT:
                        size_value = OPD_VALUE(operands[2]) / size;
                        linux_x86_64_elf_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rcx, size_value, false);
                        break;
                    case LL_IR_OPERAND_REGISTER_BIT:
                        OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_SHR, rm64_i8, ((X86_64_Instruction_Parameters) { .reg0 = b->registers.items[OPD_VALUE(operands[2])], .immediate = log2_u32(size << 3) }) );

                        if (b->registers.items[OPD_VALUE(operands[2])] != X86_64_OPERAND_REGISTER_rcx) {
                            linux_x86_64_elf_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rcx, operands[2], true);
                        }
                        break;
                    default:
                        linux_x86_64_elf_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rcx, operands[2], true);
                        break;
                    }

                    const static int opcodes[] = { [1] = X86_64_OPCODE_MOVSB, [2] = X86_64_OPCODE_MOVSW, [4] = X86_64_OPCODE_MOVSD, [8] = X86_64_OPCODE_MOVSQ };
                    OC_X86_64_WRITE_INSTRUCTION(b, opcodes[size], noarg, ((X86_64_Instruction_Parameters) { .rep = X86_64_PREFIX_REP }) );
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_POP, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rcx }) );

                    break;
                }
                default:
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx }) );

                    linux_x86_64_elf_generate_mov_to_register(cc, b, bir, cc->typer->ty_int64, X86_64_OPERAND_REGISTER_rdx, operands[2], false);

                    params.relative = 0;
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_CALL, rel32, params);

                    oc_array_append(
                        &cc->tmp_arena,
                        &b->relocations,
                        ((Elf64_Rela) { .r_offset = b->current_section->count - 4, .r_info = ELF64_R_INFO(b->memcpy_ident.symbol_index, R_X86_64_PLT32), .r_addend = -4 })
                    );

                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_POP, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdx }) );

                    break;
                }

                OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_POP, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rsi }) );
                OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_POP, rm64, ((X86_64_Instruction_Parameters) { .reg0 = X86_64_OPERAND_REGISTER_rdi }) );
            }
            default: oc_todo("handle other operands"); break;
            }
            break;
        }

        case LL_IR_OPCODE_EQ:
            b->registers.items[OPD_VALUE(operands[0])] = X86_64_OPCODE_JE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_NEQ:
            b->registers.items[OPD_VALUE(operands[0])] = X86_64_OPCODE_JNE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_GTE:
            b->registers.items[OPD_VALUE(operands[0])] = X86_64_OPCODE_JGE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_GT:
            b->registers.items[OPD_VALUE(operands[0])] = X86_64_OPCODE_JG;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_LTE:
            b->registers.items[OPD_VALUE(operands[0])] = X86_64_OPCODE_JLE;
            goto DO_OPCODE_COMPARE;
        case LL_IR_OPCODE_LT: {
            b->registers.items[OPD_VALUE(operands[0])] = X86_64_OPCODE_JL;
            LL_Type* type;
DO_OPCODE_COMPARE:
            type = ir_get_operand_type(b->fn, operands[1]);
            switch (type->kind) {
            case LL_TYPE_ANYBOOL:
            case LL_TYPE_BOOL:
            case LL_TYPE_ANYINT:
            case LL_TYPE_UINT:
            case LL_TYPE_INT: {
                // @collapse_use
                collapse1 = x86_64_get_collapse(cc, b, bir, OPD_VALUE(operands[1]));
                collapse2 = x86_64_get_collapse(cc, b, bir, OPD_VALUE(operands[2]));
                if (collapse1->op_did_collapse) {
                    if (collapse1->op_did_collapse == COLLAPSE_KIND_MEM) {
                        params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                        params.displacement = -collapse1->op_parameter;
                    } else if (collapse1->op_did_collapse == COLLAPSE_KIND_REG) {
                        params.reg0 = collapse1->op_parameter;
                    }
                } else {
                    params.reg0 = b->registers.items[OPD_VALUE(operands[1])];
                }

                if (collapse2->op_did_collapse) {
                    if (collapse2->op_did_collapse == COLLAPSE_KIND_REG) {
                        params.reg0 = collapse2->op_parameter;
                    } else if (collapse2->op_did_collapse == COLLAPSE_KIND_IMM) {
                        params.immediate = collapse2->op_parameter;
                        get_variant.immediate = true;
                    }
                } else {
                    params.reg1 = b->registers.items[OPD_VALUE(operands[2])];
                }

                OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_CMP, linux_x86_64_get_variant(cc, b, bir, type, get_variant), params);
                break;
            }
            default: oc_todo("handle other type"); break;
            }
            break;
        }
        case LL_IR_OPCODE_SUB:
            opcode1 = X86_64_OPCODE_SUB;
            opcode2 = X86_64_OPCODE_DEC;
            goto DO_OPCODE_ARITHMETIC;
        case LL_IR_OPCODE_ADD: {
            LL_Type* type;
            opcode1 = X86_64_OPCODE_ADD;
            opcode2 = X86_64_OPCODE_INC;
DO_OPCODE_ARITHMETIC:
            type = ir_get_operand_type(b->fn, operands[0]);
            switch (type->kind) {
            case LL_TYPE_ANYINT:
            case LL_TYPE_INT: {
                // @collapse_use
                collapse1 = x86_64_get_collapse(cc, b, bir, OPD_VALUE(operands[1]));
                collapse2 = x86_64_get_collapse(cc, b, bir, OPD_VALUE(operands[2]));
                if (collapse1->op_did_collapse) {
                    if (collapse1->op_did_collapse == COLLAPSE_KIND_MEM) {
                        params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                        params.displacement = -collapse1->op_parameter;

                        collapse1 = x86_64_get_collapse(cc, b, bir, OPD_VALUE(operands[0]));
                        if (collapse1->op_collapse & COLLAPSE_KIND_MEM) {
                            collapse1->op_did_collapse |= COLLAPSE_KIND_MEM;
                            collapse1->op_parameter = -params.displacement;
                        }

                    } else if (collapse1->op_did_collapse == COLLAPSE_KIND_REG) {
                        params.reg0 = collapse1->op_parameter;
                    }
                } else {
                    params.reg0 = b->registers.items[OPD_VALUE(operands[1])];
                }

                if (collapse2->op_did_collapse) {
                    if (collapse2->op_did_collapse == COLLAPSE_KIND_REG) {
                        params.reg0 = collapse2->op_parameter;
                    } else if (collapse2->op_did_collapse == COLLAPSE_KIND_IMM) {
                        params.immediate = collapse2->op_parameter;
                        get_variant.immediate = true;
                    }
                } else {
                    params.reg1 = b->registers.items[OPD_VALUE(operands[2])];
                }


                if (get_variant.immediate && params.immediate == 1) {
                    get_variant.immediate = false;
                    get_variant.single = true;
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode2, linux_x86_64_get_variant(cc, b, bir, type, get_variant), params);
                } else {
                    OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode1, linux_x86_64_get_variant(cc, b, bir, type, get_variant), params);
                }

/* 				/1* oc_todo: max immeidiate is 28 bits *1/ */
/* 				switch (OPD_TYPE(operands[1])) { */
/* 				case LL_IR_OPERAND_REGISTER_BIT: { */
/* 					switch (OPD_TYPE(operands[2])) { */
/* 					case LL_IR_OPERAND_IMMEDIATE_BIT: */
/* 						if (LINUX_X86_64_REGISTERS_EQL(operands[1], operands[0])) { */
/* 							params.reg0 = b->registers.items[OPD_VALUE(operands[0])]; */
/* 							if (OPD_VALUE(operands[2]) == 1) { */
/* 								OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode2, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params); */
/* 							} else { */
/* 								params.immediate = OPD_VALUE(operands[2]); */
/* 								OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode1, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params); */
/* 							} */
/* 						} */
/* 						break; */
/* 					case LL_IR_OPERAND_REGISTER_BIT: */
/* 						if (LINUX_X86_64_REGISTERS_EQL(operands[1], operands[0])) { */
/* 							params.reg0 = b->registers.items[OPD_VALUE(operands[0])]; */
/* 							params.reg1 = b->registers.items[OPD_VALUE(operands[2])]; */
/* 							OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode1, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) {}), params); */
/* 						} */
/* 						break; */
/* 					default: printf("oc_todo: handle add other register rhs"); break; */
/* 					} */
/* 					break; */
/* 				} */
/* 				case LL_IR_OPERAND_IMMEDIATE_BIT: { */
/* 					switch (OPD_TYPE(operands[2])) { */
/* 					case LL_IR_OPERAND_IMMEDIATE_BIT: */
/* 						linux_x86_64_elf_generate_mov(cc, b, bir, operands[0], operands[1]); */
/* 						params.reg0 = b->registers.items[OPD_VALUE(operands[0])]; */
/* 						params.immediate = OPD_VALUE(operands[2]); */
/* 						OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode1, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params); */
/* 						break; */
/* 					case LL_IR_OPERAND_REGISTER_BIT: */
/* 						if (OPD_VALUE(operands[2]) == 1) { */
/* 							params.reg0 = b->registers.items[OPD_VALUE(operands[0])]; */
/* 							OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode2, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params); */
/* 						} else { */
/* 							params.immediate = OPD_VALUE(operands[2]); */
/* 							if (LINUX_X86_64_REGISTERS_EQL(operands[1], operands[0])) { */
/* 								params.reg0 = b->registers.items[OPD_VALUE(operands[0])]; */
/* 								OC_X86_64_WRITE_INSTRUCTION_DYN(b, opcode1, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .immediate = true }), params); */
/* 							} */
/* 						} */
/* 						break; */
/* 					} */
/* 					break; */
/* 				} */
/* 				} */
                break;
            }
            default: oc_todo("handle other type"); break;
            }
            break;
        }
        case LL_IR_OPCODE_MUL: {
            LL_Type* type = ir_get_operand_type(b->fn, operands[0]);

            collapse1 = x86_64_get_collapse(cc, b, bir, OPD_VALUE(operands[1]));
            if (collapse1->op_did_collapse) {
                params.reg0 = collapse1->op_parameter;
            } else {
                params.reg0 = b->registers.items[OPD_VALUE(operands[1])];
            }

            switch (type->kind) {
            case LL_TYPE_INT:
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_IMUL, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params);
                break;
            case LL_TYPE_UINT:
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_MUL, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params);
                break;
            default: oc_todo("implement other types"); break;
            }

            break;
        }
        case LL_IR_OPCODE_DIV: {
            LL_Type* type = ir_get_operand_type(b->fn, operands[0]);
            params.reg0 = b->registers.items[OPD_VALUE(operands[2])];

            switch (type->kind) {
            case LL_TYPE_INT:
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_IDIV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params);
                break;
            case LL_TYPE_UINT:
                OC_X86_64_WRITE_INSTRUCTION_DYN(b, X86_64_OPCODE_DIV, linux_x86_64_get_variant(cc, b, bir, type, (X86_64_Get_Variant_Params) { .single = true }), params);
                break;
            default: oc_todo("implement other types"); break;
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
                /* oc_todo: max immeidiate is 28 bits */
                params.reg0 = b->registers.items[OPD_VALUE(operands[0])];
                switch (OPD_TYPE(operands[1])) {
                case LL_IR_OPERAND_LOCAL_BIT: {
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement = -b->locals.items[OPD_VALUE(operands[1])];
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_LEA, r64_rm64, params);
                    break;
                }
                case LL_IR_OPERAND_DATA_BIT: {
                    params.reg0 = b->registers.items[OPD_VALUE(operands[0])];
                    params.immediate = 0;
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_MOV, r64_i64, params);
                    /* oc_array_append(&cc->tmp_arena, &b->internal_relocations, ((Linux_x86_64_Internal_Relocation) { .data_item = OPD_VALUE(operands[1]), .text_rel_byte_offset = b->current_section->count - 4 /1* sizeof displacement *1/ })); */
                    oc_array_append(&cc->tmp_arena, &b->relocations, ((Elf64_Rela) { .r_offset = b->current_section->count - 8, .r_info = ELF64_R_INFO(3, R_X86_64_64), .r_addend = bir->data_items.items[OPD_VALUE(operands[1])].binary_offset }));
                    break;
                }
                default: oc_todo("add lea operands"); break;
                }
                break;
            }
            default: ll_print_type_raw(type, &stderr_writer); oc_todo("add lea types"); break;
            }

            break;
        }
        case LL_IR_OPCODE_LEA_INDEX: {
            LL_Type* type = ir_get_operand_type(b->fn, operands[0]);
            switch (type->kind) {
            case LL_TYPE_ARRAY:
            case LL_TYPE_STRING:
            case LL_TYPE_ANYINT:
            case LL_TYPE_INT: {
                /* oc_todo: max immeidiate is 28 bits */
                params.reg0 = b->registers.items[OPD_VALUE(operands[0])];


                switch (OPD_TYPE(operands[2])) {
                case LL_IR_OPERAND_IMMEDIATE_BIT: {
                    /* params.use_sib = true; */
                    params.displacement = (OPD_VALUE(operands[2]) * OPD_VALUE(operands[3]));
                    break;
                }
                case LL_IR_OPERAND_REGISTER_BIT: {
                    params.use_sib = 1 | X86_64_SIB_INDEX | X86_64_SIB_SCALE;
                    params.index = b->registers.items[OPD_VALUE(operands[2])];

                    switch (OPD_VALUE(operands[3])) {
                    case 1: params.scale = 0; break;
                    case 2: params.scale = 1; break;
                    case 4: params.scale = 2; break;
                    case 8: params.scale = 3; break;
                    default: oc_todo("add index size"); break;
                    }

                    break;
                }
                default: oc_todo("add lea operands"); break;
                }

                switch (OPD_TYPE(operands[1])) {
                case LL_IR_OPERAND_REGISTER_BIT: {
                    params.reg1 = b->registers.items[OPD_VALUE(operands[1])] | X86_64_REG_BASE;
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_LEA, r64_rm64, params);
                    break;
                }
                case LL_IR_OPERAND_LOCAL_BIT: {
                    params.reg1 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                    params.displacement += -b->locals.items[OPD_VALUE(operands[1])];
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_LEA, r64_rm64, params);
                    break;
                }
                case LL_IR_OPERAND_DATA_BIT: {
                    params.reg0 = b->registers.items[OPD_VALUE(operands[0])];
                    params.immediate = 0;
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_MOV, r64_i64, params);
                    /* oc_array_append(&cc->tmp_arena, &b->internal_relocations, ((Linux_x86_64_Internal_Relocation) { .data_item = OPD_VALUE(operands[1]), .text_rel_byte_offset = b->current_section->count - 4 /1* sizeof displacement *1/ })); */
                    oc_array_append(&cc->tmp_arena, &b->relocations, ((Elf64_Rela) { .r_offset = b->current_section->count - 8, .r_info = ELF64_R_INFO(3, R_X86_64_64), .r_addend = bir->data_items.items[OPD_VALUE(operands[1])].binary_offset }));
                    break;
                }
                default: oc_todo("add lea operands"); break;
                }
                break;
            }
            default: oc_todo("add lea types"); break;
            }

            break;
        }
        case LL_IR_OPCODE_INVOKE: {
            uint8_t reg;
            uint32_t invokee = operands[0];
            uint32_t count = operands[1];
            Linux_x86_64_Elf_Call_Convention callconv = linux_x86_64_call_convention_systemv(b);

            for (uint32_t j = 0; j < count; ++j) {
                switch (OPD_TYPE(operands[2 + j])) {
                    case LL_IR_OPERAND_REGISTER_BIT: 
                        reg = linux_x86_64_call_convention_next_reg(b, &callconv);

                        if (reg == X86_64_OPERAND_REGISTER_invalid) {
                            int32_t offset = linux_x86_64_call_convention_next_mem(b, &callconv);
                            params.reg0 = X86_64_OPERAND_REGISTER_rsp | X86_64_REG_BASE;
                            params.displacement = offset;
                            params.reg1 = b->registers.items[OPD_VALUE(operands[2 + j])];
                            OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_MOV, rm64_r64, params);
                        }
                        break;
                    case LL_IR_OPERAND_IMMEDIATE_BIT:
                        reg = linux_x86_64_call_convention_next_reg(b, &callconv);
                        if (reg == X86_64_OPERAND_REGISTER_invalid) {
                            int32_t offset = -linux_x86_64_call_convention_next_mem(b, &callconv);
                            params.reg0 = X86_64_OPERAND_REGISTER_rbp | X86_64_REG_BASE;
                            params.displacement = offset;
                            params.immediate = OPD_VALUE(operands[2 + j]);
                            OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_MOV, rm64_i32, params);
                        } else {
                            params.reg0 = reg;
                            params.immediate = OPD_VALUE(operands[2 + j]);
                            OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_MOV, rm64_i32, params);
                        }
                        break;
                    default: oc_todo("unhadnled argument operand");
                }
            }

            switch (OPD_TYPE(invokee)) {
            case LL_IR_OPERAND_FUNCTION_BIT: {
                LL_Ir_Function* fn = &bir->fns.items[OPD_VALUE(invokee)];
                Code_Ident* ident = fn->ident;

                if (fn->flags & LL_IR_FUNCTION_FLAG_EXTERN || fn->generated_offset == LL_IR_FUNCTION_OFFSET_INVALID) {
                    params.relative = 0;

                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_CALL, rel32, params);

                    if (fn->flags & LL_IR_FUNCTION_FLAG_EXTERN) {
                        if (ident->symbol_index != -1) {
                            oc_array_append(
                                &cc->tmp_arena,
                                &b->relocations,
                                ((Elf64_Rela) { .r_offset = b->current_section->count - 4, .r_info = ELF64_R_INFO(ident->symbol_index, R_X86_64_PLT32), .r_addend = -4 })
                            );
                            break;
                        } else {
                            ident->symbol_index = (int32_t)b->symbols.count;
                            oc_array_append(
                                &cc->tmp_arena,
                                &b->relocations,
                                ((Elf64_Rela) { .r_offset = b->current_section->count - 4, .r_info = ELF64_R_INFO(b->symbols.count, R_X86_64_PLT32), .r_addend = -4 })
                            );
                        }
                    } else {
                        oc_array_append(
                            &cc->tmp_arena,
                            &b->internal_relocations,
                            ((Linux_x86_64_Internal_Relocation) { b->current_section->count - 4, OPD_VALUE(invokee) })
                        );
                        ident->symbol_index = (int32_t)b->symbols.count;
                    }

                    Elf64_Sym sym;
                    sym.st_name = b->section_strtab.count;
                    oc_array_append_many(&cc->arena, &b->section_strtab, ident->str.ptr, ident->str.len);
                    oc_array_append(&cc->arena, &b->section_strtab, 0);

                    sym.st_info = ELF32_ST_INFO(STB_GLOBAL, STT_NOTYPE);
                    sym.st_other = STV_DEFAULT;
                    sym.st_shndx = 0;
                    sym.st_value = 0;
                    sym.st_size = 0;
                    oc_array_append(&cc->arena, &b->symbols, sym);
                } else {
                    params.relative = (int32_t)(fn->generated_offset - (int64_t)b->current_section->count);
                    OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_CALL, rel32, params);
                }
                break;
            }
            default: oc_todo("handle inveok type"); break;
            }

               break;
        }
        /* case LL_IR_OPCODE_INVOKE: { */
        /* 	uint32_t count = operands[2]; */
        /* 	for (uint32_t j = 0; j < count; ++j) { */
        /* 	} */

        /*    	break; */
        /* } */
        default: oc_todo("handle other op\n"); break;
        }

        size_t count = ir_get_op_count(cc, bir, block->ops.items, i);
        i += count;
    }
}

void linux_x86_64_elf_generate(Compiler_Context* cc, Linux_x86_64_Elf_Backend* b, LL_Backend_Ir* bir) {
    Elf64_Sym *psym;
    int fi;

    for (fi = 0; fi < bir->data_items.count; ++fi) {
        bir->data_items.items[fi].binary_offset = b->section_data.count;
        oc_array_append_many(&cc->arena, &b->section_data, bir->data_items.items[fi].ptr, bir->data_items.items[fi].len);
        oc_array_append(&cc->arena, &b->section_data, 0);
    }

    for (fi = 1; fi < bir->fns.count; ++fi) {
        LL_Ir_Function* fn = &bir->fns.items[fi];
        LL_Ir_Block_Ref block = fn->entry;
        if (fn->flags & LL_IR_FUNCTION_FLAG_EXTERN) continue;
        b->fn = fn;
        b->next_reg = 0;

        oc_array_reserve(&cc->tmp_arena, &b->locals, fn->locals.count);
        oc_array_reserve(&cc->tmp_arena, &b->registers, fn->registers.count);
        oc_array_reserve(&cc->tmp_arena, &b->register_collapses, fn->registers.count);

        uint64_t offset = 0;
        for (int li = 0; li < fn->locals.count; ++li) {
            LL_Backend_Layout l = linux_x86_64_elf_get_layout(fn->locals.items[li].ident->base.type);
            offset = oc_align_forward(offset + max(l.size, l.alignment), l.alignment);
            print("local {} layout {}, {}, off: {}\n", li, l.size, l.alignment, offset);
            b->locals.items[li] = offset;
        }
        b->stack_used = (uint32_t)offset;

        /* printf("function " FMT_SV_FMT ":\n", FMT_SV_ARG(fn->ident->str)); */

        size_t function_offset = b->current_section->count;
        OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_PUSH, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp }));
        OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_MOV, rm64_r64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp, .reg1 = X86_64_OPERAND_REGISTER_rsp }));
        OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_SUB, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = 0 }));
        size_t stack_size_offset = b->current_section->count - 4;
        fn->generated_offset = (int64_t)function_offset;

        // symbol could be valid in the case that it's used before defined
        if (fn->ident->symbol_index == CODE_IDENT_SYMBOL_INVALID) {
            fn->ident->symbol_index = (int32_t)b->symbols.count;
            oc_array_append(&cc->arena, &b->symbols, (Elf64_Sym){ 0 });
        }

        while (block) {
            linux_x86_64_elf_generate_block(cc, b, bir, &bir->blocks.items[block]);
            block = bir->blocks.items[block].next;
        }

        /* for (bi = 0; bi < b->registers.count; ++bi) { */
        /* 	printf("reg r%d -> %u\n", bi, b->registers.items[bi]); */
        /* } */

        uint32_t stack_used = oc_align_forward(b->stack_used, 16);
        int32_t* pstack_size = (int32_t*)&b->current_section->items[stack_size_offset];
        *pstack_size = stack_used;
        OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_ADD, rm64_i32, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rsp, .immediate = stack_used }));
        OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_POP, rm64, ((X86_64_Instruction_Parameters){ .reg0 = X86_64_OPERAND_REGISTER_rbp }));
        OC_X86_64_WRITE_INSTRUCTION(b, X86_64_OPCODE_RET, noarg, ((X86_64_Instruction_Parameters){ 0 }));

        psym = &b->symbols.items[fn->ident->symbol_index]; // must lookup again in case of realloc
                                                           
        psym->st_name = b->section_strtab.count;
        oc_array_append_many(&cc->arena, &b->section_strtab, fn->ident->str.ptr, fn->ident->str.len);
        oc_array_append(&cc->arena, &b->section_strtab, 0);

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

