
#include "eval.h"

/* typedef enum { */
/* 	LL_EVAL_KIND_INT, */
/* } LL_Eval_Kind; */

typedef struct {
	union {
		int64_t iuval;
		uint64_t uval;
	};
} LL_Eval_Value;


void ll_eval_expression(Compiler_Context* cc, void* b, LL_Backend_Ir* bir) {
	switch ()
}


