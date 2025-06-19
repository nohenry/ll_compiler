
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
		fprintf(stderr, "' \x1b[0m\n", tok.kind);

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
		case '{':
			result = (Ast_Base*)parser_parse_block(cc, parser);
			break;
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
	CONSUME();
	PEEK(&token);
	Ast_Block block = { .base.kind = AST_KIND_BLOCK };

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
		default: return 0;
	}
}

int get_postfix_precedence(LL_Token token) {
	switch (token.kind) {
		case '*':
			return 150;
		default: return 0;
	}
}

Ast_Base* parser_parse_expression(Compiler_Context* cc, LL_Parser* parser, int last_precedence, bool from_statement) {
	LL_Token token;
	Ast_Base *left, *right, *body, *update;
	PEEK(&token);
	switch (token.kind) {
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
	case LL_TOKEN_KIND_IDENT:
		if (token.str.ptr == LL_KEYWORD_IF.ptr) {
			// parse if statement
			CONSUME();
			left = parser_parse_expression(cc, parser, 0, false);
			PEEK(&token);
			if (token.kind == '{') {
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
		}
	default:
		left = parser_parse_primary(cc, parser);
		break;
	}
	
	while (PEEK(&token)) {
		int precedence = get_binary_precedence(token, from_statement);
		if (precedence == 0 || precedence < last_precedence) break;
		CONSUME();

		right = parser_parse_expression(cc, parser, precedence, false);
        left = CREATE_NODE(AST_KIND_BINARY_OP, ((Ast_Operation){ .left = left, .right = right, .op = token.kind }));
	}

	while (PEEK(&token)) {
		switch (token.kind) {
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

				continue;
			}
			case '*': {
				if (!from_statement) break;
				CONSUME();
				
				left = CREATE_NODE(AST_KIND_TYPE_POINTER, ((Ast_Type_Pointer){ .element = left }));

				continue;
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
				continue;
			}
			default: break;
		}

		break;
	}

    return left;
}

Ast_Base* parser_parse_primary(Compiler_Context* cc, LL_Parser* parser) {
    LL_Token pk;
    Ast_Base* result;
    PEEK(&pk);

    switch (pk.kind) {
    case '(':
        CONSUME();
        result = parser_parse_expression(cc, parser, 0, false);
        EXPECT(')', NULL);
        break;
    
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
		case AST_KIND_BLOCK: return "Block";
		case AST_KIND_VARIABLE_DECLARATION: return "Variable_Declaration";
		case AST_KIND_FUNCTION_DECLARATION: return "Function_Declaration";
		case AST_KIND_PARAMETER: return "Parameter";
		case AST_KIND_RETURN: return "Return";
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
		case AST_KIND_BLOCK: break;
		case AST_KIND_VARIABLE_DECLARATION: print_storage_class(AST_AS(node, Ast_Variable_Declaration)->storage_class); break;
		case AST_KIND_FUNCTION_DECLARATION: print_storage_class(AST_AS(node, Ast_Function_Declaration)->storage_class); break;
		case AST_KIND_PARAMETER: break;
		case AST_KIND_RETURN: break;
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
