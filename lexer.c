
#include <inttypes.h>

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
		lexer->has_peeked_token = lexer_next_token(cc, lexer, out);
		memcpy(&lexer->peeked_token, out, sizeof(*out));
		return lexer->has_peeked_token;
	}
}

void lexer_consume(Compiler_Context *cc, LL_Lexer* lexer) {
    lexer->has_peeked_token = false;
}

bool lexer_next_token(Compiler_Context *cc, LL_Lexer* lexer, LL_Token* out) {
	if (lexer->has_peeked_token) {
		memcpy(out, &lexer->peeked_token, sizeof(*out));
        lexer->has_peeked_token = false;
		return true;
	} 

    while (lexer->pos < lexer->source.len) switch (lexer->source.ptr[lexer->pos]) {
        case '\t':
        case '\n':
        case ' ':
            lexer->pos++;
            continue;
		case '/':
			if (lexer->pos + 1 < lexer->source.len && lexer->source.ptr[lexer->pos + 1] == '/') {
				lexer->pos += 2;

				while (lexer->pos < lexer->source.len) switch (lexer->source.ptr[lexer->pos]) {
				case '\n': goto DONE_LINE_COMMENT;
				default: lexer->pos++; continue;
				}
			} else {
				goto DONE_PHANTOM;
			}
DONE_LINE_COMMENT:
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
		case '"': {
            bool needs_alloc = false;
            String_Builder alloc_string = { 0 };
			lexer->pos++;
            size_t last_copied = lexer->pos;

			out->kind = LL_TOKEN_KIND_STRING;
            out->str.ptr = lexer->source.ptr + lexer->pos;
            out->str.len = 0;

            while (lexer->pos < lexer->source.len && lexer->source.ptr[lexer->pos] != '"') {
                switch (lexer->source.ptr[lexer->pos]) {
                    case '\\':
                        if (lexer->pos + 1 < lexer->source.len) {
                            switch (lexer->source.ptr[lexer->pos + 1]) {
                                case 'n':
                                    arena_sb_append_buf(&cc->arena, &alloc_string, &lexer->source.ptr[last_copied], lexer->pos - last_copied);
                                    arena_sb_append_cstr(&cc->arena, &alloc_string, "\n");
                                    last_copied = lexer->pos + 2;
                                    lexer->pos++;
                                    needs_alloc = true;
                                    break;
                                case 't':
                                    arena_sb_append_buf(&cc->arena, &alloc_string, &lexer->source.ptr[last_copied], lexer->pos - last_copied);
                                    arena_sb_append_cstr(&cc->arena, &alloc_string, "\t");
                                    last_copied = lexer->pos + 2;
                                    lexer->pos++;
                                    needs_alloc = true;
                                    break;
                                case 'r':
                                    arena_sb_append_buf(&cc->arena, &alloc_string, &lexer->source.ptr[last_copied], lexer->pos - last_copied);
                                    arena_sb_append_cstr(&cc->arena, &alloc_string, "\r");
                                    last_copied = lexer->pos + 2;
                                    lexer->pos++;
                                    needs_alloc = true;
                                    break;
                                case 'x': {
                                    uint8_t b = 0;
                                    arena_sb_append_buf(&cc->arena, &alloc_string, &lexer->source.ptr[last_copied], lexer->pos - last_copied);
                                    last_copied = lexer->pos + 4;
                                    lexer->pos++;

                                    if (lexer->pos + 2 >= lexer->source.len) {
                                        fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: Expected a byte value after \\x\x1b[0m\n");
                                        continue;
                                    }
                                    switch (lexer->source.ptr[lexer->pos + 1]) {
                                        case '0' ... '9': b += 16 * (lexer->source.ptr[lexer->pos + 1] - '0'); break;
                                        case 'a' ... 'f': b += 16 * (lexer->source.ptr[lexer->pos + 1] - 'a' + 0xa); break;
                                        case 'A' ... 'F': b += 16 * (lexer->source.ptr[lexer->pos + 1] - 'A' + 0xa); break;
                                        default:
                                            fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: Expected a valid hex value after \\x\x1b[0m\n");
                                            continue;
                                    }
                                    switch (lexer->source.ptr[lexer->pos + 2]) {
                                        case '0' ... '9': b += (lexer->source.ptr[lexer->pos + 2] - '0'); break;
                                        case 'a' ... 'f': b += (lexer->source.ptr[lexer->pos + 2] - 'a' + 0xa); break;
                                        case 'A' ... 'F': b += (lexer->source.ptr[lexer->pos + 2] - 'A' + 0xa); break;
                                        default:
                                            fprintf(stderr, "\x1b[31;1merror\x1b[0;1m: Expected a valid hex value after \\x\x1b[0m\n");
                                            continue;
                                    }
                                    arena_da_append(&cc->arena, &alloc_string, b);
                                    lexer->pos += 2;
                                    needs_alloc = true;
                                    break;
                                }
                            }
                        }
                        break;
                }
				lexer->pos++;
                out->str.len++;
			}
			lexer->pos++; // for closing quote

            if (needs_alloc) {
                arena_sb_append_buf(&cc->arena, &alloc_string, &lexer->source.ptr[last_copied], lexer->pos - last_copied - 1);
                out->str.ptr = alloc_string.items;
                out->str.len = alloc_string.count;
                out->str = ll_intern_string(cc, out->str);
            }

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
        case '&':
            out->kind = (LL_Token_Kind)lexer->source.ptr[lexer->pos++];
            return true;

		case '<': lexer_prefixed(cc, lexer, out, '=', LL_TOKEN_KIND_LTE); return true;
		case '>': lexer_prefixed(cc, lexer, out, '=', LL_TOKEN_KIND_GTE); return true;

		case '!': lexer_prefixed(cc, lexer, out, '=', LL_TOKEN_KIND_NEQUALS); return true;
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

void lexer_print_token_raw(LL_Token* token) {
	lexer_print_token_raw_to_fd(token, stdout);
}

void lexer_print_token_kind(LL_Token_Kind kind, FILE* fd) {
    switch (kind) {
    case LL_TOKEN_KIND_IDENT:
        fprintf(fd, "identifier");
        break;
    case LL_TOKEN_KIND_BUILTIN:
        fprintf(fd, "builtin");
        break;
    case LL_TOKEN_KIND_INT:
        fprintf(fd, "integer");
        break;
    case LL_TOKEN_KIND_STRING:
        fprintf(fd, "string");
        break;

    case LL_TOKEN_KIND_ASSIGN_PLUS: fprintf(fd, "+="); break;
    case LL_TOKEN_KIND_ASSIGN_MINUS: fprintf(fd, "-="); break;
    case LL_TOKEN_KIND_ASSIGN_TIMES: fprintf(fd, "*="); break;
    case LL_TOKEN_KIND_ASSIGN_DIVIDE: fprintf(fd, "/="); break;
    case LL_TOKEN_KIND_ASSIGN_PERCENT: fprintf(fd, "%%="); break;

    case LL_TOKEN_KIND_EQUALS: fprintf(fd, "=="); break;
    case LL_TOKEN_KIND_NEQUALS: fprintf(fd, "=="); break;
    case LL_TOKEN_KIND_LTE: fprintf(fd, "<="); break;
    case LL_TOKEN_KIND_GTE: fprintf(fd, ">="); break;

    case LL_TOKEN_KIND_RANGE: fprintf(fd, ".."); break;
    
    default:
        fprintf(fd, "%c", (char)kind);
        break;
    }
}


void lexer_print_token_raw_to_fd(LL_Token* token, FILE* fd) {
    switch (token->kind) {
    case LL_TOKEN_KIND_IDENT:
        fprintf(fd, FMT_SV_FMT, FMT_SV_ARG(token->str));
        break;
    case LL_TOKEN_KIND_BUILTIN:
        fprintf(fd, FMT_SV_FMT, FMT_SV_ARG(token->str));
        break;
    case LL_TOKEN_KIND_INT:
        fprintf(fd, "%" PRId64, token->i64);
        break;
    case LL_TOKEN_KIND_STRING:
        fprintf(fd, FMT_SV_FMT, FMT_SV_ARG(token->str));
        break;

    case LL_TOKEN_KIND_ASSIGN_PLUS: fprintf(fd, "+="); break;
    case LL_TOKEN_KIND_ASSIGN_MINUS: fprintf(fd, "-="); break;
    case LL_TOKEN_KIND_ASSIGN_TIMES: fprintf(fd, "*="); break;
    case LL_TOKEN_KIND_ASSIGN_DIVIDE: fprintf(fd, "/="); break;
    case LL_TOKEN_KIND_ASSIGN_PERCENT: fprintf(fd, "%%="); break;

    case LL_TOKEN_KIND_EQUALS: fprintf(fd, "=="); break;
    case LL_TOKEN_KIND_NEQUALS: fprintf(fd, "=="); break;
    case LL_TOKEN_KIND_LTE: fprintf(fd, "<="); break;
    case LL_TOKEN_KIND_GTE: fprintf(fd, ">="); break;

    case LL_TOKEN_KIND_RANGE: fprintf(fd, ".."); break;
    
    default:
        fprintf(fd, "%c", (char)token->kind);
        break;
    }
}

void lexer_print_token(LL_Token* token) {
    printf("Token: ");
    lexer_print_token_raw(token);
    printf("\n");
}
