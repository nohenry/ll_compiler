#pragma once

// #include <string.h>
// #include <stddef.h>
// #include <stdbool.h>
// #include <stdbool.h>

#include "../core/core1.h"

#define EQL(a, b) (is_eql((a), (b), sizeof((a))))
#define HASH_VALUE_FN(key, seed) stbds_siphash_bytes(&(key), sizeof(key), (seed))

struct ll_type;
inline bool is_eql(void* a, void* b, size_t size) {
    return memcmp(a, b, size) == 0;
}

#define LL_DEFAULT_MAP_ENTRY_COUNT (256u)
#define Hash_Map(K, V) struct { struct { K _key; V _value; uint32 filled; }* entries; uint32 count_filled, capacity; }
#define Array(I, V) struct { V* items; I count, capacity; }

#define Enum(_name, _type, ...) typedef _type _name; enum { __VA_ARGS__ }

#define hash_map_get(arena, hm, key) ({                                             \
        __typeof__((hm)->entries[0]._value)* result = NULL;                          \
        if ((hm)->capacity) { \
            uint32 index = MAP_DEFAULT_HASH_FN(key, MAP_DEFAULT_SEED) % (hm)->capacity; \
            uint32 until_index = index;                                                 \
            do {                                                                        \
                if ((hm)->entries[index].filled && MAP_DEFAULT_EQL_FN((hm)->entries[index]._key, key)) {                \
                    result = &(hm)->entries[index]._value;                               \
                    break;                                                              \
                }                                                                       \
                index = (index + 1) % (hm)->capacity;                                   \
            } while ((hm)->entries[index].filled && index != until_index);              \
        } \
        result;                                                                     \
    })

#define hash_map_reserve(arena, hm, reserve_capacity) do {                                                                                              \
        __typeof__((hm)->entries) new_entries = oc_arena_realloc((arena), (hm)->entries, (hm)->capacity * sizeof(*(hm)->entries), (reserve_capacity) * sizeof(*(hm)->entries)); \
        memset(new_entries, 0, (reserve_capacity) * sizeof(*(hm)->entries)); \
        if ((hm)->capacity) {                                                                                                                           \
            for (uint32 i = 0; i < ((hm)->capacity); ++i) {                                                                                               \
                if (!(hm)->entries[i].filled) continue;                                                                                               \
                uint32 index = MAP_DEFAULT_HASH_FN((hm)->entries[i]._key, MAP_DEFAULT_SEED) % (reserve_capacity);                                   \
                while (new_entries[index].filled) {                                                                                               \
                    if (MAP_DEFAULT_EQL_FN(new_entries[index]._key, (hm)->entries[i]._key)) {                                                                       \
                        break;                                                                                                                      \
                    }                                                                                                                               \
                    index = (index + 1) % (reserve_capacity);                                                                                       \
                }                                                                                                                                   \
                new_entries[index]._key = (hm)->entries[i]._key;                                                                                                    \
                new_entries[index]._value = (hm)->entries[i]._value;                                                                                                \
                new_entries[index].filled = 1;                                                                                                    \
            }                                                                                                                                           \
        }                                                                                                                                               \
        (hm)->entries = new_entries;                                                                                                                        \
        (hm)->capacity = (reserve_capacity);                                                                                                            \
    } while (0)

#define hash_map_put(arena, hm, key, value) do {                                                                                                 \
        if ((hm)->count_filled >= (hm)->capacity / 2) {                                                                                          \
            uint32 new_cap = (hm)->capacity ? (hm)->capacity * 4 : 32;                                                                          \
            hash_map_reserve(arena, hm, new_cap);                                                                                         \
        }                                                                                                                                        \
        uint32 index = MAP_DEFAULT_HASH_FN(key, MAP_DEFAULT_SEED) % (hm)->capacity;                                                                \
        while ((hm)->entries[index].filled) {                                                                                                      \
            if (MAP_DEFAULT_EQL_FN((hm)->entries[index]._key, key)) {                                                                               \
                break;                                                                                                                           \
            }                                                                                                                                    \
            index = (index + 1) % (hm)->capacity;                                                                                                  \
        }                                                                                                                                        \
        (hm)->entries[index]._key = key;                                                                                               \
        (hm)->entries[index]._value = value;                                                                                               \
        (hm)->entries[index].filled = 1;                                                                                               \
        (hm)->count_filled++; \
    } while(0)

typedef struct string_intern_map_entry {
    string value;
    struct string_intern_map_entry* next;
} String_Intern_Map_Entry;

typedef struct {
    Oc_Arena arena, tmp_arena;
    String_Intern_Map_Entry* string_interns[LL_DEFAULT_MAP_ENTRY_COUNT];
    struct ll_typer* typer;
    struct ll_typer2* typer2;
    struct ll_eval_context* eval_context;
    struct ll_backend_ir* bir;
    struct ll_backend *target, *native_target;
    struct ll_lexer* lexer;
	bool quiet;
    bool exit_0;
} Compiler_Context;

extern string LL_KEYWORD_CONST;
extern string LL_KEYWORD_CAST;
extern string LL_KEYWORD_IF;
extern string LL_KEYWORD_FOR;
extern string LL_KEYWORD_WHILE;
extern string LL_KEYWORD_ELSE;
extern string LL_KEYWORD_DO;
extern string LL_KEYWORD_MATCH;
extern string LL_KEYWORD_STRUCT;
extern string LL_KEYWORD_EXTERN;
extern string LL_KEYWORD_NATIVE;
extern string LL_KEYWORD_RETURN;
extern string LL_KEYWORD_BREAK;
extern string LL_KEYWORD_CONTINUE;
extern string LL_KEYWORD_MACRO;
extern string LL_KEYWORD_LET;
extern string LL_KEYWORD_SIZEOF;

extern string LL_KEYWORD_BOOL;
extern string LL_KEYWORD_BOOL8;
extern string LL_KEYWORD_BOOL16;
extern string LL_KEYWORD_BOOL32;
extern string LL_KEYWORD_BOOL64;
extern string LL_KEYWORD_TRUE;
extern string LL_KEYWORD_FALSE;
extern string LL_KEYWORD_NULL;

extern string LL_KEYWORD_UINT8;
extern string LL_KEYWORD_UINT16;
extern string LL_KEYWORD_UINT32;
extern string LL_KEYWORD_UINT64;
extern string LL_KEYWORD_UINT;
extern string LL_KEYWORD_INT8;
extern string LL_KEYWORD_INT16;
extern string LL_KEYWORD_INT32;
extern string LL_KEYWORD_INT64;
extern string LL_KEYWORD_INT;
extern string LL_KEYWORD_FLOAT16;
extern string LL_KEYWORD_FLOAT32;
extern string LL_KEYWORD_FLOAT64;
extern string LL_KEYWORD_FLOAT;
extern string LL_KEYWORD_STRING;
extern string LL_KEYWORD_VOID;
extern string LL_KEYWORD_CHAR;

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
            __typeof__(map[0]) entry = oc_arena_alloc(allocator, sizeof(*map[0]));            \
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
            __typeof__(map[0]) entry = oc_arena_alloc(allocator, sizeof(*map[0]));                \
            size_t hash = hash_fn((key), (seed)) % (sizeof(map) / sizeof((map)[0]));           \
            entry->next = (map)[hash];                                                         \
            (map)[hash] = entry;                                                               \
            memcpy(&entry->value, &_value, sizeof(*result));                                   \
            result = &entry->value;                                                            \
        } 													                                   \
        result; 										   	                                   \
    })

#define MAP_DEFAULT &cc->arena, MAP_DEFAULT_HASH_FN, MAP_DEFAULT_EQL_FN, MAP_DEFAULT_SEED
#define MAP_DEFAULT_HASH_FN(key, value) _Generic((key),	\
        string : stbds_hash_string_atom,					\
        struct ll_type* : ll_type_hash 						\
    )(key, value)

#define MAP_DEFAULT_EQL_FN(a, b) _Generic((a),				\
        string : string_atom_eql,						\
        struct ll_type* : ll_type_eql 						\
    )(a, b)

#define MAP_DEFAULT_SEED (16u)

#define STBDS_SIZE_T_BITS           ((sizeof (size_t)) * 8)
#define STBDS_ROTATE_LEFT(val, n)   (((val) << (n)) | ((val) >> (STBDS_SIZE_T_BITS - (n))))
#define STBDS_ROTATE_RIGHT(val, n)  (((val) >> (n)) | ((val) << (STBDS_SIZE_T_BITS - (n))))

size_t stbds_hash_string(string str, size_t seed);
size_t stbds_hash_string_atom(string str, size_t seed);
size_t stbds_siphash_bytes(void *p, size_t len, size_t seed);

Compiler_Context ll_compiler_context_create(void);
string ll_intern_string(Compiler_Context* cc, string str);
size_t ll_type_hash(struct ll_type* type, size_t seed);
bool ll_type_eql(struct ll_type* a, struct ll_type* b);

uint32_t log2_u32(uint32_t x);

size_t hash_combine(size_t lhs, size_t rhs);
