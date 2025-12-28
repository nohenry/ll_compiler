
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
#define CREATE_NODE(_kind, value, ...) ({ __typeof__(value) v = (value); _CREATE_ASSIGN_KIND((_kind), v); Code* node = create_node(cc, (Code*)&v, sizeof(v)); ((Parse_Result) { .code = node, __VA_ARGS__ }); })

#define _CREATE_ASSIGN_KIND(_kind, type) _Generic((type), \
        Code : (void)0,                                   \
        Code_Variable_Declaration : ((Code_Variable_Declaration*)&v)->base.base.kind = _kind,      \
        Code_Function_Declaration : ((Code_Function_Declaration*)&v)->base.base.kind = _kind,      \
        default : ((Code_Scope*)&v)->base.kind = _kind                     \
    )

static Code* create_node(Compiler_Context* cc, Code* node, size_t size) {
    return oc_arena_dup(&cc->arena, node, size);
}

static Code_Ident* create_ident(Compiler_Context* cc, string sym) {
	Parse_Result result = CREATE_NODE(CODE_KIND_IDENT, ((Code_Ident){ .str = sym, .symbol_index = CODE_IDENT_SYMBOL_INVALID }));
    Code_Ident* ident = (Code_Ident*)result.code;
    if (ident->str.ptr[0] == '$') {
        ident->str.ptr++;
        ident->str.len--;
        ident->str = ll_intern_string(cc, ident->str);
        ident->flags |= CODE_IDENT_FLAG_EXPAND;
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

static inline void insert_into_block(Compiler_Context* cc, Code_Scope* block, Parse_Result stmt) {
	if (!stmt.code) return;

    switch (stmt.code->kind) {
    case CODE_KIND_VARIABLE_DECLARATION: {
		Parse_Result assign = CREATE_NODE(CODE_KIND_BINARY_OP, ((Code_Operation){
            .left = (Code*)CODE_AS(stmt.code, Code_Variable_Declaration)->base.ident,
            .right = CODE_AS(stmt.code, Code_Variable_Declaration)->initializer,
            .op = CODE_AS(stmt.code, Code_Variable_Declaration)->base.base.token_info, // eql
        }));

        oc_array_append(&cc->arena, &block->statements, assign.code);
    } // fallthrough
    case CODE_KIND_FUNCTION_DECLARATION: {
        Code_Declaration* decl = CODE_AS(stmt.code, Code_Declaration);
        string name = decl->ident->str;
        print("name: {}\n", name);
        oc_assert(hash_map_get(&cc->arena, &block->declarations, name) == NULL);
        hash_map_put(&cc->arena, &block->declarations, name, decl);
    } break;
    default:
        print("append\n");
        oc_array_append(&cc->arena, &block->statements, stmt.code);
        break;
    }
}

Parse_Result parser_parse_file(Compiler_Context* cc, LL_Parser* parser) {
	Parse_Result block_result = CREATE_NODE(CODE_KIND_BLOCK, ((Code_Scope){ .base.kind = CODE_KIND_BLOCK }));
    Code_Scope* block = (Code_Scope*)block_result.code;
    LL_Token token;

    parser->current_scope = block;
    while (parser->lexer.pos < parser->lexer.source.len) {
        Parse_Result stmt = parser_parse_statement(cc, parser);
        print_node(stmt.code, 0, &stdout_writer);
        /* insert_into_block(cc, block, stmt); */
        PEEK(&token);
    }

    return (Parse_Result) { .code = (Code*)block };
}

Parse_Result parser_parse_statement(Compiler_Context* cc, LL_Parser* parser) {
    Parse_Result result;
    LL_Token token;
    LL_Storage_Class storage_class = 0;
    PEEK(&token);

START_SWITCH:
    if (!PEEK(&token)) {
        return (Parse_Result) { 0 };
    }
    switch (token.kind) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '{':
            result.code = (Code*)parser_parse_block(cc, parser);
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
                        result.code = (Code*)parser_parse_block(cc, parser);
                        result = CREATE_NODE(CODE_KIND_CONST, ((Code_Marker){ .expr = result.code }));
                        result.code->token_info = kw;
                        return result;
                    } else if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_DO.ptr) {
                        result = parser_parse_expression(cc, parser, NULL, 0, false);
                        result = CREATE_NODE(CODE_KIND_CONST, ((Code_Marker){ .expr = result.code }));
                        result.code->token_info = kw;
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
            /* if (result && (result->code->kind == CODE_KIND_IF || result-->kind == CODE_KIND_FOR  || result->kind == CODE_KIND_WHILE)) break; */
HANDLE_TRY_DECL:
            PEEK(&token);
            if (token.kind == LL_TOKEN_KIND_IDENT) // this includes macro keyword
                result = parser_parse_declaration(cc, parser, &result, storage_class);
            else
                EXPECT(';', &token);

            break;
    }
    return result;
}

Code_Scope* parser_parse_block(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
	Parse_Result block_result = CREATE_NODE(CODE_KIND_BLOCK, ((Code_Scope){ .base.kind = CODE_KIND_BLOCK }));
    Code_Scope* block = (Code_Scope*)block_result.code;

    PEEK(&token);
    if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_DO.ptr) {
        block->base.token_info = TOKEN_INFO(token);
        CONSUME();
        block->flags |= CODE_BLOCK_FLAG_EXPR;
        EXPECT('{', &token);
        block->c_open = TOKEN_INFO(token);
    } else {
        CONSUME();
        block->c_open = TOKEN_INFO(token);
    }

    PEEK(&token);

    block->parent_scope = parser->current_scope;
    parser->current_scope = block;
    while (token.kind != '}') {
        Parse_Result stmt = parser_parse_statement(cc, parser);
        /* insert_into_block(cc, block, stmt); */
        PEEK(&token);
    }
    parser->current_scope = block->parent_scope;

    CONSUME();
    block->c_close = TOKEN_INFO(token);
    return block;
}

Code_Parameter parser_parse_parameter(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
    PEEK(&token);
    Parse_Result type, init;
    LL_Parameter_Flags flags = 0;

    if (token.kind == LL_TOKEN_KIND_RANGE) {
        CONSUME();
        type.code = NULL;
        flags |= LL_PARAMETER_FLAG_VARIADIC;	
    } else if (token.kind == '%') {
        CONSUME();
        if (!EXPECT(LL_TOKEN_KIND_IDENT, &token)) return (Code_Parameter) { 0 };
        Code_Ident* ident = create_ident(cc, token.str);
        ident->base.token_info = TOKEN_INFO(token);

        type = CREATE_NODE(CODE_KIND_GENERIC, ((Code_Generic) { .ident = ident }));
        type = parser_parse_expression(cc, parser, &type, 0, true);
    } else {
        type = parser_parse_expression(cc, parser, NULL, 0, true);
    }

    PEEK(&token);
    Code_Ident* ident = NULL;
    if (token.kind == LL_TOKEN_KIND_IDENT) {
        CONSUME();
        ident = create_ident(cc, token.str);
        ident->base.token_info = TOKEN_INFO(token);
    }

    PEEK(&token);
    LL_Token_Info eql_info = { 0 };
    if (token.kind == '=') {
        CONSUME();
        eql_info = TOKEN_INFO(token);
        init = parser_parse_expression(cc, parser, NULL, 0, false);
    } else {
        init.code = NULL;
    }

    return (Code_Parameter) {
        .base.kind = CODE_KIND_PARAMETER,
        .base.token_info = eql_info,
        .type = type.code,
        .ident = ident,
        .initializer = init.code,
        .flags = flags,
    };
}

Parse_Result parser_parse_struct(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
    PEEK(&token);
    LL_Token_Info struct_kw = TOKEN_INFO(token);
    CONSUME(); // struct kw

    Code_Ident* ident = NULL;

    PEEK(&token);
    if (token.kind == LL_TOKEN_KIND_IDENT) {
        CONSUME();
        ident = create_ident(cc, token.str);
        ident->base.token_info = TOKEN_INFO(token);
    }

    Code_Scope* block = parser_parse_block(cc, parser);
    Parse_Result result = CREATE_NODE(CODE_KIND_STRUCT, ((Code_Struct){ .ident = ident, .block = block }));
    result.code->token_info = struct_kw;
    return result;
}

Parse_Result parser_parse_declaration(Compiler_Context* cc, LL_Parser* parser, Parse_Result* incoming_type, LL_Storage_Class storage_class) {
    LL_Token token;
	Parse_Result type;
	if (incoming_type) type = *incoming_type;
	else {
        type = parser_parse_expression(cc, parser, NULL, 0, true);
    }

    if (!EXPECT(LL_TOKEN_KIND_IDENT, &token)) return (Parse_Result) { 0 };
    if (token.str.ptr == LL_KEYWORD_MACRO.ptr) {
        storage_class |= LL_STORAGE_CLASS_MACRO;
        EXPECT(LL_TOKEN_KIND_IDENT, &token);
    }
    Code_Ident* ident = create_ident(cc, token.str);
    ident->base.token_info = TOKEN_INFO(token);

    bool fn = false;
    Code_Parameter_List parameters = { 0 };
    Parse_Result body_or_init;
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
                Code_Parameter parameter = parser_parse_parameter(cc, parser);
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
                body_or_init.code = (Code*)parser_parse_block(cc, parser);
            } else {
                EXPECT(';', &token);
                body_or_init.code = NULL;
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
            body_or_init.code = NULL;
            EXPECT(';', &token);
            break;
    }

	Parse_Result result;
    if (fn) {
        result = CREATE_NODE(CODE_KIND_FUNCTION_DECLARATION, ((Code_Function_Declaration){
            .base.type = type.code,
            .base.ident = ident,
            .parameters = parameters,
            .body = (Code_Scope*)body_or_init.code,
            .storage_class = storage_class,
            .p_open = p_open, .p_close = p_close,
        }));
    } else {
		result = CREATE_NODE(CODE_KIND_VARIABLE_DECLARATION, ((Code_Variable_Declaration){
            .base.base.token_info = eql,
            .base.type = type.code,
            .base.ident = ident,
            .initializer = body_or_init.code,
            .storage_class = storage_class,
        }));
    }

	oc_assert(hash_map_get(&cc->arena, &parser->current_scope->declarations, ident->str) == NULL);
	hash_map_put(&cc->arena, &parser->current_scope->declarations, ident->str, (Code_Declaration*)result.code);

	return result;
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

Parse_Result parser_parse_initializer(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
    Parse_Result expr1, expr2;
    Code_Initializer result = { 0 };
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
			oc_todo("handle non Codes");
            expr1 = CREATE_NODE(CODE_KIND_KEY_VALUE, ((Code_Key_Value) { .key = expr1.code, .value = expr2.code }));
            expr1.code->token_info = TOKEN_INFO(token);
        }

		oc_todo("handle non Codes");
        oc_array_append(&cc->arena, &result, expr1.code);
        
        PEEK(&token);
        if (token.kind != '}') {
            EXPECT(',', &token);
            PEEK(&token);
            continue;
        }
    }

    EXPECT('}', &token);
    result.c_close = TOKEN_INFO(token);
    return CREATE_NODE(CODE_KIND_INITIALIZER, result);
}

Parse_Result parser_parse_array_initializer(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
    Parse_Result expr1, expr2;
    Code_Initializer result = { 0 };
    PEEK(&token);
    result.base.token_info = TOKEN_INFO(token);
    CONSUME(); // consume [

    PEEK(&token);
    while (token.kind != ']') {
        expr1 = parser_parse_expression(cc, parser, NULL, 30, false);
        if (token.kind == '=') {
            CONSUME();
            expr2 = parser_parse_expression(cc, parser, NULL, 0, false);
			oc_todo("handle resutl non Codes");
            expr1 = CREATE_NODE(CODE_KIND_KEY_VALUE, ((Code_Key_Value) { .key = expr1.code, .value = expr2.code }));
            expr1.code->token_info = TOKEN_INFO(token);
        }
		oc_todo("handle resutl non Codes");
        oc_array_append(&cc->arena, &result, expr1.code);
        
        PEEK(&token);
        if (token.kind != ']') {
            EXPECT(',', &token);
            PEEK(&token);
            continue;
        }
    }

    EXPECT(']', &token);
    result.c_close = TOKEN_INFO(token);
    return CREATE_NODE(CODE_KIND_ARRAY_INITIALIZER, result);
}

Parse_Result parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, Parse_Result* incoming_left, int last_precedence, bool from_statement) {
    LL_Token token;
	Parse_Result left, right;
	if (incoming_left) left = *incoming_left;
	else {
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
            
            PEEK(&token);
            if (op_tok.kind == '*') {
            switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
            case '*':
            case '[':
            case '%':
            case ')':
            case '}':
            case ';':
            case '.':
            case ',':
#pragma GCC diagnostic pop
                left = CREATE_NODE(CODE_KIND_TYPE_POINTER, ((Code_Type_Pointer){ .element = left.code }));
                left.code->token_info = TOKEN_INFO(op_tok);
                continue;
            default: break;
            }
            }

            right = parser_parse_primary(cc, parser, false);
            // if (op_tok.kind == '.') {
            //     oc_breakpoint();
            // }
            while (PEEK(&token)) {
                int next_bin_precedence = get_binary_precedence(token, false);
                int next_post_precedence = get_postfix_precedence(token);
                if (next_bin_precedence != 0 && next_bin_precedence > bin_precedence) {
                    right = parser_parse_expression(cc, parser, &right, bin_precedence + 1, false);
                } else if (next_post_precedence != 0 && next_post_precedence > bin_precedence) {
                    Parse_Result new_right = parser_parse_expression(cc, parser, &right, next_post_precedence, false);
                    if (new_right.code == right.code) break;
                    right = new_right;
                } else break;
            }

            /* left = CREATE_NODE(CODE_KIND_BINARY_OP, ((Code_Operation){ .left = left.code, .right= right.code, .op = TOKEN_INFO(op_tok) })); */
            /* left.code->token_info = TOKEN_INFO(op_tok); */
			LL_Operation_Kind op_kind;
			switch (token.kind) {
			case '+': op_kind = LL_OPERATION_ADD;
			case '=': op_kind = LL_OPERATION_ASSIGN;
			default: oc_todo("uniplented");
			}

			LL_Typecheck_Value* tval = parser->ops_values->items + parser->ops_values->count;
			oc_array_extend_count_unint(&cc->arena, &parser->ops_values[op_kind], 2);
			tval[0].type = parser->linear_grid[left.kind].items[left.value].type;
			tval[1].type = parser->linear_grid[right.kind].items[right.value].type;

			left.kind = op_kind;
			left.value = (uint32)parser->ops[op_kind].count;
			oc_array_append(&cc->arena, &parser->ops[op_kind], ((LL_Typecheck_Value) { .type = tval[0].type }));

            from_statement = false;
        } else if (post_precedence != 0 && post_precedence >= last_precedence) {
            switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
                case '(': {
                    LL_Token_Info ti = TOKEN_INFO(token);
                    CONSUME();

                    Code_List arguments = { 0 };
                    Parse_Result expr1, expr2;
                    PEEK(&token);
                    while (token.kind != ')') {
                        expr1 = parser_parse_expression(cc, parser, NULL, 30, false);
                        PEEK(&token);
                        if (token.kind == '=') {
                            CONSUME();
                            expr2 = parser_parse_expression(cc, parser, NULL, 0, false);
							oc_todo("handle non Codes");
                            expr1 = CREATE_NODE(CODE_KIND_KEY_VALUE, ((Code_Key_Value) { .key = expr1.code, .value = expr2.code }));
                        }
						oc_todo("handle non Codes");
                        oc_array_append(&cc->arena, &arguments, expr1.code);

                        PEEK(&token);
                        if (token.kind != ')') {
                            EXPECT(',', &token);
                            PEEK(&token);
                            continue;
                        }
                    }

                    CONSUME();

					oc_todo("handle non Codes");
                    left = CREATE_NODE(CODE_KIND_INVOKE, ((Code_Invoke){ .expr = left.code, .arguments = arguments, .p_close = TOKEN_INFO(token) }));
                    left.code->token_info = ti;

                    break;
                }
                case '*': {
                    if (!from_statement) return left;
                    CONSUME();
                    
					oc_todo("handle non Codes");
                    left = CREATE_NODE(CODE_KIND_TYPE_POINTER, ((Code_Type_Pointer){ .element = left.code }));
                    left.code->token_info = TOKEN_INFO(token);

                    break;
                }
                case '[': {
                    Parse_Result start = { 0 }, stop = { 0 };
                    LL_Token_Info ti = TOKEN_INFO(token);
                    CONSUME();
                    Code_Kind kind = CODE_KIND_INDEX;

                    PEEK(&token);
                    switch (token.kind) {
                    case ']':
                        break;
                    case ':':
                        CONSUME();
                        kind = CODE_KIND_SLICE;

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
                            kind = CODE_KIND_SLICE;

                            PEEK(&token);
                            if (token.kind != ']') {
                                stop = parser_parse_expression(cc, parser, NULL, 0, false);
                            }
                        }

                        break;
                    }
                    EXPECT(']', &token);

                    left = CREATE_NODE(kind, ((Code_Slice){ .ptr = left.code, .start = start.code, .stop = stop.code }));
                    left.code->token_info = ti;
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

Parse_Result parser_parse_primary(Compiler_Context* cc, LL_Parser* parser, bool from_statement) {
    LL_Token token;
    Parse_Result result, right, body, update;
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
	#if 0
    case '-':
        CONSUME();
        result = CREATE_NODE(CODE_KIND_PRE_OP, ((Code_Operation){ .op = TOKEN_INFO(token), .right = parser_parse_expression(cc, parser, NULL, 140, false) }));
        break;
    case '*':
        CONSUME();
        result = CREATE_NODE(CODE_KIND_PRE_OP, ((Code_Operation){ .op = TOKEN_INFO(token), .right = parser_parse_expression(cc, parser, NULL, 140, false) }));
        break;
    case '&':
        CONSUME();
        result = CREATE_NODE(CODE_KIND_PRE_OP, ((Code_Operation){ .op = TOKEN_INFO(token), .right = parser_parse_expression(cc, parser, NULL, 140, false) }));
        break;
    case '%':
        CONSUME();
        if (!EXPECT(LL_TOKEN_KIND_IDENT, &token)) return NULL;
        Code_Ident* ident = create_ident(cc, token.str);
        ident->base.token_info = TOKEN_INFO(token);

        result = CREATE_NODE(CODE_KIND_GENERIC, ((Code_Generic) { .ident = ident }));
        result = parser_parse_expression(cc, parser, result, 0, true);
        break;
#pragma GCC diagnostic pop
	#endif
    
    case LL_TOKEN_KIND_INT:
        CONSUME();
        result = CREATE_NODE(CODE_KIND_LITERAL_INT, ((Code_Literal){ .u64 = token.u64 }));
        result.code->token_info = TOKEN_INFO(token);

		result.kind = RESULT_KIND_INT;
		result.value = (uint32)parser->ints.count;
		oc_array_extend_count_unint(&cc->arena, &parser->ints, 1);
        break;

    case LL_TOKEN_KIND_FLOAT:
        CONSUME();
        result = CREATE_NODE(CODE_KIND_LITERAL_FLOAT, ((Code_Literal){ .f64 = token.f64 }));
        result.code->token_info = TOKEN_INFO(token);

		result.kind = RESULT_KIND_FLOAT;
		result.value = (uint32)parser->floats.count;
		oc_array_extend_count_unint(&cc->arena, &parser->floats, 1);
        break;

#if 0
    case LL_TOKEN_KIND_STRING:
        result = CREATE_NODE(CODE_KIND_LITERAL_STRING, ((Code_Literal){ .str = token.str }));
        result->token_info = TOKEN_INFO(token);
        CONSUME();
        break;
	#endif

    case LL_TOKEN_KIND_IDENT:
	#if 0
        if (token.str.ptr == LL_KEYWORD_IF.ptr) {
            LL_Token_Info if_kw = TOKEN_INFO(token);
            CONSUME();

            // parse if statement
            result = parser_parse_expression(cc, parser, NULL, 0, false);
            PEEK(&token);
            if (token.kind == '{' || (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_DO.ptr)) {
                body = (Code*)parser_parse_block(cc, parser);
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
                    right = (Code*)parser_parse_block(cc, parser);
                } else {
                    right = parser_parse_expression(cc, parser, NULL, 0, false);
                    if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_IF.ptr) {}
                    else if (from_statement) EXPECT(';', &token);
                }
            } else {
                right = NULL;
            }

            result = CREATE_NODE(CODE_KIND_IF, ((Code_If){ .cond = result, .body = body, .else_clause = right, .else_kw = else_kw }));
            result->token_info = if_kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_WHILE.ptr) {
            LL_Token_Info for_kw = TOKEN_INFO(token);
            CONSUME();

            right = parser_parse_expression(cc, parser, NULL, 0, false);

            PEEK(&token);
            if (token.kind == '{') {
                body = (Code*)parser_parse_block(cc, parser);
            } else {
                body = parser_parse_expression(cc, parser, NULL, 0, false);
                if (from_statement) EXPECT(';', &token);
            }

            result = CREATE_NODE(CODE_KIND_WHILE, ((Code_Loop){ .cond = right, .body = body }));
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
                body = (Code*)parser_parse_block(cc, parser);
            } else {
                update = parser_parse_expression(cc, parser, NULL, 0, false);
                PEEK(&token);
                if (token.kind == '{') {
                    body = (Code*)parser_parse_block(cc, parser);
                } else {
                    body = parser_parse_expression(cc, parser, NULL, 0, false);
                    if (from_statement) EXPECT(';', &token);
                }
            }

            result = CREATE_NODE(CODE_KIND_FOR, ((Code_Loop){ .init = result, .cond = right, .update = update, .body = body }));
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

            result = CREATE_NODE(CODE_KIND_CAST, ((Code_Cast){ .cast_type = result, .expr = right, .p_open = p_open, .p_close = p_close }));
            result->token_info = cast_kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_DO.ptr) {
            result = (Code*)parser_parse_block(cc, parser);
            break;
        } else if (token.str.ptr == LL_KEYWORD_CONST.ptr) {
            LL_Token_Info kw = TOKEN_INFO(token);
            CONSUME();
            result = parser_parse_expression(cc, parser, NULL, 0, false);
            result = CREATE_NODE(CODE_KIND_CONST, ((Code_Marker){ .expr = result }));
            result->token_info = kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_RETURN.ptr) {
            CONSUME();
            LL_Token_Info kw = TOKEN_INFO(token);
            PEEK(&token);
            if (token.kind != ';')
                result = parser_parse_expression(cc, parser, NULL, 0, false);
            else result = NULL;
            result = CREATE_NODE(CODE_KIND_RETURN, ((Code_Control_Flow){ .expr = result }));
            result->token_info = kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_BREAK.ptr) {
            CONSUME();
            LL_Token_Info kw = TOKEN_INFO(token);
            result = NULL;
            Code_Control_Flow_Target target = CODE_CONTROL_FLOW_TARGET_ANY;

            PEEK(&token);
            if (token.kind != ';') {
                if (token.kind == LL_TOKEN_KIND_IDENT) {
                    if (token.str.ptr == LL_KEYWORD_DO.ptr) {
                        CONSUME();
                        target = CODE_CONTROL_FLOW_TARGET_DO;
                    } else if (token.str.ptr == LL_KEYWORD_FOR.ptr) {
                        CONSUME();
                        target = CODE_CONTROL_FLOW_TARGET_FOR;
                    // } else if (token.str.ptr == LL_KEYWORD_IF.ptr) {
                    //     CONSUME();
                    //     target = CODE_CONTROL_FLOW_TARGET_IF;
                    } else if (token.str.ptr == LL_KEYWORD_WHILE.ptr) {
                        CONSUME();
                        target = CODE_CONTROL_FLOW_TARGET_WHILE;
                    }
                }
                PEEK(&token);
                if (token.kind != ';') {
                    result = parser_parse_expression(cc, parser, NULL, 0, false);
                }
            };
            result = CREATE_NODE(CODE_KIND_BREAK, ((Code_Control_Flow){ .expr = result, .target = target }));
            result->token_info = kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_CONTINUE.ptr) {
            CONSUME();
            LL_Token_Info kw = TOKEN_INFO(token);
            PEEK(&token);
            if (token.kind != ';')
                result = parser_parse_expression(cc, parser, NULL, 0, false);
            else result = NULL;
            result = CREATE_NODE(CODE_KIND_CONTINUE, ((Code_Control_Flow){ .expr = result }));
            result->token_info = kw;
            break;
        }
	#endif
        CONSUME();
		result.kind = RESULT_KIND_INT;
		result.value = (uint32)parser->idents.count;
		oc_array_extend_count_unint(&cc->arena, &parser->idents, 1);

        result.code = (Code*)create_ident(cc, token.str);
        result.code->token_info = TOKEN_INFO(token);
        break;

    default:
        UNEXPECTED();
        result.code = NULL;
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

const char* ast_get_node_kind(Code* node) {
    switch (node->kind) {
        case CODE_KIND_LITERAL_INT: return "Int_Literal";
        case CODE_KIND_LITERAL_FLOAT: return "Float_Literal";
        case CODE_KIND_LITERAL_STRING: return "String_Literal";
        case CODE_KIND_IDENT: return "Identifier";
        case CODE_KIND_BINARY_OP: return "Binary_Operator";
        case CODE_KIND_PRE_OP: return "Prefix_Operator";
        case CODE_KIND_INVOKE: return "Invoke";
        case CODE_KIND_INITIALIZER: return "Initializer";
        case CODE_KIND_ARRAY_INITIALIZER: return "Array_Initializer";
        case CODE_KIND_KEY_VALUE: return "Key_Value";
        case CODE_KIND_BLOCK: return "Block";
        case CODE_KIND_CONST: return "Const";
        case CODE_KIND_VARIABLE_DECLARATION: return "Variable_Declaration";
        case CODE_KIND_FUNCTION_DECLARATION: return "Function_Declaration";
        case CODE_KIND_PARAMETER: return "Parameter";
        case CODE_KIND_RETURN: return "Return";
        case CODE_KIND_BREAK: return "Break";
        case CODE_KIND_CONTINUE: return "Continue";
        case CODE_KIND_IF: return "If";
        case CODE_KIND_FOR: return "For";
        case CODE_KIND_WHILE: return "While";
        case CODE_KIND_INDEX: return "Index";
        case CODE_KIND_SLICE: return "Slice";
        case CODE_KIND_CAST: return "Cast";
        case CODE_KIND_STRUCT: return "Struct";
        case CODE_KIND_GENERIC: return "Generic";
        case CODE_KIND_TYPE_POINTER: return "Pointer";
        default: oc_unreachable("");
    }
}

void print_node_value(Code* node, Oc_Writer* w) {
    switch (node->kind) {
        case CODE_KIND_LITERAL_INT:    wprint(w, "{}", CODE_AS(node, Code_Literal)->u64); break;
        case CODE_KIND_LITERAL_FLOAT:  wprint(w, "{}", CODE_AS(node, Code_Literal)->f64); break;
        case CODE_KIND_LITERAL_STRING: wprint(w, "{}", CODE_AS(node, Code_Literal)->str); break;
        case CODE_KIND_IDENT:          wprint(w, "{}", CODE_AS(node, Code_Ident)->str); break;
        case CODE_KIND_BINARY_OP: lexer_print_token_info_raw_to_writer(&CODE_AS(node, Code_Operation)->op, w); break;
        case CODE_KIND_PRE_OP:    lexer_print_token_info_raw_to_writer(&CODE_AS(node, Code_Operation)->op, w); break;
        case CODE_KIND_INVOKE: break;
        case CODE_KIND_INITIALIZER: break;
        case CODE_KIND_ARRAY_INITIALIZER: break;
        case CODE_KIND_KEY_VALUE: break;
        case CODE_KIND_BLOCK: break;
        case CODE_KIND_CONST: break;
        case CODE_KIND_VARIABLE_DECLARATION: print_storage_class(CODE_AS(node, Code_Variable_Declaration)->storage_class, w); break;
        case CODE_KIND_FUNCTION_DECLARATION: print_storage_class(CODE_AS(node, Code_Function_Declaration)->storage_class, w); break;
        case CODE_KIND_PARAMETER: break;
        case CODE_KIND_RETURN: break;
        case CODE_KIND_BREAK: break;
        case CODE_KIND_CONTINUE: break;
        case CODE_KIND_IF: break;
        case CODE_KIND_FOR: break;
        case CODE_KIND_WHILE: break;
        case CODE_KIND_INDEX: break;
        case CODE_KIND_SLICE: break;
        case CODE_KIND_GENERIC: break;
        case CODE_KIND_CAST:
            if (node->type)
                /* ll_print_type_raw(node->type, &stdout_writer); */
            break;
        case CODE_KIND_STRUCT:
            print_node_value(&CODE_AS(node, Code_Struct)->ident->base, w);
            break;
        case CODE_KIND_TYPE_POINTER: break;
        default: oc_unreachable("");
    }

    if (node->has_const) {
        wprint(w, " {}", node->const_value.as_u64);
    }
}

void print_node(Code* node, uint32_t indent, Oc_Writer* w) {
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
        /* ll_print_type_raw(node->type, w); */
    }
    wprint(w, "\n");
    switch (node->kind) {
        case CODE_KIND_BINARY_OP: 
            print_node(CODE_AS(node, Code_Operation)->left, indent + 1, w);
            print_node(CODE_AS(node, Code_Operation)->right, indent + 1, w);
            break;
        case CODE_KIND_PRE_OP: 
            print_node(CODE_AS(node, Code_Operation)->right, indent + 1, w);
            break;

        case CODE_KIND_INVOKE: 
            print_node(CODE_AS(node, Code_Invoke)->expr, indent + 1, w);
            // for (i = 0; i < CODE_AS(node, Code_Invoke)->arguments.count; ++i) {
            //     print_node((Code*)CODE_AS(node, Code_Invoke)->arguments.items[i], indent + 1, w);
            // }
            for (i = 0; i < CODE_AS(node, Code_Invoke)->ordered_arguments.count; ++i) {
                print_node((Code*)CODE_AS(node, Code_Invoke)->ordered_arguments.items[i], indent + 1, w);
            }
            break;

        case CODE_KIND_INITIALIZER: 
        case CODE_KIND_ARRAY_INITIALIZER: 
            for (i = 0; i < CODE_AS(node, Code_Initializer)->count; ++i) {
                print_node((Code*)CODE_AS(node, Code_Initializer)->items[i], indent + 1, w);
            }
            break;

        case CODE_KIND_KEY_VALUE: 
            print_node((Code*)CODE_AS(node, Code_Key_Value)->key, indent + 1, w);
            print_node((Code*)CODE_AS(node, Code_Key_Value)->value, indent + 1, w);
            break;

        case CODE_KIND_CONST:
            print_node(CODE_AS(node, Code_Marker)->expr, indent + 1, w);
            break;
        case CODE_KIND_VARIABLE_DECLARATION:
            print_node((Code*)CODE_AS(node, Code_Variable_Declaration)->base.type, indent + 1, w);
            print_node((Code*)CODE_AS(node, Code_Variable_Declaration)->base.ident, indent + 1, w);
            if (CODE_AS(node, Code_Variable_Declaration)->initializer) print_node(CODE_AS(node, Code_Variable_Declaration)->initializer, indent + 1, w);
            break;

        case CODE_KIND_FUNCTION_DECLARATION:
            print_node(CODE_AS(node, Code_Function_Declaration)->base.type, indent + 1, w);
            print_node((Code*)CODE_AS(node, Code_Function_Declaration)->base.ident, indent + 1, w);
            for (i = 0; i < CODE_AS(node, Code_Function_Declaration)->parameters.count; ++i) {
                print_node((Code*)&CODE_AS(node, Code_Function_Declaration)->parameters.items[i], indent + 1, w);
            }
            if (CODE_AS(node, Code_Function_Declaration)->body) print_node((Code*)CODE_AS(node, Code_Function_Declaration)->body, indent + 1, w);
            break;

        case CODE_KIND_PARAMETER:
            if (CODE_AS(node, Code_Variable_Declaration)->base.type)
                print_node(CODE_AS(node, Code_Variable_Declaration)->base.type, indent + 1, w);
            if (CODE_AS(node, Code_Variable_Declaration)->base.ident)
                print_node((Code*)CODE_AS(node, Code_Variable_Declaration)->base.ident, indent + 1, w);
            break;
        case CODE_KIND_RETURN:
            if (CODE_AS(node, Code_Control_Flow)->expr)
                print_node(CODE_AS(node, Code_Control_Flow)->expr, indent + 1, w);
            break;
        case CODE_KIND_BREAK:
            if (CODE_AS(node, Code_Control_Flow)->expr)
                print_node(CODE_AS(node, Code_Control_Flow)->expr, indent + 1, w);
            break;
        case CODE_KIND_CONTINUE:
            if (CODE_AS(node, Code_Control_Flow)->expr)
                print_node(CODE_AS(node, Code_Control_Flow)->expr, indent + 1, w);
            break;
        case CODE_KIND_IF:
            if (CODE_AS(node, Code_If)->cond)
                print_node(CODE_AS(node, Code_If)->cond, indent + 1, w);
            if (CODE_AS(node, Code_If)->body)
                print_node(CODE_AS(node, Code_If)->body, indent + 1, w);
            if (CODE_AS(node, Code_If)->else_clause)
                print_node(CODE_AS(node, Code_If)->else_clause, indent + 1, w);
            break;
        case CODE_KIND_WHILE:
        case CODE_KIND_FOR:
            if (CODE_AS(node, Code_Loop)->init)
                print_node(CODE_AS(node, Code_Loop)->init, indent + 1, w);
            if (CODE_AS(node, Code_Loop)->cond)
                print_node(CODE_AS(node, Code_Loop)->cond, indent + 1, w);
            if (CODE_AS(node, Code_Loop)->update)
                print_node(CODE_AS(node, Code_Loop)->update, indent + 1, w);
            if (CODE_AS(node, Code_Loop)->body)
                print_node(CODE_AS(node, Code_Loop)->body, indent + 1, w);
            break;

        case CODE_KIND_INDEX:
            print_node(CODE_AS(node, Code_Operation)->left, indent + 1, w);
            if (CODE_AS(node, Code_Operation)->right)
                print_node(CODE_AS(node, Code_Operation)->right, indent + 1, w);
            break;

        case CODE_KIND_CAST:
            if (CODE_AS(node, Code_Cast)->cast_type) print_node(CODE_AS(node, Code_Cast)->cast_type, indent + 1, w);
            print_node(CODE_AS(node, Code_Cast)->expr, indent + 1, w);
            break;

        case CODE_KIND_STRUCT:
            print_node((Code*)CODE_AS(node, Code_Struct)->block, indent + 1, w);
            break;

        case CODE_KIND_GENERIC:
            print_node((Code*)CODE_AS(node, Code_Generic)->ident, indent + 1, w);
            break;

        case CODE_KIND_TYPE_POINTER:
            print_node(CODE_AS(node, Code_Type_Pointer)->element, indent + 1, w);
            break;

        case CODE_KIND_BLOCK:
            for (i = 0; i < CODE_AS(node, Code_Scope)->declarations.capacity; ++i) {
                if (CODE_AS(node, Code_Scope)->declarations.entries[i].filled)
                    print_node((Code*)CODE_AS(node, Code_Scope)->declarations.entries[i]._value, indent + 1, w);
            }
            for (i = 0; i < CODE_AS(node, Code_Scope)->statements.count; ++i) {
                print_node(CODE_AS(node, Code_Scope)->statements.items[i], indent + 1, w);
            }

        default: break;
    }
}

Code* ast_clone_node_deep(Compiler_Context* cc, Code* node, LL_Code_Clone_Params params) {
    uint32_t i;
    Parse_Result result;
    if (!node) return NULL;

    switch (node->kind) {
    case CODE_KIND_BINARY_OP:
        result = CREATE_NODE(node->kind, ((Code_Operation) {
            .base.token_info = node->token_info,
            .left = ast_clone_node_deep(cc, CODE_AS(node, Code_Operation)->left, params),
            .op = CODE_AS(node, Code_Operation)->op,
            .right = ast_clone_node_deep(cc, CODE_AS(node, Code_Operation)->right, params),
        }));
        break;
    case CODE_KIND_PRE_OP: 
        result = CREATE_NODE(node->kind, ((Code_Operation) {
            .base.token_info = node->token_info,
            .op = CODE_AS(node, Code_Operation)->op,
            .right = ast_clone_node_deep(cc, CODE_AS(node, Code_Operation)->right, params),
        }));
        break;

    case CODE_KIND_INVOKE: {
        Code_List newargs = { 0 };
        for (i = 0; i < CODE_AS(node, Code_Invoke)->arguments.count; ++i) {
            oc_array_append(&cc->arena, &newargs, ast_clone_node_deep(cc, CODE_AS(node, Code_Invoke)->arguments.items[i], params));
        }
        result = CREATE_NODE(node->kind, ((Code_Invoke) {
            .base.token_info = node->token_info,
            .expr = CODE_AS(node, Code_Invoke)->expr,
            .arguments = newargs,
        }));
        // for (i = 0; i < CODE_AS(node, Code_Invoke)->ordered_arguments.count; ++i) {
        //     print_node((Code*)CODE_AS(node, Code_Invoke)->ordered_arguments.items[i], indent + 1, w);
        // }
    } break;

    case CODE_KIND_INITIALIZER: 
    case CODE_KIND_ARRAY_INITIALIZER: {
        Code_List newargs = { 0 };
        for (i = 0; i < CODE_AS(node, Code_Initializer)->count; ++i) {
            oc_array_append(&cc->arena, &newargs, ast_clone_node_deep(cc, CODE_AS(node, Code_Initializer)->items[i], params));
        }
        result = CREATE_NODE(node->kind, ((Code_Initializer) {
            .base.token_info = node->token_info,
            .items = newargs.items,
            .count = newargs.count,
            .capacity = newargs.capacity,
        }));
        // for (i = 0; i < CODE_AS(node, Code_Initializer)->count; ++i) {
        //     print_node((Code*)CODE_AS(node, Code_Initializer)->items[i], indent + 1, w);
        // }
    } break;

    case CODE_KIND_KEY_VALUE: 
        result = CREATE_NODE(node->kind, ((Code_Key_Value) {
            .base.token_info = node->token_info,
            .key = ast_clone_node_deep(cc, CODE_AS(node, Code_Key_Value)->key, params),
            .value = ast_clone_node_deep(cc, CODE_AS(node, Code_Key_Value)->value, params),
        }));
        break;

    case CODE_KIND_CONST:
        result = CREATE_NODE(node->kind, ((Code_Marker) {
            .base.token_info = node->token_info,
            .expr = ast_clone_node_deep(cc, CODE_AS(node, Code_Marker)->expr, params),
        }));
        break;
    case CODE_KIND_VARIABLE_DECLARATION:
        result = CREATE_NODE(node->kind, ((Code_Variable_Declaration) {
            .base.base.token_info = node->token_info,
            .base.type = ast_clone_node_deep(cc, CODE_AS(node, Code_Variable_Declaration)->base.type, params),
            .base.ident = (Code_Ident*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Variable_Declaration)->base.ident, params),
            .initializer = ast_clone_node_deep(cc, CODE_AS(node, Code_Variable_Declaration)->initializer, params),
            .storage_class = CODE_AS(node, Code_Variable_Declaration)->storage_class,
        }));
        break;

    case CODE_KIND_FUNCTION_DECLARATION: {
        Code_Parameter_List newargs = { 0 };
        for (i = 0; i < CODE_AS(node, Code_Function_Declaration)->parameters.count; ++i) {
            oc_array_append(&cc->arena, &newargs, ((Code_Parameter){
                .base.kind = CODE_KIND_PARAMETER,
                .base.token_info = CODE_AS(node, Code_Function_Declaration)->parameters.items[i].base.token_info,
                .type = ast_clone_node_deep(cc, CODE_AS(node, Code_Function_Declaration)->parameters.items[i].type, params),
                .ident = (Code_Ident*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Function_Declaration)->parameters.items[i].ident, params),
                .initializer = ast_clone_node_deep(cc, CODE_AS(node, Code_Function_Declaration)->parameters.items[i].initializer, params),
                .flags = CODE_AS(node, Code_Function_Declaration)->parameters.items[i].flags,
            }));
        }

        result = CREATE_NODE(node->kind, ((Code_Function_Declaration) {
            .base.base.token_info = node->token_info,
            .base.type = ast_clone_node_deep(cc, CODE_AS(node, Code_Function_Declaration)->base.type, params),
            .base.ident = (Code_Ident*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Function_Declaration)->base.ident, params),
            .body = (Code_Scope*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Function_Declaration)->body, params),
            .storage_class = CODE_AS(node, Code_Function_Declaration)->storage_class,
            .parameters = newargs,
        }));
    } break;

    case CODE_KIND_PARAMETER:
        result = CREATE_NODE(node->kind, ((Code_Parameter){
            .base.token_info = node->token_info,
            .type = ast_clone_node_deep(cc, CODE_AS(node, Code_Parameter)->type, params),
            .ident = (Code_Ident*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Parameter)->ident, params),
            .initializer = ast_clone_node_deep(cc, CODE_AS(node, Code_Parameter)->initializer, params),
            .flags = CODE_AS(node, Code_Parameter)->flags,
        }));
        break;
    case CODE_KIND_CONTINUE:
    case CODE_KIND_BREAK:
    case CODE_KIND_RETURN:
        result = CREATE_NODE(node->kind, ((Code_Control_Flow){
            .base.token_info = node->token_info,
            .expr = ast_clone_node_deep(cc, CODE_AS(node, Code_Control_Flow)->expr, params),
        }));
        break;
    case CODE_KIND_IF:
        result = CREATE_NODE(node->kind, ((Code_If){
            .base.token_info = node->token_info,
            .cond = ast_clone_node_deep(cc, CODE_AS(node, Code_If)->cond, params),
            .body = ast_clone_node_deep(cc, CODE_AS(node, Code_If)->body, params),
            .else_clause = ast_clone_node_deep(cc, CODE_AS(node, Code_If)->else_clause, params),
        }));
        break;
    case CODE_KIND_WHILE:
    case CODE_KIND_FOR:
        result = CREATE_NODE(node->kind, ((Code_Loop){
            .base.token_info = node->token_info,
            .init = ast_clone_node_deep(cc, CODE_AS(node, Code_Loop)->init, params),
            .cond = ast_clone_node_deep(cc, CODE_AS(node, Code_Loop)->cond, params),
            .update = ast_clone_node_deep(cc, CODE_AS(node, Code_Loop)->update, params),
            .body = ast_clone_node_deep(cc, CODE_AS(node, Code_Loop)->body, params),
        }));
        break;

    case CODE_KIND_INDEX:
    case CODE_KIND_SLICE:
        result = CREATE_NODE(node->kind, ((Code_Slice){
            .base.token_info = node->token_info,
            .ptr = ast_clone_node_deep(cc, CODE_AS(node, Code_Slice)->ptr, params),
            .start = ast_clone_node_deep(cc, CODE_AS(node, Code_Slice)->start, params),
            .stop = ast_clone_node_deep(cc, CODE_AS(node, Code_Slice)->stop, params),
        }));
        break;

    case CODE_KIND_CAST:
        result = CREATE_NODE(node->kind, ((Code_Cast){
            .base.token_info = node->token_info,
            .cast_type = ast_clone_node_deep(cc, CODE_AS(node, Code_Cast)->cast_type, params),
            .expr = ast_clone_node_deep(cc, CODE_AS(node, Code_Cast)->expr, params),
        }));
        break;

    case CODE_KIND_STRUCT: {
        result = CREATE_NODE(node->kind, ((Code_Struct){
            .base.token_info = node->token_info,
            .ident = (Code_Ident*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Struct)->ident, params),
            .block = (Code_Scope*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Struct)->block, params),
        }));
    } break;

    case CODE_KIND_GENERIC:
        result = CREATE_NODE(node->kind, ((Code_Generic){
            .base.token_info = node->token_info,
            .ident = (Code_Ident*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Generic)->ident, params),
        }));
        break;

    case CODE_KIND_TYPE_POINTER:
        result = CREATE_NODE(node->kind, ((Code_Type_Pointer){
            .base.token_info = node->token_info,
            .element = ast_clone_node_deep(cc, CODE_AS(node, Code_Type_Pointer)->element, params),
        }));
        break;

    case CODE_KIND_BLOCK: {
        Code_Scope_Flags flags = CODE_AS(node, Code_Scope)->flags;
        if (params.expand_first_block) {
            params.expand_first_block = false;
            flags |= CODE_BLOCK_FLAG_MACRO_EXPANSION;
        }

        typeof(CODE_AS(node, Code_Scope)->statements) newargs = { 0 };
        for (i = 0; i < CODE_AS(node, Code_Scope)->statements.count; ++i) {
            oc_array_append(&cc->arena, &newargs, ast_clone_node_deep(cc, CODE_AS(node, Code_Scope)->statements.items[i], params));
        }

        typeof(CODE_AS(node, Code_Scope)->declarations) newdecls = { 0 };
        hash_map_reserve(&cc->arena, &newdecls, CODE_AS(node, Code_Scope)->declarations.capacity);
        for (i = 0; i < CODE_AS(node, Code_Scope)->declarations.capacity; ++i) {
            newdecls.entries[i].filled = CODE_AS(node, Code_Scope)->declarations.entries[i].filled;
            if (newdecls.entries[i].filled) {
                newdecls.entries[i]._key = CODE_AS(node, Code_Scope)->declarations.entries[i]._key;
                newdecls.entries[i]._value = (Code_Declaration*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Scope)->declarations.entries[i]._value, params);
            }
        }

        result = CREATE_NODE(CODE_KIND_BLOCK, ((Code_Scope){
            .base.token_info = node->token_info,
            .flags = flags,
            .statements = newargs,
            .declarations = newdecls,
        }));
    } break;

    case CODE_KIND_LITERAL_INT: {
        result = CREATE_NODE(node->kind, ((Code_Literal){
            .base.token_info = node->token_info,
            .u64 = CODE_AS(node, Code_Literal)->u64,
        }));
    } break;
    case CODE_KIND_LITERAL_FLOAT: {
        result = CREATE_NODE(node->kind, ((Code_Literal){
            .base.token_info = node->token_info,
            .f64 = CODE_AS(node, Code_Literal)->f64,
        }));
    } break;
    case CODE_KIND_LITERAL_STRING: {
        result = CREATE_NODE(node->kind, ((Code_Literal){
            .base.token_info = node->token_info,
            .str = CODE_AS(node, Code_Literal)->str,
        }));
    } break;
    case CODE_KIND_IDENT: {
        result = CREATE_NODE(node->kind, ((Code_Ident){
            .base.token_info = node->token_info,
            .str = CODE_AS(node, Code_Ident)->str,
            .flags = CODE_AS(node, Code_Ident)->flags | (params.convert_all_idents_to_expansion ? CODE_IDENT_FLAG_EXPAND : 0),
        }));
    } break;
    // default: oc_unreachable(""); break;
    }
    return result.code;
}
