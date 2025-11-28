#pragma once
#include <stdbool.h>
#include <stdarg.h>
#include <inttypes.h>

#if defined(_WIN32) || defined(_WIN64)
    #define OC_PLATFORM_WINDOWS 1
#else
    #define OC_PLATFORM_UNIX 1
#endif

#if __STDC_VERSION__ == 201710L
    // #define __typeof__(a) int
#endif

#if defined __STDC_VERSION__ && __STDC_VERSION__ >= 202311L
#else
    #define typeof __typeof__
#endif

typedef unsigned char          uint8;
typedef signed char            sint8;
typedef unsigned short int     uint16;
typedef signed short int       sint16;
typedef unsigned int           uint32;
typedef signed int             sint32;
typedef unsigned long long int uint64;
typedef signed long long int   sint64;
typedef unsigned long long int uword;
typedef signed long long int   sword;

typedef struct {
    char* ptr;
    uword len;
} string;

#if OC_PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include "Windows.h"
#elif OC_PLATFORM_UNIX
    #include <sys/mman.h>
    #include <unistd.h>
    #define MAP_UNINITIALIZED 0x4000000
#else
    #error "invalid platform"
#endif

#define typeinfo_gen_type_variants_ptr1(type) \
                   type*                : OC_TYPE_POINTER, \
    const          type*                : OC_TYPE_POINTER, \
    const volatile type*                : OC_TYPE_VOLATILE_POINTER, \
          volatile type*                : OC_TYPE_VOLATILE_POINTER, \
          
#if __STDC_VERSION__ >= 201112L
#define typeinfo_kind(x) _Generic((x),                 \
        char:               OC_TYPE_CHAR,              \
        unsigned char:      OC_TYPE_UNSIGNED_CHAR,     \
        unsigned short:     OC_TYPE_UNSIGNED_SHORT,    \
        short:              OC_TYPE_SHORT,             \
        unsigned int:       OC_TYPE_UNSIGNED_INT,      \
        int:                OC_TYPE_INT,               \
        unsigned long:      OC_TYPE_UNSIGNED_LONG,     \
        long:               OC_TYPE_LONG,              \
        unsigned long long: OC_TYPE_UNSIGNED_LONG_LONG,\
        long long:          OC_TYPE_LONG_LONG,         \
        float:              OC_TYPE_FLOAT,             \
        double:             OC_TYPE_DOUBLE,            \
        long double:        OC_TYPE_LONG_DOUBLE,       \
        char*:              OC_TYPE_CHAR_STRING,       \
        const char*:        OC_TYPE_CHAR_STRING,       \
        typeinfo_gen_type_variants_ptr1(unsigned char)      \
        typeinfo_gen_type_variants_ptr1(unsigned short)     \
        typeinfo_gen_type_variants_ptr1(short)              \
        typeinfo_gen_type_variants_ptr1(unsigned int)       \
        typeinfo_gen_type_variants_ptr1(int)                \
        typeinfo_gen_type_variants_ptr1(unsigned long)      \
        typeinfo_gen_type_variants_ptr1(long)               \
        typeinfo_gen_type_variants_ptr1(unsigned long long) \
        typeinfo_gen_type_variants_ptr1(long long)          \
        typeinfo_gen_type_variants_ptr1(float)              \
        typeinfo_gen_type_variants_ptr1(double)             \
        typeinfo_gen_type_variants_ptr1(long double)        \
        typeinfo_gen_type_variants_ptr1(void)        \
\
        char**:               OC_TYPE_POINTER_POINTER,               \
        unsigned char**:      OC_TYPE_POINTER_POINTER,               \
        unsigned short**:     OC_TYPE_POINTER_POINTER,               \
        short**:              OC_TYPE_POINTER_POINTER,               \
        unsigned int**:       OC_TYPE_POINTER_POINTER,               \
        int**:                OC_TYPE_POINTER_POINTER,               \
        unsigned long**:      OC_TYPE_POINTER_POINTER,               \
        long**:               OC_TYPE_POINTER_POINTER,               \
        unsigned long long**: OC_TYPE_POINTER_POINTER,               \
        long long**:          OC_TYPE_POINTER_POINTER,               \
        float**:              OC_TYPE_POINTER_POINTER,               \
        double**:             OC_TYPE_POINTER_POINTER,               \
        long double**:        OC_TYPE_POINTER_POINTER,               \
        void**:               OC_TYPE_POINTER_POINTER,               \
\
        string: OC_TYPE_STRING, \
        string*: OC_TYPE_POINTER, \
        string**: OC_TYPE_POINTER_POINTER, \
        default:  oc_assert(false && "unsupported type: "#x) \
    )
#else
#error "unsupported typeinfo"
#define typeinfo_kind(x) 0
#endif

typedef enum {
    OC_TYPE_CHAR,
    OC_TYPE_UNSIGNED_CHAR,
    OC_TYPE_SIGNED_CHAR,
    OC_TYPE_UNSIGNED_SHORT,
    OC_TYPE_SHORT,
    OC_TYPE_UNSIGNED_INT,
    OC_TYPE_INT,
    OC_TYPE_UNSIGNED_LONG,
    OC_TYPE_LONG,
    OC_TYPE_UNSIGNED_LONG_LONG,
    OC_TYPE_LONG_LONG,
    OC_TYPE_FLOAT,
    OC_TYPE_DOUBLE,
    OC_TYPE_LONG_DOUBLE,
    OC_TYPE_STRING,
    OC_TYPE_CHAR_STRING,
    OC_TYPE_POINTER,
    OC_TYPE_VOLATILE_POINTER,
    OC_TYPE_POINTER_VOLATILE,
    OC_TYPE_VOLATILE_POINTER_VOLATILE,
    OC_TYPE_POINTER_POINTER,
} Oc_Type_Kind;

typedef struct {
    Oc_Type_Kind kind;
} Oc_Type_Info;

typedef struct {
    Oc_Type_Kind kind;
    void* data;
} Oc_Generic;

typedef uword (*WriteFunction)(void* writer, const void* data, uword data_size);

typedef struct oc_writer {
    WriteFunction write;
} Oc_Writer;

typedef struct oc_arena_chunk {
    // this must be word aligned
    uword used, size;
    struct oc_arena_chunk* next;
    uword data[];
} Oc_Arena_Chunk;

typedef struct {
    Oc_Arena_Chunk* head;
    Oc_Arena_Chunk* current;
} Oc_Arena;

typedef struct {
    Oc_Arena_Chunk* chunk;
    uword used;
} Oc_Arena_Save;

typedef struct {
    Oc_Writer writer;
    Oc_Arena* arena;
    char* items;
    uword count, capacity;
} Oc_String_Builder;

#ifndef NULL
#define NULL     ((void*)0)
#endif
#ifndef INFINITY
#define INFINITY __builtin_inff()
#endif
#ifndef NAN
#define NAN      __builtin_nanf("")
#endif
#define lit(s)   ((string){ s, sizeof(s)-1 })

#ifndef max
#define max(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); _a > _b ? _a : _b; })
#endif
#ifndef min
#define min(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); _a < _b ? _a : _b; })
#endif

#define _oc_generic_tmp(___y) typeof(_Generic((___y), float: (float)0, double: (double)0, long double: (long double)0, string: ((string){0}), default: (uword)0))
#define _oc_generic_cast(x) _Generic((x), string: (x), default: (_oc_generic_tmp(x))(x))

#if __STDC_VERSION__ == 201710L
// #define OC_MAKE_GENERIC(x) ((Oc_Generic){ typeinfo_kind(x), NULL })
#else
#define OC_MAKE_GENERIC(x) ((Oc_Generic){ typeinfo_kind(x), ({ _oc_generic_tmp(x) ___p = _oc_generic_cast(x); &___p; }) })
#endif

#define OC_VA_NARGS(...) OC_VA_NARGS_IMPL(, ##__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define OC_VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, N, ...) N

#define OC_CONCAT(a, b) OC_CONCAT_IMPL(a, b)
#define OC_CONCAT_IMPL(a, b) a##b

// Map each argument to a transformation macro
#define OC_MAP(transform_macro, ...) OC_CONCAT(OC_MAP_, OC_VA_NARGS(__VA_ARGS__))(transform_macro, __VA_ARGS__)
// #define OC_MAP(transform_macro, ...) OC_VA_NARGS(__VA_ARGS__)
#define OC_MAP_0(transform_macro, ...)
#define OC_MAP_1(transform_macro, a1) , transform_macro(a1)
#define OC_MAP_2(transform_macro, a1, a2) , transform_macro(a1), transform_macro(a2)
#define OC_MAP_3(transform_macro, a1, a2, a3) , transform_macro(a1), transform_macro(a2), transform_macro(a3)
#define OC_MAP_4(transform_macro, a1, a2, a3, a4) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4)
#define OC_MAP_5(transform_macro, a1, a2, a3, a4, a5) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5)
#define OC_MAP_6(transform_macro, a1, a2, a3, a4, a5, a6) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5), transform_macro(a6)
#define OC_MAP_7(transform_macro, a1, a2, a3, a4, a5, a6, a7) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5), transform_macro(a6), transform_macro(a7)
#define OC_MAP_8(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5), transform_macro(a6), transform_macro(a7), transform_macro(a8)
#define OC_MAP_9(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5), transform_macro(a6), transform_macro(a7), transform_macro(a8), transform_macro(a9)
#define OC_MAP_10(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5), transform_macro(a6), transform_macro(a7), transform_macro(a8), transform_macro(a9), transform_macro(a10)
#define OC_MAP_11(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5), transform_macro(a6), transform_macro(a7), transform_macro(a8), transform_macro(a9), transform_macro(a10), transform_macro(a11)
#define OC_MAP_12(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5), transform_macro(a6), transform_macro(a7), transform_macro(a8), transform_macro(a9), transform_macro(a10), transform_macro(a11), transform_macro(a12)
#define OC_MAP_13(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5), transform_macro(a6), transform_macro(a7), transform_macro(a8), transform_macro(a9), transform_macro(a10), transform_macro(a11), transform_macro(a12), transform_macro(a13)
#define OC_MAP_14(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5), transform_macro(a6), transform_macro(a7), transform_macro(a8), transform_macro(a9), transform_macro(a10), transform_macro(a11), transform_macro(a12), transform_macro(a13), transform_macro(a14)
#define OC_MAP_15(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5), transform_macro(a6), transform_macro(a7), transform_macro(a8), transform_macro(a9), transform_macro(a10), transform_macro(a11), transform_macro(a12), transform_macro(a13), transform_macro(a14), transform_macro(a15)
#define OC_MAP_16(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) , transform_macro(a1), transform_macro(a2), transform_macro(a3), transform_macro(a4), transform_macro(a5), transform_macro(a6), transform_macro(a7), transform_macro(a8), transform_macro(a9), transform_macro(a10), transform_macro(a11), transform_macro(a12), transform_macro(a13), transform_macro(a14), transform_macro(a15), transform_macro(a16)

#define OC_MAP_SEQ(transform_macro, ...) OC_CONCAT(OC_MAP_SEQ_, OC_VA_NARGS(__VA_ARGS__))(transform_macro, ##__VA_ARGS__)
#define OC_MAP_SEQ_0(transform_macro)
#define OC_MAP_SEQ_1(transform_macro, a1) transform_macro(a1)
#define OC_MAP_SEQ_2(transform_macro, a1, a2) transform_macro(a1) transform_macro(a2)
#define OC_MAP_SEQ_3(transform_macro, a1, a2, a3) transform_macro(a1) transform_macro(a2) transform_macro(a3)
#define OC_MAP_SEQ_4(transform_macro, a1, a2, a3, a4) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4)
#define OC_MAP_SEQ_5(transform_macro, a1, a2, a3, a4, a5) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5)
#define OC_MAP_SEQ_6(transform_macro, a1, a2, a3, a4, a5, a6) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5) transform_macro(a6)
#define OC_MAP_SEQ_7(transform_macro, a1, a2, a3, a4, a5, a6, a7) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5) transform_macro(a6) transform_macro(a7)
#define OC_MAP_SEQ_8(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5) transform_macro(a6) transform_macro(a7) transform_macro(a8)
#define OC_MAP_SEQ_9(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5) transform_macro(a6) transform_macro(a7) transform_macro(a8) transform_macro(a9)
#define OC_MAP_SEQ_10(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5) transform_macro(a6) transform_macro(a7) transform_macro(a8) transform_macro(a9) transform_macro(a10)
#define OC_MAP_SEQ_11(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5) transform_macro(a6) transform_macro(a7) transform_macro(a8) transform_macro(a9) transform_macro(a10) transform_macro(a11)
#define OC_MAP_SEQ_12(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5) transform_macro(a6) transform_macro(a7) transform_macro(a8) transform_macro(a9) transform_macro(a10) transform_macro(a11) transform_macro(a12)
#define OC_MAP_SEQ_13(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5) transform_macro(a6) transform_macro(a7) transform_macro(a8) transform_macro(a9) transform_macro(a10) transform_macro(a11) transform_macro(a12) transform_macro(a13)
#define OC_MAP_SEQ_14(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5) transform_macro(a6) transform_macro(a7) transform_macro(a8) transform_macro(a9) transform_macro(a10) transform_macro(a11) transform_macro(a12) transform_macro(a13) transform_macro(a14)
#define OC_MAP_SEQ_15(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5) transform_macro(a6) transform_macro(a7) transform_macro(a8) transform_macro(a9) transform_macro(a10) transform_macro(a11) transform_macro(a12) transform_macro(a13) transform_macro(a14) transform_macro(a15)
#define OC_MAP_SEQ_16(transform_macro, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) transform_macro(a1) transform_macro(a2) transform_macro(a3) transform_macro(a4) transform_macro(a5) transform_macro(a6) transform_macro(a7) transform_macro(a8) transform_macro(a9) transform_macro(a10) transform_macro(a11) transform_macro(a12) transform_macro(a13) transform_macro(a14) transform_macro(a15) transform_macro(a16)



/* --------  macro api -------- */

#define OC_DEFAULT_MAP_ENTRY_COUNT 512
#define OC_DEFAULT_MAP_SEED 0xf8abc103ba79eb85LLu
#define OC_ARENA_CHUNK_SIZE (4096)

#define oc_assert(expr) ((expr) ? 1 : _oc_assert_fail(#expr, __FILE__, __LINE__, __func__))
#define oc_unreachable(expr) ((expr) ? 1 : _oc_assert_fail(#expr, __FILE__, __LINE__, __func__))
#define wprint(writer, fmt, ...) _oc_printw((writer), fmt OC_MAP(OC_MAKE_GENERIC, __VA_ARGS__))
#define print(fmt, ...) _oc_printw(&stdout_writer, fmt OC_MAP(OC_MAKE_GENERIC, __VA_ARGS__))
#define eprint(fmt, ...) _oc_printw(&stderr_writer, fmt OC_MAP(OC_MAKE_GENERIC, __VA_ARGS__))
#define oc_todo(fmt, ...) do { print("oc_todo - {}:{} - ", __FILE__, __LINE__); oc_exit(-1); } while (0)
#define oc_len(arr) (sizeof(arr)/sizeof((arr)[0]))
#define oc_pun(value, type) ({ __typeof__(value) _v = (value); *(type*)&_v; })
#define oc_oom() do { print("Out of memory: {}:{}\n", __FILE__, __LINE__); oc_exit(-1); } while (0)
#define oc_array_append(arena, array, value)                                                     \
    do {                                                                                         \
        if ((array)->count + 1 > (array)->capacity) {                                            \
            uword new_cap = (array)->capacity ? (array)->capacity * 2 : 16;                      \
            void* new_ptr = oc_arena_realloc(arena, (array)->items, (array)->capacity, new_cap); \
            (array)->items = new_ptr;                                                            \
            (array)->capacity = new_cap;                                                         \
        }                                                                                        \
        (array)->items[(array)->count++] = value;                                                \
    } while (0)
#define oc_array_append_many(arena, array, ptr, len)                                             \
    do {                                                                                         \
        if ((array)->count + (len) > (array)->capacity) {                                            \
            uword new_cap = (array)->capacity ? ((array)->count + (len) + (array)->capacity * 2) : (((array)->count + (len)) * 8); \
            void* new_ptr = oc_arena_realloc(arena, (array)->items, (array)->capacity, new_cap); \
            (array)->items = new_ptr;                                                            \
            (array)->capacity = new_cap;                                                         \
        }                                                                                        \
        memcpy((array)->items + (array)->count, ptr, len);  \
        (array)->count += len;                              \
    } while (0)
#define oc_array_reserve(arena, array, _count)                                                     \
    do {                                                                                           \
        if ((_count) > (array)->capacity) {                                                        \
            uword new_cap = (_count) * 2;                                                           \
            void* new_ptr = oc_arena_realloc(arena, (array)->items, (array)->capacity, new_cap);   \
            (array)->items = new_ptr;                                                              \
            (array)->capacity = new_cap;                                                           \
        }                                                                                          \
        (array)->count = (_count);                                                                 \
    } while (0)

static inline uword oc_align_forward(uword value, uword alignment_in_bytes) {
    return (value + alignment_in_bytes - 1) & ~(alignment_in_bytes - 1);
}

// start is clamped to zero if negative
// end < 0 means to use the end of s
static inline string string_slice(string s, sword start, sword end) {
    if (start < 0) start = 0;
    if (end < 0) end = s.len;

    s.ptr += start;
    s.len = end - start;
    return s;
}

// start is clamped to zero if negative
static inline string string_slice_count(string s, sword start, sword count) {
    if (start < 0) start = 0;

    s.ptr += start;
    s.len = count;
    return s;
}

/* --------    libc forwards  -------- */
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
size_t strlen(const char *s);
_Noreturn void exit(int status);

/* -------- Platform Specific -------- */
void* oc_allocate_pages(uword required_size);
uword stdout_write(void* writer, const uint8* data, uword data_size);
uword stderr_write(void* writer, const uint8* data, uword data_size);

/* --------     Others        -------- */
_Noreturn int _oc_assert_fail(const char *assertion, const char *file, unsigned int line, const char *function);
Oc_Arena_Chunk* oc_arena_new_chunk(Oc_Arena* arena, uword size_in_bytes);
Oc_Arena_Save oc_arena_save(Oc_Arena* arena);
void oc_arena_restore(Oc_Arena* arena, Oc_Arena_Save restore_point);
void oc_arena_reset(Oc_Arena* arena);
void* oc_arena_alloc(Oc_Arena* arena, uint64 size);
void* oc_arena_realloc(Oc_Arena* arena, void* old_ptr, uint64 old_size, uint64 size);
void* oc_arena_dup(Oc_Arena* arena, void* data, uword size);
string oc_sprintf(Oc_Arena* arena, const char* fmt, ...);
void oc_sb_append_char(Oc_String_Builder* sb, char c);
void oc_sb_append_string(Oc_String_Builder* sb, string s);
void oc_sb_append_char_str(Oc_String_Builder* sb, const char* c);
void oc_sb_append_char_str_len(Oc_String_Builder* sb, const char* c, uword len);
string oc_sb_to_string(Oc_String_Builder* sb);
uword oc_sb_writer_write(void* writer, const void* data, uword data_size);
void oc_sb_init(Oc_String_Builder* sb, Oc_Arena* arena);
void oc_writer_format_and_write_int(Oc_Writer *writer, uint64 ivalue, uint64 base);
void oc_writer_format_and_write_float(Oc_Writer *writer, double fvalue);
void _oc_printw(void *writer, const char* fmt, ...);
void _oc_vprintw(void *writer, const char* fmt, va_list args);
_Noreturn void oc_exit(int status);

extern Oc_Writer stdout_writer;
extern Oc_Writer stderr_writer;

#ifdef OC_CORE_IMPLEMENTATION
    #ifdef OC_PLATFORM_WINDOWS
        void* oc_allocate_pages(uword required_size) {
            return VirtualAlloc(NULL, required_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        }

        uword stdout_write(void* writer, const uint8* data, uword data_size) {
            HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
            uword total_written = 0;

            while (total_written < data_size) {
                uint32 written = 0;
                if (!WriteFile(handle, data, data_size, (unsigned long*)&written, NULL)) {
                    eprint("WriteFile failed\n");
                    return 0;
                }
                total_written += written;
            }
            return total_written;
        }

        uword stderr_write(void* writer, const uint8* data, uword data_size) {
            HANDLE handle = GetStdHandle(STD_ERROR_HANDLE);
            uword total_written = 0;

            while (total_written < data_size) {
                uint32 written = 0;
                if (!WriteFile(handle, data, data_size, (unsigned long*)&written, NULL)) {
                    eprint("WriteFile failed\n");
                    return 0;
                }
                total_written += written;
            }
            return total_written;
        }

        _Noreturn void oc_exit(int status)  {
            ExitProcess(status);
        }
    #else
        void* oc_allocate_pages(uword required_size) {
            return mmap(NULL, required_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_UNINITIALIZED, -1, 0);
        }

        uword stdout_write(void* writer, const uint8* data, uword data_size) {
            return write(STDOUT_FILENO, data, (size_t)data_size);
        }

        uword stderr_write(void* writer, const uint8* data, uword data_size) {
            return write(STDERR_FILENO, data, (size_t)data_size);
        }

        _Noreturn void oc_exit(int status)  {
            exit(status);
        }
    #endif


_Noreturn int _oc_assert_fail(const char *assertion, const char *file, unsigned int line, const char *function)  {
    oc_exit(-1);
}

Oc_Arena_Chunk* oc_arena_new_chunk(Oc_Arena* arena, uword size_in_bytes) {
    // Oc_Arena_Chunk* chunk = malloc(OC_ARENA_CHUNK_SIZE /* should be word aligned */);
    uword aligned_bytes = oc_align_forward(size_in_bytes, OC_ARENA_CHUNK_SIZE);
    Oc_Arena_Chunk* chunk = oc_allocate_pages(aligned_bytes);
    if (arena->current && chunk == (void*)(arena->current->data + chunk->size)) {
        // if new chunk is right after current chunk, just extend current chunk
        chunk->size += aligned_bytes;
        return arena->current;
    } else {
        chunk->used = sizeof(Oc_Arena_Chunk) / sizeof(uword);
        chunk->size = (aligned_bytes - sizeof(Oc_Arena_Chunk)) / sizeof(uword);
        chunk->next = NULL;
        return chunk;
    }
}

Oc_Arena_Save oc_arena_save(Oc_Arena* arena) {
    return (Oc_Arena_Save) {
        .chunk = arena->current,
        .used = arena->current->used,
    };
}

void oc_arena_restore(Oc_Arena* arena, Oc_Arena_Save restore_point) {
    arena->current = restore_point.chunk;
    arena->current->used = restore_point.used;
}

void oc_arena_reset(Oc_Arena* arena) {
    arena->current = arena->head;
    arena->current->used = sizeof(Oc_Arena_Chunk) / sizeof(uword);
}

void* oc_arena_alloc(Oc_Arena* arena, uint64 size) {
    oc_assert(size != 0);
    // 1 -> 0 -> 0 -> 1
    // 2 -> 1 -> 0 -> 1
    // 7 -> 6 -> 0 -> 1
    // 8 -> 7 -> 0 -> 1
    // 9 -> 8 -> 1 -> 2
    uword words = (size - 1) / sizeof(uword) + 1;
    if (!arena->current) {
        arena->head = arena->current = oc_arena_new_chunk(arena, size);
        if (!arena->head) oc_oom();
    } else if (arena->current->used + words > arena->current->size) {
        if (arena->current->next == NULL) {
            arena->current->next = oc_arena_new_chunk(arena, size);
            if (!arena->current->next) oc_oom();
        }
        arena->current = arena->current->next;
    }

    void* result = arena->current->data + arena->current->used;
    arena->current->used += words;
    return result;
}

void* oc_arena_realloc(Oc_Arena* arena, void* old_ptr, uint64 old_size, uint64 size) {
    oc_assert(arena != NULL);
    // oc_assert(old_ptr != NULL);
    if (old_ptr == NULL)  return oc_arena_alloc(arena, size);
    if (size <= old_size) return old_ptr;

    uword old_words = (old_size - 1) / sizeof(uword) + 1;
    uword new_words = (size - 1) / sizeof(uword) + 1;

    // print("oc_arena_realloc: old_ptr = {}, old_size = {}, size = {}\n", old_ptr, old_size, size);
    oc_assert(arena->current);
    if ((uint8*)old_ptr + old_size == (uint8*)(arena->current->data + arena->current->used)) {
        if (arena->current->used + (new_words - old_words) <= arena->current->size) {
            // print("oc_arena_realloc: Reusing memory :)\n");
            // reuse memory
            arena->current->used += new_words - old_words;
            return old_ptr;
        }
    } else if (arena->current->used + new_words <= arena->current->size) {
        // print("oc_arena_realloc: Reusing chunk, but copying old data :|\n");
        // copy old data
        void* new_ptr = arena->current->data + arena->current->used;
        memcpy(new_ptr, old_ptr, old_size);
        arena->current->used += new_words;
        return new_ptr;
    }


    void* new_ptr = oc_arena_alloc(arena, size);
    if (!new_ptr) oc_oom();

    void* end_of_last_used = arena->current->data + arena->current->used;
    if ((uint8*)old_ptr + old_size == end_of_last_used && new_ptr == (uint8*)old_ptr + old_size) {
        // print("oc_arena_realloc: Reusing extended chunk :)\n");
        // If the new pointer is right after the old pointer, we can just extend it
        arena->current->used += new_words - old_words;
        return old_ptr;
    } else {
        // print("oc_arena_realloc: New chunk, copying data :(\n");
        void* new_ptr = arena->current->data + arena->current->used;
        memcpy(new_ptr, old_ptr, old_size);
        arena->current->used += new_words;
        return new_ptr;
    }
}

void* oc_arena_dup(Oc_Arena* arena, void* data, uword size) {
    void* new_ptr = oc_arena_alloc(arena, size);
    memcpy(new_ptr, data, size);
    return new_ptr;
}

string oc_sprintf(Oc_Arena* arena, const char* fmt, ...) {
    Oc_String_Builder b;
    b.arena = arena;

    va_list args;
    va_start(args, fmt);
    _oc_vprintw(&b.writer, fmt, args);
    va_end(args);

    return oc_sb_to_string(&b);
}

void oc_sb_append_char(Oc_String_Builder* sb, char c) {
    oc_assert(sb->arena != NULL);
    if (sb->count + 1 > sb->capacity) {
        // Resize the buffer
        uword new_cap = sb->capacity ? sb->capacity * 2 : 16;
        char* new_ptr = oc_arena_realloc(sb->arena, sb->items, sb->capacity, new_cap);
        if (!new_ptr) oc_oom();
        sb->items = new_ptr;
        sb->capacity = new_cap;
    }
    sb->items[sb->count++] = c;
}

void oc_sb_append_string(Oc_String_Builder* sb, string s) {
    oc_assert(sb->arena != NULL);
    if (sb->count + s.len > sb->capacity) {
        // Resize the buffer
        uword new_cap = sb->capacity ? max(s.len, sb->capacity * 2) : 16;
        char* new_ptr = oc_arena_realloc(sb->arena, sb->items, sb->capacity, new_cap);
        if (!new_ptr) oc_oom();
        sb->items = new_ptr;
        sb->capacity = new_cap;
    }
    memcpy(sb->items + sb->count, s.ptr, s.len);
    sb->count += s.len;
}

void oc_sb_append_char_str(Oc_String_Builder* sb, const char* c) {
    oc_assert(sb->arena != NULL);
    uword len = strlen(c);
    oc_sb_append_string(sb, (string) { .ptr = (char*)c, .len = len });
}

void oc_sb_append_char_str_len(Oc_String_Builder* sb, const char* c, uword len) {
    oc_assert(sb->arena != NULL);
    oc_sb_append_string(sb, (string) { .ptr = (char*)c, .len = len });
}

string oc_sb_to_string(Oc_String_Builder* sb) {
    oc_sb_append_char(sb, 0);
    return (string) { .ptr = sb->items, .len = sb->count - 1 };
}


uword oc_sb_writer_write(void* writer, const void* data, uword data_size) {
    Oc_String_Builder* w = writer;
    oc_sb_append_string(w, (string) { .ptr = (char*)data, .len = data_size });
    return data_size;
}

void oc_sb_init(Oc_String_Builder* sb, Oc_Arena* arena) {
    memset(sb, 0, sizeof(Oc_String_Builder));
    sb->writer.write = oc_sb_writer_write;
    sb->arena = arena;
}

Oc_Writer stdout_writer = {
    .write = (WriteFunction)stdout_write,
};

Oc_Writer stderr_writer = {
    .write = (WriteFunction)stderr_write,
};

void oc_writer_format_and_write_int(Oc_Writer *writer, uint64 ivalue, uint64 base) {
    char int_buffer[128];
    uword int_buffer_offset = 0;

    int_buffer_offset = sizeof(int_buffer) - 1;
    if (ivalue == 0) {
        writer->write(writer, "0", 1);
    } else {
        uint64 value = ivalue;
        while (value) {
            uint64 current = value % base;
            char c = current < 10 ? (current + '0') : (current - 10 + 'a');
            int_buffer[int_buffer_offset--] = c;
            value /= base;
        }
        writer->write(writer, int_buffer + int_buffer_offset + 1, sizeof(int_buffer) - int_buffer_offset - 1);
    }
}

void oc_writer_format_and_write_float(Oc_Writer *writer, double fvalue) {
    char int_buffer[128];
    uword int_buffer_offset = 0;

    int_buffer_offset = sizeof(int_buffer) - 1;
    uint64 ivalue = fvalue;
    double dvalue = fvalue - ivalue;

    if (ivalue == 0) {
        writer->write(writer, "0.0", 1);
    } else {
        while (ivalue) {
            uint64 current = ivalue % 10;
            char c = current < 10 ? (current + '0') : (current - 10 + 'a');
            int_buffer[int_buffer_offset--] = c;
            ivalue /= 10;
        }
        writer->write(writer, int_buffer + int_buffer_offset + 1, sizeof(int_buffer) - int_buffer_offset - 1);
    }

    uint64 decimal_places = 6;
    double e = 0.5;
    for (int i = 0; i < decimal_places; ++i) e /= 10.0;
    dvalue += e;

    uint64 multiply = 10;
    double shifted = dvalue;
    double acc = 0.0;
    int_buffer_offset = 0;
    int_buffer[int_buffer_offset++] = '.';
    if ((dvalue - acc) > e) {
        while ((dvalue - acc) > e) {
            double d = shifted * 10;
            uint64 i = (uint64)d;
            acc += (double)i / (double)multiply;
            int_buffer[int_buffer_offset++] = i + '0';
            shifted = d - i;
            multiply *= 10;
        }
    } else {
        int_buffer[int_buffer_offset++] = '0';
    }
    writer->write(writer, int_buffer, int_buffer_offset);
}

void _oc_printw(void *writer, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    _oc_vprintw(writer, fmt, args);
    va_end(args);
}

void _oc_vprintw(void *writer, const char* fmt, va_list args) {
    Oc_Writer* w = writer;

    while (*fmt) {
        char c = *fmt;
        if (c == '{') {
            Oc_Generic value = va_arg(args, Oc_Generic);

            while (*fmt && *fmt != '}') fmt++;

            switch (value.kind) {
            case OC_TYPE_CHAR:
                w->write(w, (uint8*)value.data, 1);
                break;
            case OC_TYPE_UNSIGNED_CHAR: {
                int ivalue = *(unsigned char*)value.data;
                oc_writer_format_and_write_int(w, ivalue, 10);
            } break;
            case OC_TYPE_SIGNED_CHAR: {
                int ivalue = *(signed char*)value.data;
                if (ivalue < 0) {
                    ivalue = -ivalue;
                    w->write(w, "-", 1);
                }
                oc_writer_format_and_write_int(w, ivalue, 10);
            } break;
            case OC_TYPE_UNSIGNED_SHORT: {
                int ivalue = *(unsigned short*)value.data;
                oc_writer_format_and_write_int(w, ivalue, 10);
            } break;
            case OC_TYPE_SHORT: {
                int ivalue = *(signed short*)value.data;
                if (ivalue < 0) {
                    ivalue = -ivalue;
                    w->write(w, "-", 1);
                }
                oc_writer_format_and_write_int(w, ivalue, 10);
            } break;
            case OC_TYPE_UNSIGNED_INT: {
                int ivalue = *(unsigned int*)value.data;
                oc_writer_format_and_write_int(w, ivalue, 10);
            } break;
            case OC_TYPE_INT: {
                int ivalue = *(signed int*)value.data;
                if (ivalue < 0) {
                    ivalue = -ivalue;
                    w->write(w, "-", 1);
                }
                oc_writer_format_and_write_int(w, ivalue, 10);
            } break;
            case OC_TYPE_UNSIGNED_LONG: {
                uint64 ivalue = *(unsigned long*)value.data;
                oc_writer_format_and_write_int(w, ivalue, 10);
            } break;
            case OC_TYPE_LONG: {
                sint64 ivalue = *(signed long*)value.data;
                if (ivalue < 0) {
                    ivalue = -ivalue;
                    w->write(w, "-", 1);
                }
                oc_writer_format_and_write_int(w, ivalue, 10);
            } break;
            case OC_TYPE_UNSIGNED_LONG_LONG: {
                uint64 ivalue = *(unsigned long long*)value.data;
                oc_writer_format_and_write_int(w, ivalue, 10);
            } break;
            case OC_TYPE_LONG_LONG: {
                sint64 ivalue = *(signed long long*)value.data;
                if (ivalue < 0) {
                    ivalue = -ivalue;
                    w->write(w, "-", 1);
                }
                oc_writer_format_and_write_int(w, ivalue, 10);
            } break;
            case OC_TYPE_FLOAT: {
                float dvalue = *(float*)value.data;
                if (__builtin_signbit(dvalue)) {
                    dvalue = -dvalue;
                    w->write(w, "-", 1);
                }
                if (__builtin_isinf(dvalue)) {
                    w->write(w, "inf", 3);
                    break;
                } else if (__builtin_isnan(dvalue)) {
                    w->write(w, "nan", 3);
                    break;
                }
                oc_writer_format_and_write_float(w, dvalue);
            } break;
            case OC_TYPE_DOUBLE: {
                double dvalue = *(double*)value.data;
                if (__builtin_signbit(dvalue)) {
                    dvalue = -dvalue;
                    w->write(w, "-", 1);
                }
                if (__builtin_isinf(dvalue)) {
                    w->write(w, "inf", 3);
                    break;
                } else if (__builtin_isnan(dvalue)) {
                    w->write(w, "nan", 3);
                    break;
                }
                oc_writer_format_and_write_float(w, dvalue);
            } break;
            case OC_TYPE_LONG_DOUBLE: {
                long double dvalue = *(long double*)value.data;
                if (__builtin_signbit(dvalue)) {
                    dvalue = -dvalue;
                    w->write(w, "-", 1);
                }
                if (__builtin_isinf(dvalue)) {
                    w->write(w, "inf", 3);
                    break;
                } else if (__builtin_isnan(dvalue)) {
                    w->write(w, "nan", 3);
                    break;
                }
                oc_writer_format_and_write_float(w, dvalue);
            } break;
            case OC_TYPE_STRING: {
                string* str = (string*)value.data;
                w->write(w, str->ptr, str->len);
            } break;
            case OC_TYPE_CHAR_STRING: {
                const char* str = *(const char**)value.data;
                w->write(w, str, strlen(str));
            } break;
            case OC_TYPE_POINTER: {
                uint64 ivalue = *(uword*)value.data;
                w->write(w, "0x", 2);
                oc_writer_format_and_write_int(w, ivalue, 16);
            } break;
            case OC_TYPE_VOLATILE_POINTER: {
                uint64 ivalue = *(volatile uword*)value.data;
                w->write(w, "0x", 2);
                oc_writer_format_and_write_int(w, ivalue, 16);
            } break;
            case OC_TYPE_POINTER_VOLATILE: {
                uint64 ivalue = *(volatile uword*)value.data;
                w->write(w, "0x", 2);
                oc_writer_format_and_write_int(w, ivalue, 16);
            } break;
            case OC_TYPE_VOLATILE_POINTER_VOLATILE: {
                uint64 ivalue = *(volatile uword* volatile)value.data;
                w->write(w, "0x", 2);
                oc_writer_format_and_write_int(w, ivalue, 16);
            } break;
            case OC_TYPE_POINTER_POINTER: {
                uint64 ivalue = *(uword*)value.data;
                w->write(w, "0x", 2);
                oc_writer_format_and_write_int(w, ivalue, 16);
            } break;
            }

        } else {
            w->write(w, (uint8*)fmt, 1);
        }
        fmt++;
    }
}

#endif // OC_CORE_IMPLEMENTATION
