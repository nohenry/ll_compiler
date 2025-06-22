#pragma once

#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdbool.h>

#include "arena.h"

typedef struct {
	char* ptr;
	size_t len;
} String_View;

typedef struct {
	size_t count;
	size_t capacity;
	char* items;
} String_Builder;

#define str_lit(str) ((String_View) { (str), (sizeof(str)-1)/sizeof((str)[0]) })
#define EQL(a, b) (is_eql((a), (b), sizeof((a))))

struct ll_type;
inline bool is_eql(void* a, void* b, size_t size) {
	return memcmp(a, b, size) == 0;
}

#define LL_DEFAULT_MAP_ENTRY_COUNT (256u)

typedef struct string_intern_map_entry {
	String_View value;
	struct string_intern_map_entry* next;
} String_Intern_Map_Entry;

typedef struct {
    Arena arena, tmp_arena;
	String_Intern_Map_Entry* string_interns[LL_DEFAULT_MAP_ENTRY_COUNT];
	struct ll_typer* typer;
	struct ll_eval_context* eval_context;
	struct ll_backend_ir* bir;
} Compiler_Context;

extern String_View LL_KEYWORD_CONST;
extern String_View LL_KEYWORD_IF;
extern String_View LL_KEYWORD_FOR;
extern String_View LL_KEYWORD_WHILE;
extern String_View LL_KEYWORD_ELSE;
extern String_View LL_KEYWORD_DO;
extern String_View LL_KEYWORD_MATCH;
extern String_View LL_KEYWORD_STRUCT;
extern String_View LL_KEYWORD_EXTERN;
extern String_View LL_KEYWORD_RETURN;
extern String_View LL_KEYWORD_BREAK;
extern String_View LL_KEYWORD_CONTINUE;

extern String_View LL_KEYWORD_BOOL;
extern String_View LL_KEYWORD_BOOL8;
extern String_View LL_KEYWORD_BOOL16;
extern String_View LL_KEYWORD_BOOL32;
extern String_View LL_KEYWORD_BOOL64;
extern String_View LL_KEYWORD_TRUE;
extern String_View LL_KEYWORD_FALSE;

extern String_View LL_KEYWORD_UINT8;
extern String_View LL_KEYWORD_UINT16;
extern String_View LL_KEYWORD_UINT32;
extern String_View LL_KEYWORD_UINT64;
extern String_View LL_KEYWORD_UINT;
extern String_View LL_KEYWORD_INT8;
extern String_View LL_KEYWORD_INT16;
extern String_View LL_KEYWORD_INT32;
extern String_View LL_KEYWORD_INT64;
extern String_View LL_KEYWORD_INT;
extern String_View LL_KEYWORD_FLOAT16;
extern String_View LL_KEYWORD_FLOAT32;
extern String_View LL_KEYWORD_FLOAT64;
extern String_View LL_KEYWORD_FLOAT;
extern String_View LL_KEYWORD_STRING;
extern String_View LL_KEYWORD_VOID;

#define max(a, b) ({		 \
		__typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		 _a > _b ? _a : _b;	 \
	})

#define min(a, b) ({		 \
		__typeof__ (a) _a = (a); \
		__typeof__ (b) _b = (b); \
		 _a < _b ? _a : _b;	 \
	})

#define ABORT(msg) ({ \
    fprintf(stderr, "Abort(%s:%d): %s\n", __FILE__, __LINE__, msg); \
    abort(); \
    0; \
})

#define FMT_SV_FMT "%.*s"
#define FMT_SV_ARG(sv) ((int)(sv).len), (sv).ptr

#define arena_sb_append_strview(a, da, sv) arena_da_append_many(a, da, sv.ptr, sv.len)

#define LEN(array) (sizeof(array) / sizeof((array)[0]))

#define MAP_GET(map, key, allocator, hash_fn, eql_fn, seed) ({                     \
		size_t hash = hash_fn((key), (seed)) % (sizeof(map) / sizeof((map)[0])); \
		__typeof((map)[0]) current = (map)[hash]; 	 		                                   \
		__typeof__(current->value)* result = NULL;                                     \
		while (current) { 						                                   \
			if (eql_fn(current->value, (key))) { 	                               \
				result = &current->value; 		                                   \
				break; 							                                   \
			} 									                                   \
			current = current->next; 			                                   \
		} 										                                   \
		result; 								                                   \
	})

#define MAP_PUT(map, key, _value, ...) MAP_PUT_(map, key, _value, __VA_ARGS__)
#define MAP_PUT_(map, key, _value, allocator, hash_fn, eql_fn, seed) ({ 	               \
		__typeof__(map[0]->value)* result = NULL;  			   	                           \
		result = MAP_GET(map, key, allocator, hash_fn, eql_fn, seed);                   \
		if (!result) { 										                           \
			__typeof__(map[0]) entry = arena_alloc(allocator, sizeof(*map[0]));            \
            size_t hash = hash_fn((key), (seed)) % (sizeof(map) / sizeof((map)[0])); \
            entry->next = (map)[hash];                                                 \
            (map)[hash] = entry;                                                       \
            result = &entry->value;                                                            \
		} 													                           \
        memcpy(result, &_value, sizeof(*result));                                       \
		result; 										   	                           \
	})

#define MAP_GET_OR_PUT(map, key, _value, ...) MAP_GET_OR_PUT_(map, key, _value, __VA_ARGS__)
#define MAP_GET_OR_PUT_(map, key, _value, allocator, hash_fn, eql_fn, seed) ({ 	               \
		__typeof__(map[0]->value)* result = NULL;  			   	                               \
		result = MAP_GET(map, key, allocator, hash_fn, eql_fn, seed);                          \
		if (!result) { 										                                   \
			__typeof__(map[0]) entry = arena_alloc(allocator, sizeof(*map[0]));                \
            size_t hash = hash_fn((key), (seed)) % (sizeof(map) / sizeof((map)[0]));           \
            entry->next = (map)[hash];                                                         \
            (map)[hash] = entry;                                                               \
            memcpy(&entry->value, &_value, sizeof(*result));                                   \
            result = &entry->value;                                                            \
		} 													                                   \
		result; 										   	                                   \
	})

#define FOO(a, b, h) h(a, b)

#define MAP_DEFAULT &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED
#define MAP_DEFAULT_HASH_FN(key, value) _Generic((key),	\
		String_View : stbds_hash_string,					\
		struct ll_type* : ll_type_hash 						\
	)(key, value)

#define MAP_DEFAULT_EQL_FN(a, b) _Generic((a),				\
		String_View : string_view_eql,						\
		struct ll_type* : ll_type_eql 						\
	)(a, b)

#define MAP_DEFAULT_SEED (16u)

#define STBDS_SIZE_T_BITS           ((sizeof (size_t)) * 8)
#define STBDS_ROTATE_LEFT(val, n)   (((val) << (n)) | ((val) >> (STBDS_SIZE_T_BITS - (n))))
#define STBDS_ROTATE_RIGHT(val, n)  (((val) >> (n)) | ((val) << (STBDS_SIZE_T_BITS - (n))))

size_t stbds_hash_string(String_View str, size_t seed);
size_t stbds_siphash_bytes(void *p, size_t len, size_t seed);

bool string_view_eql(String_View a, String_View b);
bool string_view_starts_with(String_View haystack, String_View needle);

Compiler_Context ll_compiler_context_create();
String_View ll_intern_string(Compiler_Context* cc, String_View str);
size_t ll_type_hash(struct ll_type* type, size_t seed);
bool ll_type_eql(struct ll_type* a, struct ll_type* b);

static inline uint64_t align_forward(uint64_t offset, uint64_t alignment) {
      return ((offset + alignment - 1) & (~(alignment - 1)));
}

#define TODO(fmt, ...) do { \
		fprintf(stderr, "\x1b[31;1mTODO\x1b[0m(%s:%d): ", __FILE__, __LINE__); \
		fprintf(stderr, fmt, ## __VA_ARGS__); \
		fprintf(stderr, "\n"); \
	} while (0)

