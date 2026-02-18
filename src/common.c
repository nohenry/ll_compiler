#define OC_CORE_IMPLEMENTATION
#include "../core/core1.h"
#include "common.h"
#include "ast.h"
#include "typer.h"
#include "eval.h"
#include "../backends/ir.h"

size_t stbds_hash_string(string str, size_t seed)
{
    size_t hash = seed;
    while (str.len-- > 0)
        hash = STBDS_ROTATE_LEFT(hash, 9) + (unsigned char) *str.ptr++;

    // Thomas Wang 64-to-32 bit mix function, hopefully also works in 32 bits
    hash ^= seed;
    hash = (~hash) + (hash << 18);
    hash ^= hash ^ STBDS_ROTATE_RIGHT(hash,31);
    hash = hash * 21;
    hash ^= hash ^ STBDS_ROTATE_RIGHT(hash,11);
    hash += (hash << 6);
    hash ^= STBDS_ROTATE_RIGHT(hash,22);
    return hash+seed;
}

size_t stbds_hash_string_atom(string str, size_t seed) {
    (void)seed;
    size_t key = (size_t)str.ptr;
    key = (~key) + (key << 21); // key = (key << 21) - key - 1;
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key = key ^ (key >> 28);
    key = key + (key << 31);
    return key;
}

#define STBDS_SIPHASH_C_ROUNDS 1
#define STBDS_SIPHASH_D_ROUNDS 1

size_t stbds_siphash_bytes(void *p, size_t len, size_t seed)
{
  unsigned char *d = (unsigned char *) p;
  size_t i,j;
  size_t v0,v1,v2,v3, data;

  // hash that works on 32- or 64-bit registers without knowing which we have
  // (computes different results on 32-bit and 64-bit platform)
  // derived from siphash, but on 32-bit platforms very different as it uses 4 32-bit state not 4 64-bit
  v0 = ((((size_t) 0x736f6d65 << 16) << 16) + 0x70736575) ^  seed;
  v1 = ((((size_t) 0x646f7261 << 16) << 16) + 0x6e646f6d) ^ ~seed;
  v2 = ((((size_t) 0x6c796765 << 16) << 16) + 0x6e657261) ^  seed;
  v3 = ((((size_t) 0x74656462 << 16) << 16) + 0x79746573) ^ ~seed;

  #define STBDS_SIPROUND() \
    do {                   \
      v0 += v1; v1 = STBDS_ROTATE_LEFT(v1, 13);  v1 ^= v0; v0 = STBDS_ROTATE_LEFT(v0,STBDS_SIZE_T_BITS/2); \
      v2 += v3; v3 = STBDS_ROTATE_LEFT(v3, 16);  v3 ^= v2;                                                 \
      v2 += v1; v1 = STBDS_ROTATE_LEFT(v1, 17);  v1 ^= v2; v2 = STBDS_ROTATE_LEFT(v2,STBDS_SIZE_T_BITS/2); \
      v0 += v3; v3 = STBDS_ROTATE_LEFT(v3, 21);  v3 ^= v0;                                                 \
    } while (0)

  for (i=0; i+sizeof(size_t) <= len; i += sizeof(size_t), d += sizeof(size_t)) {
    data = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
    data |= (size_t) (d[4] | (d[5] << 8) | (d[6] << 16) | (d[7] << 24)) << 16 << 16; // discarded if size_t == 4

    v3 ^= data;
    for (j=0; j < STBDS_SIPHASH_C_ROUNDS; ++j)
      STBDS_SIPROUND();
    v0 ^= data;
  }
  data = len << (STBDS_SIZE_T_BITS-8);
  switch (len - i) {
    case 7: data |= ((size_t) d[6] << 24) << 24; // fall through
    case 6: data |= ((size_t) d[5] << 20) << 20; // fall through
    case 5: data |= ((size_t) d[4] << 16) << 16; // fall through
    case 4: data |= (d[3] << 24); // fall through
    case 3: data |= (d[2] << 16); // fall through
    case 2: data |= (d[1] << 8); // fall through
    case 1: data |= d[0]; // fall through
    case 0: break;
  }
  v3 ^= data;
  for (j=0; j < STBDS_SIPHASH_C_ROUNDS; ++j)
    STBDS_SIPROUND();
  v0 ^= data;
  v2 ^= 0xff;
  for (j=0; j < STBDS_SIPHASH_D_ROUNDS; ++j)
    STBDS_SIPROUND();

  return v1^v2^v3; // slightly stronger since v0^v3 in above cancels out final round operation? I tweeted at the authors of SipHash about this but they didn't reply
}

string LL_KEYWORD_CONST;
string LL_KEYWORD_CAST;
string LL_KEYWORD_IF;
string LL_KEYWORD_FOR;
string LL_KEYWORD_WHILE;
string LL_KEYWORD_ELSE;
string LL_KEYWORD_DO;
string LL_KEYWORD_MATCH;
string LL_KEYWORD_STRUCT;
string LL_KEYWORD_EXTERN;
string LL_KEYWORD_NATIVE;
string LL_KEYWORD_RETURN;
string LL_KEYWORD_BREAK;
string LL_KEYWORD_CONTINUE;
string LL_KEYWORD_MACRO;
string LL_KEYWORD_LET;
string LL_KEYWORD_SIZEOF;

string LL_KEYWORD_BOOL;
string LL_KEYWORD_BOOL8;
string LL_KEYWORD_BOOL16;
string LL_KEYWORD_BOOL32;
string LL_KEYWORD_BOOL64;
string LL_KEYWORD_TRUE;
string LL_KEYWORD_FALSE;
string LL_KEYWORD_NULL;

string LL_KEYWORD_UINT;
string LL_KEYWORD_UINT8;
string LL_KEYWORD_UINT16;
string LL_KEYWORD_UINT32;
string LL_KEYWORD_UINT64;
string LL_KEYWORD_INT;
string LL_KEYWORD_INT8;
string LL_KEYWORD_INT16;
string LL_KEYWORD_INT32;
string LL_KEYWORD_INT64;
string LL_KEYWORD_FLOAT16;
string LL_KEYWORD_FLOAT32;
string LL_KEYWORD_FLOAT64;
string LL_KEYWORD_FLOAT;
string LL_KEYWORD_STRING;
string LL_KEYWORD_VOID;
string LL_KEYWORD_CHAR;

Compiler_Context ll_compiler_context_create() {
    Compiler_Context result = { 0 };
    LL_KEYWORD_CONST = ll_intern_string(&result, lit("const"));
    LL_KEYWORD_CAST = ll_intern_string(&result, lit("cast"));
    LL_KEYWORD_IF = ll_intern_string(&result, lit("if"));
    LL_KEYWORD_FOR = ll_intern_string(&result, lit("for"));
    LL_KEYWORD_WHILE = ll_intern_string(&result, lit("while"));
    LL_KEYWORD_ELSE = ll_intern_string(&result, lit("else"));
    LL_KEYWORD_DO = ll_intern_string(&result, lit("do"));
    LL_KEYWORD_MATCH = ll_intern_string(&result, lit("match"));
    LL_KEYWORD_STRUCT = ll_intern_string(&result, lit("struct"));
    LL_KEYWORD_EXTERN = ll_intern_string(&result, lit("extern"));
    LL_KEYWORD_NATIVE = ll_intern_string(&result, lit("native"));
    LL_KEYWORD_RETURN = ll_intern_string(&result, lit("return"));
    LL_KEYWORD_BREAK = ll_intern_string(&result, lit("break"));
    LL_KEYWORD_CONTINUE = ll_intern_string(&result, lit("continue"));
    LL_KEYWORD_MACRO = ll_intern_string(&result, lit("macro"));
    LL_KEYWORD_LET = ll_intern_string(&result, lit("let"));
    LL_KEYWORD_SIZEOF = ll_intern_string(&result, lit("sizeof"));

    LL_KEYWORD_BOOL = ll_intern_string(&result, lit("bool"));
    LL_KEYWORD_BOOL8 = ll_intern_string(&result, lit("bool8"));
    LL_KEYWORD_BOOL16 = ll_intern_string(&result, lit("bool16"));
    LL_KEYWORD_BOOL32 = ll_intern_string(&result, lit("bool32"));
    LL_KEYWORD_BOOL64 = ll_intern_string(&result, lit("bool64"));
    LL_KEYWORD_TRUE = ll_intern_string(&result, lit("true"));
    LL_KEYWORD_FALSE = ll_intern_string(&result, lit("false"));
    LL_KEYWORD_NULL = ll_intern_string(&result, lit("null"));

    LL_KEYWORD_UINT = ll_intern_string(&result, lit("uint"));
    LL_KEYWORD_UINT8 = ll_intern_string(&result, lit("uint8"));
    LL_KEYWORD_UINT16 = ll_intern_string(&result, lit("uint16"));
    LL_KEYWORD_UINT32 = ll_intern_string(&result, lit("uint32"));
    LL_KEYWORD_UINT64 = ll_intern_string(&result, lit("uint64"));
    LL_KEYWORD_INT = ll_intern_string(&result, lit("int"));
    LL_KEYWORD_INT8 = ll_intern_string(&result, lit("int8"));
    LL_KEYWORD_INT16 = ll_intern_string(&result, lit("int16"));
    LL_KEYWORD_INT32 = ll_intern_string(&result, lit("int32"));
    LL_KEYWORD_INT64 = ll_intern_string(&result, lit("int64"));
    LL_KEYWORD_FLOAT16 = ll_intern_string(&result, lit("float16"));
    LL_KEYWORD_FLOAT32 = ll_intern_string(&result, lit("float32"));
    LL_KEYWORD_FLOAT64 = ll_intern_string(&result, lit("float64"));
    LL_KEYWORD_FLOAT = ll_intern_string(&result, lit("float"));
    LL_KEYWORD_STRING = ll_intern_string(&result, lit("string"));
    LL_KEYWORD_VOID = ll_intern_string(&result, lit("void"));
    LL_KEYWORD_CHAR = ll_intern_string(&result, lit("char"));

    return result;
}

string ll_intern_string(Compiler_Context* cc, string str) {
    string* s = MAP_GET_OR_PUT(cc->string_interns, str, str, &cc->arena, stbds_hash_string, string_eql, MAP_DEFAULT_SEED);
    return *s;
}


uint32_t log2_u32(uint32_t x) {
    uint32_t y;
    #ifdef __x86_64__
        __asm__ ( "\tbsr %1, %0\n"
            : "=r"(y)
            : "r" (x)
        );
    #elif __aarch64__   
        __asm__( "\tclz %w0, %w1\n"
            : "=r"(y)
            : "r"(x)
        );
        y = 31 - y;
    #endif
    return y;
}

size_t hash_combine(size_t lhs, size_t rhs) {
#if __WORDSIZE == 64
	lhs ^= rhs + 0x517cc1b727220a95 + (lhs << 6) + (lhs >> 2);
#else
	lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
#endif
	return lhs;
}

void resolve_dependencies(Compiler_Context* cc, LL_Queued* queued, LL_Stage_Kind stage) {
    for (uint32 i = 0; i < queued->dependants.count; ++i) {
        LL_Dependency dep = queued->dependants.items[i];
        if (dep.flags & (1 << stage)) {
            dep.flags &= ~(1 << stage);
            oc_assert(dep.target->dependency_counter[stage] > 0);
            dep.target->dependency_counter[stage]--;
            if (dep.target->dependency_counter[stage] == 0) {
                oc_assert(dep.target->max_completed_stage < stage);
                actually_queue(cc, dep.target->max_completed_stage + 1, dep.target);
            }
            if (dep.flags == 0) {
                oc_array_unordered_remove(&cc->arena, &queued->dependants, i);
                --i;
            }
        }
    }
}

uint32 output_queued(Compiler_Context* cc, LL_Stage_Kind stage_kind, Oc_Writer* w, LL_Queued* queued) {
    switch (queued->code->kind) {
    case CODE_KIND_BLOCK: {
        Code_Scope* scope = (Code_Scope*)queued->code;
        wprint(w, "node{} [label=\"anon_scope {}\\n{}\\ndependencies: {}", queued->s, queued->s, (sint64)(sint32)queued->index_in_stage, queued->dependency_counter[stage_kind]);
        if (scope->c_open.kind != LL_TOKEN_KIND_NONE) {
            LL_Line_Info line_info = lexer_get_line_info(cc->lexer, scope->c_open);
            wprint(w, "\\nline {}, col {}", line_info.line, line_info.column);
        }
        wprint(w, "\"];\n");
    } break;
    case CODE_KIND_STRUCT:
    case CODE_KIND_VARIABLE_DECLARATION:
    case CODE_KIND_FUNCTION_DECLARATION: {
        Code_Declaration* decl = (Code_Declaration*)queued->code;

        wprint(w, "node{} [label=\"{} {}\\n{}\\ndependencies: {}", queued->s, decl->ident->str, queued->s, (sint64)(sint32)queued->index_in_stage, queued->dependency_counter[stage_kind]);
        if (decl->base.token_info.kind != LL_TOKEN_KIND_NONE) {
            LL_Line_Info line_info = lexer_get_line_info(cc->lexer, decl->base.token_info);
            wprint(w, "\\nline {}, col {}", line_info.line, line_info.column);
        }
        if (queued->code->type) {
            wprint(w, "\\n");
            ll_print_type_raw(queued->code->type, w);
        }
        wprint(w, "\"];\n");
    } break;
    default: {
        wprint(w, "node{} [label=\"{} {}\\n{}\\ndependencies: {}", queued->s, ast_get_node_kind(queued->code), queued->s, (sint64)(sint32)queued->index_in_stage, queued->dependency_counter[stage_kind]);
        if (queued->code->token_info.kind != LL_TOKEN_KIND_NONE) {
            LL_Line_Info line_info = lexer_get_line_info(cc->lexer, queued->code->token_info);
            wprint(w, "\\nline {}, col {}", line_info.line, line_info.column);
        }
        if (queued->code->type) {
            wprint(w, "\\n");
            ll_print_type_raw(queued->code->type, w);
        }
        wprint(w, "\"];\n");
    } break;
    }
    // if (queued->code && queued->code->kind == CODE_KIND_IDENT) {
    //     Code_Ident* ident = CODE_AS(queued->code, Code_Ident);
    //     wprint(w, "node{} [label={}];\n", this_n, ident->str);
    // } else if (!queued->code) {


    // }

    
    // for (uint32 i = 0; i < queued->dependencies.count; ++i) {
    //     LL_Dependency* dep = &queued->dependencies.items[i];
    //     if (dep->target == queued) continue;
    //     uint32 dep_n = output_queued(cc, stage, w, n, dep->target);

    //     wprint(w, "node{} -> node{}\n", this_n, dep_n);
    // }

    for (uint32 i = 0; i < queued->dependants.count; ++i) {
        LL_Dependency* dep = &queued->dependants.items[i];
        if (dep->target == queued) continue;
        uint32 dep_n = output_queued(cc, stage_kind, w, dep->target);

        // wprint(w, "node{} <- node{}\n", queued->s, dep_n);
        wprint(w, "node{} -> node{}\n", dep_n, queued->s);
        // wprint(w, "node{} -> node{}\n", queued->s, dep_n);
    }
    return queued->s;
}

const char* get_stage_name(LL_Stage_Kind stage_kind) {
    switch (stage_kind) {
    case 0: return "STAGE_NONE";
    case STAGE_TYPECHECK: return "STAGE_TYPECHECK";
    case STAGE_IR: return "STAGE_IR";
    case STAGE_EVAL: return "STAGE_EVAL";
    default: oc_assert(false);
    }
}
void input_graph(Compiler_Context* cc, LL_Stage_Kind stage_kind) {
    LL_Stage* stage = &cc->stages[stage_kind];
    static int in_graph = 0;
    Oc_String_Builder sb;
    oc_sb_init(&sb, &cc->arena);

    wprint(&sb.writer, "digraph Comiler {{\n");

    for (uint32 i = 0; i < stage->input.count; ++i) {
        output_queued(cc, stage_kind, &sb.writer, stage->input.items[i]);
    }

    wprint(&sb.writer, "}");

    string s = oc_format(&cc->arena, "in{}_{}.dot", in_graph, get_stage_name(stage_kind));
    in_graph++;

    FILE* f;
    if (fopen_s(&f, s.ptr, "wb")) {
        eprint("Unable to open output file: %s\n", s.ptr);
        return;
    }
    fwrite(sb.items, 1, sb.count, f);
    fclose(f);
}

void output_graph(Compiler_Context* cc, LL_Stage_Kind stage_kind) {
    LL_Stage* stage = &cc->stages[stage_kind];
    static int out_graph = 0;
    Oc_String_Builder sb;
    oc_sb_init(&sb, &cc->arena);

    wprint(&sb.writer, "digraph Comiler {{\n");

    for (uint32 i = 0; i < stage->output.count; ++i) {
        output_queued(cc, stage_kind, &sb.writer, stage->output.items[i]);
    }

    wprint(&sb.writer, "}");

    string s = oc_format(&cc->arena, "out{}_{}.dot", out_graph, get_stage_name(stage_kind));
    out_graph++;

    FILE* f;
    if (fopen_s(&f, s.ptr, "wb")) {
        eprint("Unable to open output file: %s\n", s.ptr);
        return;
    }
    fwrite(sb.items, 1, sb.count, f);
    fclose(f);
}

// bool compiller_consume_dependencies(Compiler_Context* cc, LL_Queued* queued) {
//     (void)cc;
//     while (queued->dependency_cursor < queued->dependencies.count) {
//         LL_Dependency* dep = &queued->dependencies.items[queued->dependency_cursor];
//         LL_Stage_Flag max_flag = 1u << dep->target->max_completed_stage;
//         if (max_flag & dep->flags) {
//             dep->flags &= ~max_flag;
//             if (!dep->flags) {
//                 queued->dependency_cursor++;
//                 continue;
//             }
//         }
//         break;
//     }
//     return queued->dependency_cursor < queued->dependencies.count;
// }

void compiler_cycle_stage_typecheck(Compiler_Context* cc, uint32* number_of_deletions, uint32* number_of_insertions) {
    LL_Stage* stage = &cc->stages[STAGE_TYPECHECK];
    LL_Typer* typer = cc->typer;

    cc->current_stage = STAGE_TYPECHECK;

    cc->number_of_queued = number_of_insertions;

    for (uint32 i = 0; i < stage->input.count; ++i) {
        typer->waited_on_code = NULL;
        LL_Queued* queued_item = stage->input.items[i];
        oc_assert(queued_item->dependency_counter[STAGE_TYPECHECK] == 0);
        oc_assert(queued_item->index_in_stage == i);
        // if (compiller_consume_dependencies(cc, queued_item)) continue;
        // queued_item->dependencies.count = 0;
        // queued_item->dependency_cursor = 0;
        
        bool result = true;

        LL_Resume_Info resume_info = { .code = queued_item->code };
        oc_array_append(&cc->arena, &cc->queued_stack, queued_item);

        if (queued_item->imperative_index != (uint32)-1) {
            Code_Scope** scope = (Code_Scope**)&queued_item->code;
            oc_assert((*scope)->flags & CODE_SCOPE_FLAG_IMPERATIVE);
            typer->current_scope = *scope;
            typer->current_function = queued_item->function;
            result = ll_typer_type_statement(cc, typer, (Code**)scope, &resume_info);
        } else {
            // oc_assert(queued_item->scope->flags & CODE_SCOPE_FLAG_DECLARATIVE);
            typer->current_scope = queued_item->scope;
            typer->current_function = queued_item->function;
            result = ll_typer_type_statement(cc, typer, &queued_item->code, &resume_info);
        }

        cc->queued_stack.count--;

        if (result) {
            if (queued_item->max_completed_stage < STAGE_TYPECHECK) {
                queued_item->max_completed_stage = STAGE_TYPECHECK;
            }
            resolve_dependencies(cc, queued_item, STAGE_TYPECHECK);

            queued_item->index_in_stage = stage->output.count;
            oc_array_append(&cc->arena, &stage->output, queued_item);

            stage->input.items[i] = stage->input.items[stage->input.count - 1];
            stage->input.items[i]->index_in_stage = i;
            
            stage->input.count--;
            (*number_of_deletions)++;
            i--;
        }
    }
    print("finish cycle\n");
}



void compiler_cycle_stage_ir(Compiler_Context* cc, uint32* number_of_deletions, uint32* number_of_insertions) {
    LL_Stage* stage = &cc->stages[STAGE_IR];
    LL_Typer* typer = cc->typer;
    
    cc->current_stage = STAGE_IR;

    {
        LL_Stage* last_stage = &cc->stages[STAGE_TYPECHECK];
        for (uint32 i = 0; i < last_stage->output.count; ++i) {
            LL_Queued* queued = last_stage->output.items[i];
            if (queued->dependency_counter[STAGE_IR] == 0) {
                queued->index_in_stage = stage->input.count;
                oc_array_append(&cc->arena, &stage->input, queued);
            } else {
                queued->index_in_stage = -1;
            }
        }
        last_stage->output.count = 0;
    }
    input_graph(cc, STAGE_IR);

    cc->number_of_queued = number_of_insertions;

    for (uint32 i = 0; i < stage->input.count; ++i) {
        typer->waited_on_code = NULL;
        LL_Queued* queued_item = stage->input.items[i];
        oc_assert(queued_item->index_in_stage == i);
        // if (compiller_consume_dependencies(cc, queued_item)) continue;
        // queued_item->dependencies.count = 0;
        // queued_item->dependency_cursor = 0;
        
        bool result = true;
        oc_array_append(&cc->arena, &cc->queued_stack, queued_item);

        if (queued_item->fn_ir_override) {
            // we are an expression
            cc->bir->current_function = queued_item->fn_ir_override;
            cc->bir->current_block = cc->bir->fns.items[cc->bir->current_function].exit;
            oc_assert(cc->bir->fns.items[cc->bir->current_function].flags & LL_IR_FUNCTION_FLAG_APPEND_RET_FOR_EXPR);
            LL_Ir_Operand value = ir_generate_expression(cc, cc->bir, queued_item->code, false, &result);
            ir_append_expr_ret(cc, cc->bir, value);
        } else {
            ir_generate_statement(cc, cc->bir, queued_item->code, &result);
        }

        cc->queued_stack.count--;

        if (result) {
            if (queued_item->max_completed_stage < STAGE_IR) {
                queued_item->max_completed_stage = STAGE_IR;
            }
            resolve_dependencies(cc, queued_item, STAGE_IR);

            queued_item->index_in_stage = stage->output.count;
            oc_array_append(&cc->arena, &stage->output, queued_item);

            stage->input.items[i] = stage->input.items[stage->input.count - 1];
            stage->input.items[i]->index_in_stage = i;
            
            stage->input.count--;
            (*number_of_deletions)++;
            i--;
        }
    }
}

void compiler_cycle_stage_eval(Compiler_Context* cc, uint32* number_of_deletions, uint32* number_of_insertions) {
    LL_Stage* stage = &cc->stages[STAGE_EVAL];
    LL_Typer* typer = cc->typer;
    
    cc->current_stage = STAGE_EVAL;

    {
        LL_Stage* last_stage = &cc->stages[STAGE_IR];
        for (uint32 i = 0; i < last_stage->output.count; ++i) {
            LL_Queued* queued = last_stage->output.items[i];
            if (queued->needs_eval) {
                queued->index_in_stage = stage->input.count;
                oc_array_append(&cc->arena, &stage->input, queued);
                oc_array_unordered_remove(&cc->arena, &last_stage->output, i);
                --i;
            } else {
                queued->index_in_stage = -1;
            }
        }
    }
    input_graph(cc, STAGE_EVAL);

    cc->number_of_queued = number_of_insertions;

    for (uint32 i = 0; i < stage->input.count; ++i) {
        typer->waited_on_code = NULL;
        LL_Queued* queued_item = stage->input.items[i];
        oc_assert(queued_item->index_in_stage == i);
        // if (compiller_consume_dependencies(cc, queued_item)) continue;
        // queued_item->dependencies.count = 0;
        // queued_item->dependency_cursor = 0;
        
        bool result = true;
        oc_array_append(&cc->arena, &cc->queued_stack, queued_item);

        LL_Eval_Value value = ll_eval_node(cc, cc->eval_context, cc->bir, queued_item->code);
        (void)value;

        cc->queued_stack.count--;

        if (result) {
            if (queued_item->max_completed_stage < STAGE_EVAL) {
                queued_item->max_completed_stage = STAGE_EVAL;
            }
            resolve_dependencies(cc, queued_item, STAGE_EVAL);

            queued_item->index_in_stage = stage->output.count;
            oc_array_append(&cc->arena, &stage->output, queued_item);

            stage->input.items[i] = stage->input.items[stage->input.count - 1];
            stage->input.items[i]->index_in_stage = i;
            
            stage->input.count--;
            (*number_of_deletions)++;
            i--;
        }
    }
}


void compiler_run_stages(Compiler_Context* cc) {
    uint32 number_of_deletions = 0;
    uint32 number_of_insertions = 0;

    while (true) {
        number_of_deletions = 0;
        number_of_insertions = 0;

        input_graph(cc, STAGE_TYPECHECK);
        compiler_cycle_stage_typecheck(cc, &number_of_deletions, &number_of_insertions);
        output_graph(cc, STAGE_TYPECHECK);
        compiler_cycle_stage_ir(cc, &number_of_deletions, &number_of_insertions);
        output_graph(cc, STAGE_IR);
        compiler_cycle_stage_eval(cc, &number_of_deletions, &number_of_insertions);
        output_graph(cc, STAGE_EVAL);

        if (number_of_insertions == 0 && number_of_deletions == 0) break;
    }

    // for (uint32 i = 0; i < typer->queue.count; ++i) {
    //     typer->waited_on_code = NULL;
    //     LL_Queued* queued_item = typer->queue.items[i];

    //     if (queued_item->code) {
    //         if (queued_item->imperative_index != (uint32)-1) {
    //             ll_typer_report_error(((LL_Error){ .main_token = CODE_AS(queued_item->code, Code_Ident)->base.token_info }), "Symbol '{}' not found", CODE_AS(queued_item->code, Code_Ident)->str);
    //             // ll_typer_report_error_done(cc, typer);
    //         } else {
    //             ll_typer_report_error(((LL_Error){ .main_token = CODE_AS(queued_item->code, Code_Ident)->base.token_info }), "Symbol '{}' not found", CODE_AS(queued_item->code, Code_Ident)->str);
    //             // ll_typer_report_error_done(cc, typer);
    //         }
    //     } else {
    //         if (queued_item->imperative_index != (uint32)-1) {
    //             ll_typer_report_error(((LL_Error){ .main_token = queued_item->scope->statements.items[queued_item->imperative_index]->token_info }), "Symbol '{}' not found", CODE_AS(queued_item->code, Code_Ident)->str);
    //         } else {
    //             Code_Declaration** v = hash_map_get_from_hash(&cc.arena, &queued_item->scope->declarations, typer->queue.items[i]->decl_str, typer->queue.items[i]->decl_yielded_hash);
    //             ll_typer_report_error(((LL_Error){ .main_token = (*v)->base.token_info }), "Symbol '{}' not found", CODE_AS(queued_item->code, Code_Ident)->str);
    //         }
    //     }
    // }
}
