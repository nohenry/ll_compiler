
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "common.h"
#include "parser.h"
#include "lexer.h"
#include "ast.h"

LL_Parser parser_create_from_file(Compiler_Context* cc, char* filename) {
    FILE* input = fopen(filename, "r");

    fseek(input, 0, SEEK_END);
    size_t input_size = ftell(input);
    fseek(input, 0, SEEK_SET);

    uint8_t* input_contents = malloc(input_size);
    if (!input_contents) ABORT("OOM");

    fread(input_contents, 1, input_size, input);
	fclose(input);

	LL_Parser result = {
		.lexer = {
			.pos = 0,
			.source.ptr = (char*)input_contents,
			.source.len = input_size,
		}
	};

	return result;
}

#define PEEK(out) (lexer_peek_token(cc, &(parser)->lexer, out))
#define CONSUME() lexer_consume(cc, &(parser)->lexer)
#define UNEXPECTED() unexpected_token(cc, parser, __FILE__, __LINE__)
#define EXPECT(kind, out) expect_token(cc, parser, kind, out, __FILE__, __LINE__)
#define CREATE_NODE(_kind, value) ({ __typeof__(value) v = (value); _CREATE_ASSIGN_KIND((_kind), value); create_node(cc, parser, (Ast_Base*)&v, sizeof(v)); })

#define _CREATE_ASSIGN_KIND(_kind, type) _Generic((type), \
        Ast_Base : 1,                       \
        default : v.base.kind = _kind                    \
    )

static Ast_Base* create_node(Compiler_Context* cc, LL_Parser* parser, Ast_Base* node, size_t size) {
    return arena_memdup(&cc->arena, node, size);
}

static void unexpected_token(Compiler_Context* cc, LL_Parser* parser, char* file, int line) {
    LL_Token tok;
    PEEK(&tok);
    fprintf(stderr, "%s line %d: \x1b[31;1merror\x1b[0m\x1b[1m: unexpected token '", file, line);
    lexer_print_token_raw_to_fd(&tok, stderr);
    fprintf(stderr, "' (%d)\x1b[0m\n", tok.kind);
}

static bool expect_token(Compiler_Context* cc, LL_Parser* parser, LL_Token_Kind kind, LL_Token* out, char* file, int line) {
    LL_Token tok;
    PEEK(&tok);
    if (tok.kind != kind) {

		fprintf(stderr, "%s line %d: \x1b[31;1merror\x1b[0m\x1b[1m: expected token '", file, line);
		lexer_print_token_kind(kind, stderr);
		fprintf(stderr, "' (%d), but found '", tok.kind);
		lexer_print_token_raw_to_fd(&tok, stderr);
		fprintf(stderr, "' \x1b[0m\n");

        return false;
    } else {
        if (out != NULL)
            memcpy(out, &tok, sizeof(tok));
        CONSUME();
        return true;
    }
}

Ast_Base* parser_parse_file(Compiler_Context* cc, LL_Parser* parser) {
	Ast_Base* b = parser_parse_statement(cc, parser);
	print_node(b, 0);
	return b;
}

Ast_Base* parser_parse_statement(Compiler_Context* cc, LL_Parser* parser) {
	Ast_Base* result;
	LL_Token token;
	LL_Storage_Class storage_class = 0;
	PEEK(&token);

START_SWITCH:
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
			result = parser_parse_expression(cc, parser, 0, true);
			if (result && (result->kind == AST_KIND_IF || result->kind == AST_KIND_FOR)) break;

			PEEK(&token);
			if (token.kind == LL_TOKEN_KIND_IDENT)
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
		CONSUME();
		block.flags |= AST_BLOCK_FLAG_EXPR;
		EXPECT('{', &token);
	}

	CONSUME();
	PEEK(&token);

	while (token.kind != '}') {
		arena_da_append(&cc->arena, &block, parser_parse_statement(cc, parser));
		PEEK(&token);
	}

	CONSUME();
	return arena_memdup(&cc->arena, &block, sizeof(block));
}

Ast_Parameter parser_parse_parameter(Compiler_Context* cc, LL_Parser* parser) {
	LL_Token token;
	PEEK(&token);
	Ast_Base* type;
	LL_Parameter_Flags flags = 0;

	if (token.kind == LL_TOKEN_KIND_RANGE) {
		CONSUME();
		type = NULL;
		flags |= LL_PARAMETER_FLAG_VARIADIC;	
	} else {
		type = parser_parse_expression(cc, parser, 0, true);
	}

	if (!EXPECT(LL_TOKEN_KIND_IDENT, &token)) return (Ast_Parameter) { 0 };
	Ast_Ident* ident = (Ast_Ident*)CREATE_NODE(AST_KIND_IDENT, ((Ast_Ident){ .str = token.str, .symbol_index = AST_IDENT_SYMBOL_INVALID }));

	return (Ast_Parameter) {
		.base.kind = AST_KIND_PARAMETER,
		.type = type,
		.ident = ident,
		.flags = flags,
	};
}

Ast_Base* parser_parse_declaration(Compiler_Context* cc, LL_Parser* parser, Ast_Base* type, LL_Storage_Class storage_class) {
	LL_Token token;
	if (!type) {
		type = parser_parse_expression(cc, parser, 0, true);
	}

	if (!EXPECT(LL_TOKEN_KIND_IDENT, &token)) return NULL;
	Ast_Ident* ident = (Ast_Ident*)CREATE_NODE(AST_KIND_IDENT, ((Ast_Ident){ .str = token.str, .symbol_index = AST_IDENT_SYMBOL_INVALID }));

	bool fn = false;
	Ast_Parameter_List parameters = { 0 };
	Ast_Base* body_or_init;

	PEEK(&token);
	switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
		case '(':
			fn = true;
			CONSUME();

			PEEK(&token);
			while (token.kind != ')') {
				Ast_Parameter parameter = parser_parse_parameter(cc, parser);
				arena_da_append(&cc->arena, &parameters, parameter);
				PEEK(&token);

				if (token.kind != ')') {
					EXPECT(',', &token);
					PEEK(&token);
					continue;
				}
			}

			CONSUME();

			PEEK(&token);
			if (token.kind == '{') {
				body_or_init = (Ast_Base*)parser_parse_block(cc, parser);
			} else {
				EXPECT(';', &token);
				body_or_init = NULL;
			}

			break;
		case '=':
			CONSUME();
			body_or_init = parser_parse_expression(cc, parser, 0, false);
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
		}));
	} else {
		return CREATE_NODE(AST_KIND_VARIABLE_DECLARATION, ((Ast_Variable_Declaration){
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
		default: return 0;
	}
}

Ast_Base* parser_parse_initializer(Compiler_Context* cc, LL_Parser* parser) {
	LL_Token token;
    Ast_Base *expr1, *expr2;
    Ast_Initializer result = { 0 };
    CONSUME(); // consume {

    PEEK(&token);
    while (token.kind != '}') {
        expr1 = parser_parse_primary(cc, parser);
        PEEK(&token);
        if (token.kind == '=') {
            CONSUME();
            expr2 = parser_parse_expression(cc, parser, 0, false);
            expr1 = CREATE_NODE(AST_KIND_KEY_VALUE, ((Ast_Key_Value) { .key = expr1, .value = expr2 }));
        }

        arena_da_append(&cc->arena, &result, expr1);
        
        PEEK(&token);
        if (token.kind != '}') {
            EXPECT(',', &token);
            PEEK(&token);
            continue;
        }
    }

    EXPECT('}', &token);
    return CREATE_NODE(AST_KIND_INITIALIZER, result);
}

Ast_Base* parser_parse_array_initializer(Compiler_Context* cc, LL_Parser* parser) {
	LL_Token token;
    Ast_Base *expr1, *expr2;
    Ast_Initializer result = { 0 };
    CONSUME(); // consume [

    PEEK(&token);
    while (token.kind != ']') {
        expr1 = parser_parse_primary(cc, parser);
        if (token.kind == '=') {
            CONSUME();
            expr2 = parser_parse_expression(cc, parser, 0, false);
            expr1 = CREATE_NODE(AST_KIND_KEY_VALUE, ((Ast_Key_Value) { .key = expr1, .value = expr2 }));
        }
        arena_da_append(&cc->arena, &result, expr1);
        
        PEEK(&token);
        if (token.kind != ']') {
            EXPECT(',', &token);
            PEEK(&token);
            continue;
        }
    }

    EXPECT(']', &token);
    return CREATE_NODE(AST_KIND_ARRAY_INITIALIZER, result);
}

Ast_Base* parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, int last_precedence, bool from_statement) {
	LL_Token token;
	Ast_Base *left, *right, *body, *update;
	PEEK(&token);
	switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
	case '-':
		CONSUME();
		left = CREATE_NODE(AST_KIND_PRE_OP, ((Ast_Operation){ .op = '-', .right = parser_parse_expression(cc, parser, 140, false) }));
		break;
	case '*':
		CONSUME();
		left = CREATE_NODE(AST_KIND_PRE_OP, ((Ast_Operation){ .op = '*', .right = parser_parse_expression(cc, parser, 140, false) }));
		break;
	case '&':
		CONSUME();
		left = CREATE_NODE(AST_KIND_PRE_OP, ((Ast_Operation){ .op = '&', .right = parser_parse_expression(cc, parser, 140, false) }));
		break;
#pragma GCC diagnostic pop
	case LL_TOKEN_KIND_IDENT:
		if (token.str.ptr == LL_KEYWORD_IF.ptr) {
			// parse if statement
			CONSUME();
			left = parser_parse_expression(cc, parser, 0, false);
			PEEK(&token);
			if (token.kind == '{' || (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_DO.ptr)) {
				body = (Ast_Base*)parser_parse_block(cc, parser);
			} else {
				body = parser_parse_expression(cc, parser, 0, false);
				EXPECT(';', &token);
			}

			// parse else clause
			PEEK(&token);
			if (token.kind == LL_TOKEN_KIND_IDENT && token.str.ptr == LL_KEYWORD_ELSE.ptr) {
				CONSUME();
				PEEK(&token);
				if (token.kind == '{') {
					right = (Ast_Base*)parser_parse_block(cc, parser);
				} else {
					right = parser_parse_expression(cc, parser, 0, false);
					EXPECT(';', &token);
				}
			} else {
				right = NULL;
			}

			left = CREATE_NODE(AST_KIND_IF, ((Ast_If){ .cond = left, .body = body, .else_clause = right }));
			return left;
		} else if (token.str.ptr == LL_KEYWORD_FOR.ptr) {
			CONSUME();
			PEEK(&token);
			if (token.kind == ';') {
				left = NULL;
				CONSUME();
			} else {
				left = parser_parse_declaration(cc, parser, NULL, 0);
			}
			PEEK(&token);
			if (token.kind == ';') {
				right = NULL;
			} else {
				right = parser_parse_expression(cc, parser, 0, false);
			}
			EXPECT(';', &token);

			PEEK(&token);
			if (token.kind == '{') {
				update = NULL;
				body = (Ast_Base*)parser_parse_block(cc, parser);
			} else {
				update = parser_parse_expression(cc, parser, 0, false);
				PEEK(&token);
				if (token.kind == '{') {
					body = (Ast_Base*)parser_parse_block(cc, parser);
				} else {
					body = parser_parse_expression(cc, parser, 0, false);
				}
			}

			left = CREATE_NODE(AST_KIND_FOR, ((Ast_Loop){ .init = left, .cond = right, .update = update, .body = body }));
			return left;
		} else if (token.str.ptr == LL_KEYWORD_DO.ptr) {
			left = (Ast_Base*)parser_parse_block(cc, parser);
			return left;
		} else if (token.str.ptr == LL_KEYWORD_CONST.ptr) {
			CONSUME();
			left = parser_parse_expression(cc, parser, 0, false);
			left = CREATE_NODE(AST_KIND_CONST, ((Ast_Marker){ .expr = left }));
			return left;
		}
	default:
		left = parser_parse_primary(cc, parser);
		break;
	}
	
	while (PEEK(&token)) {
		int bin_precedence = get_binary_precedence(token, from_statement);
		int post_precedence = get_postfix_precedence(token);

		if (bin_precedence != 0 && bin_precedence >= last_precedence) {
			CONSUME();
			right = parser_parse_expression(cc, parser, bin_precedence, false);
			left = CREATE_NODE(AST_KIND_BINARY_OP, ((Ast_Operation){ .left = left, .right = right, .op = token.kind }));
		} else if (post_precedence != 0 && post_precedence >= last_precedence) {
			switch (token.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
				case '(': {
					CONSUME();

					Ast_List arguments = { 0 };
					PEEK(&token);
					while (token.kind != ')') {
						arena_da_append(&cc->arena, &arguments, parser_parse_expression(cc, parser, 0, false));

						PEEK(&token);
						if (token.kind != ')') {
							EXPECT(',', &token);
							PEEK(&token);
							continue;
						}
					}

					CONSUME();

					left = CREATE_NODE(AST_KIND_INVOKE, ((Ast_Invoke){ .expr = left, .arguments = arguments }));

					break;
				}
				case '*': {
					if (!from_statement) break;
					CONSUME();
					
					left = CREATE_NODE(AST_KIND_TYPE_POINTER, ((Ast_Type_Pointer){ .element = left }));

					break;
				}
				case '[': {
					CONSUME();
					PEEK(&token);

					if (token.kind == ']') {
						CONSUME();
						right = NULL;
					} else {
						right = parser_parse_expression(cc, parser, 0, false);
						EXPECT(']', &token);
					}

					left = CREATE_NODE(AST_KIND_INDEX, ((Ast_Operation){ .left = left, .right = right }));
					break;
				}
#pragma GCC diagnostic pop
				default: assert(false); break;
			}
		} else break;
	}

    return left;
}

Ast_Base* parser_parse_primary(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token pk;
    Ast_Base* result;
    PEEK(&pk);

    switch (pk.kind) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    case '(':
        CONSUME();
        result = parser_parse_expression(cc, parser, 0, false);
        EXPECT(')', NULL);
        break;
    case '{':
        result = parser_parse_initializer(cc, parser);
        break;
    case '[':
        result = parser_parse_array_initializer(cc, parser);
        break;
#pragma GCC diagnostic pop
    
    case LL_TOKEN_KIND_INT:
        result = CREATE_NODE(AST_KIND_LITERAL_INT, ((Ast_Literal){ .i64 = pk.i64 }));
		CONSUME();
        break;

    case LL_TOKEN_KIND_STRING:
        result = CREATE_NODE(AST_KIND_LITERAL_STRING, ((Ast_Literal){ .str = pk.str }));
		CONSUME();
        break;

    case LL_TOKEN_KIND_IDENT:
		CONSUME();
		if (pk.str.ptr == LL_KEYWORD_RETURN.ptr) {
			PEEK(&pk);
			if (pk.kind != ';')
				result = parser_parse_expression(cc, parser, 0, false);
			else result = NULL;
			result = CREATE_NODE(AST_KIND_RETURN, ((Ast_Control_Flow){ .expr = result }));
			break;
		} else if (pk.str.ptr == LL_KEYWORD_BREAK.ptr) {
			PEEK(&pk);
			if (pk.kind != ';')
				result = parser_parse_expression(cc, parser, 0, false);
			else result = NULL;
			result = CREATE_NODE(AST_KIND_BREAK, ((Ast_Control_Flow){ .expr = result }));
			break;
		} else if (pk.str.ptr == LL_KEYWORD_CONTINUE.ptr) {
			PEEK(&pk);
			if (pk.kind != ';')
				result = parser_parse_expression(cc, parser, 0, false);
			else result = NULL;
			result = CREATE_NODE(AST_KIND_CONTINUE, ((Ast_Control_Flow){ .expr = result }));
			break;
		}
        result = CREATE_NODE(AST_KIND_IDENT, ((Ast_Ident){ .str = pk.str, .symbol_index = AST_IDENT_SYMBOL_INVALID }));
        break;

    default:
        UNEXPECTED();
        break;
    }

    return result;
}

void print_storage_class(LL_Storage_Class storage_class) {
	if (storage_class & LL_STORAGE_CLASS_EXTERN) printf("extern ");
	if (storage_class & LL_STORAGE_CLASS_STATIC) printf("static ");
}

const char* get_node_kind(Ast_Base* node) {
	switch (node->kind) {
		case AST_KIND_LITERAL_INT: return "Int_Literal";
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
		case AST_KIND_INDEX: return "Index";
		case AST_KIND_TYPE_POINTER: return "Pointer";
	}
}

void print_node_value(Ast_Base* node) {
	switch (node->kind) {
		case AST_KIND_LITERAL_INT: printf("%" PRId64, AST_AS(node, Ast_Literal)->i64); break;
		case AST_KIND_LITERAL_STRING: printf(FMT_SV_FMT, FMT_SV_ARG(AST_AS(node, Ast_Literal)->str)); break;
		case AST_KIND_IDENT: printf(FMT_SV_FMT, FMT_SV_ARG(AST_AS(node, Ast_Ident)->str)); break;
		case AST_KIND_BINARY_OP: lexer_print_token_raw(&AST_AS(node, Ast_Operation)->op); break;
		case AST_KIND_PRE_OP: lexer_print_token_raw(&AST_AS(node, Ast_Operation)->op); break;
		case AST_KIND_INVOKE: break;
		case AST_KIND_INITIALIZER: break;
		case AST_KIND_ARRAY_INITIALIZER: break;
		case AST_KIND_KEY_VALUE: break;
		case AST_KIND_BLOCK: break;
		case AST_KIND_CONST: break;
		case AST_KIND_VARIABLE_DECLARATION: print_storage_class(AST_AS(node, Ast_Variable_Declaration)->storage_class); break;
		case AST_KIND_FUNCTION_DECLARATION: print_storage_class(AST_AS(node, Ast_Function_Declaration)->storage_class); break;
		case AST_KIND_PARAMETER: break;
		case AST_KIND_RETURN: break;
		case AST_KIND_BREAK: break;
		case AST_KIND_CONTINUE: break;
		case AST_KIND_IF: break;
		case AST_KIND_FOR: break;
		case AST_KIND_INDEX: break;
		case AST_KIND_TYPE_POINTER: break;
	}
}

void print_node(Ast_Base* node, int indent) {
	int i;
	for (i = 0; i < indent; ++i) {
		printf("  ");
	}
	printf("%s ", get_node_kind(node));
	print_node_value(node);
	printf("\n");
	switch (node->kind) {
		case AST_KIND_BINARY_OP: 
			print_node(AST_AS(node, Ast_Operation)->left, indent + 1);
			print_node(AST_AS(node, Ast_Operation)->right, indent + 1);
			break;
		case AST_KIND_PRE_OP: 
			print_node(AST_AS(node, Ast_Operation)->right, indent + 1);
			break;

		case AST_KIND_INVOKE: 
			print_node(AST_AS(node, Ast_Invoke)->expr, indent + 1);
			for (i = 0; i < AST_AS(node, Ast_Invoke)->arguments.count; ++i) {
				print_node((Ast_Base*)AST_AS(node, Ast_Invoke)->arguments.items[i], indent + 1);
			}
			break;

		case AST_KIND_INITIALIZER: 
		case AST_KIND_ARRAY_INITIALIZER: 
			for (i = 0; i < AST_AS(node, Ast_Initializer)->count; ++i) {
				print_node((Ast_Base*)AST_AS(node, Ast_Initializer)->items[i], indent + 1);
			}
			break;

		case AST_KIND_KEY_VALUE: 
			print_node((Ast_Base*)AST_AS(node, Ast_Key_Value)->key, indent + 1);
			print_node((Ast_Base*)AST_AS(node, Ast_Key_Value)->value, indent + 1);
			break;

		case AST_KIND_CONST:
			print_node(AST_AS(node, Ast_Marker)->expr, indent + 1);
			break;
		case AST_KIND_VARIABLE_DECLARATION:
			print_node((Ast_Base*)AST_AS(node, Ast_Variable_Declaration)->type, indent + 1);
			print_node((Ast_Base*)AST_AS(node, Ast_Variable_Declaration)->ident, indent + 1);
			if (AST_AS(node, Ast_Variable_Declaration)->initializer) print_node(AST_AS(node, Ast_Variable_Declaration)->initializer, indent + 1);
			break;

		case AST_KIND_FUNCTION_DECLARATION:
			print_node(AST_AS(node, Ast_Function_Declaration)->return_type, indent + 1);
			print_node((Ast_Base*)AST_AS(node, Ast_Function_Declaration)->ident, indent + 1);
			for (i = 0; i < AST_AS(node, Ast_Function_Declaration)->parameters.count; ++i) {
				print_node((Ast_Base*)&AST_AS(node, Ast_Function_Declaration)->parameters.items[i], indent + 1);
			}
			if (AST_AS(node, Ast_Function_Declaration)->body) print_node(AST_AS(node, Ast_Function_Declaration)->body, indent + 1);
			break;

		case AST_KIND_PARAMETER:
			if (AST_AS(node, Ast_Variable_Declaration)->type)
				print_node(AST_AS(node, Ast_Variable_Declaration)->type, indent + 1);
			print_node((Ast_Base*)AST_AS(node, Ast_Variable_Declaration)->ident, indent + 1);
			break;
		case AST_KIND_RETURN:
			if (AST_AS(node, Ast_Control_Flow)->expr)
				print_node(AST_AS(node, Ast_Control_Flow)->expr, indent + 1);
			break;
		case AST_KIND_BREAK:
			if (AST_AS(node, Ast_Control_Flow)->expr)
				print_node(AST_AS(node, Ast_Control_Flow)->expr, indent + 1);
			break;
		case AST_KIND_CONTINUE:
			if (AST_AS(node, Ast_Control_Flow)->expr)
				print_node(AST_AS(node, Ast_Control_Flow)->expr, indent + 1);
			break;
		case AST_KIND_IF:
			if (AST_AS(node, Ast_If)->cond)
				print_node(AST_AS(node, Ast_If)->cond, indent + 1);
			if (AST_AS(node, Ast_If)->body)
				print_node(AST_AS(node, Ast_If)->body, indent + 1);
			if (AST_AS(node, Ast_If)->else_clause)
				print_node(AST_AS(node, Ast_If)->else_clause, indent + 1);
			break;
		case AST_KIND_FOR:
			if (AST_AS(node, Ast_Loop)->init)
				print_node(AST_AS(node, Ast_Loop)->init, indent + 1);
			if (AST_AS(node, Ast_Loop)->cond)
				print_node(AST_AS(node, Ast_Loop)->cond, indent + 1);
			if (AST_AS(node, Ast_Loop)->update)
				print_node(AST_AS(node, Ast_Loop)->update, indent + 1);
			if (AST_AS(node, Ast_Loop)->body)
				print_node(AST_AS(node, Ast_Loop)->body, indent + 1);
			break;

		case AST_KIND_INDEX:
			print_node(AST_AS(node, Ast_Operation)->left, indent + 1);
			if (AST_AS(node, Ast_Operation)->right)
				print_node(AST_AS(node, Ast_Operation)->right, indent + 1);
			break;

		case AST_KIND_TYPE_POINTER:
			print_node(AST_AS(node, Ast_Type_Pointer)->element, indent + 1);
			break;

		case AST_KIND_BLOCK:
			for (i = 0; i < AST_AS(node, Ast_Block)->count; ++i) {
				print_node(AST_AS(node, Ast_Block)->items[i], indent + 1);
			}

		default: break;
	}
}
