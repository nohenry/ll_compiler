
#include "typer2.h"
#include "parser.h"

void ll_typer_print_error_line(Compiler_Context* cc, LL_Typer* typer, LL_Line_Info line_info, LL_Token_Info start_info, LL_Token_Info end_info, bool print_dot_dot_dot, bool print_underline) {
    (void)typer;
    Oc_String_Builder sb;
    oc_sb_init(&sb, &cc->arena);
    wprint(&sb.writer, " {} | ", line_info.line);
    size_t line_offset = sb.count;
    eprint("{}", oc_sb_to_string(&sb));

    bool do_color = oc_fd_supports_color(OC_FD_ERROR);

    if (start_info.kind || end_info.kind) {

        if (do_color) {
            string start     = string_slice(cc->lexer->source, line_info.start_pos  , start_info.position);
            string highlight = string_slice(cc->lexer->source, start_info.position  , end_info.position);
            string end       = string_slice(cc->lexer->source, end_info.position, line_info.end_pos);
            eprint("\x1b[32m{}\x1b[31m{}\x1b[32m{}\x1b[0m\n", start, highlight, end);
        } else {
            string line = string_slice(cc->lexer->source, line_info.start_pos, line_info.end_pos);
            eprint("{}\n", line);
        }
    }

    if (print_underline && start_info.kind && end_info.kind) {
        for (size_t i = 0; i < line_offset - 2; ++i) {
            stderr_writer.write(&stderr_writer, " ", 1);
        }
        eprint("| ");
        if (print_dot_dot_dot && start_info.position > line_info.start_pos + 3) {
            eprint("...");
            for (size_t i = 0; i < start_info.position - line_info.start_pos - 3; ++i) {
                stderr_writer.write(&stderr_writer, " ", 1);
            }
        } else {
            for (size_t i = 0; i < start_info.position - line_info.start_pos; ++i) {
                stderr_writer.write(&stderr_writer, " ", 1);
            }
        }
        if (do_color) stderr_writer.write(&stderr_writer, "\x1b[31m", sizeof( "\x1b[31m") - 1);
        for (size_t i = start_info.position; i < end_info.position; ++i) {
            stderr_writer.write(&stderr_writer, "^", 1);
        }
    }
    eprint("\n");
    if (do_color) eprint("\x1b[0m");
}

void ll_typer_report_error_raw(Compiler_Context* cc, LL_Typer* typer, LL_Error error, const char* fmt, ...) {
    LL_Line_Info line_info, end_line_info;
    if (error.highlight_start.kind || error.highlight_end.kind) {
        oc_assert(error.highlight_start.kind);
        oc_assert(error.highlight_end.kind);
        line_info = lexer_get_line_info(cc->lexer, error.highlight_start);
        end_line_info = lexer_get_line_info(cc->lexer, error.highlight_end);
    } else if (error.main_token.kind) {
        line_info = lexer_get_line_info(cc->lexer, error.main_token);
        end_line_info = line_info;
    }

    bool do_color = oc_fd_supports_color(OC_FD_ERROR);

    if (error.highlight_start.kind || error.highlight_end.kind || error.main_token.kind) {
        if (do_color) {
            eprint("{}:{}:{}: \x1b[31;1merror\x1b[0m: \x1b[1m", cc->lexer->filename, line_info.line, line_info.column);
        } else {
            eprint("{}:{}:{}: error: ", cc->lexer->filename, line_info.line, line_info.column);
        }
    }

    va_list list;
    va_start(list, fmt);
    _oc_vprintw(&stderr_writer, fmt, list);
    va_end(list);
    if (do_color) eprint("\x1b[0m");
    eprint("\n");

    if (error.highlight_start.kind || error.highlight_end.kind) {
        if (line_info.line == end_line_info.line) {
            ll_typer_print_error_line(cc, typer, line_info, error.highlight_start, (LL_Token_Info){ 1, error.highlight_end.position + 1 }, false, true);
        } else {
            ll_typer_print_error_line(cc, typer, line_info, error.highlight_start, (LL_Token_Info){ 1, line_info.end_pos }, true, true);
            ll_typer_print_error_line(cc, typer, end_line_info, (LL_Token_Info){ 1, end_line_info.start_pos }, (LL_Token_Info){ 1, error.highlight_end.position + 1 }, false, true);
        }
    } else if (error.main_token.kind) {
        int64_t token_length = lexer_get_token_length(cc, cc->lexer, error.main_token);
        ll_typer_print_error_line(cc, typer, line_info, error.main_token, (LL_Token_Info){ 1, error.main_token.position + token_length}, false, true);
    }
}

void ll_typer_report_error_note_raw(Compiler_Context* cc, LL_Typer* typer, LL_Error error, const char* fmt, ...) {
    LL_Line_Info line_info, end_line_info;
    if (error.highlight_start.kind || error.highlight_end.kind) {
        oc_assert(error.highlight_start.kind);
        oc_assert(error.highlight_end.kind);
        line_info = lexer_get_line_info(cc->lexer, error.highlight_start);
        end_line_info = lexer_get_line_info(cc->lexer, error.highlight_end);
    } else if (error.main_token.kind) {
        line_info = lexer_get_line_info(cc->lexer, error.main_token);
        end_line_info = line_info;
    }

    bool do_color = oc_fd_supports_color(OC_FD_ERROR);

    if (error.highlight_start.kind || error.highlight_end.kind || error.main_token.kind) {
        if (do_color) {
            eprint("{}:{}:{}: \x1b[34;1mnote\x1b[0m: \x1b[1m", cc->lexer->filename, line_info.line, line_info.column);
        } else {
            eprint("{}:{}:{}: note: ", cc->lexer->filename, line_info.line, line_info.column);
        }
    }

    va_list list;
    va_start(list, fmt);
    _oc_vprintw(&stderr_writer, fmt, list);
    va_end(list);
    if (do_color) eprint("\x1b[0m");
    eprint("\n");

    if (error.highlight_start.kind || error.highlight_end.kind) {
        if (line_info.line == end_line_info.line) {
            ll_typer_print_error_line(cc, typer, line_info, error.highlight_start, (LL_Token_Info){ 1, error.highlight_end.position + 1 }, false, true);
        } else {
            ll_typer_print_error_line(cc, typer, line_info, error.highlight_start, (LL_Token_Info){ 1, line_info.end_pos }, true, true);
            ll_typer_print_error_line(cc, typer, end_line_info, (LL_Token_Info){ 1, end_line_info.start_pos }, (LL_Token_Info){ 1, error.highlight_end.position + 1 }, false, true);
        }
    } else if (error.main_token.kind) {
        int64_t token_length = lexer_get_token_length(cc, cc->lexer, error.main_token);
        ll_typer_print_error_line(cc, typer, line_info, error.main_token, (LL_Token_Info){ 1, error.main_token.position + token_length}, false, true);
    }
}

void ll_typer_report_error_info_raw(Compiler_Context* cc, LL_Typer* typer, LL_Error error, const char* fmt, ...) {
    LL_Line_Info line_info, end_line_info;
    if (error.highlight_start.kind || error.highlight_end.kind) {
        oc_assert(error.highlight_start.kind);
        oc_assert(error.highlight_end.kind);
        line_info = lexer_get_line_info(cc->lexer, error.highlight_start);
        end_line_info = lexer_get_line_info(cc->lexer, error.highlight_end);
    } else if (error.main_token.kind) {
        line_info = lexer_get_line_info(cc->lexer, error.main_token);
        end_line_info = line_info;
    }

    bool do_color = oc_fd_supports_color(OC_FD_ERROR);

    // if (error.main_token.kind) {
    //     eprint("{}:{}:{}: \x1b[31;1merror\x1b[0m: \x1b[1m", cc->lexer->filename, line_info.line, line_info.column);
    // } else {
    //     eprint("{}: \x1b[31;1merror\x1b[0m: \x1b[1m", cc->lexer->filename);
    // }
    if (do_color) eprint("\x1b[1m");

    va_list list;
    va_start(list, fmt);
    _oc_vprintw(&stderr_writer, fmt, list);
    va_end(list);
    if (do_color) eprint("\x1b[0m");
    eprint("\n");

    if (error.highlight_start.kind || error.highlight_end.kind) {
        if (line_info.line == end_line_info.line) {
            ll_typer_print_error_line(cc, typer, line_info, error.highlight_start, (LL_Token_Info){ 1, error.highlight_end.position + 1 }, false, false);
        } else {
            ll_typer_print_error_line(cc, typer, line_info, error.highlight_start, (LL_Token_Info){ 1, line_info.end_pos }, true, false);
            ll_typer_print_error_line(cc, typer, end_line_info, (LL_Token_Info){ 1, end_line_info.start_pos }, (LL_Token_Info){ 1, error.highlight_end.position + 1 }, false, false);
        }
    } else if (error.main_token.kind) {
        eprint(" {} | {}\n", line_info.line, line_info.line);
    }
}

void ll_typer_report_error_no_src_raw(Compiler_Context* cc, LL_Typer* typer, const char* fmt, ...) {
    (void)cc;
    (void)typer;
    bool do_color = oc_fd_supports_color(OC_FD_ERROR);
    if (do_color) eprint("\x1b[0;1m");
    va_list list;
    va_start(list, fmt);
    _oc_vprintw(&stderr_writer, fmt, list);
    va_end(list);
}

void ll_typer_report_error_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* type) {
    (void)cc;
    (void)typer;
    bool do_color = oc_fd_supports_color(OC_FD_ERROR);
    if (do_color) {
        eprint("\x1b[0;36m");
        ll_print_type_raw(type, &stderr_writer);
        eprint("\x1b[0m");
    } else {
        eprint("'");
        ll_print_type_raw(type, &stderr_writer);
        eprint("'");
    }
}

void ll_typer_report_error_type_no_fmt(Compiler_Context* cc, LL_Typer* typer, LL_Type* type) {
    (void)cc;
    (void)typer;
    ll_print_type_raw(type, &stderr_writer);
}

void ll_typer_report_error_done(Compiler_Context* cc, LL_Typer* typer) {
    (void)cc;
    (void)typer;
    if (cc->exit_0) oc_exit(0);
    oc_exit(-1);
}

LL_Typer ll_typer_create(Compiler_Context* cc) {
    LL_Typer result = { 0 };
    LL_Typer* typer = &result;

	oc_array_reserve(&cc->arena, &result.types, 128);
	result.types.count = 0;

#define INSERT_BUILTIN_TYPE(_kind, _group, _cast, _implicit_cast) oc_array_append(&cc->arena, &result.types, ((LL_Type) { .kind = _kind, .group = _group, .cast = (_cast) | (_kind), .implicit_cast = (_implicit_cast) | (_kind) }))
#define _CASTTO(ty, n) (1 << (ty))
#define CASTTO(...) OC_MAP_SEPARATOR(_CASTTO, |, __VA_ARGS__)
#define ALL_INT() CASTTO(LL_TYPE_INT8, LL_TYPE_INT16, LL_TYPE_INT32, LL_TYPE_INT64, LL_TYPE_UINT8, LL_TYPE_UINT16, LL_TYPE_UINT32, LL_TYPE_UINT64)

	INSERT_BUILTIN_TYPE(LL_TYPE_UNKNOWN, LL_TYPE_GROUP_UNKNOWN, 0, 0);
	INSERT_BUILTIN_TYPE(LL_TYPE_INT8,    LL_TYPE_GROUP_INT,   0, CASTTO(LL_TYPE_INT16, LL_TYPE_INT32, LL_TYPE_INT64));
	INSERT_BUILTIN_TYPE(LL_TYPE_INT16,   LL_TYPE_GROUP_INT,   0, CASTTO(LL_TYPE_INT32, LL_TYPE_INT64));
	INSERT_BUILTIN_TYPE(LL_TYPE_INT32,   LL_TYPE_GROUP_INT,   0, CASTTO(LL_TYPE_INT64));
	INSERT_BUILTIN_TYPE(LL_TYPE_INT64,   LL_TYPE_GROUP_INT,   0, 0);
	INSERT_BUILTIN_TYPE(LL_TYPE_UINT8,   LL_TYPE_GROUP_UINT,  0, CASTTO(LL_TYPE_INT16, LL_TYPE_INT32, LL_TYPE_INT64, LL_TYPE_UINT16, LL_TYPE_UINT32, LL_TYPE_UINT64));
	INSERT_BUILTIN_TYPE(LL_TYPE_UINT16,  LL_TYPE_GROUP_UINT,  0, CASTTO(LL_TYPE_INT32, LL_TYPE_INT64, LL_TYPE_UINT32, LL_TYPE_UINT64));
	INSERT_BUILTIN_TYPE(LL_TYPE_UINT32,  LL_TYPE_GROUP_UINT,  0, CASTTO(LL_TYPE_INT64, LL_TYPE_UINT64));
	INSERT_BUILTIN_TYPE(LL_TYPE_UINT64,  LL_TYPE_GROUP_UINT,  0, 0);
	INSERT_BUILTIN_TYPE(LL_TYPE_CHAR8,   LL_TYPE_GROUP_CHAR,  0, CASTTO(LL_TYPE_INT8, LL_TYPE_INT16, LL_TYPE_INT32, LL_TYPE_INT64, LL_TYPE_UINT8, LL_TYPE_UINT16, LL_TYPE_UINT32, LL_TYPE_UINT64));
	INSERT_BUILTIN_TYPE(LL_TYPE_CHAR16,  LL_TYPE_GROUP_CHAR,  0, CASTTO(LL_TYPE_INT16, LL_TYPE_INT32, LL_TYPE_INT64, LL_TYPE_UINT16, LL_TYPE_UINT32, LL_TYPE_UINT64));
	// INSERT_BUILTIN_TYPE(LL_TYPE_CHAR32,  LL_TYPE_GROUP_CHAR,  0, CASTTO(LL_TYPE_INT32, LL_TYPE_INT64, LL_TYPE_UINT32, LL_TYPE_UINT64));
	// INSERT_BUILTIN_TYPE(LL_TYPE_CHAR64,  LL_TYPE_GROUP_CHAR,  0, CASTTO(LL_TYPE_INT64, LL_TYPE_UINT64));
	INSERT_BUILTIN_TYPE(LL_TYPE_BOOL1,   LL_TYPE_GROUP_BOOL,  0, ALL_INT());
	INSERT_BUILTIN_TYPE(LL_TYPE_BOOL8,   LL_TYPE_GROUP_BOOL,  0, ALL_INT());
	// INSERT_BUILTIN_TYPE(LL_TYPE_BOOL16,  LL_TYPE_GROUP_BOOL,  0, ALL_INT());
	INSERT_BUILTIN_TYPE(LL_TYPE_BOOL32,  LL_TYPE_GROUP_BOOL,  0, ALL_INT());
	// INSERT_BUILTIN_TYPE(LL_TYPE_BOOL64,  LL_TYPE_GROUP_BOOL,  0, ALL_INT());
	// INSERT_BUILTIN_TYPE(LL_TYPE_FLOAT16, LL_TYPE_GROUP_FLOAT, 0, CASTTO(LL_TYPE_FLOAT32, LL_TYPE_FLOAT64));
	INSERT_BUILTIN_TYPE(LL_TYPE_FLOAT32, LL_TYPE_GROUP_FLOAT, 0, CASTTO(LL_TYPE_FLOAT64));
	INSERT_BUILTIN_TYPE(LL_TYPE_FLOAT64, LL_TYPE_GROUP_FLOAT, 0, 0);

#undef INSERT_BUILTIN_TYPE
	return result;
}

#include <arm_neon.h>

void ll_typer_run(Compiler_Context* cc, LL_Typer* typer, LL_Parser* parser, Code* root) {
    uint32* old_counts = alloca(oc_len(parser->linear_grid));

    // for (uword i = 0; i < oc_len(parser->linear_grid); ++i) {
    //     uword aligned = oc_align_forward((uword)parser->linear_grid[i].types.count, 16);
    //     uword need_to_append = aligned - (uword)parser->linear_grid[i].types.count;
    //     parser_extend_zeroed_typecheck_value(cc, &parser->linear_grid[i], need_to_append);
    //     old_counts[i] = parser->linear_grid[i].types.count;
    // }

    // uint16x8_t zeros = vdupq_n_u16(0);
    // uint16x8_t ones = vdupq_n_u16(1);

    // assign
    for (uint32 i = 0; i < parser->ops->types.count; i += 8) {
        uint16 lhs_type = 1 << parser->ops_lhs->types.items[i];
        uint16 mask = parser->ops_rhs->types_implicit_cast.items[i];
        uint16 good = lhs_type & mask;

        if (good) {
            parser->ops->types.items[i] = parser->ops_lhs->types.items[i];
            parser->ops->types_cast.items[i] = parser->ops_lhs->types_cast.items[i];
            parser->ops->types_implicit_cast.items[i] = parser->ops_lhs->types_implicit_cast.items[i];
        } else {
            ll_typer_report_error(((LL_Error){}), "Can't assign value to variable");

            ll_typer_report_error_no_src("    variable is declared with type ");
            ll_typer_report_error_type(cc, typer, &parser->ops->types.items[i]);
            ll_typer_report_error_no_src(", but tried to initialize it with type ");
            ll_typer_report_error_type(cc, typer, &parser->ops_rhs->types.items[i]);
            ll_typer_report_error_no_src("\n");

            ll_typer_report_error_done(cc, typer);
        }

        // uint16x8_t lhs_type = vld1q_u16(&parser->ops_lhs->types.items[i]);
        // uint16x8_t lhs_cast = vld1q_u16(&parser->ops_lhs->types_cast.items[i]);
        // uint16x8_t lhs_implicit_cast = vld1q_u16(&parser->ops_lhs->types_implicit_cast.items[i]);
        // lhs_type = vshlq_u16(ones, lhs_type);

        // uint16x8_t mask = vld1q_u16(&parser->ops_rhs->types_implicit_cast.items[i]);
        // uint16x8_t good = vandq_u16(lhs_type, mask);

        // uint16x8_t rhs_type = vld1q_u16(&parser->ops_rhs->types.items[i]);
        // uint16x8_t rhs_cast = vld1q_u16(&parser->ops_rhs->types_cast.items[i]);

        // lhs_type = vbslq_u16(good, lhs_type, rhs_type);
        // lhs_cast = vbslq_u16(good, lhs_cast, rhs_cast);
        // lhs_implicit_cast = vbslq_u16(good, lhs_implicit_cast, mask);

        // vst1q_u16(&parser->ops->types.items[i], lhs_type);
        // vst1q_u16(&parser->ops->types_cast.items[i], lhs_cast);
        // vst1q_u16(&parser->ops->types_implicit_cast.items[i], lhs_implicit_cast);
    }

    // for (uword i = 0; i < oc_len(parser->linear_grid); ++i) {
    //     parser->linear_grid[i].types.count = old_counts[i];
    //     parser->linear_grid[i].types_cast.count = old_counts[i];
    //     parser->linear_grid[i].types_implicit_cast.count = old_counts[i];
    // }
}

LL_Type* ll_typer_get_type_from_typename(Compiler_Context* cc, LL_Typer* typer, Code* typename) {
    LL_Type* result = NULL;

    switch (typename->kind) {
    case CODE_KIND_GENERIC:
        break;
    case CODE_KIND_IDENT:
        if (CODE_AS(typename, Code_Ident)->str.ptr == LL_KEYWORD_LET.ptr) return NULL;

        LL_Scope* scope = ll_typer_find_symbol_up_scope(cc, typer, CODE_AS(typename, Code_Ident));
        if (!scope) {
            ll_typer_report_error(((LL_Error){ .main_token = CODE_AS(typename, Code_Ident)->base.token_info }), "Type '{}' not found", CODE_AS(typename, Code_Ident)->str);
            ll_typer_report_error_done(cc, typer);
        }

        switch (scope->kind) {
        case LL_SCOPE_KIND_STRUCT:
        case LL_SCOPE_KIND_TYPENAME:
            result = scope->declared_type;
            break;
        case LL_SCOPE_KIND_LOCAL:
        case LL_SCOPE_KIND_PARAMETER:
            if (scope->ident->base.has_const) {
                result = scope->ident->base.const_value.as_type;
                break;
            }
            // fallthrough

        default:
            ll_typer_report_error(((LL_Error){ .main_token = CODE_AS(typename, Code_Ident)->base.token_info }), "Expected '{}' to be a type, but it's not", CODE_AS(typename, Code_Ident)->str);
            ll_typer_report_error_done(cc, typer);
            break;
        }

        break;
    case CODE_KIND_TYPE_POINTER: {
        result = ll_typer_get_type_from_typename(cc, typer, CODE_AS(typename, Code_Type_Pointer)->element);
        if (!result) return NULL;
        result = ll_typer_get_ptr_type(cc, typer, result);
    } break;
    case CODE_KIND_INDEX: {
        oc_assert(CODE_AS(typename, Code_Slice)->stop == NULL);
        LL_Type* element_type = ll_typer_get_type_from_typename(cc, typer, CODE_AS(typename, Code_Slice)->ptr);
        if (!element_type) return NULL;
        ll_typer_type_expression(cc, typer, &CODE_AS(typename, Code_Slice)->start, NULL, NULL);

        uint64_t array_width;
        if (CODE_AS(typename, Code_Slice)->start->has_const) {
            array_width = CODE_AS(typename, Code_Slice)->start->const_value.as_u64;
        } else {
            LL_Eval_Value value = ll_eval_node(cc, cc->eval_context, cc->bir, CODE_AS(typename, Code_Slice)->start);
            array_width = value.as_u64;
        }

        result = ll_typer_get_array_type(cc, typer, element_type, array_width);
        break;
    }
    case CODE_KIND_SLICE: {
        LL_Type* element_type = ll_typer_get_type_from_typename(cc, typer, CODE_AS(typename, Code_Slice)->ptr);
        if (!element_type) return NULL;
        result = ll_typer_get_slice_type(cc, typer, element_type);
        break;
    }

    default: eprint("\x1b[31;1mTODO:\x1b[0m typename node {}\n", typename->kind);
    }

    typename->type = result;
    return result;
}

LL_Type* ll_typer_get_ptr_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* element_type) {
    LL_Type_Pointer ptr_type = { 0 };
    ptr_type.base.kind = LL_TYPE_POINTER;
    ptr_type.element_type = element_type;


    LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, (LL_Type*)&ptr_type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

    if (t) {
        res = *t;
    } else {
        res = oc_arena_dup(&cc->arena, &ptr_type, sizeof(ptr_type));
        MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
    }

    return res;
}

LL_Type* ll_typer_get_array_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* element_type, size_t size) {
    LL_Type_Array array_type = { 0 };
    array_type.base.kind = LL_TYPE_ARRAY;
    array_type.base.width = size;
    array_type.element_type = element_type;

    LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, (LL_Type*)&array_type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

    if (t) {
        res = *t;
    } else {
        res = oc_arena_dup(&cc->arena, &array_type, sizeof(array_type));
        MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
    }

    return res;
}

LL_Type* ll_typer_get_slice_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* element_type) {
    LL_Type_Slice slice_type = { 0 };
    slice_type.base.kind = LL_TYPE_SLICE;
    slice_type.element_type = element_type;

    LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, (LL_Type*)&slice_type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

    if (t) {
        res = *t;
    } else {
        res = oc_arena_dup(&cc->arena, &slice_type, sizeof(slice_type));
        MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
    }

    return res;
}

LL_Type* ll_typer_get_fn_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* return_type, LL_Type** parameter_types, size_t parameter_count, bool is_variadic) {
    LL_Type_Function fn_type = { 0 };
    fn_type.base.kind = LL_TYPE_FUNCTION;
    fn_type.return_type = return_type;
    fn_type.parameter_count = parameter_count;
    fn_type.parameters = parameter_types;
    fn_type.is_variadic = is_variadic;

    LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, (LL_Type*)&fn_type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

    if (t) {
        res = *t;
    } else {
        fn_type.parameters = oc_arena_dup(&cc->arena, fn_type.parameters, sizeof(*fn_type.parameters) * fn_type.parameter_count);
        res = oc_arena_dup(&cc->arena, &fn_type, sizeof(fn_type));
        MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
    }

    return res;
}

LL_Type* ll_typer_get_struct_type(Compiler_Context* cc, LL_Typer* typer, LL_Type** field_types, size_t field_count) {
    LL_Type_Struct struct_type = { 0 }; // have to zero for deterministic hashing
    struct_type.base.kind = LL_TYPE_STRUCT;
    struct_type.field_count = field_count;
    struct_type.fields = field_types;

    LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, (LL_Type*)&struct_type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

    if (t) {
        res = *t;
    } else {
        struct_type.fields  = oc_arena_dup(&cc->arena, struct_type.fields, sizeof(*struct_type.fields) * struct_type.field_count);
        struct_type.offsets = oc_arena_alloc(&cc->arena, sizeof(*struct_type.offsets) * struct_type.field_count);
        struct_type.has_offsets = false;

        // struct_type.offsets = oc_arena_dup(&cc->arena, struct_type.offsets, sizeof(*struct_type.offsets) * struct_type.field_count);
        res = oc_arena_dup(&cc->arena, &struct_type, sizeof(struct_type));
        MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
    }

    return res;
}


void ll_print_type_raw(LL_Type* type, Oc_Writer* w) {
    uint32_t i;
    switch (type->kind) {
    case LL_TYPE_UNKNOWN:     wprint(w, "unknown"); break;
    case LL_TYPE_INT8:     wprint(w, "int8"); break;
	case LL_TYPE_INT16:    wprint(w, "int16"); break;
	case LL_TYPE_INT32:    wprint(w, "int32"); break;
	case LL_TYPE_INT64:    wprint(w, "int64"); break;
    case LL_TYPE_UINT8:    wprint(w, "uint8"); break;
	case LL_TYPE_UINT16:   wprint(w, "uint16"); break;
	case LL_TYPE_UINT32:   wprint(w, "uint32"); break;
	case LL_TYPE_UINT64:   wprint(w, "uint64"); break;
    case LL_TYPE_BOOL1:    wprint(w, "bool1"); break;
    case LL_TYPE_BOOL8:    wprint(w, "bool8"); break;
    // case LL_TYPE_BOOL16:   wprint(w, "bool16"); break;
    case LL_TYPE_BOOL32:   wprint(w, "bool32"); break;
    // case LL_TYPE_BOOL64:   wprint(w, "bool64"); break;
    case LL_TYPE_CHAR8:    wprint(w, "char8"); break;
    case LL_TYPE_CHAR16:   wprint(w, "char16"); break;
    // case LL_TYPE_CHAR32:   wprint(w, "char32"); break;
    // case LL_TYPE_CHAR64:   wprint(w, "char64"); break;

    // case LL_TYPE_FLOAT16:  wprint(w, "float16"); break;
    case LL_TYPE_FLOAT32:  wprint(w, "float32"); break;
    case LL_TYPE_FLOAT64:  wprint(w, "float64"); break;

    case LL_TYPE_STRING:   wprint(w, "string"); break;
    case LL_TYPE_POINTER: {
        LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)type;
        ll_print_type_raw(ptr_type->element_type, w);
        wprint(w, "*");
        break;
    }
#if 0
    case LL_TYPE_ARRAY: {
        LL_Type_Array* array_type = (LL_Type_Array*)type;
        ll_print_type_raw(array_type->element_type, w);
        wprint(w, "[{}]", array_type->base.width);
        break;
    }
#endif
    case LL_TYPE_SLICE: {
        LL_Type_Array* array_type = (LL_Type_Array*)type;
        ll_print_type_raw(array_type->element_type, w);
        wprint(w, "[:]");
        break;
    }
    case LL_TYPE_FUNCTION: {
        LL_Type_Function* fn_type = (LL_Type_Function*)type;
        if (fn_type->return_type) ll_print_type_raw(fn_type->return_type, w);
        wprint(w, " (");
        for (i = 0; i < fn_type->parameter_count; ++i) {
            if (i > 0) wprint(w, ", ");
            if (fn_type->parameters[i]) ll_print_type_raw(fn_type->parameters[i], w);
        }
        wprint(w, ")");
        break;
    }
    case LL_TYPE_STRUCT: {
        LL_Type_Struct* struct_type = (LL_Type_Struct*)type;
        wprint(w, "struct {} {{ ", struct_type->field_count);
        for (i = 0; i < struct_type->field_count; ++i) {
            if (struct_type->fields[i]) {
                ll_print_type_raw(struct_type->fields[i], w);
                if (struct_type->offsets) {
                    wprint(w, "({})", struct_type->offsets[i]);
                }
                wprint(w, "; ");
            }
        }
        wprint(w, "}");
    } break;
    case LL_TYPE_NAMED:
        wprint(w, "named {}", ((LL_Type_Named*)type)->scope->ident->str);
        // wprint(w, "named {} (", ((LL_Type_Named*)type)->scope->ident->str);
        // ll_print_type_raw(((LL_Type_Named*)type)->actual_type, w);
        // wprint(w, ")");
        break;
    case LL_TYPE_TYPE:
        wprint(w, "type");
        break;
    default: oc_assert(false); break;
    }
}

void ll_print_type(LL_Type* type) {
    ll_print_type_raw(type, &stdout_writer);
    print("\n");
}


