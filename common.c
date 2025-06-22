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

#define STBDS_SIPHASH_C_ROUNDS 1
#define STBDS_SIPHASH_D_ROUNDS 1

size_t stbds_siphash_bytes(void *p, size_t len, size_t seed)
{
  unsigned char *d = (unsigned char *) p;
  size_t i,j;
  size_t v0,v1,v2,v3, data;

  // hash that works on 32- or 64-bit registers without knowing which we have
  // (computes different results on 32-bit and 64-bit platform)
  // derived from siphash, but on 32-bit platforms very different as it uses 4 32-bit state not 4 64-bit
  v0 = ((((size_t) 0x736f6d65 << 16) << 16) + 0x70736575) ^  seed;
  v1 = ((((size_t) 0x646f7261 << 16) << 16) + 0x6e646f6d) ^ ~seed;
  v2 = ((((size_t) 0x6c796765 << 16) << 16) + 0x6e657261) ^  seed;
  v3 = ((((size_t) 0x74656462 << 16) << 16) + 0x79746573) ^ ~seed;

  #define STBDS_SIPROUND() \
    do {                   \
      v0 += v1; v1 = STBDS_ROTATE_LEFT(v1, 13);  v1 ^= v0; v0 = STBDS_ROTATE_LEFT(v0,STBDS_SIZE_T_BITS/2); \
      v2 += v3; v3 = STBDS_ROTATE_LEFT(v3, 16);  v3 ^= v2;                                                 \
      v2 += v1; v1 = STBDS_ROTATE_LEFT(v1, 17);  v1 ^= v2; v2 = STBDS_ROTATE_LEFT(v2,STBDS_SIZE_T_BITS/2); \
      v0 += v3; v3 = STBDS_ROTATE_LEFT(v3, 21);  v3 ^= v0;                                                 \
    } while (0)

  for (i=0; i+sizeof(size_t) <= len; i += sizeof(size_t), d += sizeof(size_t)) {
    data = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
    data |= (size_t) (d[4] | (d[5] << 8) | (d[6] << 16) | (d[7] << 24)) << 16 << 16; // discarded if size_t == 4

    v3 ^= data;
    for (j=0; j < STBDS_SIPHASH_C_ROUNDS; ++j)
      STBDS_SIPROUND();
    v0 ^= data;
  }
  data = len << (STBDS_SIZE_T_BITS-8);
  switch (len - i) {
    case 7: data |= ((size_t) d[6] << 24) << 24; // fall through
    case 6: data |= ((size_t) d[5] << 20) << 20; // fall through
    case 5: data |= ((size_t) d[4] << 16) << 16; // fall through
    case 4: data |= (d[3] << 24); // fall through
    case 3: data |= (d[2] << 16); // fall through
    case 2: data |= (d[1] << 8); // fall through
    case 1: data |= d[0]; // fall through
    case 0: break;
  }
  v3 ^= data;
  for (j=0; j < STBDS_SIPHASH_C_ROUNDS; ++j)
    STBDS_SIPROUND();
  v0 ^= data;
  v2 ^= 0xff;
  for (j=0; j < STBDS_SIPHASH_D_ROUNDS; ++j)
    STBDS_SIPROUND();

  return v1^v2^v3; // slightly stronger since v0^v3 in above cancels out final round operation? I tweeted at the authors of SipHash about this but they didn't reply
}

bool string_view_eql(String_View a, String_View b) {
	return strncmp(a.ptr, b.ptr, min(a.len, b.len)) == 0;
}

bool string_view_starts_with(String_View haystack, String_View needle) {
	if (needle.len > haystack.len) return false;
	return memcmp(haystack.ptr, needle.ptr, needle.len) == 0;
}

String_View LL_KEYWORD_CONST;
String_View LL_KEYWORD_IF;
String_View LL_KEYWORD_FOR;
String_View LL_KEYWORD_WHILE;
String_View LL_KEYWORD_ELSE;
String_View LL_KEYWORD_DO;
String_View LL_KEYWORD_MATCH;
String_View LL_KEYWORD_STRUCT;
String_View LL_KEYWORD_EXTERN;
String_View LL_KEYWORD_RETURN;
String_View LL_KEYWORD_BREAK;
String_View LL_KEYWORD_CONTINUE;

String_View LL_KEYWORD_BOOL;
String_View LL_KEYWORD_BOOL8;
String_View LL_KEYWORD_BOOL16;
String_View LL_KEYWORD_BOOL32;
String_View LL_KEYWORD_BOOL64;
String_View LL_KEYWORD_TRUE;
String_View LL_KEYWORD_FALSE;

String_View LL_KEYWORD_UINT;
String_View LL_KEYWORD_UINT8;
String_View LL_KEYWORD_UINT16;
String_View LL_KEYWORD_UINT32;
String_View LL_KEYWORD_UINT64;
String_View LL_KEYWORD_INT;
String_View LL_KEYWORD_INT8;
String_View LL_KEYWORD_INT16;
String_View LL_KEYWORD_INT32;
String_View LL_KEYWORD_INT64;
String_View LL_KEYWORD_FLOAT16;
String_View LL_KEYWORD_FLOAT32;
String_View LL_KEYWORD_FLOAT64;
String_View LL_KEYWORD_FLOAT;
String_View LL_KEYWORD_STRING;
String_View LL_KEYWORD_VOID;

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
	LL_KEYWORD_EXTERN = ll_intern_string(&result, str_lit("extern"));
	LL_KEYWORD_RETURN = ll_intern_string(&result, str_lit("return"));
	LL_KEYWORD_BREAK = ll_intern_string(&result, str_lit("break"));
	LL_KEYWORD_CONTINUE = ll_intern_string(&result, str_lit("continue"));

	LL_KEYWORD_BOOL = ll_intern_string(&result, str_lit("bool"));
	LL_KEYWORD_BOOL8 = ll_intern_string(&result, str_lit("bool8"));
	LL_KEYWORD_BOOL16 = ll_intern_string(&result, str_lit("bool16"));
	LL_KEYWORD_BOOL32 = ll_intern_string(&result, str_lit("bool32"));
	LL_KEYWORD_BOOL64 = ll_intern_string(&result, str_lit("bool64"));
	LL_KEYWORD_TRUE = ll_intern_string(&result, str_lit("true"));
	LL_KEYWORD_FALSE = ll_intern_string(&result, str_lit("false"));

	LL_KEYWORD_UINT = ll_intern_string(&result, str_lit("uint"));
	LL_KEYWORD_UINT8 = ll_intern_string(&result, str_lit("uint8"));
	LL_KEYWORD_UINT16 = ll_intern_string(&result, str_lit("uint16"));
	LL_KEYWORD_UINT32 = ll_intern_string(&result, str_lit("uint32"));
	LL_KEYWORD_UINT64 = ll_intern_string(&result, str_lit("uint64"));
	LL_KEYWORD_INT = ll_intern_string(&result, str_lit("int"));
	LL_KEYWORD_INT8 = ll_intern_string(&result, str_lit("int8"));
	LL_KEYWORD_INT16 = ll_intern_string(&result, str_lit("int16"));
	LL_KEYWORD_INT32 = ll_intern_string(&result, str_lit("int32"));
	LL_KEYWORD_INT64 = ll_intern_string(&result, str_lit("int64"));
	LL_KEYWORD_FLOAT16 = ll_intern_string(&result, str_lit("float16"));
	LL_KEYWORD_FLOAT32 = ll_intern_string(&result, str_lit("float32"));
	LL_KEYWORD_FLOAT64 = ll_intern_string(&result, str_lit("float64"));
	LL_KEYWORD_FLOAT = ll_intern_string(&result, str_lit("float"));
	LL_KEYWORD_STRING = ll_intern_string(&result, str_lit("string"));
	LL_KEYWORD_VOID = ll_intern_string(&result, str_lit("void"));

	return result;
}

String_View ll_intern_string(Compiler_Context* cc, String_View str) {
    String_View* s = MAP_GET_OR_PUT(cc->string_interns, str, str, MAP_DEFAULT);
	return *s;
}


