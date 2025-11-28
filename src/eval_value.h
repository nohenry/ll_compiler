#pragma once

typedef struct ll_eval_value {
    union {
        int64_t  ival;
        uint64_t uval;
    };
} LL_Eval_Value;

