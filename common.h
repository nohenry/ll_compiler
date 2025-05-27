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


#define LL_DEFAULT_MAP_ENTRY_COUNT (255u)

typedef struct string_intern_map_entry {
	String_View value;
	struct string_intern_map_entry* next;
} String_Intern_Map_Entry;

typedef struct {
    Arena arena, tmp_arena;
	String_Intern_Map_Entry* string_interns[LL_DEFAULT_MAP_ENTRY_COUNT];
} Compiler_Context;

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
#define FMT_SV_ARG(sv) (sv).len, (sv).ptr

bool string_view_eql(String_View a, String_View b);
String_View ll_intern_string(Compiler_Context* cc, String_View str);

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

#define MAP_PUT(map, key, _value, ...) ({ 	               \
		__typeof__(map[0]->value)* result = NULL;  			   	                           \
		result = MAP_GET(map, key, __VA_ARGS__)                   \
		if (!result) { 										                           \
			__typeof__(map[0]) entry = arena_alloc(allocator, sizeof(*map[0]));            \
            size_t hash = (hash_fn)((key), (seed)) % (sizeof(map) / sizeof((map)[0])); \
            entry->next = (map)[hash];                                                 \
            (map)[hash] = entry;                                                       \
		} 													                           \
        memcpy(result, _value, sizeof(*result));                                       \
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
		String_View : stbds_hash_string						\
	)(key, value)

#define MAP_DEFAULT_EQL_FN(a, b) _Generic((a),				\
		String_View : string_view_eql						\
	)(a, b)

#define MAP_DEFAULT_SEED (16u)

#define STBDS_SIZE_T_BITS           ((sizeof (size_t)) * 8)
#define STBDS_ROTATE_LEFT(val, n)   (((val) << (n)) | ((val) >> (STBDS_SIZE_T_BITS - (n))))
#define STBDS_ROTATE_RIGHT(val, n)  (((val) >> (n)) | ((val) << (STBDS_SIZE_T_BITS - (n))))

size_t stbds_hash_string(String_View str, size_t seed);

