
#include "common.h"
#include "lexer.h"

bool lexer_next_token(Compiler_Context *cc, LL_Lexer* lexer, LL_Token* out) {
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
        case 'A' ... 'Z':
        case 'a' ... 'z':
        case '_': {
            out->kind = LL_TOKEN_KIND_IDENT;
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
            int64_t base = 0;
            bool did_zero = false;

            while (lexer->pos < lexer->source.len) switch (lexer->source.ptr[lexer->pos]) {
                case '0': did_zero = true; // fallthrough
                case '1' ... '9': {
                    int64_t value = (int64_t)(lexer->source.ptr[lexer->pos] - '0');
                    if (value >= base) goto DONE_NUMBER;

                    integral *= base;
                    integral += value;
                    lexer->pos++;
                    break;
                }
                case 'a':
                case 'c' ... 'n': {
NUMBER_LOWER_ALPHA:
                    int64_t value = (int64_t)(lexer->source.ptr[lexer->pos] - 'a' + 10);
                    if (value >= base) goto DONE_NUMBER;

                    integral *= base;
                    integral += value;
                    break;
                }
                case 'A':
                case 'C' ... 'N': {
NUMBER_UPPER_ALPHA:
                    int64_t value = (int64_t)(lexer->source.ptr[lexer->pos] - 'A' + 10);
                    if (value >= base) goto DONE_NUMBER;

                    integral *= base;
                    integral += value;
                    break;
                }
                case 'b': {
                    if (!did_zero) goto NUMBER_LOWER_ALPHA;
                    did_zero = false;
                    base = 2;
                    break;
                }
                case 'B': {
                    if (!did_zero) goto NUMBER_UPPER_ALPHA;
                    did_zero = false;
                    base = 2;
                    break;
                }
                case 'o': {
                    if (!did_zero) goto NUMBER_LOWER_ALPHA;
                    did_zero = false;
                    base = 8;
                    break;
                }
                case 'x': {
                    if (!did_zero) goto NUMBER_LOWER_ALPHA;
                    did_zero = false;
                    base = 16;
                    break;
                }
                default: goto DONE_NUMBER;
            }
DONE_NUMBER:
            out->kind = LL_TOKEN_KIND_INT;
            out->i64 = integral;
        }
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case ':':
        case ';':
            out->kind = (LL_Token_Kind)lexer->source.ptr[lexer->pos++];
            return true;
        default: break;
    }

    return false;
}

void lexer_print_token(LL_Lexer* lexer, LL_Token* token) {
    printf("Token: ");
    switch (token->kind) {
    case LL_TOKEN_KIND_IDENT:
        printf(FMT_SV_FMT, FMT_SV_ARG(token->str));
        break;
    case LL_TOKEN_KIND_INT:
        printf("%lld", token->i64);
        break;
    
    default:
        printf("%c", (char)token->kind);
        break;
    }
    printf("\n");
}
