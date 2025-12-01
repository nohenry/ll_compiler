
#include "ir.h"
#include "../src/common.h"
#include "../src/ast.h"
#include "../src/eval.h"

#define NEXTREG(type_) ir_get_next_reg(cc, b, (type_))
#define FUNCTION() ((b->current_function & CURRENT_CONST_STACK) ? (&b->const_stack.items[b->current_function & CURRENT_INDEX]) : (&b->fns.items[b->current_function & CURRENT_INDEX]))
#define BLOCK() (&b->blocks.items[b->current_block])

#define IR_APPEND_OP(opcode, ...) do { \
            LL_Ir_Operand _ops[] = {__VA_ARGS__}; \
            ir_append_op(cc, b, b->current_block, opcode, _ops, oc_len(_ops)); \
        } while(0)

#define IR_APPEND_OP_DST(opcode, type_, ...) ir_append_op_dst(cc, b, b->current_block, opcode, type_, ((LL_Ir_Operand[]){__VA_ARGS__}), oc_len(((LL_Ir_Operand[]){__VA_ARGS__})))

#define INDENT "    "

#define IR_INVALID_FUNCTION ((uint32_t)-1)

static LL_Ir_Operand ir_get_next_reg(Compiler_Context* cc, LL_Backend_Ir* b, LL_Type* type) {
    uint32_t reg = FUNCTION()->registers.count; 												\
    oc_array_append(&cc->arena, &FUNCTION()->registers, ((LL_Ir_Register){ .type = type })); 	\
    return LL_IR_OPERAND_REGISTER_BIT | reg; 															\
}

static void ir_append_op(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Block_Ref block, LL_Ir_Opcode opcode, LL_Ir_Operand* operands, size_t operands_count) {
    oc_array_append(&cc->arena, &b->blocks.items[block].ops, opcode);
    oc_array_append_many(&cc->arena, &b->blocks.items[block].ops, operands, operands_count);
}

static LL_Ir_Operand ir_append_op_dst(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Block_Ref block, LL_Ir_Opcode opcode, LL_Type* dst_type, LL_Ir_Operand* operands, size_t operands_count) {
    LL_Ir_Operand dst = NEXTREG(dst_type);
    oc_array_append(&cc->arena, &b->blocks.items[block].ops, opcode);
    oc_array_append(&cc->arena, &b->blocks.items[block].ops, dst);
    oc_array_append_many(&cc->arena, &b->blocks.items[block].ops, operands, operands_count);
    return dst;
}

size_t ir_get_op_count(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Opcode* opcode_list, size_t i) {
    (void)cc;
    (void)b;
    LL_Ir_Opcode opcode = opcode_list[i];

    switch (opcode) {
    case LL_IR_OPCODE_RET: return 1;
    case LL_IR_OPCODE_RETVALUE: return 2;
    case LL_IR_OPCODE_STORE: return 3;
    case LL_IR_OPCODE_MEMCOPY: return 4;
    case LL_IR_OPCODE_LOAD: return 3;
    case LL_IR_OPCODE_CAST: return 3;

    case LL_IR_OPCODE_SUB:
    case LL_IR_OPCODE_MUL:
    case LL_IR_OPCODE_DIV:
    case LL_IR_OPCODE_LT:
    case LL_IR_OPCODE_LTE:
    case LL_IR_OPCODE_GT:
    case LL_IR_OPCODE_GTE:
    case LL_IR_OPCODE_EQ:
    case LL_IR_OPCODE_NEQ:
    case LL_IR_OPCODE_AND:
    case LL_IR_OPCODE_OR:
    case LL_IR_OPCODE_XOR:
    case LL_IR_OPCODE_ADD: return 4;
    case LL_IR_OPCODE_NEG: return 3;

    case LL_IR_OPCODE_BRANCH: return 2;
    case LL_IR_OPCODE_BRANCH_COND: return 4;

    case LL_IR_OPCODE_LEA: return 3;
    case LL_IR_OPCODE_LEA_INDEX: return 5;
    case LL_IR_OPCODE_INVOKEVALUE: {
        uint32_t count = opcode_list[i + 1 + 2];
        return 4 + count;
    }
    case LL_IR_OPCODE_INVOKE: {
        uint32_t count = opcode_list[i + 1 + 1];
        return 3 + count;
    }
    default: oc_unreachable(""); return 1;
    }
}

static void ir_gen_reverse_ops(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Block_Ref block_ref) {
    LL_Ir_Block* block = &b->blocks.items[block_ref];
    oc_array_reserve(&cc->arena, &block->rops, block->ops.count);

    size_t i;
    for (i = 0; i < block->ops.count;) {
        size_t count = ir_get_op_count(cc, b, block->ops.items, i);
        size_t dst_i = block->ops.count - i - count;
        memcpy(&block->rops.items[dst_i], &block->ops.items[i], sizeof(block->ops.items[i]) * count);
        i += count;
    }
}

void ir_print_op(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Opcode* opcode_list, size_t i, Oc_Writer* w) {
    (void)cc;
    (void)b;
    LL_Ir_Opcode opcode = opcode_list[i];
    LL_Ir_Operand* operands = &opcode_list[i + 1];
    int offset = 0;

    switch (opcode) {
    case LL_IR_OPCODE_RET:         wprint(w, INDENT "ret"); break;
    case LL_IR_OPCODE_RETVALUE:    wprint(w, INDENT "ret " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0])); break;
    case LL_IR_OPCODE_STORE:       wprint(w, INDENT "store " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1])); break;
    case LL_IR_OPCODE_MEMCOPY:     wprint(w, INDENT "memcpy " OPERAND_FMT ", " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_LOAD:        wprint(w, INDENT OPERAND_FMT " = load " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1])); break;
    case LL_IR_OPCODE_LEA:         wprint(w, INDENT OPERAND_FMT " = lea " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1])); break;
    case LL_IR_OPCODE_LEA_INDEX:   wprint(w, INDENT OPERAND_FMT " = lea " OPERAND_FMT " + " OPERAND_FMT " * " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2]), OPERAND_FMT_VALUE(operands[3])); break;
    case LL_IR_OPCODE_CAST:        wprint(w, INDENT OPERAND_FMT " = cast " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1])); break;

    case LL_IR_OPCODE_ADD:         wprint(w, INDENT OPERAND_FMT " = add " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_SUB:         wprint(w, INDENT OPERAND_FMT " = sub " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_MUL:         wprint(w, INDENT OPERAND_FMT " = mul " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_DIV:         wprint(w, INDENT OPERAND_FMT " = div " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_LT:          wprint(w, INDENT OPERAND_FMT " = lt " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_LTE:         wprint(w, INDENT OPERAND_FMT " = lte " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_GT:          wprint(w, INDENT OPERAND_FMT " = gt " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_GTE:         wprint(w, INDENT OPERAND_FMT " = gte " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_EQ:          wprint(w, INDENT OPERAND_FMT " = eq " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_NEQ:         wprint(w, INDENT OPERAND_FMT " = neq " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_NEG:         wprint(w, INDENT OPERAND_FMT " = neg " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1])); break;
    case LL_IR_OPCODE_AND:         wprint(w, INDENT OPERAND_FMT " = and " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_OR:          wprint(w, INDENT OPERAND_FMT " = or " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;
    case LL_IR_OPCODE_XOR:         wprint(w, INDENT OPERAND_FMT " = xor " OPERAND_FMT ", " OPERAND_FMT, OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]), OPERAND_FMT_VALUE(operands[2])); break;

    case LL_IR_OPCODE_BRANCH:      wprint(w, INDENT "branch babs{}", operands[0] & LL_IR_OPERAND_VALUE_MASK); break;
    case LL_IR_OPCODE_BRANCH_COND: wprint(w, INDENT "branch_cond " OPERAND_FMT ", babs{}, babs{}", OPERAND_FMT_VALUE(operands[0]), operands[1] & LL_IR_OPERAND_VALUE_MASK, operands[2] & LL_IR_OPERAND_VALUE_MASK); break;

    case LL_IR_OPCODE_INVOKEVALUE:
        offset = 1;
    case LL_IR_OPCODE_INVOKE: {
        uint32_t count = operands[1 + offset];
        if (opcode == LL_IR_OPCODE_INVOKEVALUE) {
            wprint(w, INDENT OPERAND_FMT " = call " OPERAND_FMT " ", OPERAND_FMT_VALUE(operands[0]), OPERAND_FMT_VALUE(operands[1]));
        } else {
            wprint(w, INDENT "call " OPERAND_FMT " ", OPERAND_FMT_VALUE(operands[0]));
        }
        for (uint32_t j = 0; j < count; ++j) {
            if (j > 0) wprint(w, ", ");
            wprint(w, OPERAND_FMT, OPERAND_FMT_VALUE(operands[2 + offset + j]));
        }
        break;
    }
    }
}

static void ir_print_block(Compiler_Context* cc, LL_Backend_Ir* b, LL_Ir_Block* block) {
    size_t i;
    oc_hex_dump(block->ops.items, block->ops.count * sizeof(*block->ops.items), 0, -1);
    for (i = 0; i < block->ops.count; ) {
        ir_print_op(cc, b, block->ops.items, i, &stdout_writer);
        print("\n");
        i += ir_get_op_count(cc, b, block->ops.items, i);
    }
}

static void ir_print(Compiler_Context* cc, LL_Backend_Ir* b, Oc_Writer* w) {
    uint32_t fi;
    for (fi = 0; fi < b->fns.count; ++fi) {
        LL_Ir_Function* fn = &b->fns.items[fi];
        LL_Ir_Block_Ref block = fn->entry;

        if (fn->ident) {
            wprint(w, "function {}({}):\n", fn->ident->str, fi);
        } else {
            wprint(w, "function({}):\n", fi);
        }

        int bi = 0;
        while (block) {
            wprint(w, "b{}(abs{}):\n", bi, block);
            ir_print_block(cc, b, &b->blocks.items[block]);
            bi++;
            block = b->blocks.items[block].next;
        }
        wprint(w, "\n");
    }
}

void ir_init(Compiler_Context* cc, LL_Backend_Ir* b) {
    memset(b, 0, sizeof(*b));
    b->current_function = IR_INVALID_FUNCTION;
    // first function is used for const eval
    oc_array_append(&cc->arena, &b->fns, ((LL_Ir_Function){}));
    oc_array_append(&cc->arena, &b->blocks, ((LL_Ir_Block){}));
}

// void ir_insert_native_fn(Compiler_Context* cc, LL_Backend_Ir* b, const char* name, void *fn_ptr) {
//     string name_str = oc_sprintf(&cc->arena, "%s", name);
//     Ast_Ident ident = {
//         .base.kind = name_str,
//         .str = name_str, .symbol_index = AST_IDENT_SYMBOL_INVALID,
//     };
//     oc_arena_dup(cc, b);

//     LL_Ir_Function fn = {
//         .ident = fn_decl->ident,
//         .entry = entry_block_ref,
//         .exit = entry_block_ref,
//         .flags = 0,
//         .generated_offset = LL_IR_FUNCTION_OFFSET_INVALID,
//         .block_count = 1,
//     };

//     fn_decl->ir_index = b->fns.count;
// }

bool ir_write_to_file(Compiler_Context* cc, LL_Backend_Ir* b, char* filepath) {
    (void)filepath;
    ir_print(cc, b, &stdout_writer);
    return false;
}

LL_Ir_Block_Ref ir_create_block(Compiler_Context* cc, LL_Backend_Ir* b, bool append) {
    LL_Ir_Block block;
    LL_Ir_Block_Ref result = b->free_block ? b->free_block : b->blocks.count;

    memset(&block.ops, 0, sizeof(block.ops));
    memset(&block.rops, 0, sizeof(block.rops));
    block.did_branch = false;
    block.bi = FUNCTION()->block_count;
    FUNCTION()->block_count++; 
    block.ref1 = block.ref2 = 0;
    block.generated_offset = -1;

    block.next = 0;
    if (append) {
        block.prev = FUNCTION()->exit;
        b->blocks.items[FUNCTION()->exit].next = result;
        FUNCTION()->exit = result;
    } else {
        block.prev = 0;
    }

    if (b->free_block) {
        b->free_block = b->blocks.items[b->free_block].next;
        memcpy(&b->blocks.items[b->free_block], &block, sizeof(block));
    } else {
        oc_array_append(&cc->arena, &b->blocks, block);
    }

    return result;
}

LL_Ir_Operand ir_generate_cast_if_needed(Compiler_Context* cc, LL_Backend_Ir* b, LL_Type* to_type, LL_Ir_Operand from, LL_Type* from_type) {
    if (to_type == from_type) return from;

    switch (from_type->kind) {
    case LL_TYPE_INT:
        switch (to_type->kind) {
            case LL_TYPE_INT:
            case LL_TYPE_UINT:
                if (from_type->width == to_type->width) return from;
                break;
            default: oc_todo("handle cast types to {}\n", (int)to_type->kind); break;
        }
        break;
    case LL_TYPE_UINT:
        switch (to_type->kind) {
            case LL_TYPE_UINT:
            case LL_TYPE_INT:
                if (from_type->width == to_type->width) return from;
                break;
            default: oc_todo("handle cast types to {}\n", (int)to_type->kind); break;
        }
        break;
    default: oc_todo("handle cast types from\n"); return from;
    }

    return IR_APPEND_OP_DST(LL_IR_OPCODE_CAST, to_type, from);
}

LL_Ir_Operand ir_generate_lhs_load_if_needed(Compiler_Context* cc, LL_Backend_Ir* b, LL_Type* type, LL_Ir_Operand value) {
    switch (LL_IR_OPERAND_TYPE_MASK & value) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
        if (b->flags & LL_BACKEND_IR_FLAG_LHS_IMMEDIATE) return value;
        return IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, type, value);
    default: return value;
    }
}

LL_Ir_Operand ir_generate_rhs_load_if_needed(Compiler_Context* cc, LL_Backend_Ir* b, LL_Type* type, LL_Ir_Operand value) {
    switch (LL_IR_OPERAND_TYPE_MASK & value) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
        if (b->flags & LL_BACKEND_IR_FLAG_RHS_IMMEDIATE) return value;
        return IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, type, value);
    default: return value;
    }
}

void ir_generate_statement(Compiler_Context* cc, LL_Backend_Ir* b, Ast_Base* stmt) {
    uint32_t i;
    switch (stmt->kind) {
    case AST_KIND_BLOCK:
        for (i = 0; i < AST_AS(stmt, Ast_Block)->count; ++i) {
            ir_generate_statement(cc, b, AST_AS(stmt, Ast_Block)->items[i]);
        }
        break;
    case AST_KIND_VARIABLE_DECLARATION: {
        Ast_Variable_Declaration* var_decl = AST_AS(stmt, Ast_Variable_Declaration);
        if (var_decl->storage_class & LL_STORAGE_CLASS_EXTERN) break;
        oc_assert(b->current_function != IR_INVALID_FUNCTION);

        LL_Ir_Local var = {
            .ident = var_decl->ident,
        };
        var_decl->ir_index = FUNCTION()->locals.count;
        oc_array_append(&cc->arena, &FUNCTION()->locals, var);

        if (var_decl->initializer) {
            LL_Ir_Operand last_copy_operand = b->copy_operand;
            b->copy_operand = LL_IR_OPERAND_LOCAL_BIT | var_decl->ir_index;
            LL_Ir_Operand op = ir_generate_expression(cc, b, var_decl->initializer, false);
            b->copy_operand = last_copy_operand;

            switch (var_decl->ident->base.type->kind) {
            case LL_TYPE_ARRAY:
                if (op != (uint32_t)-1) {
                    LL_Backend_Layout layout = cc->target->get_layout(var_decl->ident->base.type);
                    IR_APPEND_OP(LL_IR_OPCODE_MEMCOPY, LL_IR_OPERAND_LOCAL_BIT | var_decl->ir_index, op, layout.size);
                }
                break;
            default:
                // if (var_decl->initializer->type != var_decl->type->type) {
                //     op = IR_APPEND_OP_DST(LL_IR_OPCODE_CAST, var_decl->type->type, op);
                // }
                IR_APPEND_OP(LL_IR_OPCODE_STORE, LL_IR_OPERAND_LOCAL_BIT | var_decl->ir_index, op);
                break;
            }
        }

        break;
    }
    case AST_KIND_FUNCTION_DECLARATION: {
        Ast_Function_Declaration* fn_decl = AST_AS(stmt, Ast_Function_Declaration);

        LL_Ir_Block_Ref entry_block_ref = b->blocks.count;
        LL_Ir_Block entry_block = { 0 };
        entry_block.generated_offset = -1;
        oc_array_append(&cc->arena, &b->blocks, entry_block);

        LL_Ir_Function fn = {
            .ident = fn_decl->ident,
            .entry = entry_block_ref,
            .exit = entry_block_ref,
            .flags = 0,
            .generated_offset = LL_IR_FUNCTION_OFFSET_INVALID,
            .block_count = 1,
        };

        fn_decl->ir_index = b->fns.count;

        if (fn_decl->storage_class & LL_STORAGE_CLASS_EXTERN) {
            fn.flags |= LL_IR_FUNCTION_FLAG_EXTERN;
        }
        if (fn_decl->storage_class & LL_STORAGE_CLASS_NATIVE) {
            fn.flags |= LL_IR_FUNCTION_FLAG_NATIVE;
        }
        oc_array_append(&cc->arena, &b->fns, fn);

        if (fn_decl->body) {
            int32_t last_function = b->current_function;
            LL_Ir_Block_Ref last_block = b->current_block;
            b->current_function = fn_decl->ir_index;
            b->current_block = fn.entry;

            ir_generate_statement(cc, b, fn_decl->body);

            if (!b->blocks.items[b->current_block].did_branch) {
                b->current_block = FUNCTION()->exit;
                IR_APPEND_OP(LL_IR_OPCODE_RET);
            }

            LL_Ir_Block_Ref next_block = FUNCTION()->exit;
            while (next_block) {
                ir_gen_reverse_ops(cc, b, next_block);
                next_block = b->blocks.items[next_block].prev;
            }

            b->current_block = last_block;
            b->current_function = last_function;
        }
        break;
    }
    default:
        ir_generate_expression(cc, b, stmt, false);
        break;
    }
}

LL_Ir_Operand ir_generate_expression(Compiler_Context* cc, LL_Backend_Ir* b, Ast_Base* expr, bool lvalue) {
    LL_Ir_Operand result = 696969;
    LL_Ir_Opcode op, r1, r2;
    uint32_t i;

    switch (expr->kind) {
    case AST_KIND_BLOCK: {
        LL_Ir_Operand last_block_value = b->block_value;

        Ast_Ident* block_ident = oc_arena_alloc(&cc->arena, sizeof(Ast_Ident));
        block_ident->base.type = expr->type;
        block_ident->str = oc_sprintf(&cc->arena, "block_result\n");
        LL_Ir_Local var = {
            .ident = block_ident,
        };
        uint32_t index = FUNCTION()->locals.count;
        oc_array_append(&cc->arena, &FUNCTION()->locals, var);
        b->block_value = LL_IR_OPERAND_LOCAL_BIT | index;

        for (i = 0; i < AST_AS(expr, Ast_Block)->count; ++i) {
            ir_generate_expression(cc, b, AST_AS(expr, Ast_Block)->items[i], false);
        }

        result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, expr->type, b->block_value);
        b->block_value = last_block_value;
        break;
    }
    case AST_KIND_LITERAL_INT:
        if (AST_AS(expr, Ast_Literal)->i64 >= 0 && AST_AS(expr, Ast_Literal)->i64 <= 0xFFFFFFF) {
            return AST_AS(expr, Ast_Literal)->i64;
        } else {
            eprint("oc_todo: implement bigger ints\n");
        }
        oc_assert(!lvalue);
        break;
    case AST_KIND_LITERAL_STRING: {
        Ast_Literal* lit = AST_AS(expr, Ast_Literal);
        oc_assert((b->data_items.count & 0xF0000000u) == 0); // oc_todo: maybe support more
        result = LL_IR_OPERAND_DATA_BIT | (uint32_t)b->data_items.count;
        oc_array_append(&cc->arena, &b->data_items, ((LL_Ir_Data_Item) { .ptr = lit->str.ptr, .len = lit->str.len }));
        result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA, expr->type, result);
        break;
    }
    case AST_KIND_PRE_OP:
        switch (AST_AS(expr, Ast_Operation)->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '-': {
            result = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
            result = IR_APPEND_OP_DST(LL_IR_OPCODE_NEG, expr->type, result);
            break;
        }
        case '*': {
            result = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
            if (!lvalue) {
                result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, expr->type, result);
            }
            break;
        }
        case '&': {
            result = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, true);
            if (AST_AS(expr, Ast_Operation)->right->kind != AST_KIND_INDEX) {
                result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA, expr->type, result);
            }
            break;
        }
#pragma GCC diagnostic push
        default: break;
        }

        break;
    case AST_KIND_BINARY_OP:
        switch (AST_AS(expr, Ast_Operation)->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
		case '.': {
            Ast_Operation* opr = AST_AS(expr, Ast_Operation);
            Ast_Ident* right_ident = AST_AS(opr->right, Ast_Ident);

            Ast_Base* decl = right_ident->resolved_scope->decl;
            if (!decl) return 0;

            switch (decl->kind) {
            case AST_KIND_FUNCTION_DECLARATION: result = LL_IR_OPERAND_FUNCTION_BIT | AST_AS(decl, Ast_Function_Declaration)->ir_index; break;
            default: oc_assert(false);
            }

            if (!lvalue) {
                result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, right_ident->base.type, result);
            }

            return result;
        } break;
        case '+': op = LL_IR_OPCODE_ADD; break;
        case '-': op = LL_IR_OPCODE_SUB; break;
        case '*': op = LL_IR_OPCODE_MUL; break;
        case '/': op = LL_IR_OPCODE_DIV; break;

        case '<':
            op = LL_IR_OPCODE_LT;
            goto DO_BIN_OP_BOOLEAN;
        case LL_TOKEN_KIND_LTE:
            op = LL_IR_OPCODE_LTE;
            goto DO_BIN_OP_BOOLEAN;
        case '>':
            op = LL_IR_OPCODE_GT;
            goto DO_BIN_OP_BOOLEAN;
        case LL_TOKEN_KIND_GTE:
            op = LL_IR_OPCODE_GTE;
            goto DO_BIN_OP_BOOLEAN;
        case LL_TOKEN_KIND_EQUALS:
               op = LL_IR_OPCODE_EQ;
            goto DO_BIN_OP_BOOLEAN;
        case LL_TOKEN_KIND_NEQUALS:
            op = LL_IR_OPCODE_NEQ;
            goto DO_BIN_OP_BOOLEAN;
DO_BIN_OP_BOOLEAN:
            r2 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
            r2 = ir_generate_lhs_load_if_needed(cc, b, AST_AS(expr, Ast_Operation)->right->type, r2);

            r1 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->left, false);
            r1 = ir_generate_lhs_load_if_needed(cc, b, AST_AS(expr, Ast_Operation)->left->type, r1);

            result = IR_APPEND_OP_DST(op, expr->type, r1, r2);
            return result;

        case LL_TOKEN_KIND_ASSIGN_PERCENT:
            op = LL_IR_OPCODE_DIV;
            goto DO_BIN_OP_ASSIGN_OP;
        case LL_TOKEN_KIND_ASSIGN_DIVIDE:
            op = LL_IR_OPCODE_DIV;
            goto DO_BIN_OP_ASSIGN_OP;
        case LL_TOKEN_KIND_ASSIGN_TIMES:
            op = LL_IR_OPCODE_MUL;
            goto DO_BIN_OP_ASSIGN_OP;
        case LL_TOKEN_KIND_ASSIGN_MINUS:
            op = LL_IR_OPCODE_SUB;
            goto DO_BIN_OP_ASSIGN_OP;
        case LL_TOKEN_KIND_ASSIGN_PLUS:
            op = LL_IR_OPCODE_ADD;
DO_BIN_OP_ASSIGN_OP:
            r2 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
            r2 = ir_generate_cast_if_needed(cc, b, expr->type, r2, AST_AS(expr, Ast_Operation)->right->type);

            r1 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->left, false);
            r1 = ir_generate_cast_if_needed(cc, b, expr->type, r1, AST_AS(expr, Ast_Operation)->left->type);
            r1 = ir_generate_lhs_load_if_needed(cc, b, expr->type, r1);

            r1 = IR_APPEND_OP_DST(op, expr->type, r1, r2);

            result = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->left, true);
            IR_APPEND_OP(LL_IR_OPCODE_STORE, result, r1);
            return r1;
        case '=':
            result = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->left, true);
            r2 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
            r2 = ir_generate_cast_if_needed(cc, b, expr->type, r2, AST_AS(expr, Ast_Operation)->right->type);
            IR_APPEND_OP(LL_IR_OPCODE_STORE, result, r2);
            return r2;

#pragma GCC diagnostic pop
        default: oc_assert(false);
        }

        r1 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->left, false);
        r1 = ir_generate_cast_if_needed(cc, b, expr->type, r1, AST_AS(expr, Ast_Operation)->left->type);
        r1 = ir_generate_lhs_load_if_needed(cc, b, expr->type, r1);

        r2 = ir_generate_expression(cc, b, AST_AS(expr, Ast_Operation)->right, false);
        r2 = ir_generate_cast_if_needed(cc, b, expr->type, r2, AST_AS(expr, Ast_Operation)->right->type);
        r2 = ir_generate_lhs_load_if_needed(cc, b, expr->type, r2);

        result = IR_APPEND_OP_DST(op, expr->type, r1, r2);
        break;
    case AST_KIND_CAST: {
        Ast_Cast* cast = AST_AS(expr, Ast_Cast);
        result = ir_generate_expression(cc, b, cast->expr, false);
        result = IR_APPEND_OP_DST(LL_IR_OPCODE_CAST, cast->base.type, result);
        break;
    }
    case AST_KIND_INVOKE: {
        oc_assert(!lvalue);
        Ast_Invoke* inv = AST_AS(expr, Ast_Invoke);

        LL_Ir_Operand invokee = ir_generate_expression(cc, b, inv->expr, true);
        LL_Type_Function* fn_type = (LL_Type_Function*)inv->expr->type;

        LL_Ir_Operand ops[3 + inv->ordered_arguments.count];

        uword offset = 0;
        LL_Ir_Operand opcode;
        if (fn_type->return_type && fn_type->return_type->kind != LL_TYPE_VOID) {
            opcode = LL_IR_OPCODE_INVOKEVALUE;
            result = NEXTREG(fn_type->return_type);
            ops[offset++] = result;
        } else {
            opcode = LL_IR_OPCODE_INVOKE;
        }

        ops[offset++] = invokee;
        ops[offset++] = inv->ordered_arguments.count;
        for (i = 0; i < inv->ordered_arguments.count; ++i) {
            LL_Type* parameter_type;
            if (i >= fn_type->parameter_count - 1 && fn_type->is_variadic) {
                parameter_type = cc->typer->ty_int32;
            } else {
                parameter_type = inv->ordered_arguments.items[i]->type;
            }

            LL_Ir_Operand arg_operand = ir_generate_expression(cc, b, inv->ordered_arguments.items[i], false);
            switch (arg_operand & LL_IR_OPERAND_TYPE_MASK) {
            case LL_IR_OPERAND_IMMEDIATE_BIT:
                arg_operand = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, parameter_type, (arg_operand & LL_IR_OPERAND_VALUE_MASK));
                break;
            default: break;
            }

            ops[offset++] = arg_operand;
        }

        ir_append_op(cc, b, b->current_block, opcode, ops, offset);

        break;
    }
    case AST_KIND_ARRAY_INITIALIZER: {
        Ast_Initializer* lit = AST_AS(expr, Ast_Initializer);
        /* oc_assert((b->data_items.count & 0xF0000000u) == 0); // oc_todo: maybe support more */
        LL_Type* element_type = ((LL_Type_Array*)lit->base.type)->element_type;
        LL_Backend_Layout layout = cc->target->get_layout(element_type);
        size_t size = max(layout.size, layout.alignment);

        uint8_t* last_initializer_ptr = b->initializer_ptr;
        uint8_t* initializer_ptr;
        if (!last_initializer_ptr) {
            initializer_ptr = oc_arena_alloc(&cc->arena, lit->base.type->width * size);
            b->initializer_ptr = initializer_ptr;

            result = LL_IR_OPERAND_DATA_BIT | (uint32_t)b->data_items.count;
            IR_APPEND_OP(LL_IR_OPCODE_MEMCOPY, b->copy_operand, result, (uint32_t)(lit->base.type->width * size));
        }

        uint64_t k;
        for (i = 0, k = 0; i < lit->count; ++i, ++k) {
            if (lit->items[i]->kind == AST_KIND_KEY_VALUE) {
                Ast_Key_Value* kv = AST_AS(lit->items[i], Ast_Key_Value);
                LL_Ir_Operand vvalue = ir_generate_expression(cc, b, kv->value, false);
                if (kv->key->has_const && kv->value->has_const) {

                    if (layout.size <= 8) {
                        b->initializer_ptr[kv->key->const_value.uval] = (uint8_t)kv->value->const_value.uval;
                    } else if (layout.size <= 16) {
                        ((uint16_t*)b->initializer_ptr)[kv->key->const_value.uval] = (uint16_t)kv->value->const_value.uval;
                    } else if (layout.size <= 32) {
                        ((uint32_t*)b->initializer_ptr)[kv->key->const_value.uval] = (uint32_t)kv->value->const_value.uval;
                    } else if (layout.size <= 64) {
                        ((uint64_t*)b->initializer_ptr)[kv->key->const_value.uval] = (uint64_t)kv->value->const_value.uval;
                    } else {
                        oc_todo("implement other const size");
                        /* memcpy(&b->initializer_ptr[kv->key->const_value.uval * layout.size], &kv->value->const_value.uval, sizeof(kv->value->const_value.uval)); */
                    }
                    k = kv->key->const_value.uval;
                } else {
                    LL_Ir_Operand kvalue = ir_generate_expression(cc, b, kv->key, false);
                    result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA_INDEX, element_type, b->copy_operand, kvalue, layout.size);
                    IR_APPEND_OP(LL_IR_OPCODE_STORE, result, vvalue);
                }
            } else {
                LL_Ir_Operand vvalue = ir_generate_expression(cc, b, lit->items[i], false);
                switch (lit->items[i]->type->kind) {
                case LL_TYPE_ARRAY:
                    b->initializer_ptr += size;
                    break;
                default:
                    if (lit->items[i]->has_const) {
                        if (layout.size <= 1) {
                            b->initializer_ptr[k] = (uint8_t)lit->items[i]->const_value.uval;
                        } else if (layout.size <= 2) {
                            ((uint16_t*)b->initializer_ptr)[k] = (uint16_t)lit->items[i]->const_value.uval;
                        } else if (layout.size <= 4) {
                            ((uint32_t*)b->initializer_ptr)[k] = (uint32_t)lit->items[i]->const_value.uval;
                        } else if (layout.size <= 8) {
                            ((uint64_t*)b->initializer_ptr)[k] = (uint64_t)lit->items[i]->const_value.uval;
                        } else {
                            oc_todo("implement other const size");
                            /* memcpy(&b->initializer_ptr[kv->key->const_value.uval * layout.size], &kv->value->const_value.uval, sizeof(kv->value->const_value.uval)); */
                        }
                    } else {
                        result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA_INDEX, element_type, b->copy_operand, (uint32_t)k, layout.size);
                        IR_APPEND_OP(LL_IR_OPCODE_STORE, result, vvalue);
                    }
                    break;
                }
            }
        }

        /* void* data_ptr = oc_arena_alloc(&cc->arena, layout.size); */

        if (!last_initializer_ptr) {
            oc_array_append(&cc->arena, &b->data_items, ((LL_Ir_Data_Item) { .ptr = initializer_ptr, .len = lit->base.type->width * size }));
            b->initializer_ptr = last_initializer_ptr;
        }

        result = (uint32_t)-1;
        /* result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA, expr->type, result); */
        break;
    }
    case AST_KIND_INDEX: {
        Ast_Operation* op = AST_AS(expr, Ast_Operation);

        LL_Ir_Operand lvalue_op;
        if (op->left->type->kind == LL_TYPE_POINTER) {
            print("doing pointer\n");
            lvalue_op = ir_generate_expression(cc, b, op->left, false);
        } else {
            lvalue_op = ir_generate_expression(cc, b, op->left, true);
        }
        LL_Ir_Operand rvalue_op = ir_generate_expression(cc, b, op->right, false);
        LL_Backend_Layout layout = cc->target->get_layout(expr->type);

        result = IR_APPEND_OP_DST(LL_IR_OPCODE_LEA_INDEX, expr->type, lvalue_op, rvalue_op, max(layout.size, layout.alignment));

        if (!lvalue && expr->type->kind != LL_TYPE_ARRAY) {
            result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, expr->type, result);
        }

        break;
    }
    case AST_KIND_IDENT: {
        Ast_Ident* ident = AST_AS(expr, Ast_Ident);

        if (AST_AS(expr, Ast_Ident)->str.ptr == LL_KEYWORD_TRUE.ptr) {
            oc_assert(!lvalue);
            result = LL_IR_OPERAND_IMMEDIATE_BIT | 1u;
            break;
        } else if (AST_AS(expr, Ast_Ident)->str.ptr == LL_KEYWORD_FALSE.ptr) {
            oc_assert(!lvalue);
            result = LL_IR_OPERAND_IMMEDIATE_BIT | 0u;
            break;
        }

        Ast_Base* decl = ident->resolved_scope->decl;
        switch (decl->kind) {
        case AST_KIND_VARIABLE_DECLARATION: result = LL_IR_OPERAND_LOCAL_BIT | AST_AS(decl, Ast_Variable_Declaration)->ir_index; break;
        case AST_KIND_FUNCTION_DECLARATION: result = LL_IR_OPERAND_FUNCTION_BIT | AST_AS(decl, Ast_Function_Declaration)->ir_index; break;
        case AST_KIND_PARAMETER: result = LL_IR_OPERAND_PARMAETER_BIT | AST_AS(decl, Ast_Parameter)->ir_index; break;
        default: oc_assert(false);
        }

        if (!lvalue) {
            result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, ident->base.type, result);
        }
        
        break;
    }
    case AST_KIND_CONST: {
        Ast_Marker* cf = AST_AS(expr, Ast_Marker);
        LL_Eval_Value value = ll_eval_node(cc, cc->eval_context, b, cf->expr);
        result = value.uval;
        break;
    }
    case AST_KIND_RETURN: {
        Ast_Control_Flow* cf = AST_AS(expr, Ast_Control_Flow);
        if (cf->expr) {
            result = ir_generate_expression(cc, b, cf->expr, false);
            IR_APPEND_OP(LL_IR_OPCODE_RETVALUE, result);
        } else {
            IR_APPEND_OP(LL_IR_OPCODE_RET);
        }
        BLOCK()->did_branch = true;
        return 0;
    }
    case AST_KIND_BREAK: {
        Ast_Control_Flow* cf = AST_AS(expr, Ast_Control_Flow);
        if (cf->expr) {
            result = ir_generate_expression(cc, b, cf->expr, false);

            IR_APPEND_OP(LL_IR_OPCODE_STORE, b->block_value, result);
            /* if (result == LL_IR_OPERAND_REGISTER_BIT) { */
            /* 	b->block_value = result; */
            /* } else { */
            /* 	/1* b->block_value = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, cf->expr->type, result); *1/ */
            /* 	b->block_value = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, cf->expr->type, result); */
            /* } */
        }
        return 0;
    }
    case AST_KIND_IF: {
        Ast_If* iff = AST_AS(expr, Ast_If);
        LL_Ir_Block_Ref body_block = ir_create_block(cc, b, true);
        LL_Ir_Block_Ref else_block = ir_create_block(cc, b, true);
        LL_Ir_Block_Ref end_block = iff->else_clause ? ir_create_block(cc, b, true) : else_block;

        b->blocks.items[body_block].ref1 = b->current_block;
        b->blocks.items[end_block].ref1 = body_block;
        if (else_block != end_block) {
            b->blocks.items[else_block].ref1 = b->current_block;
            b->blocks.items[end_block].ref2 = else_block;
        }

        result = ir_generate_expression(cc, b, iff->cond, false);
        if ((result & LL_IR_OPERAND_TYPE_MASK) == LL_IR_OPERAND_IMMEDIATE_BIT) {
            result = IR_APPEND_OP_DST(LL_IR_OPCODE_LOAD, cc->typer->ty_bool, result);
            result = IR_APPEND_OP_DST(LL_IR_OPCODE_NEQ, cc->typer->ty_bool, result, 0);
        } else if (ir_get_operand_type(FUNCTION(), result)->kind != LL_TYPE_BOOL) {
            result = IR_APPEND_OP_DST(LL_IR_OPCODE_NEQ, cc->typer->ty_bool, result, 0);
        }
        IR_APPEND_OP(LL_IR_OPCODE_BRANCH_COND, result, body_block, else_block);

        if (iff->body) {
            b->current_block = body_block;
            ir_generate_statement(cc, b, iff->body);
            IR_APPEND_OP(LL_IR_OPCODE_BRANCH, end_block);
        }

        if (iff->else_clause) {
            b->current_block = else_block;
            ir_generate_statement(cc, b, iff->else_clause);
            IR_APPEND_OP(LL_IR_OPCODE_BRANCH, end_block);
        }

        b->current_block = end_block;

        return 0;
    }
    case AST_KIND_FOR: {
        Ast_Loop* loop = AST_AS(expr, Ast_Loop);
        LL_Ir_Block_Ref cond_block = ir_create_block(cc, b, true);
        LL_Ir_Block_Ref body_block = ir_create_block(cc, b, true);
        LL_Ir_Block_Ref end_block = ir_create_block(cc, b, true);
        b->blocks.items[end_block].ref1 = cond_block;
        /* cond_block->ref1 = body_block; */

        /* LL_Ir_Block* end_block = ir_create_block(cc, b, true); */

        if (loop->init) ir_generate_statement(cc, b, loop->init);

        if (loop->cond) {
            b->current_block = cond_block;
            result = ir_generate_expression(cc, b, loop->cond, false);
            IR_APPEND_OP(LL_IR_OPCODE_BRANCH_COND, result, body_block, end_block);
        }

        b->current_block = body_block;
        if (loop->body) {
            ir_generate_statement(cc, b, loop->body);
        }

        if (loop->update) {
            ir_generate_expression(cc, b, loop->update, false);
            IR_APPEND_OP(LL_IR_OPCODE_BRANCH, cond_block);
        }

        b->current_block = end_block;

        /* if (loop->cond) { */
        /* 	result = ll_typer_type_expression(cc, typer, loop->cond, typer->ty_int32); */
        /* 	switch (result->kind) { */
        /* 	case LL_TYPE_POINTER: */
        /* 	case LL_TYPE_ANYINT: */
        /* 	case LL_TYPE_UINT: */
        /* 	case LL_TYPE_INT: break; */
        /* 	default: */
        /* 		eprint("\x1b[31;1merror:\x1b[0m if statement condition should be boolean, integer or pointer\n"); */
        /* 		break; */
        /* 	} */
        /* } */
        /* if (loop->update) ll_typer_type_expression(cc, typer, loop->update, NULL); */

        /* if (loop->body) { */
        /* 	ll_typer_type_statement(cc, typer, loop->body); */
        /* } */

        return 0;
    }
    default:
        eprint("oc_todo: implement generate expr %d\n", expr->kind);
        break;
    }
    return result;
}

LL_Type* ir_get_operand_type(LL_Ir_Function* fn, LL_Ir_Operand operand) {
    switch (operand & LL_IR_OPERAND_TYPE_MASK) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
        oc_assert(false);
        return NULL;
    case LL_IR_OPERAND_REGISTER_BIT:
        return fn->registers.items[operand & LL_IR_OPERAND_VALUE_MASK].type;
    case LL_IR_OPERAND_LOCAL_BIT:
        return fn->locals.items[operand & LL_IR_OPERAND_VALUE_MASK].ident->base.type;
    case LL_IR_OPERAND_PARMAETER_BIT: {
        LL_Type_Function* fn_type = (LL_Type_Function*)fn->ident->base.type;
        return fn_type->parameters[operand & LL_IR_OPERAND_VALUE_MASK];
    }
    case LL_IR_OPERAND_FUNCTION_BIT:
        return fn->ident->base.type;
    default: oc_assert(false);
    }
}

