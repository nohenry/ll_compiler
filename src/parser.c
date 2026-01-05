
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
#define CONSUME_SEMI() do { LL_Token tok; PEEK(&tok); if (tok.kind == ';') { CONSUME(); } } while (0)

#define CREATE_NODE(_kind, value, ...) ({ __typeof__(value) v = (value); _CREATE_ASSIGN_KIND((_kind), v); Code* node = create_node(cc, (Code*)&v, sizeof(v)); node; })

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

static inline void insert_into_block(Compiler_Context* cc, Code_Scope* block, Code* stmt) {
	if (!stmt) return;

    switch (stmt->kind) {
    case CODE_KIND_VARIABLE_DECLARATION:
    case CODE_KIND_FUNCTION_DECLARATION:
        break;
    default:
        print("append\n");
        oc_array_append(&cc->arena, &block->statements, stmt);
        break;
    }
}

Code_Scope* parser_parse_file(Compiler_Context* cc, LL_Parser* parser) {
	Code* block_result = CREATE_NODE(CODE_KIND_BLOCK, ((Code_Scope){ .base.kind = CODE_KIND_BLOCK }));
    Code_Scope* block = (Code_Scope*)block_result;
    LL_Token token;

    parser->current_scope = block;
    while (parser->lexer.pos < parser->lexer.source.len) {
        Code* stmt = parser_parse_statement(cc, parser);
        insert_into_block(cc, block, stmt);
        PEEK(&token);
    }

    return block;
}

Code* parser_parse_statement(Compiler_Context* cc, LL_Parser* parser) {
    Code* result;
    LL_Token token;
    LL_Storage_Class storage_class = 0;
    PEEK(&token);

START_SWITCH:
    if (!PEEK(&token)) {
        return (Code*) { 0 };
    }
    switch (token.kind) {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
        case '{':
            result = (Code*)parser_parse_block(cc, parser, NULL);
            break;
#pragma GCC diagnostic pop
        case LL_TOKEN_KIND_IDENT:
            while (PEEK(&token) && token.kind == LL_TOKEN_KIND_IDENT) {
                if (token.str.ptr == LL_KEYWORD_EXTERN.ptr) {
                    storage_class |= LL_STORAGE_CLASS_EXTERN;
                } else if (token.str.ptr == LL_KEYWORD_FUNCTION.ptr || token.str.ptr == LL_KEYWORD_LET.ptr || token.str.ptr == LL_KEYWORD_CONST.ptr) {
                    result = parser_parse_declaration(cc, parser, 0);
                    return result;
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
            if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_CLASS.ptr) {
                result = parser_parse_class(cc, parser);
                return result;
            }

            result = parser_parse_expression(cc, parser, NULL, 0, true);
            break;
    }
    return result;
}

Code_Scope* parser_parse_block(Compiler_Context* cc, LL_Parser* parser, Code_Declaration* decl) {
    LL_Token token;
	Code* block_result = CREATE_NODE(CODE_KIND_BLOCK, ((Code_Scope){ .base.kind = CODE_KIND_BLOCK }));
    Code_Scope* block = (Code_Scope*)block_result;
    block->decl = decl;

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
        Code* stmt = parser_parse_statement(cc, parser);
        insert_into_block(cc, block, stmt);
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
    Code *type, *init;
    LL_Parameter_Flags flags = 0;

    if (token.kind == LL_TOKEN_KIND_SPREAD) {
        CONSUME();
        type = NULL;
        flags |= LL_PARAMETER_FLAG_VARIADIC;	
    }

    PEEK(&token);
    Code_Ident* ident = NULL;
    if (token.kind == LL_TOKEN_KIND_IDENT) {
        CONSUME();
        ident = create_ident(cc, token.str);
        ident->base.token_info = TOKEN_INFO(token);
    }

    PEEK(&token);
    if (token.kind == '?') {
        CONSUME();
        flags |= LL_PARAMETER_FLAG_OPTIONAL;
    }

    PEEK(&token);
    if (token.kind == ':') {
        CONSUME();
        type = parser_parse_type(cc, parser);
    } else type = NULL;

    PEEK(&token);
    LL_Token_Info eql_info = { 0 };
    if (token.kind == '=') {
        CONSUME();
        eql_info = TOKEN_INFO(token);
        init = parser_parse_expression(cc, parser, NULL, 0, false);
    } else {
        init = NULL;
    }

    return (Code_Parameter) {
        .base.kind = CODE_KIND_PARAMETER,
        .base.token_info = eql_info,
        .type = type,
        .ident = ident,
        .initializer = init,
        .flags = flags,
    };
}

Code* parser_parse_class(Compiler_Context* cc, LL_Parser* parser) {
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

    Code* result = CREATE_NODE(CODE_KIND_CLASS_DECLARATION, ((Code_Class_Declaration){ .base.ident = ident }));
    Code_Scope* block = parser_parse_block(cc, parser, CODE_AS(result, Code_Declaration));
    CODE_AS(result, Code_Class_Declaration)->block = block;

    result->token_info = struct_kw;
    return result;
}

Code* parser_parse_declaration(Compiler_Context* cc, LL_Parser* parser, LL_Storage_Class storage_class) {
    LL_Token token;
    LL_Token kind_token;
    if (!EXPECT(LL_TOKEN_KIND_IDENT, &kind_token)) return (Code*) { 0 };
    if (!EXPECT(LL_TOKEN_KIND_IDENT, &token)) return (Code*) { 0 };
    Code_Ident* ident = create_ident(cc, token.str);
    ident->base.token_info = TOKEN_INFO(token);

    Code* body_or_init;
	Code* type;
	Code* result;

    if (kind_token.str.ptr == LL_KEYWORD_FUNCTION.ptr) {
        LL_Token_Info p_open, p_close;
        Code_Parameter_List parameters = { 0 };

        EXPECT('(', &token);
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
        if (token.kind == ':') {
            CONSUME();
            type = parser_parse_type(cc, parser);
        } else type = NULL;

        PEEK(&token);
        if (token.kind == '{') {
            body_or_init = (Code*)parser_parse_block(cc, parser, NULL);
        } else {
            CONSUME_SEMI();
            body_or_init = NULL;
        }

        result = CREATE_NODE(CODE_KIND_FUNCTION_DECLARATION, ((Code_Function_Declaration){
            .base.type = type,
            .base.ident = ident,
            .base.within_scope = parser->current_scope,
            .parameters = parameters,
            .body = (Code_Scope*)body_or_init,
            .storage_class = storage_class,
            .p_open = p_open, .p_close = p_close,
        }));
        if (body_or_init)
            CODE_AS(body_or_init, Code_Scope)->decl = (Code_Declaration*)result;
    } else {
        PEEK(&token);
        if (token.kind == ':') {
            CONSUME();
            type = parser_parse_type(cc, parser);
        } else type = NULL;

        LL_Token_Info eql;
        PEEK(&token);
        if (token.kind == '=') {
            eql = TOKEN_INFO(token);
            CONSUME();
            body_or_init = parser_parse_expression(cc, parser, NULL, 0, false);
        } else body_or_init = NULL;

        CONSUME_SEMI();

		result = CREATE_NODE(CODE_KIND_VARIABLE_DECLARATION, ((Code_Variable_Declaration){
            .base.base.token_info = TOKEN_INFO(kind_token),
            .base.type = type,
            .base.ident = ident,
            .base.within_scope = parser->current_scope,
            .initializer = body_or_init,
            .storage_class = storage_class,
        }));

		if (body_or_init) {
            Code* assign = CREATE_NODE(CODE_KIND_BINARY_OP, ((Code_Operation){
                .left = (Code*)CODE_AS(result, Code_Variable_Declaration)->base.ident,
                .right = CODE_AS(result, Code_Variable_Declaration)->initializer,
                .op = eql, // eql
            }));
            oc_array_append(&cc->arena, &parser->current_scope->statements, assign);
            
            CODE_AS(body_or_init, Code_Scope)->decl = (Code_Declaration*)result;

            oc_array_append(&cc->arena, &parser->ops[LL_OPERATION_ASSIGN], body_or_init);
		}
    }

	oc_assert(hash_map_get(&cc->arena, &parser->current_scope->declarations, ident->str) == NULL);
	hash_map_put(&cc->arena, &parser->current_scope->declarations, ident->str, (Code_Declaration*)result);

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
        case LL_TOKEN_KIND_ASSIGN_AND:
        case LL_TOKEN_KIND_ASSIGN_OR:
        case LL_TOKEN_KIND_ASSIGN_NULLOR:
        case LL_TOKEN_KIND_ASSIGN_BIT_AND:
        case LL_TOKEN_KIND_ASSIGN_BIT_OR:
            return 20;
        case LL_TOKEN_KIND_NULLOR: return 40;
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
            if (token.str.ptr == LL_KEYWORD_AS.ptr) return 160;
            return 0;
        default: return 0;
    }
}

Code* parser_parse_initializer(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
    Code *expr1, *expr2;
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
            expr1 = CREATE_NODE(CODE_KIND_KEY_VALUE, ((Code_Key_Value) { .key = expr1, .value = expr2 }));
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
    return CREATE_NODE(CODE_KIND_OBJECT_INITIALIZER, result);
}

Code* parser_parse_array_initializer(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token token;
    Code *expr1, *expr2;
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
            expr1 = CREATE_NODE(CODE_KIND_KEY_VALUE, ((Code_Key_Value) { .key = expr1, .value = expr2 }));
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
    return CREATE_NODE(CODE_KIND_ARRAY_INITIALIZER, result);
}

Code* parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, Code* left, int last_precedence, bool from_statement) {
    LL_Token token;
	Code *right;
    if (!left) {
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
            } else if (token.str.ptr == LL_KEYWORD_CLASS.ptr) {
                left = parser_parse_class(cc, parser);
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
            while (PEEK(&token)) {
                int next_bin_precedence = get_binary_precedence(token, false);
                int next_post_precedence = get_postfix_precedence(token);
                if (next_bin_precedence != 0 && next_bin_precedence > bin_precedence) {
                    right = parser_parse_expression(cc, parser, right, bin_precedence + 1, false);
                } else if (next_post_precedence != 0 && next_post_precedence > bin_precedence) {
                    Code* new_right = parser_parse_expression(cc, parser, right, next_post_precedence, false);
                    if (new_right == right) break;
                    right = new_right;
                } else break;
            }

            left = CREATE_NODE(CODE_KIND_BINARY_OP, ((Code_Operation){ .left = left, .right= right, .op = TOKEN_INFO(op_tok) }));
            left->token_info = TOKEN_INFO(op_tok);

			LL_Operation_Kind op_kind;
			switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
			case '+': op_kind = LL_OPERATION_ADD;
			case '=': op_kind = LL_OPERATION_ASSIGN;
#pragma GCC diagnostic pop
			default: oc_todo("uniplented");
			}

            oc_array_append(&cc->arena, &parser->ops[op_kind], left);

            // oc_array_aligned_append(&cc->arena, &parser->ops_lhs[LL_OPERATION_ASSIGN], TC_ALIGNMENT, ((LL_Usage) { left.kind, left.value}));
            // oc_array_aligned_append(&cc->arena, &parser->ops_rhs[LL_OPERATION_ASSIGN], TC_ALIGNMENT, ((LL_Usage) { right.kind, right.value}));
            // parser_append_typecheck_value(cc,
            //     &parser->ops_lhs[op_kind],
            //     parser->linear_grid[left.kind].types.items[left.value],
            //     left.kind, left.value
            // );
            // parser_append_typecheck_value(cc,
            //     &parser->ops_rhs[op_kind],
            //     parser->linear_grid[right.kind].types.items[right.value],
            //     right.kind, right.value
            // );
			// LL_Typecheck_Value* tval = parser->ops_values[op_kind].items + parser->ops_values[op_kind].count - 2;
			// tval[0].type = parser->linear_grid[left.kind].items[left.value].type;
			// tval[1].type = parser->linear_grid[right.kind].items[right.value].type;

			// left.kind = op_kind;
			// left.value = parser_append_typecheck_value(cc, &parser->ops[op_kind], NULL, 0, 0);

            from_statement = false;
        } else if (post_precedence != 0 && post_precedence >= last_precedence) {
            switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
                case '(': {
                    LL_Token_Info ti = TOKEN_INFO(token);
                    CONSUME();

                    Code_List arguments = { 0 };
                    Code *expr1, *expr2;
                    PEEK(&token);
                    while (token.kind != ')') {
                        expr1 = parser_parse_expression(cc, parser, NULL, 30, false);
                        PEEK(&token);
                        if (token.kind == '=') {
                            CONSUME();
                            expr2 = parser_parse_expression(cc, parser, NULL, 0, false);
							// oc_todo("handle non Codes");
                            expr1 = CREATE_NODE(CODE_KIND_KEY_VALUE, ((Code_Key_Value) { .key = expr1, .value = expr2 }));
                        }
						// oc_todo("handle non Codes");
                        oc_array_append(&cc->arena, &arguments, expr1);

                        PEEK(&token);
                        if (token.kind != ')') {
                            EXPECT(',', &token);
                            PEEK(&token);
                            continue;
                        }
                    }

                    CONSUME();

					// oc_todo("handle non Codes");
                    left = CREATE_NODE(CODE_KIND_INVOKE, ((Code_Invoke){ .expr = left, .arguments = arguments, .p_close = TOKEN_INFO(token) }));
                    left->token_info = ti;

                    break;
                }
                case '[': {
                    Code *index = NULL;
                    LL_Token_Info ti = TOKEN_INFO(token);
                    CONSUME();
                    Code_Kind kind = CODE_KIND_INDEX;

                    PEEK(&token);
                    if (token.kind != ']') {
                        index = parser_parse_expression(cc, parser, NULL, 0, false);
                    }
                    EXPECT(']', &token);

                    left = CREATE_NODE(kind, ((Code_Index){ .ptr = left, .index = index }));
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

Code* parser_parse_primary(Compiler_Context* cc, LL_Parser* parser, bool from_statement) {
    LL_Token token;
    Code* result, *right, *body, *update;
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
    
    case LL_TOKEN_KIND_INT:
        CONSUME();
        result = CREATE_NODE(CODE_KIND_LITERAL_INT, ((Code_Literal){ .u64 = token.u64 }));
        result->token_info = TOKEN_INFO(token);

		// result.kind = RESULT_KIND_INT;
		// result.value = parser_append_typecheck_value(cc, &parser->ints, cc->typer->ty_int64, 0, 0);

        break;

    case LL_TOKEN_KIND_FLOAT:
        CONSUME();
        result = CREATE_NODE(CODE_KIND_LITERAL_FLOAT, ((Code_Literal){ .f64 = token.f64 }));
        result->token_info = TOKEN_INFO(token);

		// result.kind = RESULT_KIND_FLOAT;
		// result.value = parser_append_typecheck_value(cc, &parser->floats, cc->typer->ty_float64, 0, 0);

        break;

    case LL_TOKEN_KIND_STRING:
        result = CREATE_NODE(CODE_KIND_LITERAL_STRING, ((Code_Literal){ .str = token.str }));
        result->token_info = TOKEN_INFO(token);
        CONSUME();
        break;

    case LL_TOKEN_KIND_IDENT:
        if (token.str.ptr == LL_KEYWORD_IF.ptr) {
            LL_Token_Info if_kw = TOKEN_INFO(token);
            CONSUME();

            EXPECT('(', &token);
            // parse if statement
            result = parser_parse_expression(cc, parser, NULL, 0, false);
            EXPECT(')', &token);

            PEEK(&token);
            if (token.kind == '{' || (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_DO.ptr)) {
                body = (Code*)parser_parse_block(cc, parser, NULL);
            } else {
                body = parser_parse_expression(cc, parser, NULL, 0, false);
                PEEK(&token);
                if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_ELSE.ptr) {
                } else {
                    if (from_statement) CONSUME_SEMI();
                }
            }

            // parse else clause
            PEEK(&token);
            LL_Token_Info else_kw = TOKEN_INFO(token);
            if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_ELSE.ptr) {
                CONSUME();
                PEEK(&token);
                if (token.kind == '{') {
                    right = (Code*)parser_parse_block(cc, parser, NULL);
                } else {
                    right = parser_parse_expression(cc, parser, NULL, 0, false);
                    if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_IF.ptr) {}
                    else if (from_statement) CONSUME_SEMI();
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

            EXPECT('(', &token);
            right = parser_parse_expression(cc, parser, NULL, 0, false);
            EXPECT(')', &token);

            PEEK(&token);
            if (token.kind == '{') {
                body = (Code*)parser_parse_block(cc, parser, NULL);
            } else {
                body = parser_parse_expression(cc, parser, NULL, 0, false);
                if (from_statement) CONSUME_SEMI();
            }

            result = CREATE_NODE(CODE_KIND_WHILE, ((Code_Loop){ .cond = right, .body = body }));
            result->token_info = for_kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_FOR.ptr) {
            LL_Token_Info for_kw = TOKEN_INFO(token);
            CONSUME();

            EXPECT('(', &token);

            PEEK(&token);
            if (token.kind == ';') {
                result = NULL;
                CONSUME();
            } else {
                result = parser_parse_declaration(cc, parser, 0);
            }
            EXPECT(';', &token);

            PEEK(&token);
            if (token.kind == ';') {
                right = NULL;
            } else {
                right = parser_parse_expression(cc, parser, NULL, 0, false);
            }
            EXPECT(';', &token);

            PEEK(&token);
            if (token.kind == ';') {
                update = NULL;
            } else {
                update = parser_parse_expression(cc, parser, NULL, 0, false);
            }
            EXPECT(';', &token);

            PEEK(&token);
            if (token.kind == '{') {
                body = (Code*)parser_parse_block(cc, parser, NULL);
            } else {
                body = parser_parse_expression(cc, parser, NULL, 0, false);
                if (from_statement) CONSUME_SEMI();
            }
            EXPECT(')', &token);

            result = CREATE_NODE(CODE_KIND_FOR, ((Code_Loop){ .init = result, .cond = right, .update = update, .body = body }));
            result->token_info = for_kw;
            break;
        } else if (token.str.ptr == LL_KEYWORD_DO.ptr) {
            result = (Code*)parser_parse_block(cc, parser, NULL);
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
        CONSUME();
        result = (Code*)create_ident(cc, token.str);
        result->token_info = TOKEN_INFO(token);
        break;

    default:
        UNEXPECTED();
        result = NULL;
        break;
    }

    return result;
}

Code* parser_parse_type(Compiler_Context* cc, LL_Parser* parser) {
    return parser_parse_primary(cc, parser, false);
}

void print_storage_class(LL_Storage_Class storage_class, Oc_Writer* w) {
    if (storage_class & LL_STORAGE_CLASS_EXTERN) wprint(w, "extern ");
    if (storage_class & LL_STORAGE_CLASS_STATIC) wprint(w, "static ");
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
        case CODE_KIND_OBJECT_INITIALIZER: return "Initializer";
        case CODE_KIND_ARRAY_INITIALIZER: return "Array_Initializer";
        case CODE_KIND_KEY_VALUE: return "Key_Value";
        case CODE_KIND_BLOCK: return "Block";
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
        case CODE_KIND_CAST: return "Cast";
        case CODE_KIND_GENERIC: return "Generic";
        case CODE_KIND_TYPENAME: return "Typename";
        default: eprint("{}\n", node->kind); oc_unreachable("");
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
        case CODE_KIND_OBJECT_INITIALIZER: break;
        case CODE_KIND_ARRAY_INITIALIZER: break;
        case CODE_KIND_KEY_VALUE: break;
        case CODE_KIND_BLOCK: break;
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
        case CODE_KIND_GENERIC: break;
        case CODE_KIND_CAST:
            if (node->type)
                /* ll_print_type_raw(node->type, &stdout_writer); */
            break;
        case CODE_KIND_TYPENAME:
            break;
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

        case CODE_KIND_OBJECT_INITIALIZER: 
        case CODE_KIND_ARRAY_INITIALIZER: 
            for (i = 0; i < CODE_AS(node, Code_Initializer)->count; ++i) {
                print_node((Code*)CODE_AS(node, Code_Initializer)->items[i], indent + 1, w);
            }
            break;

        case CODE_KIND_KEY_VALUE: 
            print_node((Code*)CODE_AS(node, Code_Key_Value)->key, indent + 1, w);
            print_node((Code*)CODE_AS(node, Code_Key_Value)->value, indent + 1, w);
            break;

        case CODE_KIND_VARIABLE_DECLARATION:
            if (CODE_AS(node, Code_Variable_Declaration)->base.type) print_node((Code*)CODE_AS(node, Code_Variable_Declaration)->base.type, indent + 1, w);
            print_node((Code*)CODE_AS(node, Code_Variable_Declaration)->base.ident, indent + 1, w);
            if (CODE_AS(node, Code_Variable_Declaration)->initializer) print_node(CODE_AS(node, Code_Variable_Declaration)->initializer, indent + 1, w);
            break;

        case CODE_KIND_FUNCTION_DECLARATION:
            if (CODE_AS(node, Code_Function_Declaration)->base.type) print_node(CODE_AS(node, Code_Function_Declaration)->base.type, indent + 1, w);
            print_node((Code*)CODE_AS(node, Code_Function_Declaration)->base.ident, indent + 1, w);
            for (i = 0; i < CODE_AS(node, Code_Function_Declaration)->parameters.count; ++i) {
                print_node((Code*)&CODE_AS(node, Code_Function_Declaration)->parameters.items[i], indent + 1, w);
            }
            if (CODE_AS(node, Code_Function_Declaration)->body) print_node((Code*)CODE_AS(node, Code_Function_Declaration)->body, indent + 1, w);
            break;

        case CODE_KIND_PARAMETER:
            if (CODE_AS(node, Code_Parameter)->type)
                print_node(CODE_AS(node, Code_Parameter)->type, indent + 1, w);
            if (CODE_AS(node, Code_Parameter)->ident)
                print_node((Code*)CODE_AS(node, Code_Parameter)->ident, indent + 1, w);
            if (CODE_AS(node, Code_Parameter)->initializer)
                print_node((Code*)CODE_AS(node, Code_Parameter)->initializer, indent + 1, w);
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
            print_node(CODE_AS(node, Code_Index)->ptr, indent + 1, w);
            if (CODE_AS(node, Code_Index)->index)
                print_node(CODE_AS(node, Code_Index)->index, indent + 1, w);
            break;

        case CODE_KIND_CAST:
            if (CODE_AS(node, Code_Cast)->cast_type) print_node(CODE_AS(node, Code_Cast)->cast_type, indent + 1, w);
            print_node(CODE_AS(node, Code_Cast)->expr, indent + 1, w);
            break;

        case CODE_KIND_CLASS_DECLARATION:
            print_node((Code*)CODE_AS(node, Code_Class_Declaration)->block, indent + 1, w);
            break;

        case CODE_KIND_GENERIC:
            print_node((Code*)CODE_AS(node, Code_Generic)->ident, indent + 1, w);
            break;

        case CODE_KIND_BLOCK:
            for (i = 0; i < CODE_AS(node, Code_Scope)->declarations.capacity; ++i) {
                if (CODE_AS(node, Code_Scope)->declarations.entries[i].filled) {
                    print_node((Code*)CODE_AS(node, Code_Scope)->declarations.entries[i]._value, indent + 1, w);
                }
            }
            for (i = 0; i < CODE_AS(node, Code_Scope)->statements.count; ++i) {
                print_node(CODE_AS(node, Code_Scope)->statements.items[i], indent + 1, w);
            }
            break;

        case CODE_KIND_TYPENAME:
            print_node(CODE_AS(node, Code_Declaration)->declared_type, 0, w);
            break;
        
        case CODE_KIND_LITERAL_INT:
        case CODE_KIND_LITERAL_FLOAT:
        case CODE_KIND_LITERAL_STRING:
        case CODE_KIND_IDENT:
            break;
    }
}

Code* ast_clone_node_deep(Compiler_Context* cc, Code* node, LL_Code_Clone_Params params) {
    uint32_t i;
    Code* result;
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

    case CODE_KIND_OBJECT_INITIALIZER: 
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

    case CODE_KIND_VARIABLE_DECLARATION:
        result = CREATE_NODE(node->kind, ((Code_Variable_Declaration) {
            .base.base.token_info = node->token_info,
            .base.type = ast_clone_node_deep(cc, CODE_AS(node, Code_Variable_Declaration)->base.type, params),
            .base.ident = (Code_Ident*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Variable_Declaration)->base.ident, params),
            .base.within_scope = params.current_scope,
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
            .base.within_scope = params.current_scope,
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
        result = CREATE_NODE(node->kind, ((Code_Index){
            .base.token_info = node->token_info,
            .ptr = ast_clone_node_deep(cc, CODE_AS(node, Code_Index)->ptr, params),
            .index = ast_clone_node_deep(cc, CODE_AS(node, Code_Index)->index, params),
        }));
        break;

    case CODE_KIND_CAST:
        result = CREATE_NODE(node->kind, ((Code_Cast){
            .base.token_info = node->token_info,
            .cast_type = ast_clone_node_deep(cc, CODE_AS(node, Code_Cast)->cast_type, params),
            .expr = ast_clone_node_deep(cc, CODE_AS(node, Code_Cast)->expr, params),
        }));
        break;

    case CODE_KIND_CLASS_DECLARATION: {
        result = CREATE_NODE(node->kind, ((Code_Class_Declaration){
            .base.base.token_info = node->token_info,
            .base.type = ast_clone_node_deep(cc, CODE_AS(node, Code_Class_Declaration)->base.type, params),
            .base.ident = (Code_Ident*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Class_Declaration)->base.ident, params),
            .base.within_scope = params.current_scope,
            .block = (Code_Scope*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Class_Declaration)->block, params),
        }));
    } break;

    case CODE_KIND_GENERIC:
        result = CREATE_NODE(node->kind, ((Code_Generic){
            .base.token_info = node->token_info,
            .ident = (Code_Ident*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Generic)->ident, params),
        }));
        break;

    case CODE_KIND_BLOCK: {
        Code_Scope_Flags flags = CODE_AS(node, Code_Scope)->flags;
        if (params.expand_first_block) {
            params.expand_first_block = false;
            flags |= CODE_BLOCK_FLAG_MACRO_EXPANSION;
        }

        typeof(CODE_AS(node, Code_Scope)->statements) newargs = { 0 };
        typeof(CODE_AS(node, Code_Scope)->declarations) newdecls = { 0 };
        result = CREATE_NODE(CODE_KIND_BLOCK, ((Code_Scope){
            .base.token_info = node->token_info,
            .flags = flags,
        }));
        params.current_scope = (Code_Scope*)result;

        for (i = 0; i < CODE_AS(node, Code_Scope)->statements.count; ++i) {
            oc_array_append(&cc->arena, &newargs, ast_clone_node_deep(cc, CODE_AS(node, Code_Scope)->statements.items[i], params));
        }

        hash_map_reserve(&cc->arena, &newdecls, CODE_AS(node, Code_Scope)->declarations.capacity);
        for (i = 0; i < CODE_AS(node, Code_Scope)->declarations.capacity; ++i) {
            newdecls.entries[i].filled = CODE_AS(node, Code_Scope)->declarations.entries[i].filled;
            if (newdecls.entries[i].filled) {
                newdecls.entries[i]._key = CODE_AS(node, Code_Scope)->declarations.entries[i]._key;
                newdecls.entries[i]._value = (Code_Declaration*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Scope)->declarations.entries[i]._value, params);
            }
        }

        CODE_AS(result, Code_Scope)->statements = newargs;
        CODE_AS(result, Code_Scope)->declarations = newdecls;
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
    case CODE_KIND_TYPENAME: {
        result = CREATE_NODE(node->kind, ((Code_Declaration){
            .base.token_info = node->token_info,
            .type = ast_clone_node_deep(cc, CODE_AS(node, Code_Declaration)->type, params),
            .ident = (Code_Ident*)ast_clone_node_deep(cc, (Code*)CODE_AS(node, Code_Declaration)->ident, params),
            .within_scope = params.current_scope,
        }));
    } break;
    // default: oc_unreachable(""); break;
    }
    return result;
}
