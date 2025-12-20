
// #include <malloc.h>
// #include <string.h>

#include "common.h"
#include "typer.h"
#include "lexer.h"
#include "eval.h"

#define create_type(type) ({\
            __typeof__(type) t = type; \
            ll_intern_type(cc, typer, &t); \
        })

#define create_scope(kind, decl) create_scope_(cc, typer, kind, (Ast_Base*)(decl))

static LL_Scope* create_scope_(Compiler_Context* cc, LL_Typer* typer, LL_Scope_Kind kind, Ast_Base* decl) {
    (void)typer;
    LL_Scope* result;
    result = oc_arena_alloc(&cc->arena, sizeof(LL_Scope));
    result->kind = kind;
    result->ident = NULL;
    result->decl = decl;
    result->next_anon = 0;
    memset(&result->children, 0, sizeof(result->children));
    return result;
}

#define create_scope_simple(kind, decl) create_scope_simple_(cc, typer, kind, (Ast_Base*)(decl))

static LL_Scope_Simple* create_scope_simple_(Compiler_Context* cc, LL_Typer* typer, LL_Scope_Kind kind, Ast_Base* decl) {
    (void)typer;
    LL_Scope_Simple* result;
    result = oc_arena_alloc(&cc->arena, sizeof(LL_Scope_Simple));
    result->kind = kind;
    result->ident = NULL;
    result->decl = decl;
    return result;
}

#define create_scope_macro_parameter(decl) create_scope_macro_parameter_(cc, typer, (Ast_Base*)(decl))

static LL_Scope_Macro_Parameter* create_scope_macro_parameter_(Compiler_Context* cc, LL_Typer* typer, Ast_Base* decl) {
    (void)typer;
    LL_Scope_Macro_Parameter* result;
    result = oc_arena_alloc(&cc->arena, sizeof(LL_Scope_Macro_Parameter));
    result->kind = LL_SCOPE_KIND_MACRO_PARAMETER;
    result->ident = NULL;
    result->decl = decl;
    return result;
}

LL_Typer ll_typer_create(Compiler_Context* cc) {
    LL_Typer result = { 0 };
    LL_Typer* typer = &result;

    LL_Scope root_scope = {
        .kind = LL_SCOPE_KIND_PACKAGE,
    };
    result.root_scope = result.current_scope = oc_arena_dup(&cc->arena, &root_scope, sizeof(root_scope));

    #define INSERT_BUILTIN_TYPE(ty, keyword, ...) do {                \
        result.ty = create_type(((LL_Type) { __VA_ARGS__ }));                      \
        LL_Scope_Builtin* scope = oc_arena_alloc(&cc->arena, sizeof(*scope)); \
        scope->kind = LL_SCOPE_KIND_TYPENAME; \
        scope->ident = oc_arena_alloc(&cc->arena, sizeof(*scope->ident));     \
        scope->ident->resolved_scope = (LL_Scope*)scope;                                 \
        scope->ident->str = keyword;                                          \
        scope->declared_type = result.ty; \
        ll_typer_scope_put(cc, typer, (LL_Scope*)scope, false);                           \
    } while (0)
    #define INSERT_TYPE_SCOPE(ty, keyword) do {                \
        LL_Scope_Builtin* scope = oc_arena_alloc(&cc->arena, sizeof(*scope)); \
        scope->kind = LL_SCOPE_KIND_TYPENAME; \
        scope->ident = oc_arena_alloc(&cc->arena, sizeof(*scope->ident));     \
        scope->ident->resolved_scope = (LL_Scope*)scope;                                 \
        scope->ident->str = keyword;                                          \
        scope->declared_type = result.ty; \
        ll_typer_scope_put(cc, typer, (LL_Scope*)scope, false);                           \
    } while (0)
    #define INSERT_ANY_TYPE(ty, ...) do {                \
        result.ty = create_type(((LL_Type) { __VA_ARGS__ }));                      \
    } while (0)

    INSERT_BUILTIN_TYPE(ty_void, LL_KEYWORD_VOID, .kind = LL_TYPE_VOID);

    INSERT_BUILTIN_TYPE(ty_int8, LL_KEYWORD_INT8, .kind = LL_TYPE_INT, .width = 8);
    INSERT_BUILTIN_TYPE(ty_int16, LL_KEYWORD_INT16, .kind = LL_TYPE_INT, .width = 16);
    INSERT_BUILTIN_TYPE(ty_int32, LL_KEYWORD_INT32, .kind = LL_TYPE_INT, .width = 32);
    INSERT_BUILTIN_TYPE(ty_int64, LL_KEYWORD_INT64, .kind = LL_TYPE_INT, .width = 64);
    INSERT_TYPE_SCOPE(ty_int64, LL_KEYWORD_INT);
    INSERT_BUILTIN_TYPE(ty_uint8, LL_KEYWORD_UINT8, .kind = LL_TYPE_UINT, .width = 8);
    INSERT_BUILTIN_TYPE(ty_uint16, LL_KEYWORD_UINT16, .kind = LL_TYPE_UINT, .width = 16);
    INSERT_BUILTIN_TYPE(ty_uint32, LL_KEYWORD_UINT32, .kind = LL_TYPE_UINT, .width = 32);
    INSERT_BUILTIN_TYPE(ty_uint64, LL_KEYWORD_UINT64, .kind = LL_TYPE_UINT, .width = 64);
    INSERT_TYPE_SCOPE(ty_uint64, LL_KEYWORD_UINT);
    INSERT_ANY_TYPE(ty_anyint,.kind = LL_TYPE_ANYINT);

    INSERT_BUILTIN_TYPE(ty_float16, LL_KEYWORD_FLOAT16, .kind = LL_TYPE_FLOAT, .width = 16);
    INSERT_BUILTIN_TYPE(ty_float32, LL_KEYWORD_FLOAT32, .kind = LL_TYPE_FLOAT, .width = 32);
    INSERT_BUILTIN_TYPE(ty_float64, LL_KEYWORD_FLOAT64, .kind = LL_TYPE_FLOAT, .width = 64);
    INSERT_BUILTIN_TYPE(ty_float64, LL_KEYWORD_FLOAT, .kind = LL_TYPE_FLOAT, .width = 64);
    INSERT_ANY_TYPE(ty_anyfloat, .kind = LL_TYPE_ANYFLOAT);

    INSERT_BUILTIN_TYPE(ty_bool8, LL_KEYWORD_BOOL8, .kind = LL_TYPE_BOOL, .width = 8);
    INSERT_BUILTIN_TYPE(ty_bool16, LL_KEYWORD_BOOL16, .kind = LL_TYPE_BOOL, .width = 16);
    INSERT_BUILTIN_TYPE(ty_bool32, LL_KEYWORD_BOOL32, .kind = LL_TYPE_BOOL, .width = 32);
    INSERT_BUILTIN_TYPE(ty_bool64, LL_KEYWORD_BOOL64, .kind = LL_TYPE_BOOL, .width = 64);
    INSERT_BUILTIN_TYPE(ty_bool, LL_KEYWORD_BOOL, .kind = LL_TYPE_BOOL, .width = 1);
    INSERT_ANY_TYPE(ty_anybool, .kind = LL_TYPE_ANYBOOL);

    INSERT_BUILTIN_TYPE(ty_string, LL_KEYWORD_STRING, .kind = LL_TYPE_STRING);
    INSERT_BUILTIN_TYPE(ty_char, LL_KEYWORD_CHAR, .kind = LL_TYPE_CHAR, .width = 8);

    INSERT_ANY_TYPE(ty_type, .kind = LL_TYPE_TYPE);

    #undef INSERT_BUILTIN_TYPE
    #undef INSERT_TYPE_SCOPE
    #undef INSERT_ANY_TYPE

    return result;
}

typedef struct {
    LL_Type *expected, *actual;
    LL_Token_Info main_token;
    LL_Token_Info highlight_start, highlight_end;
} LL_Error;

#define ll_typer_report_error(error, fmt, ...) do { OC_MAP_SEQ(OC_MAKE_GENERIC1, __VA_ARGS__); ll_typer_report_error_raw((cc), (typer), (error), fmt OC_MAP_SEQ(OC_MAKE_GENERIC1_PARAM, __VA_ARGS__)); } while (0)
#define ll_typer_report_error_info(error, fmt, ...) do { OC_MAP_SEQ(OC_MAKE_GENERIC1, __VA_ARGS__); ll_typer_report_error_info_raw((cc), (typer), (error), fmt OC_MAP_SEQ(OC_MAKE_GENERIC1_PARAM, __VA_ARGS__)); } while (0)
#define ll_typer_report_error_note(error, fmt, ...) do { OC_MAP_SEQ(OC_MAKE_GENERIC1, __VA_ARGS__); ll_typer_report_error_note_raw((cc), (typer), (error), fmt OC_MAP_SEQ(OC_MAKE_GENERIC1_PARAM, __VA_ARGS__)); } while (0)
#define ll_typer_report_error_no_src(fmt, ...) do { OC_MAP_SEQ(OC_MAKE_GENERIC1, __VA_ARGS__); ll_typer_report_error_no_src_raw((cc), (typer), fmt OC_MAP_SEQ(OC_MAKE_GENERIC1_PARAM, __VA_ARGS__)); } while (0)

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

void ll_typer_run(Compiler_Context* cc, LL_Typer* typer, Ast_Base* node) {
    ll_typer_type_statement(cc, typer, &node);
}

LL_Type* ll_intern_type(Compiler_Context* cc, LL_Typer* typer, LL_Type* type) {
    LL_Type* res;
    LL_Type** t = MAP_GET(typer->interned_types, type, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);

    if (t) {
        res = *t;
    } else {
        res = oc_arena_dup(&cc->arena, type, sizeof(*type));
        MAP_PUT(typer->interned_types, res, res, &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED);
    }

    return res;
}

size_t ll_type_hash(LL_Type* type, size_t seed) {
    LL_Type** tts;
    switch (type->kind) {
    case LL_TYPE_POINTER: {
        LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)type;
        struct {
            LL_Type_Kind kind;
            uint32_t padding;
            LL_Type* element;
        } type_hash = {
            .kind = type->kind,
            .padding = 0,
            .element = ptr_type->element_type,
        };
        size_t hash = stbds_siphash_bytes(&type_hash, sizeof(type_hash), seed);
        return hash;
    }
    case LL_TYPE_FUNCTION: {
        LL_Type_Function* fn_type = (LL_Type_Function*)type;
        tts = alloca(sizeof(*tts) * (fn_type->parameter_count + 1 /* for return type */));
        tts[0] = fn_type->return_type;
        memcpy(tts + 1, fn_type->parameters, sizeof(*fn_type->parameters) * fn_type->parameter_count);

        return stbds_siphash_bytes(tts, sizeof(*tts) * (fn_type->parameter_count + 1 /* for return type */), seed);
    }
    case LL_TYPE_STRUCT: {
        LL_Type_Struct* struct_type = (LL_Type_Struct*)type;
        tts = alloca(sizeof(*tts) * struct_type->field_count);
        memcpy(tts, struct_type->fields, sizeof(*struct_type->fields) * struct_type->field_count);

        return stbds_siphash_bytes(tts, sizeof(*tts) * struct_type->field_count, seed);
    }
    default: return stbds_siphash_bytes(type, sizeof(*type), seed);
    }
}

bool ll_type_eql(LL_Type* a, LL_Type* b) {
    uint32_t i;
    if (a->kind != b->kind) return false;

    switch (a->kind) {
    case LL_TYPE_INT: return a->width == b->width;
    case LL_TYPE_UINT: return a->width == b->width;
    case LL_TYPE_FLOAT: return a->width == b->width;
    case LL_TYPE_BOOL: return a->width == b->width;
    case LL_TYPE_POINTER: {
        LL_Type_Pointer *fa = (LL_Type_Pointer*)a, *fb = (LL_Type_Pointer*)b;
        return fa->element_type == fb->element_type;
    }
    case LL_TYPE_FUNCTION: {
        LL_Type_Function *fa = (LL_Type_Function*)a, *fb = (LL_Type_Function*)b;
        if (fa->return_type != fb->return_type) return false;
        if (fa->parameter_count != fb->parameter_count) return false;
        for (i = 0; i < fb->parameter_count; ++i) {
            if (fa->parameters[i] != fb->parameters[i]) return false;
        }

        return true;
    } break;
    case LL_TYPE_STRUCT: {
        LL_Type_Struct *fa = (LL_Type_Struct*)a, *fb = (LL_Type_Struct*)b;
        if (fa->field_count != fb->field_count) return false;
        for (i = 0; i < fb->field_count; ++i) {
            if (fa->fields[i] != fb->fields[i]) return false;
        }

        return true;
    } break;
    case LL_TYPE_NAMED: {
        LL_Type_Named *fa = (LL_Type_Named*)a, *fb = (LL_Type_Named*)b;
        return fa->scope->ident->str.ptr == fb->scope->ident->str.ptr;
    } break;
    /* return stbds_siphash_bytes(type, sizeof(*type), seed); */
    default: return true;
    }
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

LL_Type* ll_typer_implicit_cast_tofrom(Compiler_Context* cc, LL_Typer* typer, LL_Type* from, LL_Type* to) {
    (void)cc;
    if (from == to) return to;

    if (from == typer->ty_anyint) {
        if (to->kind == LL_TYPE_INT) return to;
        else if (to->kind == LL_TYPE_UINT) return to;
        else if (to->kind == LL_TYPE_FLOAT) return to;
    } else if (from == typer->ty_anyint) {
        if (to->kind == LL_TYPE_FLOAT) return to;
    } else if (to->kind == LL_TYPE_INT) {
        if (from->kind == LL_TYPE_INT) return to;
        else if (from->kind == LL_TYPE_UINT) return to;
        else if (from->kind == LL_TYPE_FLOAT) return to;
    } else if (to->kind == LL_TYPE_UINT) {
        if (from->kind == LL_TYPE_INT) return to;
        else if (from->kind == LL_TYPE_UINT) return to;
        else if (from->kind == LL_TYPE_FLOAT) return to;
    } else if (from->kind == LL_TYPE_INT) {
        if (to->kind == LL_TYPE_INT) return to;
        else if (to->kind == LL_TYPE_UINT) return to;
        else if (to->kind == LL_TYPE_FLOAT) return to;
    } else if (from->kind == LL_TYPE_UINT) {
        if (to->kind == LL_TYPE_INT) return to;
        else if (to->kind == LL_TYPE_UINT) return to;
        else if (to->kind == LL_TYPE_FLOAT) return to;
    } else if (from->kind == LL_TYPE_BOOL) {
        if (to->kind == LL_TYPE_BOOL) return to;
    } else if (to->kind == LL_TYPE_BOOL) {
        if (from->kind == LL_TYPE_BOOL) return to;
        else if (from->kind == LL_TYPE_ANYBOOL) return to;
    }

    return NULL;
}

LL_Type* ll_typer_implicit_cast_leftright(Compiler_Context* cc, LL_Typer* typer, LL_Type* lhs, LL_Type* rhs) {
    if (lhs == rhs) return lhs;

    if (lhs == typer->ty_anyint) {
        if (rhs->kind == LL_TYPE_INT) return rhs;
        else if (rhs->kind == LL_TYPE_UINT) return rhs;
        else if (rhs->kind == LL_TYPE_FLOAT) return rhs;
        else if (rhs->kind == LL_TYPE_CHAR) return rhs;
        else if (rhs->kind == LL_TYPE_ANYINT) return lhs;
    } else if (rhs == typer->ty_anyint) {
        if (lhs->kind == LL_TYPE_INT) return lhs;
        else if (lhs->kind == LL_TYPE_UINT) return lhs;
        else if (lhs->kind == LL_TYPE_FLOAT) return lhs;
        else if (lhs->kind == LL_TYPE_CHAR) return lhs;
        else if (lhs->kind == LL_TYPE_ANYINT) return lhs;
    } else if (lhs->kind == LL_TYPE_INT) {
        if (rhs->kind == LL_TYPE_INT) return create_type(((LL_Type){ .kind = LL_TYPE_INT, .width = max(lhs->width, rhs->width) }));
        else if (rhs->kind == LL_TYPE_UINT) return create_type(((LL_Type){ .kind = LL_TYPE_INT, .width = max(lhs->width, rhs->width) }));
        else if (rhs->kind == LL_TYPE_FLOAT) return rhs;
    } else if (lhs->kind == LL_TYPE_UINT) {
        if (rhs->kind == LL_TYPE_INT) return create_type(((LL_Type){ .kind = LL_TYPE_INT, .width = max(lhs->width, rhs->width) }));
        else if (rhs->kind == LL_TYPE_UINT) return create_type(((LL_Type){ .kind = LL_TYPE_UINT, .width = max(lhs->width, rhs->width) }));
        else if (rhs->kind == LL_TYPE_FLOAT) return rhs;
    } else if (rhs->kind == LL_TYPE_INT) {
        if (rhs->kind == LL_TYPE_FLOAT) return rhs;
    } else if (rhs->kind == LL_TYPE_UINT) {
        if (rhs->kind == LL_TYPE_FLOAT) return rhs;
    } else if (lhs->kind == LL_TYPE_FLOAT && rhs->kind == LL_TYPE_FLOAT) {
        return create_type(((LL_Type){ .kind = LL_TYPE_FLOAT, .width = max(lhs->width, rhs->width) }));
    } else if (lhs == typer->ty_anybool) {
        if (rhs->kind == LL_TYPE_BOOL) return rhs;
    } else if (rhs == typer->ty_anybool) {
        if (lhs->kind == LL_TYPE_BOOL) return lhs;
    } else if (lhs->kind == LL_TYPE_BOOL && rhs->kind == LL_TYPE_BOOL) {
        return create_type(((LL_Type){ .kind = LL_TYPE_BOOL, .width = max(lhs->width, rhs->width) }));
    }

    return NULL;
}


LL_Type* ll_typer_type_statement(Compiler_Context* cc, LL_Typer* typer, Ast_Base** stmt) {
    uint32_t i;
    LL_Type** types;

    switch ((*stmt)->kind) {
    case AST_KIND_BLOCK: {
        Ast_Block* blk = AST_AS((*stmt), Ast_Block);
        LL_Scope* block_scope = NULL;

        if (
            typer->current_scope->kind != LL_SCOPE_KIND_LOOP
            && typer->current_scope->kind != LL_SCOPE_KIND_PACKAGE
        ) {
            if (blk->flags & AST_BLOCK_FLAG_MACRO_EXPANSION) {
                block_scope = create_scope(LL_SCOPE_KIND_MACRO_EXPANSION, *stmt);
            } else {
                block_scope = create_scope(LL_SCOPE_KIND_BLOCK_VALUE, *stmt);
            }

            ll_typer_scope_put(cc, typer, block_scope, false);
            typer->current_scope = block_scope;
        }

        blk->scope = block_scope;

        for (i = 0; i < blk->count; ++i) {
            ll_typer_type_statement(cc, typer, &blk->items[i]);
        }

        if (block_scope) {
            typer->current_scope = block_scope->parent;
        }
    } break;
    case AST_KIND_VARIABLE_DECLARATION: {
        Ast_Variable_Declaration* var_decl = AST_AS((*stmt), Ast_Variable_Declaration);
        LL_Type* declared_type = ll_typer_get_type_from_typename(cc, typer, var_decl->type);
        var_decl->ident->base.type = declared_type;

		LL_Scope* var_scope;
		if (typer->current_scope->kind == LL_SCOPE_KIND_STRUCT) {
			var_scope = create_scope(LL_SCOPE_KIND_FIELD, var_decl);
		} else {
			var_scope = create_scope(LL_SCOPE_KIND_LOCAL, var_decl);
		}
        var_scope->ident = var_decl->ident;
        if (var_decl->ident->flags & AST_IDENT_FLAG_EXPAND) {
            ll_typer_scope_put(cc, typer, (LL_Scope*)var_scope, true);
        } else {
            ll_typer_scope_put(cc, typer, (LL_Scope*)var_scope, false);
        }

        if (var_decl->initializer) {
            LL_Type* init_type;
            if (var_decl->ident->flags & AST_IDENT_FLAG_EXPAND) {
                init_type = ll_typer_type_expression(cc, typer, &var_decl->initializer, declared_type, NULL);
            } else {
                typer->current_scope = (LL_Scope*)var_scope;
                init_type = ll_typer_type_expression(cc, typer, &var_decl->initializer, declared_type, NULL);
                typer->current_scope = var_scope->parent;
            }

            if (declared_type) {
                if (!ll_typer_can_implicitly_cast_expression(cc, typer, var_decl->initializer, declared_type)) {
                    oc_assert(init_type != NULL);
                    oc_assert(declared_type != NULL);
                    ll_typer_report_error(((LL_Error){ .main_token = var_decl->base.token_info }), "Can't assign value to variable");

                    ll_typer_report_error_no_src("    variable is declared with type ");
                    ll_typer_report_error_type(cc, typer, declared_type);
                    ll_typer_report_error_no_src(", but tried to initialize it with type ");
                    ll_typer_report_error_type(cc, typer, init_type);
                    ll_typer_report_error_no_src("\n");

                    ll_typer_report_error_done(cc, typer);
                }

                ll_typer_add_implicit_cast(cc, typer, &var_decl->initializer, declared_type);
            } else {
                var_decl->ident->base.type = init_type;
            }

            if (var_decl->storage_class & LL_STORAGE_CLASS_CONST) {
                if (!var_decl->initializer->has_const) {
                    ll_typer_report_error(((LL_Error){ .main_token = var_decl->initializer->token_info }), "Can't assign runtime value to constant variable.");
                    ll_typer_report_error_no_src("    try wrapping initializer with `const` keyword\n");
                    ll_typer_report_error_done(cc, typer);
                }

                var_decl->ident->base.has_const = true;
                var_decl->ident->base.const_value = var_decl->initializer->const_value;
                var_decl->base.has_const = true;
                var_decl->base.const_value = var_decl->initializer->const_value;
            }
        }

		if (typer->current_scope->kind == LL_SCOPE_KIND_STRUCT) {
            var_decl->ir_index = typer->current_record->count;
			oc_array_append(&cc->arena, typer->current_record, declared_type);
			if (var_decl->initializer) {
				LL_Eval_Value value = ll_eval_node(cc, cc->eval_context, cc->bir, var_decl->initializer);
				oc_array_append(&cc->tmp_arena, typer->current_record_values, ((LL_Typer_Record_Value){ .field_scope = var_scope, .value = value, .has_init = true }));
			} else {
				oc_array_append(&cc->tmp_arena, typer->current_record_values, ((LL_Typer_Record_Value){ .has_init = false }));
			}
		}

        break;
    }
    case AST_KIND_FUNCTION_DECLARATION: {
        Ast_Function_Declaration* fn_decl = AST_AS((*stmt), Ast_Function_Declaration);

        LL_Scope* fn_scope = create_scope(LL_SCOPE_KIND_FUNCTION, fn_decl);
        fn_scope->ident = fn_decl->ident;
        fn_decl->ident->resolved_scope = fn_scope;

        ll_typer_scope_put(cc, typer, fn_scope, false);
        bool did_variadic = false;
        bool did_default = false;

        typer->current_scope = fn_scope;
        if (fn_decl->parameters.count) {
            types = alloca(sizeof(*types) * fn_decl->parameters.count);
            for (i = 0; i < fn_decl->parameters.count; ++i) {
                Ast_Parameter* parameter = &fn_decl->parameters.items[i];

                if (did_variadic) {
                    ll_typer_report_error(((LL_Error){ .highlight_start = parameter->type->token_info, .highlight_end = parameter->ident->base.token_info }), "No parameters can be after variadic parameter");
                    ll_typer_report_error_done(cc, typer);
                    break;
                }
                if (parameter->flags & LL_PARAMETER_FLAG_VARIADIC) {
                    if (parameter->type) {
                        ll_typer_report_error(((LL_Error){ .main_token = parameter->base.token_info }), "Typed variadic parameter not supported yet");
                        ll_typer_report_error_done(cc, typer);
                    }
                    did_variadic = true;
                } else {
                    if (parameter->type) {
                        types[i] = ll_typer_get_type_from_typename(cc, typer, parameter->type);
                        if (!types[i]) {
                            fn_decl->storage_class |= LL_STORAGE_CLASS_POLYMORPHIC;
                        }
                    }
                }

                if (parameter->initializer) {
                    LL_Type* init_type = ll_typer_type_expression(cc, typer, &parameter->initializer, types[i], NULL);

                    if (!ll_typer_implicit_cast_tofrom(cc, typer, init_type, types[i])) {
                        ll_typer_report_error(((LL_Error){ .main_token = parameter->base.token_info }), "Provided default parameter type does not match declared type of parameter");

                        ll_typer_report_error_no_src("    parameter is declared with type ");
                        ll_typer_report_error_type(cc, typer, types[i]);
                        ll_typer_report_error_no_src(", but tried to assign default value with type ");
                        ll_typer_report_error_type(cc, typer, init_type);
                        ll_typer_report_error_no_src("\n");

                        ll_typer_report_error_done(cc, typer);
                    }

                    did_default = true;
                } else if (did_default) {
                    ll_typer_report_error(((LL_Error){ .highlight_start = parameter->type->token_info, .highlight_end = parameter->ident->base.token_info }), "Positional arguments can only be before default arguments");
                    ll_typer_report_error_done(cc, typer);
                }

                parameter->ir_index = i;
                    
                // If we don't have an identifier it should be a generic without a name
                // @TODO: throw error if it's not a generic and doesn't ahve identifier
                if (parameter->ident) {
                    parameter->ident->base.type = types[i];
                    LL_Scope_Simple* param_scope = create_scope_simple(LL_SCOPE_KIND_PARAMETER, parameter);
                    param_scope->ident = parameter->ident;
                    ll_typer_scope_put(cc, typer, (LL_Scope*)param_scope, false);
                }
            }
        } else types = NULL;

        LL_Type* return_type;
        if (fn_decl->return_type) {
            return_type = ll_typer_get_type_from_typename(cc, typer, fn_decl->return_type);
        } else {
            return_type = typer->ty_void;
        }

        LL_Type* fn_type = ll_typer_get_fn_type(cc, typer, return_type, types, fn_decl->parameters.count, did_variadic);
        (*stmt)->type = fn_type;
        fn_decl->ident->base.type = fn_type;
        fn_decl->ident->resolved_scope = fn_scope;

        LL_Type_Function* last_fn = typer->current_fn;
        typer->current_fn = (LL_Type_Function*)fn_type;

        if (fn_decl->body) {
            if (fn_decl->storage_class & LL_STORAGE_CLASS_EXTERN) {
                Ast_Block* blk = AST_AS(fn_decl->body, Ast_Block);
                ll_typer_report_error(((LL_Error) { .main_token = blk->c_open, .highlight_start = blk->c_open, .highlight_end = blk->c_close }), "Extern function shouldn't have a body");
                ll_typer_report_error_done(cc, typer);
            }
            if ((fn_decl->storage_class & LL_STORAGE_CLASS_MACRO) == 0 && (fn_decl->storage_class & LL_STORAGE_CLASS_POLYMORPHIC) == 0) {
                ll_typer_type_statement(cc, typer, &fn_decl->body);
            }
        }

        typer->current_scope = fn_scope->parent;
        typer->current_fn = last_fn;

        break;
    }
    case AST_KIND_STRUCT: {
        Ast_Struct* strct = AST_AS(*stmt, Ast_Struct);

        LL_Scope* struct_scope = create_scope(LL_SCOPE_KIND_STRUCT, *stmt);
        struct_scope->ident = strct->ident;
        ll_typer_scope_put(cc, typer, struct_scope, false);

        LL_Type_Named named_type = { 0 };
        named_type.base.kind = LL_TYPE_NAMED;
        named_type.scope = struct_scope;

        struct_scope->decl->type = oc_arena_dup(&cc->arena, &named_type, sizeof(named_type));
        struct_scope->declared_type = struct_scope->decl->type;
        struct_scope->ident->base.type = struct_scope->decl->type;

        LL_Typer_Current_Record record = { 0 };
        LL_Typer_Record_Values record_values = { 0 };
        
        LL_Type_Named* last_named = typer->current_named;
        LL_Typer_Current_Record* last_current_record = typer->current_record;
        LL_Typer_Record_Values* last_current_record_values = typer->current_record_values;

        typer->current_named = (LL_Type_Named*)struct_scope->decl->type;
        typer->current_record = &record;
        typer->current_record_values = &record_values;
        typer->current_scope = struct_scope;

        for (i = 0; i < strct->body.count; ++i) {
            ll_typer_type_statement(cc, typer, &strct->body.items[i]);
        }

        typer->current_scope = struct_scope->parent;
        typer->current_record_values = last_current_record_values;
        typer->current_record = last_current_record;
        typer->current_named = last_named;

        LL_Type* actual_class_type = ll_typer_get_struct_type(cc, typer, record.items, record.count);
        ((LL_Type_Named*)struct_scope->decl->type)->actual_type = actual_class_type;
    } break;
    case AST_KIND_CONST: {
        Ast_Marker* cf = AST_AS((*stmt), Ast_Marker);
        (void)ll_typer_type_statement(cc, typer, &cf->expr);
        (void)ll_eval_node(cc, cc->eval_context, cc->bir, cf->expr);
    } break;
    default: return ll_typer_type_expression(cc, typer, stmt, NULL, NULL);
    }

    return NULL;
}

static LL_Eval_Value const_value_cast(LL_Eval_Value from, LL_Type* from_type, LL_Type* to_type) {
    LL_Eval_Value result;
    if (from_type == to_type) return from;

    switch (from_type->kind) {
    case LL_TYPE_ANYINT:
    case LL_TYPE_INT:
        switch (to_type->kind) {
        case LL_TYPE_INT:
            result.as_i64 = from.as_i64;
            break;
        case LL_TYPE_UINT:
            result.as_u64 = from.as_i64;
            break;
        case LL_TYPE_FLOAT:
            result.as_f64 = from.as_i64;
            break;
        default: ll_print_type(to_type); oc_todo("implement const cast"); break;
        }
        break;
    case LL_TYPE_UINT:
        switch (to_type->kind) {
        case LL_TYPE_UINT:
            result.as_u64 = from.as_u64;
            break;
        case LL_TYPE_ANYINT:
        case LL_TYPE_INT:
            result.as_i64 = from.as_u64;
            break;
        case LL_TYPE_FLOAT:
            result.as_f64 = from.as_i64;
            break;
        default: oc_todo("implement const cast"); break;
        }
        break;
    default: ll_print_type(from_type); oc_todo("implement const cast"); break;
    }

    return result;
}

bool ll_typer_can_cast(Compiler_Context* cc, LL_Typer* typer, LL_Type* src_type, LL_Type* dst_type) {
    (void)cc;
    (void)typer;
    if (src_type == dst_type) {
        return true;
    }

    switch (src_type->kind) {
    case LL_TYPE_CHAR:
        switch (dst_type->kind) {
        case LL_TYPE_CHAR:
        case LL_TYPE_INT:
        case LL_TYPE_UINT:
            return true;
        default: break;
        }
        break;

    case LL_TYPE_INT:
        switch (dst_type->kind) {
        case LL_TYPE_CHAR:
        case LL_TYPE_INT:
        case LL_TYPE_UINT:
        case LL_TYPE_ANYINT:
        case LL_TYPE_FLOAT:
            return true;
        default: break;
        }
        break;
    case LL_TYPE_UINT:
        switch (dst_type->kind) {
        case LL_TYPE_CHAR:
        case LL_TYPE_INT:
        case LL_TYPE_UINT:
        case LL_TYPE_ANYINT:
        case LL_TYPE_FLOAT:
            return true;
        default: break;
        };
        break;
    case LL_TYPE_ARRAY:
        switch (dst_type->kind) {
        case LL_TYPE_SLICE:
            return ((LL_Type_Slice*)dst_type)->element_type == ((LL_Type_Array*)src_type)->element_type;
        case LL_TYPE_STRING:
            return ((LL_Type_Array*)src_type)->element_type == typer->ty_char;
        default: break;
        }
        break;
    case LL_TYPE_POINTER:
        switch (dst_type->kind) {
        case LL_TYPE_POINTER:
            return true;
        default: break;
        }
        break;
    case LL_TYPE_FLOAT: {
        switch (dst_type->kind) {
        case LL_TYPE_INT:
        case LL_TYPE_UINT:
        case LL_TYPE_FLOAT:
            return true;
        default: break;
        };
    }

    default: break;
    }

    return false;
}

bool ll_typer_can_implicitly_cast(Compiler_Context* cc, LL_Typer* typer, LL_Type* src_type, LL_Type* dst_type) {
    (void)cc;
    (void)typer;
    if (src_type == dst_type) {
        return true;
    }

    switch (src_type->kind) {
    case LL_TYPE_ANYINT:
        switch (dst_type->kind) {
        case LL_TYPE_INT:
        case LL_TYPE_UINT:
            return true;
        default: break;
        }
        break;
    case LL_TYPE_INT:
        switch (dst_type->kind) {
        case LL_TYPE_INT:
            if (dst_type->width >= src_type->width) return true;
            break;
        case LL_TYPE_ANYINT:
            return true;
        default: break;
        }
        break;
    case LL_TYPE_UINT:
        switch (dst_type->kind) {
        case LL_TYPE_INT:
            if (dst_type->width >  src_type->width) return true;
            break;
        case LL_TYPE_UINT:
            if (dst_type->width >= src_type->width) return true;
            break;
        case LL_TYPE_ANYINT:
            return true;
        default: break;
        }
        break;
    case LL_TYPE_STRING:
        switch (dst_type->kind) {
        case LL_TYPE_SLICE:
            return ((LL_Type_Slice*)dst_type)->element_type == typer->ty_char;
        default: break;
        }
        break;

    case LL_TYPE_ARRAY:
        switch (dst_type->kind) {
        case LL_TYPE_SLICE:
            return ((LL_Type_Slice*)dst_type)->element_type == ((LL_Type_Array*)src_type)->element_type;
        case LL_TYPE_STRING:
            return ((LL_Type_Array*)src_type)->element_type == typer->ty_char;
        default: break;
        }
        break;
    case LL_TYPE_POINTER:
        switch (dst_type->kind) {
        case LL_TYPE_POINTER:
            return ((LL_Type_Pointer*)src_type)->element_type == typer->ty_void || ((LL_Type_Pointer*)dst_type)->element_type == typer->ty_void;
        default: break;
        }
        break;
    default: break;
    }

    return false;
}

bool ll_typer_can_implicitly_cast_const_value(Compiler_Context* cc, LL_Typer* typer, LL_Type* src_type, LL_Eval_Value* src_value, LL_Type* dst_type) {
    (void)cc;
    (void)typer;
    if (src_type == dst_type) {
        return true;
    }

    switch (src_type->kind) {
    case LL_TYPE_ANYINT:
    case LL_TYPE_INT:
        switch (dst_type->kind) {
        case LL_TYPE_INT: {
            int64_t cmp = (uint64_t)((int64_t)(1ull << 63ull) >> (dst_type->width - 1)) >> (64ull - dst_type->width);
            int64_t high_cmp = cmp & ~(1ull << (dst_type->width - 1));
            int64_t low_cmp = 1ull << (dst_type->width - 1);

            if (src_value->as_i64 <= high_cmp && src_value->as_i64 >= low_cmp) return true;
        } break;
        case LL_TYPE_UINT: {
            uint64_t cmp = (uint64_t)((int64_t)(1ull << 63ull) >> (dst_type->width - 1)) >> (64ull - dst_type->width);
            if (src_value->as_i64 >= 0 && (uint64_t)src_value->as_i64 <= cmp) return true;
        } break;
        case LL_TYPE_FLOAT:
            return true;
        default: break;
        }
    case LL_TYPE_UINT:
        switch (dst_type->kind) {
        case LL_TYPE_INT: {
            uint64_t cmp = (uint64_t)((int64_t)(1ull << 63ull) >> (dst_type->width - 1)) >> (64ull - dst_type->width);
            uint64_t high_cmp = cmp & ~(1ull << (dst_type->width - 1));

            if (src_value->as_u64 <= high_cmp) return true;
        } break;
        case LL_TYPE_UINT: {
            uint64_t cmp = (uint64_t)((int64_t)(1ull << 63ull) >> (dst_type->width - 1)) >> (64ull - dst_type->width);
            if (src_value->as_u64 <= cmp) return true;
        } break;
        case LL_TYPE_FLOAT:
            return true;
        default: break;
        }
    default: break;
    }

    return false;
}

bool ll_typer_can_implicitly_cast_expression(Compiler_Context* cc, LL_Typer* typer, Ast_Base* expr, LL_Type* dst_type) {
    LL_Type* src_type = expr->type;

    if (ll_typer_can_implicitly_cast(cc, typer, src_type, dst_type)) return true;
    if (expr->has_const) {
        if (ll_typer_can_implicitly_cast_const_value(cc, typer, src_type, &expr->const_value, dst_type)) {
            return true;
        }
    }

    return false;
}

void ll_typer_add_implicit_cast(Compiler_Context* cc, LL_Typer* typer, Ast_Base** expr, LL_Type* expected_type) {
    (void)typer;
    if ((*expr)->type == expected_type) {
        return;
    }

    Ast_Cast cast = {
        .base.kind = AST_KIND_CAST,
        .base.type = expected_type,
        .expr = *expr,
    };

    if ((*expr)->has_const) {
        (*expr)->const_value = const_value_cast((*expr)->const_value, (*expr)->type, expected_type);
    }

    Ast_Base* new_node = oc_arena_dup(&cc->arena, &cast, sizeof(cast));
    *expr = new_node;
}

LL_Type* ll_typer_type_expression(Compiler_Context* cc, LL_Typer* typer, Ast_Base** expr, LL_Type* expected_type, LL_Typer_Resolve_Result *resolve_result) {
    LL_Type* result;
    size_t i;
    switch ((*expr)->kind) {
    case AST_KIND_BLOCK: {
        LL_Type* last_block_type = typer->block_type;
        typer->block_type = expected_type;
        Ast_Block* blk = AST_AS((*expr), Ast_Block);
        blk->base.type = expected_type;

        LL_Scope* block_scope = NULL; 
        if (typer->current_scope->kind != LL_SCOPE_KIND_LOOP) {
            if (blk->flags & AST_BLOCK_FLAG_MACRO_EXPANSION) {
                block_scope = create_scope(LL_SCOPE_KIND_MACRO_EXPANSION, *expr);
            } else {
                block_scope = create_scope(LL_SCOPE_KIND_BLOCK_VALUE, *expr);
            }

            ll_typer_scope_put(cc, typer, block_scope, false);
            typer->current_scope = block_scope;
        }

        blk->scope = block_scope;

        for (i = 0; i < AST_AS((*expr), Ast_Block)->count; ++i) {
            ll_typer_type_statement(cc, typer, &AST_AS((*expr), Ast_Block)->items[i]);
        }

        if (block_scope) {
            typer->current_scope = block_scope->parent;
        }

        result = typer->block_type;
        typer->block_type = last_block_type;
        break;
    }
    case AST_KIND_IDENT: {
        if (AST_AS((*expr), Ast_Ident)->str.ptr == LL_KEYWORD_TRUE.ptr || AST_AS((*expr), Ast_Ident)->str.ptr == LL_KEYWORD_FALSE.ptr) {
            if (expected_type->kind == LL_TYPE_BOOL) {
                result = expected_type;
                break;
            }
            result = typer->ty_anybool;
            break;
        } else if (AST_AS((*expr), Ast_Ident)->str.ptr == LL_KEYWORD_NULL.ptr) {
            if (expected_type->kind == LL_TYPE_POINTER) {
                result = expected_type;
                break;
            }
            result = ll_typer_get_ptr_type(cc, typer, typer->ty_void);
            break;
        }
        LL_Scope* scope = ll_typer_find_symbol_up_scope(cc, typer, AST_AS((*expr), Ast_Ident));
        if (!scope) {
            ll_typer_report_error(((LL_Error){ .main_token = AST_AS((*expr), Ast_Ident)->base.token_info }), "Symbol '{}' not found", AST_AS((*expr), Ast_Ident)->str);
            ll_typer_report_error_done(cc, typer);
            return NULL;
        }

        Ast_Base* possible_const = (Ast_Base*)scope->ident;

        switch (scope->kind) {
        case LL_SCOPE_KIND_MACRO_PARAMETER: {
            LL_Scope_Macro_Parameter* macro_param = (LL_Scope_Macro_Parameter*)scope;
            *expr = ast_clone_node_deep(cc, macro_param->value, (LL_Ast_Clone_Params) { .convert_all_idents_to_expansion = true });
            possible_const = *expr;

            result = ll_typer_type_expression(cc, typer, expr, expected_type, resolve_result);
            if (macro_param->decl->type && macro_param->decl->type->kind != LL_TYPE_VOID) {
                // e.g if we pass a type int[5] into int[:], we need to handle that implicit cast every time we substitute
                ll_typer_add_implicit_cast(cc, typer, expr, macro_param->decl->type);
            }
            result = (*expr)->type;
        } break;
        case LL_SCOPE_KIND_TYPENAME:
        case LL_SCOPE_KIND_STRUCT: {
            (*expr)->has_const = 1;
            (*expr)->const_value.as_type = scope->declared_type;
            result = typer->ty_type;
        } break;
        default: {
            AST_AS((*expr), Ast_Ident)->resolved_scope = scope;

            if (resolve_result) {
                resolve_result->scope = scope;
            }

            if (expected_type) {
                if (ll_typer_can_implicitly_cast(cc, typer, scope->ident->base.type, expected_type)) {
                    result = expected_type;
                } else {
                    result = scope->ident->base.type;
                }
            } else {
                result = scope->ident->base.type;
            }
        } break;
        }

        if (possible_const->has_const) {
            (*expr)->has_const = 1;
            (*expr)->const_value = possible_const->const_value;
        }

        break;
    }
    case AST_KIND_TYPE_POINTER: {
        Ast_Base** element = &AST_AS((*expr), Ast_Type_Pointer)->element;
        result = ll_typer_type_expression(cc, typer, element, NULL, NULL);
        if (!result) return NULL;
        if ((*element)->has_const) {
            result = ll_typer_get_ptr_type(cc, typer, (*element)->const_value.as_type);
            (*expr)->has_const = 1;
            (*expr)->const_value.as_type = result;

            result = typer->ty_type;
        } else oc_todo("handle runtime");
    } break;
    case AST_KIND_LITERAL_INT:
        (*expr)->has_const = 1u;
        (*expr)->const_value.as_i64 = (int64_t)AST_AS((*expr), Ast_Literal)->u64;

        if (expected_type) {
            switch (expected_type->kind) {
            case LL_TYPE_UINT:
            case LL_TYPE_INT:
            case LL_TYPE_FLOAT:
            case LL_TYPE_CHAR:
                result = expected_type;
                break;
            default:
                result = typer->ty_anyint;
                break;
            }
        } else {
            result = typer->ty_anyint;
        }
        break;
    case AST_KIND_LITERAL_FLOAT:
        (*expr)->has_const = 1u;
        (*expr)->const_value.as_f64 = (int64_t)AST_AS((*expr), Ast_Literal)->f64;

        if (expected_type) {
            switch (expected_type->kind) {
            case LL_TYPE_FLOAT:
                result = expected_type;
                break;
            default:
                result = typer->ty_anyfloat;
                break;
            }
        } else {
            result = typer->ty_anyfloat;
        }
        break;
    case AST_KIND_LITERAL_STRING:
        result = ll_typer_get_array_type(cc, typer, typer->ty_char, AST_AS((*expr), Ast_Literal)->str.len);
        // result = typer->ty_string;
        break;
    case AST_KIND_ARRAY_INITIALIZER: {
        Ast_Initializer* init = AST_AS((*expr), Ast_Initializer);
        uint8_t* provided_elements = alloca(init->count * sizeof(*provided_elements));
        memset(provided_elements, 0, init->count * sizeof(*provided_elements));

        if (expected_type) {
            oc_assert(expected_type->kind == LL_TYPE_ARRAY);
            LL_Type_Array* arr_type = (LL_Type_Array*)expected_type;
            uint32_t element_index = 0;
            for (i = 0; i < init->count; ++i, ++element_index) {
                LL_Type* provided_type;
                if (init->items[i]->kind == AST_KIND_KEY_VALUE) {
                    Ast_Key_Value* kv = AST_AS((*expr), Ast_Key_Value);
                    LL_Eval_Value key = ll_eval_node(cc, cc->eval_context, cc->bir, kv->key);
                    element_index = (uint32_t)key.as_u64;
                    provided_type = ll_typer_type_expression(cc, typer, &kv->value, arr_type->element_type, NULL);
                } else {
                    provided_type = ll_typer_type_expression(cc, typer, &init->items[i], arr_type->element_type, NULL);
                }

                if (provided_elements[element_index]) {
                    eprint("\x1b[31;1merror\x1b[0;1m: a value for the index %u was provided more than once\n", element_index);
                }

                provided_elements[element_index] = 1u;

                if (!ll_typer_implicit_cast_tofrom(cc, typer, provided_type, arr_type->element_type)) {
                    oc_assert(provided_type != NULL);
                    oc_assert(arr_type->element_type != NULL);
                    eprint("\x1b[31;1merror\x1b[0;1m: array initializer element does not match declared type of array! Expected ");
                    ll_typer_report_error_type(cc, typer, arr_type->element_type);
                    eprint(" but got ");
                    ll_typer_report_error_type(cc, typer, provided_type);
                    eprint("\n");
                }
            }
            result = expected_type;
        } else {
            oc_assert(false);
            result = NULL;
        }
        break;
    }
    case AST_KIND_BINARY_OP: {
        Ast_Operation* opr = AST_AS((*expr), Ast_Operation);

        // handle dot member access
        switch (opr->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '.': {
            LL_Typer_Resolve_Result result = { 0 };	
            ll_typer_type_expression(cc, typer, &opr->left, NULL, &result);

            if (opr->right->kind != AST_KIND_IDENT) {
                ll_typer_report_error(((LL_Error){ .main_token = opr->right->token_info }), "Member should be identifier");
                ll_typer_report_error_done(cc, typer);
            }
            Ast_Ident* right_ident = AST_AS(opr->right, Ast_Ident);

            if (result.scope) {
                LL_Scope* member_scope;

                LL_Type* base_type = result.scope->ident->base.type;
                LL_Scope* base_scope = NULL;
                if (base_type->kind == LL_TYPE_POINTER) base_type = ((LL_Type_Pointer*)base_type)->element_type;
                while (base_type->kind == LL_TYPE_NAMED) {
                    base_scope = ((LL_Type_Named*)base_type)->scope;
                    base_type = ((LL_Type_Named*)base_type)->actual_type;
                }

                if (base_type->kind == LL_TYPE_SLICE || base_type->kind == LL_TYPE_STRING) {
                    if (string_eql(right_ident->str, lit("data"))) {
                        switch (base_type->kind) {
                        case LL_TYPE_SLICE:
                            right_ident->base.type = ((LL_Type_Slice*)base_type)->element_type;
                            break;
                        case LL_TYPE_ARRAY:
                            right_ident->base.type = ((LL_Type_Array*)base_type)->element_type;
                            break;
                        case LL_TYPE_STRING:
                            right_ident->base.type = typer->ty_char;
                            break;
                        default: oc_unreachable("invalid type"); break;
                        }
                    } else if (string_eql(right_ident->str, lit("length"))) {
                        right_ident->base.type = typer->ty_uint64;
                    }

                    (*expr)->type = right_ident->base.type;
                    return right_ident->base.type;
                } else if (base_type->kind == LL_TYPE_ARRAY) {
                    if (string_eql(right_ident->str, lit("length"))) {
                        (*expr)->has_const = true;
                        (*expr)->const_value.as_u64 = base_type->width;

                        right_ident->base.type = typer->ty_uint64;
                    }

                    (*expr)->type = right_ident->base.type;
                    return right_ident->base.type;
                }

                if (!base_scope) {
                    goto TRY_MEMBER_FUNCTION_CALL;
                }

                member_scope = ll_scope_get(base_scope, right_ident->str);
                if (!member_scope) {
                    goto TRY_MEMBER_FUNCTION_CALL;
                }
                
                oc_assert(member_scope->kind == LL_SCOPE_KIND_FIELD);
                right_ident->resolved_scope = member_scope;
                right_ident->base.type = member_scope->ident->base.type;

                if (resolve_result) {
                    resolve_result->scope = member_scope;
                }

                (*expr)->type = right_ident->base.type;
                return right_ident->base.type;
            } else {
TRY_MEMBER_FUNCTION_CALL:
                ll_typer_type_expression(cc, typer, &opr->right, NULL, &result);
                if (result.scope->kind == LL_SCOPE_KIND_FUNCTION) {
                    LL_Type_Function* fn_type = (LL_Type_Function*)result.scope->ident->base.type;
                    Ast_Function_Declaration* fn = AST_AS(result.scope->decl, Ast_Function_Declaration);
                    oc_assert(fn_type->base.kind == LL_TYPE_FUNCTION);

                    if (fn_type->parameter_count > 0) {
                        LL_Type* member_arg_parameter = fn_type->parameters[0];
                        if (!member_arg_parameter || fn->parameters.items[0].base.kind == AST_KIND_GENERIC) {
                            right_ident->resolved_scope = result.scope;
                            right_ident->base.type = (LL_Type*)fn_type;

                            if (resolve_result) {
                                resolve_result->scope = result.scope;
                                resolve_result->this_arg = &opr->left;
                            }

                            (*expr)->type = right_ident->base.type;
                            return right_ident->base.type;                           
                        }

                        if (member_arg_parameter->kind == LL_TYPE_POINTER && opr->left->type->kind != LL_TYPE_POINTER) {
                            // auto reference
                            member_arg_parameter = ((LL_Type_Pointer*)member_arg_parameter)->element_type;
                        }

                        if (ll_typer_implicit_cast_tofrom(cc, typer, opr->left->type, member_arg_parameter)) {
                            right_ident->resolved_scope = result.scope;
                            right_ident->base.type = (LL_Type*)fn_type;

                            if (resolve_result) {
                                resolve_result->scope = result.scope;
                                resolve_result->this_arg = &opr->left;
                            }

                            (*expr)->type = right_ident->base.type;
                            return right_ident->base.type;
                        }
                    }
                }

                ll_typer_report_error(((LL_Error){ .main_token = right_ident->base.token_info }), "Member or function '{}' not found", right_ident->str);
                ll_typer_report_error_done(cc, typer);
                return NULL;
            }
        }
        case LL_TOKEN_KIND_ASSIGN_PERCENT:
        case LL_TOKEN_KIND_ASSIGN_DIVIDE:
        case LL_TOKEN_KIND_ASSIGN_TIMES:
        case LL_TOKEN_KIND_ASSIGN_MINUS:
        case LL_TOKEN_KIND_ASSIGN_PLUS: {
            LL_Type* lhs_type = ll_typer_type_expression(cc, typer, &opr->left, NULL, NULL);
            LL_Type* rhs_type = ll_typer_type_expression(cc, typer, &opr->right, NULL, NULL);

            if (lhs_type->kind == LL_TYPE_ANYINT && rhs_type->kind == LL_TYPE_ANYINT && expected_type) {
                lhs_type = ll_typer_type_expression(cc, typer, &opr->left, expected_type, NULL);
                rhs_type = ll_typer_type_expression(cc, typer, &opr->right, expected_type, NULL);
            } else if (lhs_type->kind == LL_TYPE_ANYINT || lhs_type->kind == LL_TYPE_ANYFLOAT) {
                lhs_type = ll_typer_type_expression(cc, typer, &opr->left, rhs_type, NULL);
            } else if (rhs_type->kind == LL_TYPE_ANYINT || rhs_type->kind == LL_TYPE_ANYFLOAT) {
                rhs_type = ll_typer_type_expression(cc, typer, &opr->right, lhs_type, NULL);
            }

            result = ll_typer_implicit_cast_leftright(cc, typer, lhs_type, rhs_type);

            if (!ll_typer_can_implicitly_cast_expression(cc, typer, opr->right, result)) {
                ll_typer_report_error(((LL_Error){ .main_token = opr->base.token_info }), "Can't assign value to left hand side");

                ll_typer_report_error_no_src("    left hand side has the type ");
                ll_typer_report_error_type(cc, typer, result);
                ll_typer_report_error_no_src(", but tried to assign value with type ");
                ll_typer_report_error_type(cc, typer, opr->right->type);
                ll_typer_report_error_no_src(" to it. You can explicitly cast the value with `cast(");
                ll_typer_report_error_type_no_fmt(cc, typer, lhs_type);
                ll_typer_report_error_no_src(")\n");

                ll_typer_report_error_done(cc, typer);
                break;
            }

            // @oc_todo: look at casting lhs
            ll_typer_add_implicit_cast(cc, typer, &opr->right, result);
            (*expr)->type = result;
            return result;
        } break;
        case '=': {
            LL_Type* lhs_type = ll_typer_type_expression(cc, typer, &opr->left, NULL, NULL);
            LL_Type* rhs_type = ll_typer_type_expression(cc, typer, &opr->right, lhs_type, NULL);

            if (!ll_typer_can_implicitly_cast_expression(cc, typer, opr->right, lhs_type)) {
                ll_typer_report_error(((LL_Error){ .main_token = opr->base.token_info }), "Can't assign value to left hand side");

                ll_typer_report_error_no_src("    left hand side has the type ");
                ll_typer_report_error_type(cc, typer, lhs_type);
                ll_typer_report_error_no_src(", but tried to assign value with type ");
                ll_typer_report_error_type(cc, typer, rhs_type);
                ll_typer_report_error_no_src(" to it. You can explicitly cast the value with `cast(");
                ll_typer_report_error_type_no_fmt(cc, typer, lhs_type);
                ll_typer_report_error_no_src(")\n");

                ll_typer_report_error_done(cc, typer);
                break;
            }

            // @oc_todo: look at casting lhs
            ll_typer_add_implicit_cast(cc, typer, &opr->right, lhs_type);
            result = lhs_type;
            (*expr)->type = result;
            return result;
        } break;
        default: break;
#pragma GCC diagnostic pop
        }


        LL_Type* lhs_type = ll_typer_type_expression(cc, typer, &opr->left, NULL, NULL);
        LL_Type* rhs_type = ll_typer_type_expression(cc, typer, &opr->right, NULL, NULL);

        if (lhs_type->kind == LL_TYPE_ANYINT && rhs_type->kind == LL_TYPE_ANYINT && expected_type) {
            lhs_type = ll_typer_type_expression(cc, typer, &opr->left, expected_type, NULL);
            rhs_type = ll_typer_type_expression(cc, typer, &opr->right, expected_type, NULL);
        } else if (lhs_type->kind == LL_TYPE_ANYINT || lhs_type->kind == LL_TYPE_ANYFLOAT) {
            lhs_type = ll_typer_type_expression(cc, typer, &opr->left, rhs_type, NULL);
        } else if (rhs_type->kind == LL_TYPE_ANYINT || rhs_type->kind == LL_TYPE_ANYFLOAT) {
            rhs_type = ll_typer_type_expression(cc, typer, &opr->right, lhs_type, NULL);
        }

        switch (opr->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
            if (expected_type) {
                result = expected_type;

                if (!ll_typer_can_implicitly_cast_expression(cc, typer, opr->left, result)) {
                    ll_typer_report_error(((LL_Error){ .main_token = opr->base.token_info }), "Invalid operation of expression with different types.");

                    ll_typer_report_error_no_src("    left hand side has the type ");
                    ll_typer_report_error_type(cc, typer, lhs_type);
                    ll_typer_report_error_no_src(", and right hand side has the type ");
                    ll_typer_report_error_type(cc, typer, rhs_type);
                    ll_typer_report_error_no_src("\n");

                    ll_typer_report_error_no_src("    expecting type ");
                    ll_typer_report_error_type(cc, typer, result);
                    ll_typer_report_error_no_src("\n");
                    ll_typer_report_error_done(cc, typer);
                    break;
                }

                if (!ll_typer_can_implicitly_cast_expression(cc, typer, opr->right, result)) {
                    ll_typer_report_error(((LL_Error){ .main_token = opr->base.token_info }), "Invalid operation of expression with different types.");

                    ll_typer_report_error_no_src("    left hand side has the type ");
                    ll_typer_report_error_type(cc, typer, lhs_type);
                    ll_typer_report_error_no_src(", and right hand side has the type ");
                    ll_typer_report_error_type(cc, typer, rhs_type);
                    ll_typer_report_error_no_src("\n");

                    ll_typer_report_error_no_src("    expecting type ");
                    ll_typer_report_error_type(cc, typer, result);
                    ll_typer_report_error_no_src("\n");
                    ll_typer_report_error_done(cc, typer);
                    break;
                }

            } else {
                result = ll_typer_implicit_cast_leftright(cc, typer, lhs_type, rhs_type);
                if (result == NULL) {
                    ll_typer_report_error(((LL_Error){ .main_token = opr->base.token_info }), "Invalid operation of expression with different types.");

                    ll_typer_report_error_no_src("    left hand side has the type ");
                    ll_typer_report_error_type(cc, typer, lhs_type);
                    ll_typer_report_error_no_src(", and right hand side has the type ");
                    ll_typer_report_error_type(cc, typer, rhs_type);
                    ll_typer_report_error_no_src("\n");

                    ll_typer_report_error_done(cc, typer);
                    break;
                }
            }

            ll_typer_add_implicit_cast(cc, typer, &opr->left, result);
            ll_typer_add_implicit_cast(cc, typer, &opr->right, result);
            break;

        case '>':
        case '<':
        case LL_TOKEN_KIND_GTE:
        case LL_TOKEN_KIND_LTE:
        case LL_TOKEN_KIND_EQUALS:
        case LL_TOKEN_KIND_NEQUALS:
            result = ll_typer_implicit_cast_leftright(cc, typer, lhs_type, rhs_type);

            if (result == NULL) {
                ll_typer_report_error(((LL_Error){ .main_token = opr->base.token_info }), "Invalid comparison of expressions with different types (1)");

                ll_typer_report_error_no_src("    left hand side has the type ");
                ll_typer_report_error_type(cc, typer, lhs_type);
                ll_typer_report_error_no_src(", and right hand side has the type ");
                ll_typer_report_error_type(cc, typer, rhs_type);
                ll_typer_report_error_no_src("\n");

                ll_typer_report_error_done(cc, typer);

                break;
            }

            if (!ll_typer_can_implicitly_cast_expression(cc, typer, opr->left, result)) {
                ll_typer_report_error(((LL_Error){ .main_token = opr->base.token_info }), "Invalid comparison of expressions with different types (2)");

                ll_typer_report_error_no_src("    left hand side has the type ");
                ll_typer_report_error_type(cc, typer, lhs_type);
                ll_typer_report_error_no_src(", and right hand side has the type ");
                ll_typer_report_error_type(cc, typer, rhs_type);
                ll_typer_report_error_no_src("\n");

                ll_typer_report_error_done(cc, typer);

                break;
            }

            if (!ll_typer_can_implicitly_cast_expression(cc, typer, opr->right, result)) {
                ll_typer_report_error(((LL_Error){ .main_token = opr->base.token_info }), "Invalid comparison of expressions with different types (3)");

                ll_typer_report_error_no_src("    left hand side has the type ");
                ll_typer_report_error_type(cc, typer, lhs_type);
                ll_typer_report_error_no_src(", and right hand side has the type ");
                ll_typer_report_error_type(cc, typer, rhs_type);
                ll_typer_report_error_no_src("\n");

                ll_typer_report_error_done(cc, typer);
                break;
            }


            ll_typer_add_implicit_cast(cc, typer, &opr->left, result);
            ll_typer_add_implicit_cast(cc, typer, &opr->right, result);
            result = typer->ty_bool;
            break;
#pragma GCC diagnostic pop
        case LL_TOKEN_KIND_AND:
        case LL_TOKEN_KIND_OR:
            result = typer->ty_bool;
            break;
        default: 
            eprint("Error: Invalid binary operation '");
            lexer_print_token_raw_to_writer(&opr->op, &stderr_writer);
            eprint("'\n");
            exit(-1);
            break;
        }

        // at this point, we have added casts, both sides should have same types

        if (opr->left->has_const && opr->right->has_const) {
            // @const
            (*expr)->has_const = 1u;
            switch (result->kind) {
            case LL_TYPE_ANYINT:
            case LL_TYPE_INT:
                switch (opr->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
                case '+':                   (*expr)->const_value.as_i64 = opr->left->const_value.as_i64 + opr->right->const_value.as_i64; break;
                case '-':                   (*expr)->const_value.as_i64 = opr->left->const_value.as_i64 = opr->right->const_value.as_i64; break;
                case '*':                   (*expr)->const_value.as_i64 = opr->left->const_value.as_i64 * opr->right->const_value.as_i64; break;
                case '/':                   (*expr)->const_value.as_i64 = opr->left->const_value.as_i64 / opr->right->const_value.as_i64; break;
                case '>': 					(*expr)->const_value.as_u64 = opr->left->const_value.as_i64 > opr->right->const_value.as_i64; break;
                case '<': 					(*expr)->const_value.as_u64 = opr->left->const_value.as_i64 < opr->right->const_value.as_i64; break;
#pragma GCC diagnostic pop
                case LL_TOKEN_KIND_GTE: 	(*expr)->const_value.as_u64 = opr->left->const_value.as_i64 >= opr->right->const_value.as_i64; break;
                case LL_TOKEN_KIND_LTE: 	(*expr)->const_value.as_u64 = opr->left->const_value.as_i64 <= opr->right->const_value.as_i64; break;
                case LL_TOKEN_KIND_EQUALS:  (*expr)->const_value.as_u64 = opr->left->const_value.as_i64 == opr->right->const_value.as_i64; break;
                case LL_TOKEN_KIND_NEQUALS: (*expr)->const_value.as_u64 = opr->left->const_value.as_i64 != opr->right->const_value.as_i64; break;
                default: oc_assert(false); break;
                }
                break;
            case LL_TYPE_UINT:
                switch (opr->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
                case '+':                   (*expr)->const_value.as_u64 = opr->left->const_value.as_u64 + opr->right->const_value.as_u64; break;
                case '-':                   (*expr)->const_value.as_u64 = opr->left->const_value.as_u64 = opr->right->const_value.as_u64; break;
                case '*':                   (*expr)->const_value.as_u64 = opr->left->const_value.as_u64 * opr->right->const_value.as_u64; break;
                case '/':                   (*expr)->const_value.as_u64 = opr->left->const_value.as_u64 / opr->right->const_value.as_u64; break;
                case '>': 					(*expr)->const_value.as_u64 = opr->left->const_value.as_u64 > opr->right->const_value.as_u64; break;
                case '<': 					(*expr)->const_value.as_u64 = opr->left->const_value.as_u64 < opr->right->const_value.as_u64; break;
#pragma GCC diagnostic pop
                case LL_TOKEN_KIND_GTE: 	(*expr)->const_value.as_u64 = opr->left->const_value.as_u64 >= opr->right->const_value.as_u64; break;
                case LL_TOKEN_KIND_LTE: 	(*expr)->const_value.as_u64 = opr->left->const_value.as_u64 <= opr->right->const_value.as_u64; break;
                case LL_TOKEN_KIND_EQUALS:  (*expr)->const_value.as_u64 = opr->left->const_value.as_u64 == opr->right->const_value.as_u64; break;
                case LL_TOKEN_KIND_NEQUALS: (*expr)->const_value.as_u64 = opr->left->const_value.as_u64 != opr->right->const_value.as_u64; break;
                default: oc_assert(false); break;
                }
                break;
            case LL_TYPE_FLOAT:
                switch (opr->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
                case '+':                   (*expr)->const_value.as_f64 = opr->left->const_value.as_f64 + opr->right->const_value.as_f64; break;
                case '-':                   (*expr)->const_value.as_f64 = opr->left->const_value.as_f64 = opr->right->const_value.as_f64; break;
                case '*':                   (*expr)->const_value.as_f64 = opr->left->const_value.as_f64 * opr->right->const_value.as_f64; break;
                case '/':                   (*expr)->const_value.as_f64 = opr->left->const_value.as_f64 / opr->right->const_value.as_f64; break;
                case '>': 					(*expr)->const_value.as_u64 = opr->left->const_value.as_f64 > opr->right->const_value.as_f64; break;
                case '<': 					(*expr)->const_value.as_u64 = opr->left->const_value.as_f64 < opr->right->const_value.as_f64; break;
#pragma GCC diagnostic pop
                case LL_TOKEN_KIND_GTE: 	(*expr)->const_value.as_u64 = opr->left->const_value.as_f64 >= opr->right->const_value.as_f64; break;
                case LL_TOKEN_KIND_LTE: 	(*expr)->const_value.as_u64 = opr->left->const_value.as_f64 <= opr->right->const_value.as_f64; break;
                case LL_TOKEN_KIND_EQUALS:  (*expr)->const_value.as_u64 = opr->left->const_value.as_f64 == opr->right->const_value.as_f64; break;
                case LL_TOKEN_KIND_NEQUALS: (*expr)->const_value.as_u64 = opr->left->const_value.as_f64 != opr->right->const_value.as_f64; break;
                default: oc_assert(false); break;
                }
            default: ll_print_type(result); oc_todo("implement bvinary op const fold types or error"); break;
            }
        }

        if (!result) {
            eprint("\x1b[31;1merror\x1b[0;1m: types of operands to operation do not match! Found left type was ");
            ll_typer_report_error_type(cc, typer, lhs_type);
            eprint(" but right type was ");
            ll_typer_report_error_type(cc, typer, rhs_type);
            eprint("\n");

            eprint("Error: lhs type does not match rhs type\n");
        }
        break;
    }
    case AST_KIND_PRE_OP: {
        LL_Type* expr_type;
        result = NULL;
        switch (AST_AS((*expr), Ast_Operation)->op.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '-': {
            expr_type = ll_typer_type_expression(cc, typer, &AST_AS((*expr), Ast_Operation)->right, expected_type, NULL);

            switch (expr_type->kind) {
            case LL_TYPE_FLOAT:
            case LL_TYPE_ANYINT:
            case LL_TYPE_INT:
                break;
            default:
                ll_typer_report_error(((LL_Error){ .main_token = (*expr)->token_info }), "Negation only works for signed ints and floats");
                ll_typer_report_error(((LL_Error){ .main_token = AST_AS((*expr), Ast_Operation)->right->token_info }), "");
                ll_typer_report_error_no_src("    has type");
                ll_typer_report_error_type(cc, typer, expr_type);
                ll_typer_report_error_no_src("\n");
                ll_typer_report_error_done(cc, typer);

                break;
            }

            if (expected_type) {
                result = expected_type;
            } else {
                result = expr_type;
            }

            ll_typer_add_implicit_cast(cc, typer, &AST_AS((*expr), Ast_Operation)->right, result);
        } break;
        case '*': {

            if (expected_type) {
                expr_type = ll_typer_get_ptr_type(cc, typer, expected_type);
                expr_type = ll_typer_type_expression(cc, typer, &AST_AS((*expr), Ast_Operation)->right, expr_type, NULL);
            } else {
                expr_type = ll_typer_type_expression(cc, typer, &AST_AS((*expr), Ast_Operation)->right, NULL, NULL);
            }

            switch (expr_type->kind) {
            case LL_TYPE_POINTER: result = ((LL_Type_Pointer*)expr_type)->element_type; break;
            default:
                ll_typer_report_error(((LL_Error){ .main_token = (*expr)->token_info }), "Dereference only works with a pointer");
                ll_typer_report_error(((LL_Error){ .main_token = AST_AS((*expr), Ast_Operation)->right->token_info }), "");
                ll_typer_report_error_no_src("    has type");
                ll_typer_report_error_type(cc, typer, expr_type);
                ll_typer_report_error_no_src("\n");
                ll_typer_report_error_done(cc, typer);
                break;
            }
        } break;
        case '&': {
            if (expected_type && expected_type->kind == LL_TYPE_POINTER) {
                LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)expected_type;
                expr_type = ll_typer_type_expression(cc, typer, &AST_AS((*expr), Ast_Operation)->right, ptr_type->element_type, NULL);
            } else {
                expr_type = ll_typer_type_expression(cc, typer, &AST_AS((*expr), Ast_Operation)->right, NULL, NULL);
            }
            result = ll_typer_get_ptr_type(cc, typer, expr_type);
        } break;
#pragma GCC diagnostic pop
        default: break;
        }

        if (!result) {
            eprint("\x1b[31;1mTODO:\x1b[0m operator '");
            lexer_print_token_raw_to_writer(&AST_AS((*expr), Ast_Operation)->op, &stderr_writer);
            eprint("' cannot be applied to expression of type \n");
            ll_typer_report_error_type(cc, typer, expr_type);
            eprint("\n");
        }
    } break;
    case AST_KIND_CAST: {
        Ast_Cast* cast = AST_AS((*expr), Ast_Cast);
        LL_Type* specified_type = ll_typer_get_type_from_typename(cc, typer, cast->cast_type);
        LL_Type* src_type = ll_typer_type_expression(cc, typer, &cast->expr, specified_type, NULL);
        
        if (!ll_typer_can_cast(cc, typer, src_type, specified_type)) {
            ll_typer_report_error(((LL_Error){ .highlight_start = (*expr)->token_info, .highlight_end = cast->p_close }), "Incompatible cast types");

            eprint("\x1b[1m    unable to cast value with type ");
            ll_typer_report_error_type(cc, typer, src_type);
            eprint(" to type ");
            ll_typer_report_error_type(cc, typer, specified_type);
            eprint("\x1b[0m\n");

            ll_typer_report_error_done(cc, typer);
        }

        result = specified_type;
    } break;
    case AST_KIND_INVOKE: {
        uword pi, di;
        Ast_Invoke* inv = AST_AS((*expr), Ast_Invoke);
        Ast_Function_Declaration* fn_decl = NULL;
        LL_Scope* fn_scope = NULL;
        LL_Typer_Resolve_Result resolve = { 0 };
        LL_Type** polymorphic_types = NULL;

        if (inv->expr->kind == AST_KIND_IDENT && AST_AS(inv->expr, Ast_Ident)->str.ptr == LL_KEYWORD_SIZEOF.ptr) {
            // @Todo: throw error if not 1 arg
            result = ll_typer_type_expression(cc, typer, &inv->arguments.items[0], NULL, NULL);

            LL_Backend_Layout layout;
            if (result == typer->ty_type && inv->arguments.items[0]->has_const) {
                layout = cc->target->get_layout(inv->arguments.items[0]->const_value.as_type);
            } else {
                layout = cc->target->get_layout(inv->arguments.items[0]->type);
            }

            (*expr)->has_const = true;
            (*expr)->const_value.as_u64 = layout.size;

            result = typer->ty_uint64;
            break;
        }

        LL_Type_Function* fn_type = (LL_Type_Function*)ll_typer_type_expression(cc, typer, &inv->expr, NULL, &resolve);
        if (fn_type->base.kind != LL_TYPE_FUNCTION) {
            ll_typer_report_error(((LL_Error){ .main_token = inv->expr->token_info }), "Unable to call this like a function");
            ll_typer_report_error_no_src("    type ");
            ll_typer_report_error_type(cc, typer, &fn_type->base);
            ll_typer_report_error_no_src(" is not a callable function\n");
            ll_typer_report_error_done(cc, typer);
            return NULL;
        }

        // if we're directly calling a function:
        if (resolve.scope && resolve.scope->kind == LL_SCOPE_KIND_FUNCTION) {
            fn_decl = (Ast_Function_Declaration*)resolve.scope->decl;
            inv->fn_decl = fn_decl;
            fn_scope = resolve.scope;
        }

        bool fn_is_polymorphic = fn_decl && (fn_decl->storage_class & LL_STORAGE_CLASS_POLYMORPHIC);
        bool fn_is_macro = fn_decl && (fn_decl->storage_class & LL_STORAGE_CLASS_MACRO);

        if (fn_is_polymorphic) {
            polymorphic_types = alloca(sizeof(*polymorphic_types) * fn_decl->parameters.count);
        }
        // if (!fn_decl && inv->expr->kind == AST_KIND_IDENT) {
        //     Ast_Ident* ident = AST_AS(inv->expr, Ast_Ident);
        //     if (ident->resolved_scope->kind == LL_SCOPE_KIND_FUNCTION) {
        //         fn_decl = (Ast_Function_Declaration*)ident->resolved_scope->decl;
        //         fn_scope = ident->resolved_scope;
        //     }
        // }

        Ast_Base** ordered_args = oc_arena_alloc(&cc->arena, sizeof(ordered_args[0]) * (fn_type->parameter_count + inv->arguments.count));
        uint32_t*  arg_indices = oc_arena_alloc(&cc->arena, sizeof(arg_indices[0]) * (fn_type->parameter_count + inv->arguments.count));
        int ordered_arg_count = 0;
        int variadic_arg_count = 0;

        if (resolve.this_arg) {
            LL_Type* declared_type = fn_type->parameters[0];
            inv->has_this_arg = true;

            (void)ll_typer_type_expression(cc, typer, resolve.this_arg, declared_type, NULL);

            if (fn_is_polymorphic) {
                polymorphic_types[0] = (*resolve.this_arg)->type;
                if (polymorphic_types[0]->kind != LL_TYPE_POINTER && !declared_type && fn_decl->parameters.items[0].type->kind == AST_KIND_TYPE_POINTER) {
                    fn_type->parameters[0] = ll_typer_get_ptr_type(cc, typer, polymorphic_types[0]);
                }
            }

            ordered_arg_count++;
            ordered_args[0] = *resolve.this_arg;
            if (!ordered_args[0]->type) {
                ordered_args[0]->type = declared_type;
            }
            di = 1;
        } else {
            di = 0;
        }

        uword arg_count = inv->arguments.count;
        for (pi = 0; pi < arg_count; ++pi, ++di) {
            LL_Type *declared_type = NULL, *provided_type = NULL;
            Ast_Base** value;
            Ast_Base** current_arg = &inv->arguments.items[pi];
            
            if ((*current_arg)->kind == AST_KIND_KEY_VALUE) {
                Ast_Key_Value* kv = AST_AS((*current_arg), Ast_Key_Value);
                if (!fn_decl || !fn_scope) {
                    ll_typer_report_error(((LL_Error){ .highlight_start = kv->key->token_info, .highlight_end = kv->value->token_info }), "Keyed arguments are only allowed when calling a functino directly.");
                    eprint("\x1b[1m    this is probably due to calling a function pointer, but this is invalid\x1b[0m\n");
                    ll_typer_report_error_done(cc, typer);
                    break;
                }

                // can't have keyed arguments after varaible
                if (fn_type->is_variadic && di >= fn_type->parameter_count - 1) {
                    ll_typer_report_error(((LL_Error){ .highlight_start = kv->key->token_info, .highlight_end = kv->value->token_info }), "Keyed arguments are only allowed before variadic arguments\n");
                    ll_typer_report_error_done(cc, typer);
                    break;
                }

                if (kv->key->kind != AST_KIND_IDENT) {
                    ll_typer_report_error(((LL_Error){ .main_token = kv->key->token_info }), "Argument key needs to be an identifier");
                    ll_typer_report_error_done(cc, typer);
                    break;
                }

                LL_Scope* parameter_scope = ll_scope_get(fn_scope, AST_AS(kv->key, Ast_Ident)->str);
                if (parameter_scope->kind != LL_SCOPE_KIND_PARAMETER) {
                    ll_typer_report_error(((LL_Error){ .main_token = kv->key->token_info }), "Parameter '{}' does not exist for the function", AST_AS(kv->key, Ast_Ident)->str);
                    ll_typer_report_error_done(cc, typer);
                    break;
                }

                di = AST_AS(parameter_scope->decl, Ast_Parameter)->ir_index;
                value = &kv->value;
            } else {
                if (fn_type->is_variadic) {
                    if (di >= fn_type->parameter_count - 1) {
                        ll_typer_type_expression(cc, typer, &(*current_arg), NULL, NULL);
                        ordered_args[di] = (*current_arg);
                        ordered_arg_count++;
                        variadic_arg_count++;
                        continue;
                    }
                } else if (di >= fn_type->parameter_count) {
                    break;
                }

                value = current_arg;
            }

            declared_type = fn_type->parameters[di];
            
            // code ref and void are only typed checked when the parameter is expanded in macro body
            if (declared_type && declared_type->kind != LL_TYPE_VOID) {
                provided_type = ll_typer_type_expression(cc, typer, value, declared_type, NULL);
            } else if (!fn_is_macro) {
                provided_type = ll_typer_type_expression(cc, typer, value, declared_type, NULL);
            }
            bool check_type_matches = true;
            if (!declared_type) {
                if (fn_decl && !fn_decl->parameters.items[di].ident && provided_type == typer->ty_type) {
                    if (!(*value)->has_const) oc_todo("runtime type");
                    declared_type = (*value)->const_value.as_type;
                } else {
                    declared_type = provided_type;
                }
                check_type_matches = false;
            }

            if (fn_is_polymorphic) {
                polymorphic_types[di] = declared_type;
            }

            if (check_type_matches && provided_type && declared_type) {
                if (!ll_typer_can_implicitly_cast_expression(cc, typer, *value, declared_type)) {
                    ll_typer_report_error(((LL_Error){ .main_token = (*value)->token_info }), "Can't pass value to function parameter");

                    ll_typer_report_error_no_src("    the parameter expects type ");
                    ll_typer_report_error_type(cc, typer, declared_type);
                    ll_typer_report_error_no_src(", but tried passing value with type ");
                    ll_typer_report_error_type(cc, typer, provided_type);
                    ll_typer_report_error_no_src(" to it. ");
                    if (ll_typer_can_cast(cc, typer, (*value)->type, declared_type)) {
                        ll_typer_report_error_no_src("You can try explicitly casting the value with `cast(");
                        ll_typer_report_error_type_no_fmt(cc, typer, declared_type);
                        ll_typer_report_error_no_src(")`");
                    }
                    ll_typer_report_error_no_src("\n");

                    ll_typer_report_error_done(cc, typer);
                }

                ll_typer_add_implicit_cast(cc, typer, value, declared_type);
            }

            ordered_arg_count++;
            ordered_args[di] = *value;
            arg_indices[di] = (uint32_t)pi;
            if (!ordered_args[di]->type) {
                ordered_args[di]->type = declared_type;
            }
        }

        size_t missing_count = 0;
        for (di = 0; di < fn_type->parameter_count; ++di) {
            if (di == fn_type->parameter_count - 1 && fn_type->is_variadic) break;
            
            if (!ordered_args[di]) {
                if (fn_decl && fn_decl->parameters.items[di].initializer) {
                    ordered_args[di] = fn_decl->parameters.items[di].initializer;

                    ll_typer_add_implicit_cast(cc, typer, &ordered_args[di], fn_type->parameters[di]);
                    if (!ordered_args[di]->type) {
                        ordered_args[di]->type = fn_type->parameters[di];
                    }

                    ordered_arg_count++;
                } else {
                    missing_count++;
                }
            }
        }

        size_t expected_arg_count = fn_type->is_variadic ? (fn_type->parameter_count - 1) : fn_type->parameter_count;

        if (!fn_type->is_variadic && pi < inv->arguments.count) {
            ll_typer_report_error(((LL_Error){ .highlight_start = inv->base.token_info, .highlight_end = inv->p_close }), "Expected only {} arguments but got {}", expected_arg_count, inv->arguments.count);
            if (fn_decl) {
                ll_typer_report_error_info(((LL_Error){ .highlight_start = fn_decl->p_open, .highlight_end = fn_decl->p_close }), "Function defined here");
            } else {
                ll_typer_report_error_no_src("    function signature: ");
                ll_typer_report_error_type(cc, typer, &fn_type->base);
                ll_typer_report_error_no_src("\n");
            }
            ll_typer_report_error_done(cc, typer);
        }
        if (missing_count) {
            if (expected_arg_count == missing_count) {
                ll_typer_report_error(((LL_Error){ .highlight_start = inv->base.token_info, .highlight_end = inv->p_close }), "Expected {} arguments but only got none", expected_arg_count);
            } else {
                ll_typer_report_error(((LL_Error){ .highlight_start = inv->base.token_info, .highlight_end = inv->p_close }), "Expected {} arguments but only got {}", expected_arg_count, inv->arguments.count);
            }

            if (fn_decl) {
                ll_typer_report_error_info(((LL_Error){ .highlight_start = fn_decl->p_open, .highlight_end = fn_decl->p_close }), "Function defined here");
            } else {
                ll_typer_report_error_no_src("    function signature: ");
                ll_typer_report_error_type(cc, typer, &fn_type->base);
                ll_typer_report_error_no_src("\n");
            }
            ll_typer_report_error_done(cc, typer);
        }

        inv->ordered_arguments.count = ordered_arg_count;
        inv->ordered_arguments.items = ordered_args;

        if (fn_is_macro && fn_decl->body) {
            /*
                transform a macro call into something like this:

                { // block scope (holds all parameters and generic typedefs)
                    T = int;
                    T param1 = ...;
                    string param2 = ...;
                    { // macro expansion scope (macro local variables don't search past this scope +1 parent (for parameter scope), unless they are expanded variables ($foo))
                        ... macro body
                    }
                }
            */
            Ast_Base* new_body = ast_clone_node_deep(cc, fn_decl->body, (LL_Ast_Clone_Params) { .expand_first_block = true });
            oc_assert(new_body->kind == AST_KIND_BLOCK);

            LL_Scope* old_scope = typer->current_scope;
            LL_Scope* param_scope = create_scope(LL_SCOPE_KIND_BLOCK, NULL);
            ll_typer_scope_put(cc, typer, param_scope, false);

            typer->current_scope = param_scope;
            for (int arg_i = 0; arg_i < ordered_arg_count; arg_i++) {
                Ast_Parameter* parameter = &fn_decl->parameters.items[arg_i];

                if (!fn_type->parameters[arg_i]) {
                    LL_Type* provided_type = ll_typer_type_expression(cc, typer, &ordered_args[arg_i], NULL, NULL);
                    if (ll_typer_match_polymorphic(cc, typer, parameter->type, provided_type, ordered_args[arg_i], resolve.this_arg != NULL && arg_i == 0)) {
                        parameter->ident->base.type = provided_type;
                    } else {
                        Ast_Base* original_argument = inv->arguments.items[arg_indices[arg_i]];
                        ll_typer_report_error(((LL_Error){ .main_token = original_argument->token_info }), "invalid arg");
                        ll_typer_report_error_done(cc, typer);
                    }
                }

                parameter->base.type = ll_typer_get_type_from_typename(cc, typer, parameter->type);

                LL_Scope_Macro_Parameter* scope = create_scope_macro_parameter(parameter);
                scope->ident = parameter->ident;
                scope->value = ordered_args[arg_i];

                ll_typer_scope_put(cc, typer, (LL_Scope*)scope, false);
            }

            (void)ll_typer_type_expression(cc, typer, &new_body, expected_type, resolve_result);
            *expr = new_body;

            typer->current_scope = old_scope;
        } else if (fn_is_polymorphic) {
            fn_type = (LL_Type_Function*)ll_typer_get_fn_type(cc, typer, fn_type->return_type, polymorphic_types, fn_decl->parameters.count, fn_type->is_variadic);

            LL_Function_Instantiation* this_inst = NULL;
            if ((this_inst = ll_typer_function_instance_get(cc, typer, fn_decl, fn_type))) {
            } else {
                if (fn_decl->body) {


                    LL_Scope* old_scope = typer->current_scope;



                    Ast_Function_Declaration* new_fn_decl = (Ast_Function_Declaration*)ast_clone_node_deep(cc, (Ast_Base*)fn_decl, (LL_Ast_Clone_Params) { 0 });
                    oc_assert(new_fn_decl->base.kind == AST_KIND_FUNCTION_DECLARATION);

                    /*
                        scope
                            scope FUNCTION : original function
                            scope FUNCTION : function instances
                    */

                    typer->current_scope = fn_scope->parent;
                    LL_Scope* new_fn_scope = create_scope(LL_SCOPE_KIND_FUNCTION, new_fn_decl);
                    ll_typer_scope_put(cc, typer, new_fn_scope, false);
                    typer->current_scope = new_fn_scope;


                    bool did_variadic = false;
                    bool did_default = false;
                    for (int arg_i = 0; arg_i < ordered_arg_count; arg_i++) {
                        Ast_Parameter* parameter = &new_fn_decl->parameters.items[arg_i];

                        if (did_variadic) {
                            break;
                        }
                        if (parameter->flags & LL_PARAMETER_FLAG_VARIADIC) {
                            did_variadic = true;
                        }

                        // if no ident, it means this is just a type polymorph
                        if (ll_typer_match_polymorphic(cc, typer, parameter->type, polymorphic_types[arg_i], ordered_args[arg_i], resolve.this_arg != NULL && arg_i == 0)) {
                            if (parameter->ident) parameter->ident->base.type = polymorphic_types[arg_i];
                            parameter->base.type = polymorphic_types[arg_i];
                        } else {
                            Ast_Base* original_argument = inv->arguments.items[arg_indices[arg_i]];
                            ll_typer_report_error(((LL_Error){ .main_token = original_argument->token_info }), "invalid arg");
                            ll_typer_report_error_done(cc, typer);
                        }

                        parameter->ir_index = arg_i;

                        if (parameter->ident) {
                            LL_Scope_Simple* param_scope = create_scope_simple(LL_SCOPE_KIND_PARAMETER, parameter);
                            param_scope->ident = parameter->ident;

                            ll_typer_scope_put(cc, typer, (LL_Scope*)param_scope, false);
                        }
                    }

                    LL_Type_Function* last_fn = typer->current_fn;
                    typer->current_fn = (LL_Type_Function*)fn_type;

                    // type body
                    ll_typer_type_statement(cc, typer, &new_fn_decl->body);


                    LL_Function_Instantiation inst = {
                        .body = new_fn_decl->body,
                        .fn_type = fn_type,
                        .ir_index = 0,
                    };
                    this_inst = ll_typer_function_instance_put(cc, typer, fn_decl, inst);


                    typer->current_scope = old_scope;
                    typer->current_fn = last_fn;
                }
            }

            inv->resolved_fn_inst = this_inst;
        }

        result = fn_type->return_type;
    } break;
    case AST_KIND_INDEX: {
        Ast_Slice* cf = AST_AS((*expr), Ast_Slice);
        result = ll_typer_type_expression(cc, typer, &cf->ptr, NULL, NULL);

        if (result == typer->ty_type) {
            if (!cf->ptr->has_const) oc_todo("handle runtime");
            if (!cf->ptr->const_value.as_type) {
                return NULL;
            }
            ll_typer_type_expression(cc, typer, &cf->start, NULL, NULL);

            uint64_t array_width;
            if (cf->start->has_const) {
                array_width = cf->start->const_value.as_u64;
            } else {
                LL_Eval_Value value = ll_eval_node(cc, cc->eval_context, cc->bir, cf->start);
                array_width = value.as_u64;
            }

            result = ll_typer_get_array_type(cc, typer, cf->ptr->const_value.as_type, array_width);

            (*expr)->has_const = 1;
            (*expr)->const_value.as_type = result;

            result = typer->ty_type;
            break;
        }

        LL_Type* index_type = ll_typer_type_expression(cc, typer, &cf->start, NULL, NULL);
        if (!ll_typer_can_cast(cc, typer, index_type, typer->ty_anyint)) {
            ll_typer_report_error(((LL_Error){ .main_token = cf->start->token_info }), "Expected integer to index array");

            ll_typer_report_error_no_src(" you tried to index with type ");
            ll_typer_report_error_type(cc, typer, index_type);
            ll_typer_report_error_no_src("\n");

            ll_typer_report_error_done(cc, typer);
        }
        // if (index_type == typer->ty_anyint) {
        //     index_type = ll_typer_type_expression(cc, typer, &cf->start, typer->ty_int64, NULL);
        // }
        // ll_typer_add_implicit_cast(cc, typer, &cf->start, typer->ty_int64);

        switch (result->kind) {
        case LL_TYPE_ARRAY:
            result = ((LL_Type_Array*)result)->element_type;
            break;
        case LL_TYPE_POINTER:
            result = ((LL_Type_Pointer*)result)->element_type;
            break;
        case LL_TYPE_SLICE:
            result = ((LL_Type_Slice*)result)->element_type;
            break;
        case LL_TYPE_STRING:
            result = typer->ty_char;
            break;
        default:
            ll_typer_report_error(((LL_Error){ .main_token = cf->ptr->token_info }), "Index expression requires an array or pointer type on the left");

            ll_typer_report_error_no_src(" found type ");
            ll_typer_report_error_type(cc, typer, result);
            ll_typer_report_error_no_src("\n");

            ll_typer_report_error_no_src(" found type ");
            ll_typer_report_error_type(cc, typer, result);
            ll_typer_report_error_no_src("\n");

            ll_typer_report_error_done(cc, typer);
            break;
        }

    } break;
    case AST_KIND_SLICE: {
        Ast_Slice* cf = AST_AS((*expr), Ast_Slice);
        result = ll_typer_type_expression(cc, typer, &cf->ptr, NULL, NULL);

        if (result == typer->ty_type) {
            if (!cf->ptr->has_const) oc_todo("handle runtime");
            if (!cf->ptr->const_value.as_type) {
                return NULL;
            }

            result = ll_typer_get_slice_type(cc, typer, cf->ptr->const_value.as_type);

            (*expr)->has_const = 1;
            (*expr)->const_value.as_type = result;

            result = typer->ty_type;
            break;
        }


        if (cf->start) ll_typer_type_expression(cc, typer, &cf->start, typer->ty_uint64, NULL);
        if (cf->stop) ll_typer_type_expression(cc, typer, &cf->stop, typer->ty_uint64, NULL);

        switch (result->kind) {
        case LL_TYPE_SLICE:
            break;
        case LL_TYPE_STRING:
            break;
        default:
            ll_typer_report_error(((LL_Error){ .main_token = cf->ptr->token_info }), "Slice expression requires an array, pointer, or slice type on the left");
            ll_typer_report_error_no_src(" found type ");
            ll_typer_report_error_type(cc, typer, result);
            ll_typer_report_error_no_src("\n");
            ll_typer_report_error_done(cc, typer);
            break;
        }

    } break;
    case AST_KIND_CONST: {
        Ast_Marker* cf = AST_AS((*expr), Ast_Marker);
        result = ll_typer_type_expression(cc, typer, &cf->expr, expected_type, NULL);
        LL_Eval_Value const_value = ll_eval_node(cc, cc->eval_context, cc->bir, cf->expr);
        (*expr)->has_const = true;
        (*expr)->const_value = const_value;
    } break;
    case AST_KIND_RETURN: {
        Ast_Control_Flow* cf = AST_AS((*expr), Ast_Control_Flow);
        LL_Scope* current_scope = typer->current_scope;
        cf->referenced_scope = NULL;

        while (current_scope) {
            switch (current_scope->kind) {
            case LL_SCOPE_KIND_FUNCTION:
                cf->referenced_scope = current_scope;
                goto AST_RETURN_EXIT_SCOPE;
                oc_todo("");
            case LL_SCOPE_KIND_STRUCT:
            case LL_SCOPE_KIND_TYPENAME:
            case LL_SCOPE_KIND_PACKAGE:
                ll_typer_report_error(((LL_Error){ .main_token = cf->base.token_info }), "Tried returning out of a function");
                ll_typer_report_error_done(cc, typer);
                goto AST_RETURN_EXIT_SCOPE;
            case LL_SCOPE_KIND_FIELD:
            case LL_SCOPE_KIND_LOCAL:
            case LL_SCOPE_KIND_BLOCK_VALUE:
            case LL_SCOPE_KIND_BLOCK:
            case LL_SCOPE_KIND_PARAMETER:
            case LL_SCOPE_KIND_LOOP:
            case LL_SCOPE_KIND_MACRO_EXPANSION:
            case LL_SCOPE_KIND_MACRO_PARAMETER:
                break;
            }
            current_scope = current_scope->parent;
        }

AST_RETURN_EXIT_SCOPE:
        if (cf->expr) ll_typer_type_expression(cc, typer, &cf->expr, typer->current_fn->return_type, NULL);
        result = NULL;
    } break;
    case AST_KIND_BREAK: {
        Ast_Control_Flow* cf = AST_AS((*expr), Ast_Control_Flow);

        LL_Scope* current_scope = typer->current_scope;
        cf->referenced_scope = NULL;

        LL_Type* break_type = NULL;

        while (current_scope) {
            switch (current_scope->kind) {
            case LL_SCOPE_KIND_LOOP:
                if (cf->target == AST_CONTROL_FLOW_TARGET_ANY || cf->target == AST_CONTROL_FLOW_TARGET_FOR) {
                    cf->referenced_scope = current_scope;
                    break_type = current_scope->decl->type;
                    goto AST_BREAK_EXIT_SCOPE;
                } else break;
            case LL_SCOPE_KIND_BLOCK:
                if (cf->target == AST_CONTROL_FLOW_TARGET_ANY) {
                    cf->referenced_scope = current_scope;
                    break_type = current_scope->decl->type;
                    goto AST_BREAK_EXIT_SCOPE;
                } else break;
            case LL_SCOPE_KIND_BLOCK_VALUE:
                if (cf->target == AST_CONTROL_FLOW_TARGET_ANY || cf->target == AST_CONTROL_FLOW_TARGET_DO) {
                    cf->referenced_scope = current_scope;
                    break_type = current_scope->decl->type;
                    goto AST_BREAK_EXIT_SCOPE;
                } else break;
            case LL_SCOPE_KIND_FUNCTION:
                ll_typer_report_error(((LL_Error){ .main_token = cf->base.token_info }), "Tried breaking without a block to break out of");
                ll_typer_report_error_done(cc, typer);
                goto AST_BREAK_EXIT_SCOPE;
            case LL_SCOPE_KIND_STRUCT:
            case LL_SCOPE_KIND_TYPENAME:
            case LL_SCOPE_KIND_PACKAGE:
                ll_typer_report_error(((LL_Error){ .main_token = cf->base.token_info }), "Tried breaking without a block to break out of");
                ll_typer_report_error_done(cc, typer);
                goto AST_BREAK_EXIT_SCOPE;
            case LL_SCOPE_KIND_FIELD:
            case LL_SCOPE_KIND_LOCAL:
            case LL_SCOPE_KIND_PARAMETER:
            case LL_SCOPE_KIND_MACRO_EXPANSION:
            case LL_SCOPE_KIND_MACRO_PARAMETER:
                break;
            }
            current_scope = current_scope->parent;
        }

AST_BREAK_EXIT_SCOPE:
        if (cf->expr) {
            if (cf->referenced_scope->kind == LL_SCOPE_KIND_BLOCK) {
                ll_typer_report_error(((LL_Error){ .main_token = (*expr)->token_info }), "Tried breaking with a value, but scope to break from didn't expect a value");
                Ast_Block* blk = AST_AS(cf->referenced_scope->decl, Ast_Block);
                ll_typer_report_error_note(((LL_Error){ .highlight_start = blk->c_open, .highlight_end = blk->c_close }), "Breaking from this block");
                ll_typer_report_error_done(cc, typer);
            }
            if (break_type == NULL) {
                ll_typer_report_error(((LL_Error){ .main_token = (*expr)->token_info }), "Tried breaking with a value, but scope to break from didn't expect a value");
                switch (cf->referenced_scope->kind) {
                case LL_SCOPE_KIND_BLOCK_VALUE:
                    ll_typer_report_error_note(((LL_Error){ .main_token = cf->referenced_scope->decl->token_info }), "Breaking from this block");
                    break;
                case LL_SCOPE_KIND_LOOP:
                    ll_typer_report_error_note(((LL_Error){ .main_token = cf->referenced_scope->decl->token_info }), "Breaking from this loop");
                    break;
                default:
                    ll_typer_report_error_note(((LL_Error){ .main_token = cf->referenced_scope->decl->token_info }), "Breaking from this sceop");
                    break;
                }
                ll_typer_report_error_done(cc, typer);
            }

            LL_Type* value_type = ll_typer_type_expression(cc, typer, &cf->expr, break_type, NULL);

            if (!ll_typer_can_implicitly_cast_expression(cc, typer, cf->expr, break_type)) {
                ll_typer_report_error(((LL_Error){ .main_token = (*expr)->token_info }), "Tried breaking with a value that is incompatible with the expected type");

                ll_typer_report_error_no_src("    expecting type ");
                ll_typer_report_error_type(cc, typer, break_type);
                ll_typer_report_error_no_src(", but tried to break with value of type ");
                ll_typer_report_error_type(cc, typer, value_type);
                ll_typer_report_error_no_src("\n");

                switch (cf->referenced_scope->kind) {
                case LL_SCOPE_KIND_BLOCK_VALUE:
                    ll_typer_report_error_note(((LL_Error){ .main_token = cf->referenced_scope->decl->token_info }), "Breaking from this block");
                    break;
                case LL_SCOPE_KIND_LOOP:
                    ll_typer_report_error_note(((LL_Error){ .main_token = cf->referenced_scope->decl->token_info }), "Breaking from this loop");
                    break;
                default:
                    ll_typer_report_error_note(((LL_Error){ .main_token = cf->referenced_scope->decl->token_info }), "Breaking from this sceop");
                    break;
                }

                ll_typer_report_error_done(cc, typer);
            }

            ll_typer_add_implicit_cast(cc, typer, &cf->expr, break_type);
        } else {
            if (break_type != NULL) {
                ll_typer_report_error(((LL_Error){ .main_token = (*expr)->token_info }), "Tried breaking without a value, but scope to break from expects a value");
                switch (cf->referenced_scope->kind) {
                case LL_SCOPE_KIND_BLOCK_VALUE:
                    ll_typer_report_error_note(((LL_Error){ .main_token = cf->referenced_scope->decl->token_info }), "Breaking from this block");
                    break;
                case LL_SCOPE_KIND_LOOP:
                    ll_typer_report_error_note(((LL_Error){ .main_token = cf->referenced_scope->decl->token_info }), "Breaking from this loop");
                    break;
                default:
                    ll_typer_report_error_note(((LL_Error){ .main_token = cf->referenced_scope->decl->token_info }), "Breaking from this sceop");
                    break;
                }
                ll_typer_report_error_no_src("    expecting type ");
                ll_typer_report_error_type(cc, typer, break_type);
                ll_typer_report_error_no_src("\n");
                ll_typer_report_error_done(cc, typer);
            }
        }

        result = NULL;
    } break;
    case AST_KIND_IF: {
        Ast_If* iff = AST_AS((*expr), Ast_If);
        result = ll_typer_type_expression(cc, typer, &iff->cond, typer->ty_bool, NULL);
        switch (result->kind) {
        case LL_TYPE_ANYBOOL:
        case LL_TYPE_BOOL:
        case LL_TYPE_POINTER:
        case LL_TYPE_ANYINT:
        case LL_TYPE_UINT:
        case LL_TYPE_INT:
            break;
        default:
            ll_typer_report_error(((LL_Error){ .main_token = iff->cond->token_info }), "If statement condition should be boolean, integer or pointer");
            ll_typer_report_error_no_src(" found type ");
            ll_typer_report_error_type(cc, typer, result);
            ll_typer_report_error_no_src("\n");
            ll_typer_report_error_done(cc, typer);
            break;
        }

        if (iff->body) {
            ll_typer_type_statement(cc, typer, &iff->body);
        }

        if (iff->else_clause) {
            ll_typer_type_statement(cc, typer, &iff->else_clause);
        }

        result = NULL;
    } break;
    case AST_KIND_WHILE:
    case AST_KIND_FOR: {
        Ast_Loop* loop = AST_AS((*expr), Ast_Loop);

        LL_Scope* loop_scope = create_scope(LL_SCOPE_KIND_LOOP, *expr);
        ll_typer_scope_put(cc, typer, loop_scope, false);
        typer->current_scope = loop_scope;
        loop->base.type = expected_type;
        loop->scope = loop_scope;
        if (loop->init) ll_typer_type_statement(cc, typer, &loop->init);

        if (loop->cond) {
            result = ll_typer_type_expression(cc, typer, &loop->cond, typer->ty_int32, NULL);
            switch (result->kind) {
            case LL_TYPE_BOOL:
            case LL_TYPE_ANYBOOL:
            case LL_TYPE_POINTER:
            case LL_TYPE_ANYINT:
            case LL_TYPE_UINT:
            case LL_TYPE_INT: break;
            default:
                ll_typer_report_error(((LL_Error){ .main_token = loop->cond->token_info }), "Loop condition should be boolean, integer or pointer");
                ll_typer_report_error_no_src(" found type ");
                ll_typer_report_error_type(cc, typer, result);
                ll_typer_report_error_no_src("\n");
                ll_typer_report_error_done(cc, typer);
                break;
            }
        }
        if (loop->update) ll_typer_type_expression(cc, typer, &loop->update, NULL, NULL);

        if (loop->body) {
            ll_typer_type_statement(cc, typer, &loop->body);
        }

        typer->current_scope = loop_scope->parent;

        result = expected_type;
    } break;
    default:
        eprint("\x1b[31;1mTODO:\x1b[0m type expression {}\n", (*expr)->kind);
        result = NULL;
        break;
    }

    (*expr)->type = result;
    return result;
}

LL_Type* ll_typer_get_type_from_typename(Compiler_Context* cc, LL_Typer* typer, Ast_Base* typename) {
    LL_Type* result = NULL;

    switch (typename->kind) {
    case AST_KIND_GENERIC:
        break;
    case AST_KIND_IDENT:
        if (AST_AS(typename, Ast_Ident)->str.ptr == LL_KEYWORD_LET.ptr) return NULL;

        LL_Scope* scope = ll_typer_find_symbol_up_scope(cc, typer, AST_AS(typename, Ast_Ident));
        if (!scope) {
            ll_typer_report_error(((LL_Error){ .main_token = AST_AS(typename, Ast_Ident)->base.token_info }), "Type '{}' not found", AST_AS(typename, Ast_Ident)->str);
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
            ll_typer_report_error(((LL_Error){ .main_token = AST_AS(typename, Ast_Ident)->base.token_info }), "Expected '{}' to be a type, but it's not", AST_AS(typename, Ast_Ident)->str);
            ll_typer_report_error_done(cc, typer);
            break;
        }

        break;
    case AST_KIND_TYPE_POINTER: {
        result = ll_typer_get_type_from_typename(cc, typer, AST_AS(typename, Ast_Type_Pointer)->element);
        if (!result) return NULL;
        result = ll_typer_get_ptr_type(cc, typer, result);
        break;
    }
    case AST_KIND_INDEX: {
        oc_assert(AST_AS(typename, Ast_Slice)->stop == NULL);
        LL_Type* element_type = ll_typer_get_type_from_typename(cc, typer, AST_AS(typename, Ast_Slice)->ptr);
        if (!element_type) return NULL;
        ll_typer_type_expression(cc, typer, &AST_AS(typename, Ast_Slice)->start, NULL, NULL);

        uint64_t array_width;
        if (AST_AS(typename, Ast_Slice)->start->has_const) {
            array_width = AST_AS(typename, Ast_Slice)->start->const_value.as_u64;
        } else {
            LL_Eval_Value value = ll_eval_node(cc, cc->eval_context, cc->bir, AST_AS(typename, Ast_Slice)->start);
            array_width = value.as_u64;
        }

        result = ll_typer_get_array_type(cc, typer, element_type, array_width);
        break;
    }
    case AST_KIND_SLICE: {
        LL_Type* element_type = ll_typer_get_type_from_typename(cc, typer, AST_AS(typename, Ast_Slice)->ptr);
        if (!element_type) return NULL;
        result = ll_typer_get_slice_type(cc, typer, element_type);
        break;
    }

    default: eprint("\x1b[31;1mTODO:\x1b[0m typename node {}\n", typename->kind);
    }

    typename->type = result;
    return result;
}

bool ll_typer_match_polymorphic(Compiler_Context* cc, LL_Typer* typer, Ast_Base* type_decl, LL_Type* provided_type, Ast_Base* site, bool is_top_level_this_arg) {
    LL_Type* expected_type = ll_typer_get_type_from_typename(cc, typer, type_decl);
    if (expected_type == provided_type) return true;

    switch (type_decl->kind) {
    case AST_KIND_GENERIC: {
        Ast_Base* cloned_generic = ast_clone_node_deep(cc, type_decl, (LL_Ast_Clone_Params) { 0 });
        cloned_generic->type = provided_type;

        LL_Scope_Simple* scope = create_scope_simple(LL_SCOPE_KIND_STRUCT, cloned_generic);
        scope->ident = AST_AS(cloned_generic, Ast_Generic)->ident;
        scope->declared_type = provided_type;

        ll_typer_scope_put(cc, typer, (LL_Scope*)scope, false);
        return true;
    }

    case AST_KIND_TYPE_POINTER: {
        bool result = true;
        if (provided_type->kind != LL_TYPE_POINTER) {
            if (is_top_level_this_arg) {
                // func(%T* this)
                // int a;
                // a.func(); <- can pass this as pointer; autoderef
                result = ll_typer_match_polymorphic(cc, typer,
                    AST_AS(type_decl, Ast_Type_Pointer)->element,
                    provided_type,
                    site,
                    false
                );
            } else {
                result = false;
            }
        }

        if (!result) {
            ll_typer_report_error((LL_Error){ .main_token = site->token_info }, "Expected a pointer type");
            ll_typer_report_error_no_src("    you provided the type ");
            ll_typer_report_error_type(cc, typer, provided_type);
            ll_typer_report_error_no_src("\n");
            ll_typer_report_error_note(((LL_Error){ .main_token = type_decl->token_info }), "Parameter type defined here");
            ll_typer_report_error_done(cc, typer);
            return false;
        }

        return ll_typer_match_polymorphic(cc, typer,
            AST_AS(type_decl, Ast_Type_Pointer)->element,
            ((LL_Type_Pointer*)provided_type)->element_type,
            site,
            false
        );
    } break;
    case AST_KIND_INDEX: {
        Ast_Slice* slice = AST_AS(type_decl, Ast_Slice);
        if (provided_type->kind != LL_TYPE_ARRAY) {
            ll_typer_report_error((LL_Error){ .main_token = site->token_info }, "Expected an array type");
            ll_typer_report_error_no_src("    you provided the type ");
            ll_typer_report_error_type(cc, typer, provided_type);
            ll_typer_report_error_no_src("\n");
            ll_typer_report_error_note(((LL_Error){ .main_token = slice->base.token_info }), "Parameter type defined here");
            ll_typer_report_error_done(cc, typer);
            return false;
        }

        if (slice->start->kind == AST_KIND_GENERIC) {
            Ast_Base* cloned_generic = ast_clone_node_deep(cc, slice->start, (LL_Ast_Clone_Params) { 0 });
            cloned_generic->has_const = 1;

            // is it fine if SCOPE LOCAL doesn't use an Ast_Variable_Decl?
            LL_Scope_Simple* scope = create_scope_simple(LL_SCOPE_KIND_LOCAL, cloned_generic);
            scope->ident = AST_AS(cloned_generic, Ast_Generic)->ident;
            scope->ident->base.has_const = 1;
            scope->ident->base.const_value.as_u64 = provided_type->width;
            scope->ident->base.type = typer->ty_uint64;

            ll_typer_scope_put(cc, typer, (LL_Scope*)scope, false);
        } else {
            ll_typer_type_expression(cc, typer, &slice->start, NULL, NULL);
            uint64_t array_width;
            if (slice->start->has_const) {
                array_width = slice->start->const_value.as_u64;
            } else {
                LL_Eval_Value value = ll_eval_node(cc, cc->eval_context, cc->bir, slice->start);
                array_width = value.as_u64;
            }

            if (provided_type->width != array_width) {
                ll_typer_report_error((LL_Error){ .main_token = site->token_info }, "Expected array to have {} elements but got one of {} elements", array_width, provided_type->width);
                ll_typer_report_error_done(cc, typer);
                return false;
            }
        }
        

        bool result = true;
        result |= ll_typer_match_polymorphic(cc, typer,
            slice->ptr,
            ((LL_Type_Slice*)provided_type)->element_type,
            site,
            false
        );
        return result;
    } break;
    case AST_KIND_SLICE: {
        Ast_Slice* slice = AST_AS(type_decl, Ast_Slice);
        if (provided_type->kind != LL_TYPE_SLICE) {
            ll_typer_report_error((LL_Error){ .main_token = site->token_info }, "Expected a slice type");
            ll_typer_report_error_no_src("    you provided the type ");
            ll_typer_report_error_type(cc, typer, provided_type);
            ll_typer_report_error_no_src("\n");
            ll_typer_report_error_note(((LL_Error){ .main_token = slice->base.token_info}), "Parameter type defined here");
            ll_typer_report_error_done(cc, typer);
            return false;
        }

        bool result = true;
        result |= ll_typer_match_polymorphic(cc, typer,
            slice->ptr,
            ((LL_Type_Slice*)provided_type)->element_type,
            site,
            false
        );
        return result;
    } break;
    default: oc_todo("flskdfj"); break;
    }
}

void ll_print_type_raw(LL_Type* type, Oc_Writer* w) {
    uint32_t i;
    switch (type->kind) {
    case LL_TYPE_VOID:     wprint(w, "void"); break;
    case LL_TYPE_INT:      wprint(w, "int{}", type->width); break;
    case LL_TYPE_UINT:     wprint(w, "uint{}", type->width); break;
    case LL_TYPE_ANYINT:   wprint(w, "anyint"); break;
    case LL_TYPE_FLOAT:    wprint(w, "float{}", type->width); break;
    case LL_TYPE_ANYFLOAT: wprint(w, "float"); break;
    case LL_TYPE_STRING:   wprint(w, "string"); break;
    case LL_TYPE_BOOL:     wprint(w, "bool{}", type->width); break;
    case LL_TYPE_ANYBOOL:  wprint(w, "bool"); break;
    case LL_TYPE_CHAR:     wprint(w, "char"); break;
    case LL_TYPE_POINTER: {
        LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)type;
        ll_print_type_raw(ptr_type->element_type, w);
        wprint(w, "*");
        break;
    }
    case LL_TYPE_ARRAY: {
        LL_Type_Array* array_type = (LL_Type_Array*)type;
        ll_print_type_raw(array_type->element_type, w);
        wprint(w, "[{}]", array_type->base.width);
        break;
    }
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

LL_Function_Instantiation* ll_typer_function_instance_put(Compiler_Context* cc, LL_Typer* typer, Ast_Function_Declaration* fn_decl, LL_Function_Instantiation inst) {
    (void)cc;
    (void)typer;
    size_t hash;
    hash = ll_type_hash((LL_Type*)inst.fn_type, MAP_DEFAULT_SEED);

    if (!fn_decl->instantiations) {
        fn_decl->instantiations = oc_arena_alloc(&cc->arena, sizeof(*fn_decl->instantiations));
    }

    hash = hash % oc_len(*fn_decl->instantiations);
    LL_Function_Instantiation *current = (*fn_decl->instantiations)[hash];

    LL_Function_Instantiation* new_entry = oc_arena_dup(&cc->arena, &inst, sizeof(inst));
    new_entry->next = current;

    (*fn_decl->instantiations)[hash] = new_entry;
    return new_entry;
}

LL_Function_Instantiation* ll_typer_function_instance_get(Compiler_Context* cc, LL_Typer* typer, Ast_Function_Declaration* fn_decl, LL_Type_Function* fn_type) {
    (void)cc;
    (void)typer;
    size_t hash;
    hash = ll_type_hash((LL_Type*)fn_type, MAP_DEFAULT_SEED);
    hash = hash % oc_len(*fn_decl->instantiations);

    if (!fn_decl->instantiations) {
        fn_decl->instantiations = oc_arena_alloc(&cc->arena, sizeof(*fn_decl->instantiations));
    }
    LL_Function_Instantiation* current = (*fn_decl->instantiations)[hash];

    while (current) {
        if (current->fn_type == fn_type) return current;
        current = current->next;
    }

    if (current) return current;
    return NULL;
}

void ll_typer_scope_put(Compiler_Context* cc, LL_Typer* typer, LL_Scope* scope, bool hoist) {
    size_t hash;
    int kind;
    LL_Scope* parent;
    if (hoist) parent = typer->current_scope->parent->parent;
    else parent = typer->current_scope;

    if (scope->ident) {
        kind = 0;
        hash = stbds_siphash_bytes(&kind, sizeof(kind), MAP_DEFAULT_SEED);
        hash = hash_combine(hash, stbds_hash_string(scope->ident->str, MAP_DEFAULT_SEED));
    } else {
        kind = 1;
        size_t anon = parent->next_anon++;
        hash = stbds_siphash_bytes(&kind, sizeof(kind), MAP_DEFAULT_SEED);
        hash = hash_combine(hash, stbds_siphash_bytes(&anon, sizeof(anon), MAP_DEFAULT_SEED));
    }
    hash = hash % oc_len(parent->children);
    LL_Scope_Map_Entry* current = parent->children[hash];

    LL_Scope_Map_Entry* new_entry = oc_arena_alloc(&cc->arena, sizeof(*new_entry));
    new_entry->scope = scope;
    new_entry->next = current;

    parent->children[hash] = new_entry;
    scope->parent = parent;
}

LL_Scope* ll_scope_get(LL_Scope* scope, string symbol_name) {
    int kind = 0; // scope is named
    size_t hash = stbds_siphash_bytes(&kind, sizeof(kind), MAP_DEFAULT_SEED);
    hash = hash_combine(hash, stbds_hash_string(symbol_name, MAP_DEFAULT_SEED));
    hash = hash % oc_len(scope->children);
    LL_Scope_Map_Entry* current = scope->children[hash];

    while (current) {
        if (string_eql(current->scope->ident->str, symbol_name)) break;
        current = current->next;
    }

    if (current) return current->scope;
    return NULL;
}

LL_Scope* ll_typer_find_symbol_up_scope_string(Compiler_Context* cc, LL_Typer* typer, string symbol_name, bool expansion) {
    (void)cc;
    LL_Scope* found_scope;
    LL_Scope* current = typer->current_scope;

    int out_of_macro_expansion = 0;

    while (current) {
        switch (current->kind) {
        case LL_SCOPE_KIND_MACRO_EXPANSION:
            found_scope = ll_scope_get(current, symbol_name);
            out_of_macro_expansion = 1;
            if (found_scope) return found_scope;
            current = current->parent;
            continue;
        case LL_SCOPE_KIND_PACKAGE:
            found_scope = ll_scope_get(current, symbol_name);
            break;
        case LL_SCOPE_KIND_BLOCK:
            if (out_of_macro_expansion == 1 || !out_of_macro_expansion || expansion) {
                found_scope = ll_scope_get(current, symbol_name);
            }
            break;
        case LL_SCOPE_KIND_FUNCTION:
        case LL_SCOPE_KIND_BLOCK_VALUE:
        case LL_SCOPE_KIND_LOOP:
            if (!out_of_macro_expansion || expansion)
                found_scope = ll_scope_get(current, symbol_name);
            break;
        default: found_scope = NULL; break;
        }
        if (found_scope) return found_scope;
        current = current->parent;
    }

    return NULL;
}

LL_Scope* ll_typer_find_symbol_up_scope(Compiler_Context* cc, LL_Typer* typer, Ast_Ident* ident) {
    return ll_typer_find_symbol_up_scope_string(cc, typer, ident->str, ident->flags & AST_IDENT_FLAG_EXPAND);
}

void ll_scope_print(LL_Scope* scope, int indent, Oc_Writer* w) {
    int i;
    LL_Scope_Map_Entry* current;

    for (i = 0; i < indent; ++i) {
        wprint(w, "  ");
    }

    switch (scope->kind) {
    case LL_SCOPE_KIND_LOCAL:       wprint(w, "Local"); break;
    case LL_SCOPE_KIND_FIELD:       wprint(w, "Field"); break;
    case LL_SCOPE_KIND_FUNCTION:    wprint(w, "Function"); break;
    case LL_SCOPE_KIND_PACKAGE:     wprint(w, "Module"); break;
    case LL_SCOPE_KIND_PARAMETER:   wprint(w, "Parmeter"); break;
    case LL_SCOPE_KIND_MACRO_PARAMETER:       wprint(w, "Macro_Parameter"); break;
    case LL_SCOPE_KIND_BLOCK:       wprint(w, "Block"); break;
    case LL_SCOPE_KIND_BLOCK_VALUE: wprint(w, "Block_Value"); break;
    case LL_SCOPE_KIND_MACRO_EXPANSION: wprint(w, "Macro_Expansion"); break;
    case LL_SCOPE_KIND_LOOP:        wprint(w, "Loop"); break;
    case LL_SCOPE_KIND_STRUCT:      wprint(w, "Struct"); break;
    case LL_SCOPE_KIND_TYPENAME:    wprint(w, "Typename"); break;
    }
    if (scope->ident)
        wprint(w, " '{}'", scope->ident->str);
    wprint(w, "\n");

    switch (scope->kind) {
    case LL_SCOPE_KIND_FIELD:
    case LL_SCOPE_KIND_LOCAL: return;
    default: break;
    }
    for (uword i = 0; i < oc_len(scope->children); ++i) {
        current = scope->children[i];
        while (current) {
            ll_scope_print(current->scope, indent + 1, w);
            current = current->next;
        }
    }
}

