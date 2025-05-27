#include "common.h"
#define ARENA_IMPLEMENTATION
#include "arena.h"

size_t stbds_hash_string(String_View str, size_t seed)
{
	size_t hash = seed;
	while (str.len-- > 0)
		hash = STBDS_ROTATE_LEFT(hash, 9) + (unsigned char) *str.ptr++;

	// Thomas Wang 64-to-32 bit mix function, hopefully also works in 32 bits
	hash ^= seed;
	hash = (~hash) + (hash << 18);
	hash ^= hash ^ STBDS_ROTATE_RIGHT(hash,31);
	hash = hash * 21;
	hash ^= hash ^ STBDS_ROTATE_RIGHT(hash,11);
	hash += (hash << 6);
	hash ^= STBDS_ROTATE_RIGHT(hash,22);
	return hash+seed;
}

bool string_view_eql(String_View a, String_View b) {
	return strncmp(a.ptr, b.ptr, min(a.len, b.len)) == 0;
}

String_View LL_KEYWORD_CONST;
String_View LL_KEYWORD_IF;
String_View LL_KEYWORD_FOR;
String_View LL_KEYWORD_WHILE;
String_View LL_KEYWORD_ELSE;
String_View LL_KEYWORD_DO;
String_View LL_KEYWORD_MATCH;
String_View LL_KEYWORD_STRUCT;

Compiler_Context ll_compiler_context_create() {
	Compiler_Context result = { 0 };
	LL_KEYWORD_CONST = ll_intern_string(&result, str_lit("const"));
	LL_KEYWORD_IF = ll_intern_string(&result, str_lit("if"));
	LL_KEYWORD_FOR = ll_intern_string(&result, str_lit("for"));
	LL_KEYWORD_WHILE = ll_intern_string(&result, str_lit("while"));
	LL_KEYWORD_ELSE = ll_intern_string(&result, str_lit("else"));
	LL_KEYWORD_DO = ll_intern_string(&result, str_lit("do"));
	LL_KEYWORD_MATCH = ll_intern_string(&result, str_lit("match"));
	LL_KEYWORD_STRUCT = ll_intern_string(&result, str_lit("struct"));
	return result;
}

String_View ll_intern_string(Compiler_Context* cc, String_View str) {
    String_View* s = MAP_GET_OR_PUT(cc->string_interns, str, str, MAP_DEFAULT);
	return *s;
}
