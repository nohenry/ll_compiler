#pragma once

typedef struct ll_eval_value {
    union {
        int64_t  as_i64;
        uint64_t as_u64;
        double   as_f64;
    };
} LL_Eval_Value;

