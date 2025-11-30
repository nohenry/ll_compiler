#pragma once

typedef struct ll_eval_value {
    union {
        int64_t  ival;
        uint64_t uval;
        double   fval;
    };
} LL_Eval_Value;

