
// #include <stdlib.h>
// #include <stdio.h>
// #include <inttypes.h>

#include "common.h"
#include "parser.h"
#include "lexer.h"
#include "ast.h"
#include <stdio.h>

void ll_print_type_raw(struct ll_type* type, Oc_Writer* fd);

LL_Parser parser_create_from_file(Compiler_Context* cc, char* filename) {
    (void)cc;
    FILE* input;
    if (fopen_s(&input, filename, "rb")) {
        eprint("unable to open file '{}'\n", filename);
        oc_exit(-1);
    }
    if (!input) {
        eprint("unable to open file '{}'\n", filename);
        oc_exit(-1);
    }

    fseek(input, 0, SEEK_END);
    size_t input_size = ftell(input);
    fseek(input, 0, SEEK_SET);

    uint8_t* input_contents = malloc(input_size);
    if (!input_contents) oc_oom();

    size_t read_amount = fread(input_contents, 1, input_size, input);
    if (read_amount != input_size) {
        eprint("Unable to read file: ferror() = {}\n", ferror(input));
        oc_assert(false);
    }
    fclose(input);

    LL_Parser result = {
        .lexer = {
            .pos = 0,
            .source.ptr = (char*)input_contents,
            .source.len = input_size,
            .filename = {
                .ptr = filename,
                .len = strlen(filename),
            }
        }
    };

    return result;
}

#define PEEK(out) (lexer_peek_token(cc, &(parser)->lexer, out))
#define TOKEN_INFO(token) ((LL_Token_Info){ .kind = (token).kind, .position = (token).position })
#define CONSUME() lexer_consume(cc, &(parser)->lexer)
#define UNEXPECTED() unexpected_token(cc, parser, __FILE__, __LINE__)
#define EXPECT(kind, out) expect_token(cc, parser, kind, out, __FILE__, __LINE__)
#define CREATE_NODE(_kind, value) ({ __typeof__(value) v = (value); _CREATE_ASSIGN_KIND((_kind), value); create_node(cc, (Ast_Base*)&v, sizeof(v)); })

#define _CREATE_ASSIGN_KIND(_kind, type) _Generic((type), \
        Ast_Base : (void)0,                       \
        default : v.base.kind = _kind                    \
    )

static Ast_Base* create_node(Compiler_Context* cc, Ast_Base* node, size_t size) {
    return oc_arena_dup(&cc->arena, node, size);
}

static Ast_Ident* create_ident(Compiler_Context* cc, string sym) {
    Ast_Ident* ident = (Ast_Ident*)CREATE_NODE(AST_KIND_IDENT, ((Ast_Ident){ .str = sym, .symbol_index = AST_IDENT_SYMBOL_INVALID }));
    if (ident->str.ptr[0] == '$') {
        ident->str.ptr++;
        ident->str.len--;
        ident->str = ll_intern_string(cc, ident->str);
        ident->flags |= AST_IDENT_FLAG_EXPAND;
    }
    return ident;
}

static void unexpected_token(Compiler_Context* cc, LL_Parser* parser, char* file, int line) {
    (void)file;
    (void)line;
    LL_Token tok;
    PEEK(&tok);

    LL_Line_Info line_info = lexer_get_line_info(&parser->lexer, TOKEN_INFO(tok));
    eprint("{}:{}:{}: \x1b[31;1merror\x1b[0m:\x1b[1m unexpected token '", parser->lexer.filename, line_info.line, line_info.column);
    lexer_print_token_raw_to_writer(&tok, &stderr_writer);
    eprint("' ({})\x1b[0m\n", tok.kind);
}

static bool expect_token(Compiler_Context* cc, LL_Parser* parser, LL_Token_Kind kind, LL_Token* out, char* file, int line) {
    (void)file;
    (void)line;
    LL_Token tok;
    PEEK(&tok);
    if (tok.kind != kind) {
        LL_Line_Info line_info = lexer_get_line_info(&parser->lexer, TOKEN_INFO(tok));
        eprint("{}:{}:{}: \x1b[31;1merror\x1b[0m:\x1b[1m expected token '", parser->lexer.filename, line_info.line, line_info.column);
        lexer_print_token_kind(kind, &stderr_writer);
        eprint("' ({}), but found '", tok.kind);
        lexer_print_token_raw_to_writer(&tok, &stderr_writer);
        eprint("' \x1b[0m\n");

        return false;
    } else {
        if (out != NULL)
            memcpy(out, &tok, sizeof(tok));
        CONSUME();
        return true;
    }
}

Ast_Base* parser_parse_file(Compiler_Context* cc, LL_Parser* parser) {
    Ast_Block block = { .base.kind = AST_KIND_BLOCK };
    LL_Token token;

    while (parser->lexer.pos < parser->lexer.source.len) {
        oc_array_append(&cc->arena, &block, parser_parse_statement(cc, parser));
        PEEK(&token);
    }

    return oc_arena_dup(&cc->arena, &block, sizeof(block));
}

Ast_Base* parser_parse_statement(Compiler_Context* cc, LL_Parser* parser) {
    Ast_Base* result;
    LL_Token token;
    LL_Storage_Class storage_class = 0;
    PEEK(&token);

START_SWITCH:
    if (!PEEK(&token)) {
        return NULL;
    }
    switch (token.kind) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '{':
            result = (Ast_Base*)parser_parse_block(cc, parser);
            break;
#pragma GCC diagnostic pop
        case LL_TOKEN_KIND_IDENT:
            while (PEEK(&token) && token.kind == LL_TOKEN_KIND_IDENT) {
                if (token.str.ptr == LL_KEYWORD_EXTERN.ptr) {
                    storage_class |= LL_STORAGE_CLASS_EXTERN;
                } else if (token.str.ptr == LL_KEYWORD_NATIVE.ptr) {
                    storage_class |= LL_STORAGE_CLASS_NATIVE;
                } else if (token.str.ptr == LL_KEYWORD_CONST.ptr) {
                    LL_Token_Info kw = TOKEN_INFO(token);
                    CONSUME();
                    PEEK(&token);
                    if (token.kind == '{') {
                        result = (Ast_Base*)parser_parse_block(cc, parser);
                        result = CREATE_NODE(AST_KIND_CONST, ((Ast_Marker){ .expr = result }));
                        result->token_info = kw;
                        return result;
                    } else if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_DO.ptr) {
                        result = parser_parse_expression(cc, parser, NULL, 0, false);
                        result = CREATE_NODE(AST_KIND_CONST, ((Ast_Marker){ .expr = result }));
                        result->token_info = kw;
                        goto HANDLE_TRY_DECL;
                    } else {
                        storage_class |= LL_STORAGE_CLASS_CONST;
                        goto START_SWITCH;
                    }
                } else {
                    goto HANDLE_IDENT;
                }
                CONSUME();
                PEEK(&token);
            }

            goto START_SWITCH;

            break;
        default:
HANDLE_IDENT:
            PEEK(&token);
            if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_STRUCT.ptr) {
                result = parser_parse_struct(cc, parser);
                return result;
            }

            result = parser_parse_expression(cc, parser, NULL, 0, true);
            if (result && (result->kind == AST_KIND_IF || result->kind == AST_KIND_FOR  || result->kind == AST_KIND_WHILE)) break;
HANDLE_TRY_DECL:
            PEEK(&token);
            if (token.kind == LL_TOKEN_KIND_IDENT) // this includes macro keyword
                result = parser_parse_declaration(cc, parser, result, storage_class);
            else
                EXPECT(';', &token);

            break;
    }
    return result;
}

Ast_Block* parser_parse_block(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
    Ast_Block block = { .base.kind = AST_KIND_BLOCK };

    PEEK(&token);
    if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_DO.ptr) {
        block.base.token_info = TOKEN_INFO(token);
        CONSUME();
        block.flags |= AST_BLOCK_FLAG_EXPR;
        EXPECT('{', &token);
        block.c_open = TOKEN_INFO(token);
    } else {
        CONSUME();
        block.c_open = TOKEN_INFO(token);
    }

    PEEK(&token);

    while (token.kind != '}') {
        oc_array_append(&cc->arena, &block, parser_parse_statement(cc, parser));
        PEEK(&token);
    }

    CONSUME();
    block.c_close = TOKEN_INFO(token);
    return oc_arena_dup(&cc->arena, &block, sizeof(block));
}

Ast_Parameter parser_parse_parameter(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
    PEEK(&token);
    Ast_Base* type, *init;
    LL_Parameter_Flags flags = 0;

    if (token.kind == LL_TOKEN_KIND_RANGE) {
        CONSUME();
        type = NULL;
        flags |= LL_PARAMETER_FLAG_VARIADIC;	
    } else if (token.kind == '%') {
        CONSUME();
        if (!EXPECT(LL_TOKEN_KIND_IDENT, &token)) return (Ast_Parameter) { 0 };
        Ast_Ident* ident = create_ident(cc, token.str);
        ident->base.token_info = TOKEN_INFO(token);

        type = CREATE_NODE(AST_KIND_GENERIC, ((Ast_Generic) { .ident = ident }));
        type = parser_parse_expression(cc, parser, type, 0, true);
    } else {
        type = parser_parse_expression(cc, parser, NULL, 0, true);
    }

    if (!EXPECT(LL_TOKEN_KIND_IDENT, &token)) return (Ast_Parameter) { 0 };
    Ast_Ident* ident = create_ident(cc, token.str);
    ident->base.token_info = TOKEN_INFO(token);

    PEEK(&token);
    LL_Token_Info eql_info = { 0 };
    if (token.kind == '=') {
        CONSUME();
        eql_info = TOKEN_INFO(token);
        init = parser_parse_expression(cc, parser, NULL, 0, false);
    } else {
        init = NULL;
    }

    return (Ast_Parameter) {
        .base.kind = AST_KIND_PARAMETER,
        .base.token_info = eql_info,
        .type = type,
        .ident = ident,
        .initializer = init,
        .flags = flags,
    };
}

Ast_Base* parser_parse_struct(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
    PEEK(&token);
    LL_Token_Info struct_kw = TOKEN_INFO(token);
    CONSUME(); // struct kw

    Ast_List items = { 0 };
    Ast_Ident* ident;

    PEEK(&token);
    if (token.kind == LL_TOKEN_KIND_IDENT) {
        CONSUME();
        ident = create_ident(cc, token.str);
        ident->base.token_info = TOKEN_INFO(token);
    }

    EXPECT('{', &token);
    LL_Token_Info c_open = TOKEN_INFO(token);
    PEEK(&token);
    while (token.kind != '}') {
        oc_array_append(&cc->arena, &items, parser_parse_statement(cc, parser));
        PEEK(&token);
    }
    EXPECT('}', &token);
    LL_Token_Info c_close = TOKEN_INFO(token);

    Ast_Base* result = CREATE_NODE(AST_KIND_STRUCT, ((Ast_Struct){ .ident = ident, .body = items, .c_open = c_open, .c_close = c_close }));
    result->token_info = struct_kw;
    return result;
}

Ast_Base* parser_parse_declaration(Compiler_Context* cc, LL_Parser* parser, Ast_Base* type, LL_Storage_Class storage_class) {
    LL_Token token;
    if (!type) {
        type = parser_parse_expression(cc, parser, NULL, 0, true);
    }

    if (!EXPECT(LL_TOKEN_KIND_IDENT, &token)) return NULL;
    if (token.str.ptr == LL_KEYWORD_MACRO.ptr) {
        storage_class |= LL_STORAGE_CLASS_MACRO;
        EXPECT(LL_TOKEN_KIND_IDENT, &token);
    }
    Ast_Ident* ident = create_ident(cc, token.str);
    ident->base.token_info = TOKEN_INFO(token);

    bool fn = false;
    Ast_Parameter_List parameters = { 0 };
    Ast_Base* body_or_init;
    LL_Token_Info p_open, p_close, eql;

    PEEK(&token);
    switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '(':
            fn = true;
            p_open = TOKEN_INFO(token);
            CONSUME();

            PEEK(&token);
            while (token.kind != ')') {
                Ast_Parameter parameter = parser_parse_parameter(cc, parser);
                oc_array_append(&cc->arena, &parameters, parameter);
                PEEK(&token);

                if (token.kind != ')') {
                    EXPECT(',', &token);
                    PEEK(&token);
                    continue;
                }
            }

            CONSUME();
            p_close = TOKEN_INFO(token);

            PEEK(&token);
            if (token.kind == '{') {
                body_or_init = (Ast_Base*)parser_parse_block(cc, parser);
            } else {
                EXPECT(';', &token);
                body_or_init = NULL;
            }

            break;
        case '=':
            eql = TOKEN_INFO(token);
            CONSUME();
            body_or_init = parser_parse_expression(cc, parser, NULL, 0, false);
            EXPECT(';', &token);
            break;
#pragma GCC diagnostic pop
        default:
            body_or_init = NULL;
            EXPECT(';', &token);
            break;
    }

    if (fn) {
        return CREATE_NODE(AST_KIND_FUNCTION_DECLARATION, ((Ast_Function_Declaration){
            .return_type = type,
            .ident = ident,
            .parameters = parameters,
            .body = body_or_init,
            .storage_class = storage_class,
            .p_open = p_open, .p_close = p_close,
        }));
    } else {
        return CREATE_NODE(AST_KIND_VARIABLE_DECLARATION, ((Ast_Variable_Declaration){
            .base.token_info = eql,
            .type = type,
            .ident = ident,
            .initializer = body_or_init,
            .storage_class = storage_class,
        }));
    }
}

int get_binary_precedence(LL_Token token, bool from_statement) {
    switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '=':
        case LL_TOKEN_KIND_ASSIGN_PLUS:
        case LL_TOKEN_KIND_ASSIGN_MINUS:
        case LL_TOKEN_KIND_ASSIGN_TIMES:
        case LL_TOKEN_KIND_ASSIGN_DIVIDE:
        case LL_TOKEN_KIND_ASSIGN_PERCENT:
            return 20;
        case LL_TOKEN_KIND_OR: return 40;
        case LL_TOKEN_KIND_AND: return 50;
        case '|': return 60;
        case '^': return 70;
        case '&': return 80;
        case LL_TOKEN_KIND_EQUALS:
        case LL_TOKEN_KIND_NEQUALS:
            return from_statement ? 0 : 90;
        case '<':
        case '>':
        case LL_TOKEN_KIND_LTE:
        case LL_TOKEN_KIND_GTE:
            return from_statement ? 0 : 100;
        case '+':
        case '-':
            return from_statement ? 0 : 120;
        case '*':
        case '/':
        case '%':
            return from_statement ? 0 : 130;
        case '.':
            return 160;
#pragma GCC diagnostic pop
        default: return 0;
    }
}

int get_postfix_precedence(LL_Token token) {
    switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '*':
        case '[':
        case '(':
            return 150;
#pragma GCC diagnostic pop
        case LL_TOKEN_KIND_IDENT:
            if (token.str.ptr == LL_KEYWORD_CAST.ptr) return 160;
            return 0;
        default: return 0;
    }
}

Ast_Base* parser_parse_initializer(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
    Ast_Base *expr1, *expr2;
    Ast_Initializer result = { 0 };
    PEEK(&token);
    result.base.token_info = TOKEN_INFO(token);
    CONSUME(); // consume {

    PEEK(&token);
    while (token.kind != '}') {
        expr1 = parser_parse_primary(cc, parser, false);
        PEEK(&token);
        if (token.kind == '=') {
            CONSUME();
            expr2 = parser_parse_expression(cc, parser, NULL, 0, false);
            expr1 = CREATE_NODE(AST_KIND_KEY_VALUE, ((Ast_Key_Value) { .key = expr1, .value = expr2 }));
            expr1->token_info = TOKEN_INFO(token);
        }

        oc_array_append(&cc->arena, &result, expr1);
        
        PEEK(&token);
        if (token.kind != '}') {
            EXPECT(',', &token);
            PEEK(&token);
            continue;
        }
    }

    EXPECT('}', &token);
    result.c_close = TOKEN_INFO(token);
    return CREATE_NODE(AST_KIND_INITIALIZER, result);
}

Ast_Base* parser_parse_array_initializer(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
    Ast_Base *expr1, *expr2;
    Ast_Initializer result = { 0 };
    PEEK(&token);
    result.base.token_info = TOKEN_INFO(token);
    CONSUME(); // consume [

    PEEK(&token);
    while (token.kind != ']') {
        expr1 = parser_parse_expression(cc, parser, NULL, 30, false);
        if (token.kind == '=') {
            CONSUME();
            expr2 = parser_parse_expression(cc, parser, NULL, 0, false);
            expr1 = CREATE_NODE(AST_KIND_KEY_VALUE, ((Ast_Key_Value) { .key = expr1, .value = expr2 }));
            expr1->token_info = TOKEN_INFO(token);
        }
        oc_array_append(&cc->arena, &result, expr1);
        
        PEEK(&token);
        if (token.kind != ']') {
            EXPECT(',', &token);
            PEEK(&token);
            continue;
        }
    }

    EXPECT(']', &token);
    result.c_close = TOKEN_INFO(token);
    return CREATE_NODE(AST_KIND_ARRAY_INITIALIZER, result);
}

Ast_Base* parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, Ast_Base* left, int last_precedence, bool from_statement) {
    LL_Token token;
    Ast_Base *right;
    if (left == NULL) {
        PEEK(&token);
        switch (token.kind) {
        case LL_TOKEN_KIND_IDENT:
            if (token.str.ptr == LL_KEYWORD_IF.ptr) {
                left = parser_parse_primary(cc, parser, from_statement);
                return left;
            } else if (token.str.ptr == LL_KEYWORD_FOR.ptr) {
                left = parser_parse_primary(cc, parser, from_statement);
                return left;
            } else if (token.str.ptr == LL_KEYWORD_WHILE.ptr) {
                left = parser_parse_primary(cc, parser, from_statement);
                return left;
            } else if (token.str.ptr == LL_KEYWORD_STRUCT.ptr) {
                left = parser_parse_struct(cc, parser);
                return left;
            }
            // fallthrough
        default:
            left = parser_parse_primary(cc, parser, from_statement);
            break;
        }
    }
    
    while (PEEK(&token)) {
        int bin_precedence = get_binary_precedence(token, from_statement);
        int post_precedence = get_postfix_precedence(token);

        if (bin_precedence != 0 && bin_precedence >= last_precedence) {
            CONSUME();
            LL_Token op_tok = token;
            right = parser_parse_primary(cc, parser, false);
            // if (op_tok.kind == '.') {
            //     oc_breakpoint();
            // }
            while (PEEK(&token)) {
                int next_bin_precedence = get_binary_precedence(token, false);
                int next_post_precedence = get_postfix_precedence(token);
                if (next_bin_precedence != 0 && next_bin_precedence > bin_precedence) {
                    right = parser_parse_expression(cc, parser, right, bin_precedence + 1, false);
                } else if (next_post_precedence != 0 && next_post_precedence > bin_precedence) {
                    Ast_Base* new_right = parser_parse_expression(cc, parser, right, next_post_precedence, false);
                    if (new_right == right) break;
                    right = new_right;
                } else break;
            }

            left = CREATE_NODE(AST_KIND_BINARY_OP, ((Ast_Operation){ .left = left, .right = right, .op = op_tok }));
            left->token_info = TOKEN_INFO(op_tok);
            from_statement = false;
        } else if (post_precedence != 0 && post_precedence >= last_precedence) {
            switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
                case '(': {
                    LL_Token_Info ti = TOKEN_INFO(token);
                    CONSUME();

                    Ast_List arguments = { 0 };
                    Ast_Base *expr1, *expr2;
                    PEEK(&token);
                    while (token.kind != ')') {
                        expr1 = parser_parse_expression(cc, parser, NULL, 30, false);
                        PEEK(&token);
                        if (token.kind == '=') {
                            CONSUME();
                            expr2 = parser_parse_expression(cc, parser, NULL, 0, false);
                            expr1 = CREATE_NODE(AST_KIND_KEY_VALUE, ((Ast_Key_Value) { .key = expr1, .value = expr2 }));
                        }
                        oc_array_append(&cc->arena, &arguments, expr1);

                        PEEK(&token);
                        if (token.kind != ')') {
                            EXPECT(',', &token);
                            PEEK(&token);
                            continue;
                        }
                    }

                    CONSUME();

                    left = CREATE_NODE(AST_KIND_INVOKE, ((Ast_Invoke){ .expr = left, .arguments = arguments, .p_close = TOKEN_INFO(token) }));
                    left->token_info = ti;

                    break;
                }
                case '*': {
                    if (!from_statement) return left;
                    CONSUME();
                    
                    left = CREATE_NODE(AST_KIND_TYPE_POINTER, ((Ast_Type_Pointer){ .element = left }));
                    left->token_info = TOKEN_INFO(token);

                    break;
                }
                case '[': {
                    Ast_Base *start = NULL, *stop = NULL;
                    LL_Token_Info ti = TOKEN_INFO(token);
                    CONSUME();
                    Ast_Kind kind = AST_KIND_INDEX;

                    PEEK(&token);
                    switch (token.kind) {
                    case ']':
                        break;
                    case ':':
                        CONSUME();
                        kind = AST_KIND_SLICE;

                        PEEK(&token);
                        if (token.kind != ']') {
                            stop = parser_parse_expression(cc, parser, NULL, 0, false);
                        }

                        break;
                    default:
                        start = parser_parse_expression(cc, parser, NULL, 0, false);

                        PEEK(&token);
                        if (token.kind == ':') {
                            CONSUME();
                            kind = AST_KIND_SLICE;

                            PEEK(&token);
                            if (token.kind != ']') {
                                stop = parser_parse_expression(cc, parser, NULL, 0, false);
                            }
                        }

                        break;
                    }
                    EXPECT(']', &token);

                    left = CREATE_NODE(kind, ((Ast_Slice){ .ptr = left, .start = start, .stop = stop }));
                    left->token_info = ti;
                    break;
                }
#pragma GCC diagnostic pop
                default: oc_assert(false); break;
            }
            // from_statement = false;
        } else break;
    }

    return left;
}

Ast_Base* parser_parse_primary(Compiler_Context* cc, LL_Parser* parser, bool from_statement) {
    LL_Token token;
    Ast_Base *result, *right, *body, *update;
    PEEK(&token);

    switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    case '(':
        CONSUME();
        result = parser_parse_expression(cc, parser, NULL, 0, false);
        EXPECT(')', NULL);
        break;
    case '{':
        result = parser_parse_initializer(cc, parser);
        break;
    case '[':
        result = parser_parse_array_initializer(cc, parser);
        break;
    case '-':
        CONSUME();
        result = CREATE_NODE(AST_KIND_PRE_OP, ((Ast_Operation){ .op = token, .right = parser_parse_expression(cc, parser, NULL, 140, false) }));
        break;
    case '*':
        CONSUME();
        result = CREATE_NODE(AST_KIND_PRE_OP, ((Ast_Operation){ .op = token, .right = parser_parse_expression(cc, parser, NULL, 140, false) }));
        break;
    case '&':
        CONSUME();
        result = CREATE_NODE(AST_KIND_PRE_OP, ((Ast_Operation){ .op = token, .right = parser_parse_expression(cc, parser, NULL, 140, false) }));
        break;
#pragma GCC diagnostic pop
    
    case LL_TOKEN_KIND_INT:
        result = CREATE_NODE(AST_KIND_LITERAL_INT, ((Ast_Literal){ .u64 = token.u64 }));
        result->token_info = TOKEN_INFO(token);
        CONSUME();
        break;

    case LL_TOKEN_KIND_FLOAT:
        result = CREATE_NODE(AST_KIND_LITERAL_FLOAT, ((Ast_Literal){ .f64 = token.f64 }));
        result->token_info = TOKEN_INFO(token);
        CONSUME();
        break;

    case LL_TOKEN_KIND_STRING:
        result = CREATE_NODE(AST_KIND_LITERAL_STRING, ((Ast_Literal){ .str = token.str }));
        result->token_info = TOKEN_INFO(token);
        CONSUME();
        break;

    case LL_TOKEN_KIND_IDENT:
        if (token.str.ptr == LL_KEYWORD_IF.ptr) {
            LL_Token_Info if_kw = TOKEN_INFO(token);
            CONSUME();

            // parse if statement
            result = parser_parse_expression(cc, parser, NULL, 0, false);
            PEEK(&token);
            if (token.kind == '{' || (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_DO.ptr)) {
                body = (Ast_Base*)parser_parse_block(cc, parser);
            } else {
                body = parser_parse_expression(cc, parser, NULL, 0, false);
                PEEK(&token);
                if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_ELSE.ptr) {
                } else {
                    if (from_statement) EXPECT(';', &token);
                }
            }

            // parse else clause
            PEEK(&token);
            LL_Token_Info else_kw = TOKEN_INFO(token);
            if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_ELSE.ptr) {
                CONSUME();
                PEEK(&token);
                if (token.kind == '{') {
                    right = (Ast_Base*)parser_parse_block(cc, parser);
                } else {
                    right = parser_parse_expression(cc, parser, NULL, 0, false);
                    if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_IF.ptr) {}
                    else if (from_statement) EXPECT(';', &token);
                }
            } else {
                right = NULL;
            }

            result = CREATE_NODE(AST_KIND_IF, ((Ast_If){ .cond = result, .body = body, .else_clause = right, .else_kw = else_kw }));
            result->token_info = if_kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_WHILE.ptr) {
            LL_Token_Info for_kw = TOKEN_INFO(token);
            CONSUME();

            right = parser_parse_expression(cc, parser, NULL, 0, false);

            PEEK(&token);
            if (token.kind == '{') {
                body = (Ast_Base*)parser_parse_block(cc, parser);
            } else {
                body = parser_parse_expression(cc, parser, NULL, 0, false);
                if (from_statement) EXPECT(';', &token);
            }

            result = CREATE_NODE(AST_KIND_WHILE, ((Ast_Loop){ .cond = right, .body = body }));
            result->token_info = for_kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_FOR.ptr) {
            LL_Token_Info for_kw = TOKEN_INFO(token);
            CONSUME();
            PEEK(&token);
            if (token.kind == ';') {
                result = NULL;
                CONSUME();
            } else {
                result = parser_parse_declaration(cc, parser, false, 0);
            }
            PEEK(&token);
            if (token.kind == ';') {
                right = NULL;
            } else {
                right = parser_parse_expression(cc, parser, NULL, 0, false);
            }
            EXPECT(';', &token);

            PEEK(&token);
            if (token.kind == '{') {
                update = NULL;
                body = (Ast_Base*)parser_parse_block(cc, parser);
            } else {
                update = parser_parse_expression(cc, parser, NULL, 0, false);
                PEEK(&token);
                if (token.kind == '{') {
                    body = (Ast_Base*)parser_parse_block(cc, parser);
                } else {
                    body = parser_parse_expression(cc, parser, NULL, 0, false);
                    if (from_statement) EXPECT(';', &token);
                }
            }

            result = CREATE_NODE(AST_KIND_FOR, ((Ast_Loop){ .init = result, .cond = right, .update = update, .body = body }));
            result->token_info = for_kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_CAST.ptr) {
            LL_Token_Info cast_kw = TOKEN_INFO(token);
            CONSUME();

            EXPECT('(', &token);
            LL_Token_Info p_open = TOKEN_INFO(token);
            result = parser_parse_expression(cc, parser, NULL, 0, true);
            EXPECT(')', &token);
            LL_Token_Info p_close = TOKEN_INFO(token);
            right = parser_parse_expression(cc, parser, NULL, 150, false);

            result = CREATE_NODE(AST_KIND_CAST, ((Ast_Cast){ .cast_type = result, .expr = right, .p_open = p_open, .p_close = p_close }));
            result->token_info = cast_kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_DO.ptr) {
            result = (Ast_Base*)parser_parse_block(cc, parser);
            break;
        } else if (token.str.ptr == LL_KEYWORD_CONST.ptr) {
            LL_Token_Info kw = TOKEN_INFO(token);
            CONSUME();
            result = parser_parse_expression(cc, parser, NULL, 0, false);
            result = CREATE_NODE(AST_KIND_CONST, ((Ast_Marker){ .expr = result }));
            result->token_info = kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_RETURN.ptr) {
            CONSUME();
            LL_Token_Info kw = TOKEN_INFO(token);
            PEEK(&token);
            if (token.kind != ';')
                result = parser_parse_expression(cc, parser, NULL, 0, false);
            else result = NULL;
            result = CREATE_NODE(AST_KIND_RETURN, ((Ast_Control_Flow){ .expr = result }));
            result->token_info = kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_BREAK.ptr) {
            CONSUME();
            LL_Token_Info kw = TOKEN_INFO(token);
            result = NULL;
            Ast_Control_Flow_Target target = AST_CONTROL_FLOW_TARGET_ANY;

            PEEK(&token);
            if (token.kind != ';') {
                if (token.kind == LL_TOKEN_KIND_IDENT) {
                    if (token.str.ptr == LL_KEYWORD_DO.ptr) {
                        CONSUME();
                        target = AST_CONTROL_FLOW_TARGET_DO;
                    } else if (token.str.ptr == LL_KEYWORD_FOR.ptr) {
                        CONSUME();
                        target = AST_CONTROL_FLOW_TARGET_FOR;
                    // } else if (token.str.ptr == LL_KEYWORD_IF.ptr) {
                    //     CONSUME();
                    //     target = AST_CONTROL_FLOW_TARGET_IF;
                    } else if (token.str.ptr == LL_KEYWORD_WHILE.ptr) {
                        CONSUME();
                        target = AST_CONTROL_FLOW_TARGET_WHILE;
                    }
                }
                PEEK(&token);
                if (token.kind != ';') {
                    result = parser_parse_expression(cc, parser, NULL, 0, false);
                }
            };
            result = CREATE_NODE(AST_KIND_BREAK, ((Ast_Control_Flow){ .expr = result, .target = target }));
            result->token_info = kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_CONTINUE.ptr) {
            CONSUME();
            LL_Token_Info kw = TOKEN_INFO(token);
            PEEK(&token);
            if (token.kind != ';')
                result = parser_parse_expression(cc, parser, NULL, 0, false);
            else result = NULL;
            result = CREATE_NODE(AST_KIND_CONTINUE, ((Ast_Control_Flow){ .expr = result }));
            result->token_info = kw;
            break;
        }
        CONSUME();
        result = (Ast_Base*)create_ident(cc, token.str);
        result->token_info = TOKEN_INFO(token);
        break;

    default:
        UNEXPECTED();
        result = NULL;
        break;
    }

    return result;
}

void print_storage_class(LL_Storage_Class storage_class, Oc_Writer* w) {
    if (storage_class & LL_STORAGE_CLASS_EXTERN) wprint(w, "extern ");
    if (storage_class & LL_STORAGE_CLASS_STATIC) wprint(w, "static ");
    if (storage_class & LL_STORAGE_CLASS_NATIVE) wprint(w, "native ");
    if (storage_class & LL_STORAGE_CLASS_CONST) wprint(w, "const ");
}

const char* ast_get_node_kind(Ast_Base* node) {
    switch (node->kind) {
        case AST_KIND_LITERAL_INT: return "Int_Literal";
        case AST_KIND_LITERAL_FLOAT: return "Float_Literal";
        case AST_KIND_LITERAL_STRING: return "String_Literal";
        case AST_KIND_IDENT: return "Identifier";
        case AST_KIND_BINARY_OP: return "Binary_Operator";
        case AST_KIND_PRE_OP: return "Prefix_Operator";
        case AST_KIND_INVOKE: return "Invoke";
        case AST_KIND_INITIALIZER: return "Initializer";
        case AST_KIND_ARRAY_INITIALIZER: return "Array_Initializer";
        case AST_KIND_KEY_VALUE: return "Key_Value";
        case AST_KIND_BLOCK: return "Block";
        case AST_KIND_CONST: return "Const";
        case AST_KIND_VARIABLE_DECLARATION: return "Variable_Declaration";
        case AST_KIND_FUNCTION_DECLARATION: return "Function_Declaration";
        case AST_KIND_PARAMETER: return "Parameter";
        case AST_KIND_RETURN: return "Return";
        case AST_KIND_BREAK: return "Break";
        case AST_KIND_CONTINUE: return "Continue";
        case AST_KIND_IF: return "If";
        case AST_KIND_FOR: return "For";
        case AST_KIND_WHILE: return "While";
        case AST_KIND_INDEX: return "Index";
        case AST_KIND_SLICE: return "Slice";
        case AST_KIND_CAST: return "Cast";
        case AST_KIND_STRUCT: return "Struct";
        case AST_KIND_GENERIC: return "Generic";
        case AST_KIND_TYPE_POINTER: return "Pointer";
        default: oc_unreachable("");
    }
}

void print_node_value(Ast_Base* node, Oc_Writer* w) {
    switch (node->kind) {
        case AST_KIND_LITERAL_INT:    wprint(w, "{}", AST_AS(node, Ast_Literal)->u64); break;
        case AST_KIND_LITERAL_FLOAT:  wprint(w, "{}", AST_AS(node, Ast_Literal)->f64); break;
        case AST_KIND_LITERAL_STRING: wprint(w, "{}", AST_AS(node, Ast_Literal)->str); break;
        case AST_KIND_IDENT:          wprint(w, "{}", AST_AS(node, Ast_Ident)->str); break;
        case AST_KIND_BINARY_OP: lexer_print_token_raw_to_writer(&AST_AS(node, Ast_Operation)->op, w); break;
        case AST_KIND_PRE_OP:    lexer_print_token_raw_to_writer(&AST_AS(node, Ast_Operation)->op, w); break;
        case AST_KIND_INVOKE: break;
        case AST_KIND_INITIALIZER: break;
        case AST_KIND_ARRAY_INITIALIZER: break;
        case AST_KIND_KEY_VALUE: break;
        case AST_KIND_BLOCK: break;
        case AST_KIND_CONST: break;
        case AST_KIND_VARIABLE_DECLARATION: print_storage_class(AST_AS(node, Ast_Variable_Declaration)->storage_class, w); break;
        case AST_KIND_FUNCTION_DECLARATION: print_storage_class(AST_AS(node, Ast_Function_Declaration)->storage_class, w); break;
        case AST_KIND_PARAMETER: break;
        case AST_KIND_RETURN: break;
        case AST_KIND_BREAK: break;
        case AST_KIND_CONTINUE: break;
        case AST_KIND_IF: break;
        case AST_KIND_FOR: break;
        case AST_KIND_WHILE: break;
        case AST_KIND_INDEX: break;
        case AST_KIND_SLICE: break;
        case AST_KIND_GENERIC: break;
        case AST_KIND_CAST:
            if (node->type)
                ll_print_type_raw(node->type, &stdout_writer);
            break;
        case AST_KIND_STRUCT:
            print_node_value(&AST_AS(node, Ast_Struct)->ident->base, w);
            break;
        case AST_KIND_TYPE_POINTER: break;
        default: oc_unreachable("");
    }

    if (node->has_const) {
        wprint(w, " {}", node->const_value.as_u64);
    }
}

void print_node(Ast_Base* node, uint32_t indent, Oc_Writer* w) {
    uint32_t i;
    for (i = 0; i < indent; ++i) {
        wprint(w, "  ");
    }
    const char* node_kind = ast_get_node_kind(node);
    wprint(w, "{} ", node_kind);
    print_node_value(node, w);
    if (node->type) {
        uint32_t max_indent = 7;
        if (max_indent > indent) {
            for (i = 0; i < (max_indent - indent); ++i) {
                wprint(w, "  ");
            }
        }
        wprint(w, "\t\t\t");
        ll_print_type_raw(node->type, w);
    }
    wprint(w, "\n");
    switch (node->kind) {
        case AST_KIND_BINARY_OP: 
            print_node(AST_AS(node, Ast_Operation)->left, indent + 1, w);
            print_node(AST_AS(node, Ast_Operation)->right, indent + 1, w);
            break;
        case AST_KIND_PRE_OP: 
            print_node(AST_AS(node, Ast_Operation)->right, indent + 1, w);
            break;

        case AST_KIND_INVOKE: 
            print_node(AST_AS(node, Ast_Invoke)->expr, indent + 1, w);
            // for (i = 0; i < AST_AS(node, Ast_Invoke)->arguments.count; ++i) {
            //     print_node((Ast_Base*)AST_AS(node, Ast_Invoke)->arguments.items[i], indent + 1, w);
            // }
            for (i = 0; i < AST_AS(node, Ast_Invoke)->ordered_arguments.count; ++i) {
                print_node((Ast_Base*)AST_AS(node, Ast_Invoke)->ordered_arguments.items[i], indent + 1, w);
            }
            break;

        case AST_KIND_INITIALIZER: 
        case AST_KIND_ARRAY_INITIALIZER: 
            for (i = 0; i < AST_AS(node, Ast_Initializer)->count; ++i) {
                print_node((Ast_Base*)AST_AS(node, Ast_Initializer)->items[i], indent + 1, w);
            }
            break;

        case AST_KIND_KEY_VALUE: 
            print_node((Ast_Base*)AST_AS(node, Ast_Key_Value)->key, indent + 1, w);
            print_node((Ast_Base*)AST_AS(node, Ast_Key_Value)->value, indent + 1, w);
            break;

        case AST_KIND_CONST:
            print_node(AST_AS(node, Ast_Marker)->expr, indent + 1, w);
            break;
        case AST_KIND_VARIABLE_DECLARATION:
            print_node((Ast_Base*)AST_AS(node, Ast_Variable_Declaration)->type, indent + 1, w);
            print_node((Ast_Base*)AST_AS(node, Ast_Variable_Declaration)->ident, indent + 1, w);
            if (AST_AS(node, Ast_Variable_Declaration)->initializer) print_node(AST_AS(node, Ast_Variable_Declaration)->initializer, indent + 1, w);
            break;

        case AST_KIND_FUNCTION_DECLARATION:
            print_node(AST_AS(node, Ast_Function_Declaration)->return_type, indent + 1, w);
            print_node((Ast_Base*)AST_AS(node, Ast_Function_Declaration)->ident, indent + 1, w);
            for (i = 0; i < AST_AS(node, Ast_Function_Declaration)->parameters.count; ++i) {
                print_node((Ast_Base*)&AST_AS(node, Ast_Function_Declaration)->parameters.items[i], indent + 1, w);
            }
            if (AST_AS(node, Ast_Function_Declaration)->body) print_node(AST_AS(node, Ast_Function_Declaration)->body, indent + 1, w);
            break;

        case AST_KIND_PARAMETER:
            if (AST_AS(node, Ast_Variable_Declaration)->type)
                print_node(AST_AS(node, Ast_Variable_Declaration)->type, indent + 1, w);
            print_node((Ast_Base*)AST_AS(node, Ast_Variable_Declaration)->ident, indent + 1, w);
            break;
        case AST_KIND_RETURN:
            if (AST_AS(node, Ast_Control_Flow)->expr)
                print_node(AST_AS(node, Ast_Control_Flow)->expr, indent + 1, w);
            break;
        case AST_KIND_BREAK:
            if (AST_AS(node, Ast_Control_Flow)->expr)
                print_node(AST_AS(node, Ast_Control_Flow)->expr, indent + 1, w);
            break;
        case AST_KIND_CONTINUE:
            if (AST_AS(node, Ast_Control_Flow)->expr)
                print_node(AST_AS(node, Ast_Control_Flow)->expr, indent + 1, w);
            break;
        case AST_KIND_IF:
            if (AST_AS(node, Ast_If)->cond)
                print_node(AST_AS(node, Ast_If)->cond, indent + 1, w);
            if (AST_AS(node, Ast_If)->body)
                print_node(AST_AS(node, Ast_If)->body, indent + 1, w);
            if (AST_AS(node, Ast_If)->else_clause)
                print_node(AST_AS(node, Ast_If)->else_clause, indent + 1, w);
            break;
        case AST_KIND_WHILE:
        case AST_KIND_FOR:
            if (AST_AS(node, Ast_Loop)->init)
                print_node(AST_AS(node, Ast_Loop)->init, indent + 1, w);
            if (AST_AS(node, Ast_Loop)->cond)
                print_node(AST_AS(node, Ast_Loop)->cond, indent + 1, w);
            if (AST_AS(node, Ast_Loop)->update)
                print_node(AST_AS(node, Ast_Loop)->update, indent + 1, w);
            if (AST_AS(node, Ast_Loop)->body)
                print_node(AST_AS(node, Ast_Loop)->body, indent + 1, w);
            break;

        case AST_KIND_INDEX:
            print_node(AST_AS(node, Ast_Operation)->left, indent + 1, w);
            if (AST_AS(node, Ast_Operation)->right)
                print_node(AST_AS(node, Ast_Operation)->right, indent + 1, w);
            break;

        case AST_KIND_CAST:
            if (AST_AS(node, Ast_Cast)->cast_type) print_node(AST_AS(node, Ast_Cast)->cast_type, indent + 1, w);
            print_node(AST_AS(node, Ast_Cast)->expr, indent + 1, w);
            break;

        case AST_KIND_STRUCT:
            for (i = 0; i < AST_AS(node, Ast_Struct)->body.count; ++i) {
                print_node(AST_AS(node, Ast_Struct)->body.items[i], indent + 1, w);
            }
            break;

        case AST_KIND_GENERIC:
            print_node((Ast_Base*)AST_AS(node, Ast_Generic)->ident, indent + 1, w);
            break;

        case AST_KIND_TYPE_POINTER:
            print_node(AST_AS(node, Ast_Type_Pointer)->element, indent + 1, w);
            break;

        case AST_KIND_BLOCK:
            for (i = 0; i < AST_AS(node, Ast_Block)->count; ++i) {
                print_node(AST_AS(node, Ast_Block)->items[i], indent + 1, w);
            }

        default: break;
    }
}

Ast_Base* ast_clone_node_deep(Compiler_Context* cc, Ast_Base* node, LL_Ast_Clone_Params params) {
    uint32_t i;
    Ast_Base* result;
    if (!node) return NULL;

    switch (node->kind) {
    case AST_KIND_BINARY_OP:
        result = CREATE_NODE(node->kind, ((Ast_Operation) {
            .base.token_info = node->token_info,
            .left = ast_clone_node_deep(cc, AST_AS(node, Ast_Operation)->left, params),
            .op = AST_AS(node, Ast_Operation)->op,
            .right = ast_clone_node_deep(cc, AST_AS(node, Ast_Operation)->right, params),
        }));
        break;
    case AST_KIND_PRE_OP: 
        result = CREATE_NODE(node->kind, ((Ast_Operation) {
            .base.token_info = node->token_info,
            .op = AST_AS(node, Ast_Operation)->op,
            .right = ast_clone_node_deep(cc, AST_AS(node, Ast_Operation)->right, params),
        }));
        break;

    case AST_KIND_INVOKE: {
        Ast_List newargs = { 0 };
        for (i = 0; i < AST_AS(node, Ast_Invoke)->arguments.count; ++i) {
            oc_array_append(&cc->arena, &newargs, ast_clone_node_deep(cc, AST_AS(node, Ast_Invoke)->arguments.items[i], params));
        }
        result = CREATE_NODE(node->kind, ((Ast_Invoke) {
            .base.token_info = node->token_info,
            .expr = AST_AS(node, Ast_Invoke)->expr,
            .arguments = newargs,
        }));
        // for (i = 0; i < AST_AS(node, Ast_Invoke)->ordered_arguments.count; ++i) {
        //     print_node((Ast_Base*)AST_AS(node, Ast_Invoke)->ordered_arguments.items[i], indent + 1, w);
        // }
    } break;

    case AST_KIND_INITIALIZER: 
    case AST_KIND_ARRAY_INITIALIZER: {
        Ast_List newargs = { 0 };
        for (i = 0; i < AST_AS(node, Ast_Initializer)->count; ++i) {
            oc_array_append(&cc->arena, &newargs, ast_clone_node_deep(cc, AST_AS(node, Ast_Initializer)->items[i], params));
        }
        result = CREATE_NODE(node->kind, ((Ast_Initializer) {
            .base.token_info = node->token_info,
            .items = newargs.items,
            .count = newargs.count,
            .capacity = newargs.capacity,
        }));
        // for (i = 0; i < AST_AS(node, Ast_Initializer)->count; ++i) {
        //     print_node((Ast_Base*)AST_AS(node, Ast_Initializer)->items[i], indent + 1, w);
        // }
    } break;

    case AST_KIND_KEY_VALUE: 
        result = CREATE_NODE(node->kind, ((Ast_Key_Value) {
            .base.token_info = node->token_info,
            .key = ast_clone_node_deep(cc, AST_AS(node, Ast_Key_Value)->key, params),
            .value = ast_clone_node_deep(cc, AST_AS(node, Ast_Key_Value)->value, params),
        }));
        break;

    case AST_KIND_CONST:
        result = CREATE_NODE(node->kind, ((Ast_Marker) {
            .base.token_info = node->token_info,
            .expr = ast_clone_node_deep(cc, AST_AS(node, Ast_Marker)->expr, params),
        }));
        break;
    case AST_KIND_VARIABLE_DECLARATION:
        result = CREATE_NODE(node->kind, ((Ast_Variable_Declaration) {
            .base.token_info = node->token_info,
            .type = ast_clone_node_deep(cc, AST_AS(node, Ast_Variable_Declaration)->type, params),
            .ident = (Ast_Ident*)ast_clone_node_deep(cc, (Ast_Base*)AST_AS(node, Ast_Variable_Declaration)->ident, params),
            .initializer = ast_clone_node_deep(cc, AST_AS(node, Ast_Variable_Declaration)->initializer, params),
            .storage_class = AST_AS(node, Ast_Variable_Declaration)->storage_class,
        }));
        break;

    case AST_KIND_FUNCTION_DECLARATION: {
        Ast_Parameter_List newargs = { 0 };
        for (i = 0; i < AST_AS(node, Ast_Function_Declaration)->parameters.count; ++i) {
            oc_array_append(&cc->arena, &newargs, ((Ast_Parameter){
                .base.kind = AST_KIND_PARAMETER,
                .base.token_info = AST_AS(node, Ast_Function_Declaration)->parameters.items[i].base.token_info,
                .type = ast_clone_node_deep(cc, AST_AS(node, Ast_Function_Declaration)->parameters.items[i].type, params),
                .ident = (Ast_Ident*)ast_clone_node_deep(cc, (Ast_Base*)AST_AS(node, Ast_Function_Declaration)->parameters.items[i].ident, params),
                .initializer = ast_clone_node_deep(cc, AST_AS(node, Ast_Function_Declaration)->parameters.items[i].initializer, params),
                .flags = AST_AS(node, Ast_Function_Declaration)->parameters.items[i].flags,
            }));
        }

        result = CREATE_NODE(node->kind, ((Ast_Function_Declaration) {
            .base.token_info = node->token_info,
            .return_type = ast_clone_node_deep(cc, AST_AS(node, Ast_Function_Declaration)->return_type, params),
            .ident = (Ast_Ident*)ast_clone_node_deep(cc, (Ast_Base*)AST_AS(node, Ast_Function_Declaration)->ident, params),
            .body = ast_clone_node_deep(cc, AST_AS(node, Ast_Function_Declaration)->body, params),
            .storage_class = AST_AS(node, Ast_Function_Declaration)->storage_class,
            .parameters = newargs,
        }));
    } break;

    case AST_KIND_PARAMETER:
        result = CREATE_NODE(node->kind, ((Ast_Parameter){
            .base.token_info = node->token_info,
            .type = ast_clone_node_deep(cc, AST_AS(node, Ast_Parameter)->type, params),
            .ident = (Ast_Ident*)ast_clone_node_deep(cc, (Ast_Base*)AST_AS(node, Ast_Parameter)->ident, params),
            .initializer = ast_clone_node_deep(cc, AST_AS(node, Ast_Parameter)->initializer, params),
            .flags = AST_AS(node, Ast_Parameter)->flags,
        }));
        break;
    case AST_KIND_CONTINUE:
    case AST_KIND_BREAK:
    case AST_KIND_RETURN:
        result = CREATE_NODE(node->kind, ((Ast_Control_Flow){
            .base.token_info = node->token_info,
            .expr = ast_clone_node_deep(cc, AST_AS(node, Ast_Control_Flow)->expr, params),
        }));
        break;
    case AST_KIND_IF:
        result = CREATE_NODE(node->kind, ((Ast_If){
            .base.token_info = node->token_info,
            .cond = ast_clone_node_deep(cc, AST_AS(node, Ast_If)->cond, params),
            .body = ast_clone_node_deep(cc, AST_AS(node, Ast_If)->body, params),
            .else_clause = ast_clone_node_deep(cc, AST_AS(node, Ast_If)->else_clause, params),
        }));
        break;
    case AST_KIND_WHILE:
    case AST_KIND_FOR:
        result = CREATE_NODE(node->kind, ((Ast_Loop){
            .base.token_info = node->token_info,
            .init = ast_clone_node_deep(cc, AST_AS(node, Ast_Loop)->init, params),
            .cond = ast_clone_node_deep(cc, AST_AS(node, Ast_Loop)->cond, params),
            .update = ast_clone_node_deep(cc, AST_AS(node, Ast_Loop)->update, params),
            .body = ast_clone_node_deep(cc, AST_AS(node, Ast_Loop)->body, params),
        }));
        break;

    case AST_KIND_INDEX:
    case AST_KIND_SLICE:
        result = CREATE_NODE(node->kind, ((Ast_Slice){
            .base.token_info = node->token_info,
            .ptr = ast_clone_node_deep(cc, AST_AS(node, Ast_Slice)->ptr, params),
            .start = ast_clone_node_deep(cc, AST_AS(node, Ast_Slice)->start, params),
            .stop = ast_clone_node_deep(cc, AST_AS(node, Ast_Slice)->stop, params),
        }));
        break;

    case AST_KIND_CAST:
        result = CREATE_NODE(node->kind, ((Ast_Cast){
            .base.token_info = node->token_info,
            .cast_type = ast_clone_node_deep(cc, AST_AS(node, Ast_Cast)->cast_type, params),
            .expr = ast_clone_node_deep(cc, AST_AS(node, Ast_Cast)->expr, params),
        }));
        break;

    case AST_KIND_STRUCT: {
        Ast_List newargs = { 0 };
        for (i = 0; i < AST_AS(node, Ast_Struct)->body.count; ++i) {
            oc_array_append(&cc->arena, &newargs, ast_clone_node_deep(cc, AST_AS(node, Ast_Struct)->body.items[i], params));
        }
        result = CREATE_NODE(node->kind, ((Ast_Struct){
            .base.token_info = node->token_info,
            .ident = (Ast_Ident*)ast_clone_node_deep(cc, (Ast_Base*)AST_AS(node, Ast_Struct)->ident, params),
            .body = newargs,
        }));
    } break;

    case AST_KIND_GENERIC:
        result = CREATE_NODE(node->kind, ((Ast_Generic){
            .base.token_info = node->token_info,
            .ident = (Ast_Ident*)ast_clone_node_deep(cc, (Ast_Base*)AST_AS(node, Ast_Generic)->ident, params),
        }));
        break;

    case AST_KIND_TYPE_POINTER:
        result = CREATE_NODE(node->kind, ((Ast_Type_Pointer){
            .base.token_info = node->token_info,
            .element = ast_clone_node_deep(cc, AST_AS(node, Ast_Type_Pointer)->element, params),
        }));
        break;

    case AST_KIND_BLOCK: {
        Ast_Block_Flags flags = AST_AS(node, Ast_Block)->flags;
        if (params.expand_first_block) {
            params.expand_first_block = false;
            flags |= AST_BLOCK_FLAG_MACRO_EXPANSION;
        }
        Ast_List newargs = { 0 };
        for (i = 0; i < AST_AS(node, Ast_Block)->count; ++i) {
            oc_array_append(&cc->arena, &newargs, ast_clone_node_deep(cc, AST_AS(node, Ast_Block)->items[i], params));
        }
        result = CREATE_NODE(AST_KIND_BLOCK, ((Ast_Block){
            .base.token_info = node->token_info,
            .flags = flags,
            .items = newargs.items,
            .count = newargs.count,
            .capacity = newargs.capacity,
        }));
    } break;

    case AST_KIND_LITERAL_INT: {
        result = CREATE_NODE(node->kind, ((Ast_Literal){
            .base.token_info = node->token_info,
            .u64 = AST_AS(node, Ast_Literal)->u64,
        }));
    } break;
    case AST_KIND_LITERAL_FLOAT: {
        result = CREATE_NODE(node->kind, ((Ast_Literal){
            .base.token_info = node->token_info,
            .f64 = AST_AS(node, Ast_Literal)->f64,
        }));
    } break;
    case AST_KIND_LITERAL_STRING: {
        result = CREATE_NODE(node->kind, ((Ast_Literal){
            .base.token_info = node->token_info,
            .str = AST_AS(node, Ast_Literal)->str,
        }));
    } break;
    case AST_KIND_IDENT: {
        result = CREATE_NODE(node->kind, ((Ast_Ident){
            .base.token_info = node->token_info,
            .str = AST_AS(node, Ast_Ident)->str,
            .flags = AST_AS(node, Ast_Ident)->flags | (params.convert_all_idents_to_expansion ? AST_IDENT_FLAG_EXPAND : 0),
        }));
    } break;
    // default: oc_unreachable(""); break;
    }
    return result;
}
