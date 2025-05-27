
#include "common.h"
#include "lexer.h"

static void lexer_prefixed(Compiler_Context *cc, LL_Lexer* lexer, LL_Token* out, char next_char, LL_Token_Kind next_char_kind) {
    if (lexer->pos + 1 < lexer->source.len) {
        if (lexer->source.ptr[lexer->pos + 1] == next_char) {
			out->kind = next_char_kind;
			lexer->pos += 2;
        } else {
			out->kind = lexer->source.ptr[lexer->pos];
			lexer->pos += 1;
		}
    } else {
		out->kind = lexer->source.ptr[lexer->pos];
		lexer->pos += 1;
	}
}

bool lexer_peek_token(Compiler_Context *cc, LL_Lexer* lexer, LL_Token* out) {
	if (lexer->has_peeked_token) {
		memcpy(out, &lexer->peeked_token, sizeof(*out));
		return true;
	} else {
		return lexer_next_token(cc, lexer, out);
	}
}

bool lexer_next_token(Compiler_Context *cc, LL_Lexer* lexer, LL_Token* out) {
	if (lexer->has_peeked_token) {
		memcpy(out, &lexer->peeked_token, sizeof(*out));
		return true;
	} 

    while (lexer->pos < lexer->source.len) switch (lexer->source.ptr[lexer->pos]) {
        case '\t':
        case '\n':
        case ' ':
            lexer->pos++;
            continue;
        default: goto DONE_PHANTOM;
    }
DONE_PHANTOM:

    if (lexer->pos < lexer->source.len) switch (lexer->source.ptr[lexer->pos]) {
		case '#': 
            out->kind = LL_TOKEN_KIND_BUILTIN;
			lexer->pos++;
			goto DO_IDENTIFIER;
        case 'A' ... 'Z':
        case 'a' ... 'z':
        case '_': {
            out->kind = LL_TOKEN_KIND_IDENT;
DO_IDENTIFIER:
            out->str.ptr = lexer->source.ptr + lexer->pos;
            out->str.len = 0;

            while (lexer->pos < lexer->source.len) switch (lexer->source.ptr[lexer->pos]) {
                case 'A' ... 'Z':
                case 'a' ... 'z':
                case '0' ... '9':
                case '_':
                    lexer->pos++;
                    out->str.len++;
                    break;
                default: {
                    out->str = ll_intern_string(cc, out->str);
                    return true;
                }
            }
            assert(false);
        }
        case '0' ... '9': {
            int64_t integral = 0;
            int64_t base = 10;
            int64_t value;
            bool did_zero = false;

            while (lexer->pos < lexer->source.len) switch (lexer->source.ptr[lexer->pos]) {
                case '0': did_zero = true; // fallthrough
                case '1' ... '9': {
                    value = (int64_t)(lexer->source.ptr[lexer->pos] - '0');
                    if (value >= base) goto DONE_NUMBER;
                    lexer->pos++;

                    integral *= base;
                    integral += value;
                    break;
                }
                case 'a':
                case 'c' ... 'n': {
NUMBER_LOWER_ALPHA:
                    value = (int64_t)(lexer->source.ptr[lexer->pos] - 'a' + 10);
                    if (value >= base) goto DONE_NUMBER;
                    lexer->pos++;

                    integral *= base;
                    integral += value;
                    break;
                }
                case 'A':
                case 'C' ... 'N': {
NUMBER_UPPER_ALPHA:
                    value = (int64_t)(lexer->source.ptr[lexer->pos] - 'A' + 10);
                    if (value >= base) goto DONE_NUMBER;
                    lexer->pos++;

                    integral *= base;
                    integral += value;
                    break;
                }
                case 'b': {
                    if (!did_zero) goto NUMBER_LOWER_ALPHA;
                    lexer->pos++;
                    did_zero = false;
                    base = 2;
                    break;
                }
                case 'B': {
                    if (!did_zero) goto NUMBER_UPPER_ALPHA;
                    lexer->pos++;
                    did_zero = false;
                    base = 2;
                    break;
                }
                case 'o': {
                    if (!did_zero) goto NUMBER_LOWER_ALPHA;
                    lexer->pos++;
                    did_zero = false;
                    base = 8;
                    break;
                }
                case 'x': {
                    if (!did_zero) goto NUMBER_LOWER_ALPHA;
                    lexer->pos++;
                    did_zero = false;
                    base = 16;
                    break;
                }
                default: goto DONE_NUMBER;
            }
DONE_NUMBER:
            out->kind = LL_TOKEN_KIND_INT;
            out->i64 = integral;
            return true;
        }
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case ':':
        case ';':
        case ',':
            out->kind = (LL_Token_Kind)lexer->source.ptr[lexer->pos++];
            return true;

		case '<': lexer_prefixed(cc, lexer, out, '=', LL_TOKEN_KIND_LTE); return true;
		case '>': lexer_prefixed(cc, lexer, out, '=', LL_TOKEN_KIND_GTE); return true;

		case '=': lexer_prefixed(cc, lexer, out, '=', LL_TOKEN_KIND_EQUALS); return true;
		case '+': lexer_prefixed(cc, lexer, out, '=', LL_TOKEN_KIND_ASSIGN_PLUS); return true;
		case '-': lexer_prefixed(cc, lexer, out, '=', LL_TOKEN_KIND_ASSIGN_MINUS); return true;
		case '*': lexer_prefixed(cc, lexer, out, '=', LL_TOKEN_KIND_ASSIGN_TIMES); return true;
		case '/': lexer_prefixed(cc, lexer, out, '=', LL_TOKEN_KIND_ASSIGN_DIVIDE); return true;

		case '.': lexer_prefixed(cc, lexer, out, '.', LL_TOKEN_KIND_RANGE); return true;
        
        default: break;
    }

    return false;
}

void lexer_print_token(LL_Lexer* lexer, LL_Token* token) {
    printf("Token: ");
    switch (token->kind) {
    case LL_TOKEN_KIND_IDENT:
        printf(FMT_SV_FMT, FMT_SV_ARG(token->str));
		printf(" %p", token->str.ptr);
        break;
    case LL_TOKEN_KIND_BUILTIN:
        printf(FMT_SV_FMT, FMT_SV_ARG(token->str));
        break;
    case LL_TOKEN_KIND_INT:
        printf("%ld", token->i64);
        break;

    case LL_TOKEN_KIND_ASSIGN_PLUS: printf("+="); break;
    case LL_TOKEN_KIND_ASSIGN_MINUS: printf("-="); break;
    case LL_TOKEN_KIND_ASSIGN_TIMES: printf("*="); break;
    case LL_TOKEN_KIND_ASSIGN_DIVIDE: printf("/="); break;
    case LL_TOKEN_KIND_ASSIGN_PERCENT: printf("%%="); break;

    case LL_TOKEN_KIND_EQUALS: printf("=="); break;
    case LL_TOKEN_KIND_LTE: printf("<="); break;
    case LL_TOKEN_KIND_GTE: printf(">="); break;

    case LL_TOKEN_KIND_RANGE: printf(".."); break;
    
    default:
        printf("%c", (char)token->kind);
        break;
    }
    printf("\n");
}
