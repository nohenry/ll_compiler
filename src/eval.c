
#include "eval.h"
#include "../backends/ir.h"

#define FUNCTION() (&bir->const_stack.items[bir->current_function & CURRENT_INDEX])

static void ll_eval_set_value(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, LL_Ir_Operand lvalue, LL_Eval_Value rvalue) {
    LL_Type* type;
    type = ir_get_operand_type(FUNCTION(), lvalue);

    switch (OPD_TYPE(lvalue)) {
    case LL_IR_OPERAND_LOCAL_BIT: {
        switch (type->kind) {
        case LL_TYPE_UINT:
            b->locals.items[OPD_VALUE(lvalue)].uval = rvalue.uval;
            break;
        case LL_TYPE_INT:
            b->locals.items[OPD_VALUE(lvalue)].ival = rvalue.ival;
            break;
        default: oc_assert(false); break;
        }

        break;
    }
    case LL_IR_OPERAND_REGISTER_BIT: {
        switch (type->kind) {
        case LL_TYPE_UINT:
            b->registers.items[OPD_VALUE(lvalue)].uval = rvalue.uval;
            break;
        case LL_TYPE_INT:
            b->registers.items[OPD_VALUE(lvalue)].ival = rvalue.ival;
            break;
        default: oc_assert(false);
        }
        break;
    }
    default: oc_todo("implementn set value operand type\n"); break;
    }
}

static LL_Eval_Value ll_eval_get_value(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, LL_Ir_Operand lvalue) {
    LL_Eval_Value result;
    LL_Type* type;

    switch (OPD_TYPE(lvalue)) {
    case LL_IR_OPERAND_IMMEDIATE_BIT:
        result.uval = OPD_VALUE(lvalue);
        break;
    case LL_IR_OPERAND_LOCAL_BIT: {
        type = ir_get_operand_type(FUNCTION(), lvalue);
        switch (type->kind) {
        case LL_TYPE_ANYBOOL:
        case LL_TYPE_BOOL:
        case LL_TYPE_UINT:
            result.uval = b->locals.items[OPD_VALUE(lvalue)].uval;
            break;
        case LL_TYPE_INT:
            result.ival = b->locals.items[OPD_VALUE(lvalue)].ival;
            break;
        default: oc_assert(false); break;
        }

        break;
    }
    case LL_IR_OPERAND_REGISTER_BIT: {
        type = ir_get_operand_type(FUNCTION(), lvalue);
        switch (type->kind) {
        case LL_TYPE_ANYBOOL:
        case LL_TYPE_BOOL:
        case LL_TYPE_UINT:
            result.uval = b->registers.items[OPD_VALUE(lvalue)].uval;
            break;
        case LL_TYPE_INT:
            result.ival = b->registers.items[OPD_VALUE(lvalue)].ival;
            break;
        default: oc_assert(false);
        }
        break;
    }
    default: oc_todo("implementn get value operand type\n"); break;
    }

    return result;
}


static void ll_eval_load(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, LL_Ir_Operand result, LL_Ir_Operand src, bool load) {
    LL_Type* to_type = ir_get_operand_type(FUNCTION(), result);
    // @TODO: why did we have this???
    // LL_Type* from_type;

    // switch (OPD_TYPE(src)) {
    // case LL_IR_OPERAND_LOCAL_BIT: {
    //     from_type = FUNCTION()->locals.items[OPD_VALUE(src)].ident->base.type;
    //     break;
    // }
    // case LL_IR_OPERAND_REGISTER_BIT: {
    //     from_type = ir_get_operand_type(FUNCTION(), src);
    //     break;
    // }
    // case LL_IR_OPERAND_IMMEDIATE_BIT: {
    //     from_type = to_type;
    //     break;
    // }
    // default: oc_todo("add load operands"); break;
    // }

    switch (OPD_TYPE(src)) {
    case LL_IR_OPERAND_LOCAL_BIT: {
        switch (to_type->kind) {
        case LL_TYPE_UINT:
            b->registers.items[OPD_VALUE(result)].uval = b->locals.items[OPD_VALUE(src)].uval;
            break;
        case LL_TYPE_INT:
            b->registers.items[OPD_VALUE(result)].ival = b->locals.items[OPD_VALUE(src)].ival;
            break;
        default: break;
        }
        break;
    }
    case LL_IR_OPERAND_REGISTER_BIT: {
        if (load) {
            oc_assert(false);
        } else {
            switch (to_type->kind) {
            case LL_TYPE_UINT:
                b->registers.items[OPD_VALUE(result)].uval = b->registers.items[OPD_VALUE(src)].uval;
                break;
            case LL_TYPE_INT:
                b->registers.items[OPD_VALUE(result)].ival = b->registers.items[OPD_VALUE(src)].ival;
                break;
            default: break;
            }
        }

        break;
    }
    case LL_IR_OPERAND_IMMEDIATE_BIT: {
        switch (to_type->kind) {
        case LL_TYPE_UINT:
            b->registers.items[OPD_VALUE(result)].uval = OPD_VALUE(src);
            break;
        case LL_TYPE_INT:
            b->registers.items[OPD_VALUE(result)].ival = OPD_VALUE(src);
            break;
        default: break;
        }
        break;
    }
    default: oc_todo("add load operands"); break;
    }
}

static void ll_eval_block(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, LL_Ir_Block* block) {
    size_t i;
    int32_t opcode1, opcode2;
    LL_Type* type;

    (void)opcode1;
    (void)opcode2;

    for (i = 0; i < block->ops.count; ) {
        LL_Ir_Opcode opcode = (LL_Ir_Opcode)block->ops.items[i];
        LL_Ir_Operand* operands = (LL_Ir_Operand*)&block->ops.items[i + 1];

        switch (opcode) {
        case LL_IR_OPCODE_BRANCH:
            b->next_block = operands[0];
            return;
        case LL_IR_OPCODE_BRANCH_COND:
            if (ll_eval_get_value(cc, b, bir, operands[0]).uval) {
                b->next_block = operands[1];
            } else {
                b->next_block = operands[2];
            }
            return;
        case LL_IR_OPCODE_RET:
               break;
        case LL_IR_OPCODE_RETVALUE: {
            break;
        }
        case LL_IR_OPCODE_STORE: {
            LL_Eval_Value value = ll_eval_get_value(cc, b, bir, operands[1]);
            ll_eval_set_value(cc, b, bir, operands[0], value);
               break;
        }

#define DO_OP(op) \
            type = ir_get_operand_type(FUNCTION(), operands[0]); \
            switch (type->kind) { \
            case LL_TYPE_INT: \
                b->registers.items[OPD_VALUE(operands[0])].uval = (b->registers.items[OPD_VALUE(operands[1])].ival op b->registers.items[OPD_VALUE(operands[2])].ival) ? 1 : 0; \
                break; \
            case LL_TYPE_UINT: \
                b->registers.items[OPD_VALUE(operands[0])].uval = (b->registers.items[OPD_VALUE(operands[1])].uval op b->registers.items[OPD_VALUE(operands[2])].uval) ? 1 : 0; \
                break; \
            case LL_TYPE_ANYBOOL: \
            case LL_TYPE_BOOL: \
                b->registers.items[OPD_VALUE(operands[0])].uval = (b->registers.items[OPD_VALUE(operands[1])].uval op b->registers.items[OPD_VALUE(operands[2])].uval) ? 1 : 0; \
                break; \
            default: oc_todo("implement types for operations"); break; \
            }
        case LL_IR_OPCODE_EQ: DO_OP(==); break;
        case LL_IR_OPCODE_NEQ: DO_OP(!=); break;
        case LL_IR_OPCODE_GTE: DO_OP(>=); break;
        case LL_IR_OPCODE_GT: DO_OP(>); break;
        case LL_IR_OPCODE_LTE: DO_OP(<=); break;
        case LL_IR_OPCODE_LT: DO_OP(<); break;
#undef DO_OP
#define DO_OP(op) \
            type = ir_get_operand_type(FUNCTION(), operands[0]); \
            switch (type->kind) { \
            case LL_TYPE_INT: \
                b->registers.items[OPD_VALUE(operands[0])].ival = ll_eval_get_value(cc, b, bir, operands[1]).ival op ll_eval_get_value(cc, b, bir, operands[2]).ival; \
                break; \
            case LL_TYPE_UINT: \
                b->registers.items[OPD_VALUE(operands[0])].uval = ll_eval_get_value(cc, b, bir, operands[1]).uval op ll_eval_get_value(cc, b, bir, operands[2]).uval; \
                break; \
            default: oc_todo("implement types for operations"); break; \
            }
        case LL_IR_OPCODE_ADD: DO_OP(+) break; 
        case LL_IR_OPCODE_SUB: DO_OP(-) break; 
        case LL_IR_OPCODE_MUL: DO_OP(*) break;
        case LL_IR_OPCODE_DIV: DO_OP(/) break;
#undef DO_OP
        case LL_IR_OPCODE_CAST: {
            ll_eval_load(cc, b, bir, operands[0], operands[1], false);
            break;
        }
        case LL_IR_OPCODE_LOAD: {
            ll_eval_load(cc, b, bir, operands[0], operands[1], true);
            break;
        }
        case LL_IR_OPCODE_LEA: {
            break;
        }
        case LL_IR_OPCODE_INVOKE: {

               break;
        }
        default: oc_todo("handle other op"); break;
        }

        size_t count = ir_get_op_count(cc, bir, (LL_Ir_Opcode*)block->ops.items, i);
        i += count;
    }
}


LL_Eval_Value ll_eval_node(Compiler_Context* cc, LL_Eval_Context* b, LL_Backend_Ir* bir, Ast_Base* expr) {
    LL_Eval_Value result;
    LL_Ir_Operand result_op;
    LL_Ir_Block_Ref entry_block_ref = bir->free_block ? bir->free_block : bir->blocks.count;
    LL_Ir_Block entry_block = { 0 };
    entry_block.generated_offset = -1;
    if (bir->free_block) {
        memcpy(&bir->blocks.items[entry_block_ref], &entry_block, sizeof(entry_block));
    } else {
        oc_array_append(&cc->arena, &bir->blocks, entry_block);
    }

    LL_Ir_Function fn = {
        .entry = entry_block_ref,
        .exit = entry_block_ref,
        .flags = 0,
        .generated_offset = LL_IR_FUNCTION_OFFSET_INVALID,
        .block_count = 1,
    };

    oc_array_append(&cc->arena, &bir->const_stack, fn);
    /* memcpy(&bir->fns.items[0], &fn, sizeof(fn)); */

    int32_t last_function = bir->current_function;
    LL_Ir_Block_Ref last_block = bir->current_block;
    bir->current_function = CURRENT_CONST_STACK | (bir->const_stack.count - 1); // top of stack is current
    bir->current_block = fn.entry;

    result_op = ir_generate_expression(cc, bir, expr, false);

    b->registers.count = 0;
    oc_array_reserve(&cc->arena, &b->registers, FUNCTION()->registers.count);

    b->locals.count = 0;
    oc_array_reserve(&cc->arena, &b->locals, FUNCTION()->locals.count);

    LL_Ir_Block_Ref current_block = fn.entry;
    while (current_block) {
        b->next_block = bir->blocks.items[current_block].next;
        ll_eval_block(cc, b, bir, &bir->blocks.items[current_block]);
        current_block = b->next_block;
    }

    bir->current_block = last_block;
    bir->current_function = last_function;
    bir->free_block = fn.entry;

    switch (OPD_TYPE(result_op)) {
    case LL_IR_OPERAND_IMMEDIATE_BIT: result.ival = OPD_VALUE(result_op); break;
    case LL_IR_OPERAND_REGISTER_BIT: result = b->registers.items[OPD_VALUE(result_op)]; break;
    case LL_IR_OPERAND_LOCAL_BIT: result = b->locals.items[OPD_VALUE(result_op)]; break;
    }

    return result;
}


