
#include "typer.h"
#include "typer2.h"
#include "parser.h"

LL_Typer2 ll_typer2_create(Compiler_Context* cc) {
    LL_Typer2 result = { 0 };
    LL_Typer2* typer = &result;

// #undef INSERT_BUILTIN_TYPE
	return result;
}

// #include <arm_neon.h>

void ll_typer2_run(Compiler_Context* cc, LL_Typer2* typer, LL_Parser* parser, Code* root) {
    uint32* old_counts = alloca(oc_len(parser->linear_grid));

    // for (uword i = 0; i < oc_len(parser->linear_grid); ++i) {
    //     uword aligned = oc_align_forward((uword)parser->linear_grid[i].types.count, 16);
    //     uword need_to_append = aligned - (uword)parser->linear_grid[i].types.count;
    //     parser_extend_zeroed_typecheck_value(cc, &parser->linear_grid[i], need_to_append);
    //     old_counts[i] = parser->linear_grid[i].types.count;
    // }

    // uint16x8_t zeros = vdupq_n_u16(0);
    // uint16x8_t ones = vdupq_n_u16(1);

    // assign
    for (uint32 i = 0; i < parser->ops->count; i += 1) {
        // uint16 lhs_type = 1 << parser->ops_lhs->types.items[i];
        // uint16 mask = parser->ops_rhs->types_implicit_cast.items[i];
        // uint16 good = lhs_type & mask;

        // if (good) {
        //     parser->ops->types.items[i] = parser->ops_lhs->types.items[i];
        //     parser->ops->types_cast.items[i] = parser->ops_lhs->types_cast.items[i];
        //     parser->ops->types_implicit_cast.items[i] = parser->ops_lhs->types_implicit_cast.items[i];
        // } else {
        //     ll_typer_report_error(((LL_Error){}), "Can't assign value to variable");

        //     ll_typer_report_error_no_src("    variable is declared with type ");
        //     ll_typer_report_error_type(cc, typer, &parser->ops->types.items[i]);
        //     ll_typer_report_error_no_src(", but tried to initialize it with type ");
        //     ll_typer_report_error_type(cc, typer, &parser->ops_rhs->types.items[i]);
        //     ll_typer_report_error_no_src("\n");

        //     ll_typer_report_error_done(cc, typer);
        // }

        // uint16x8_t lhs_type = vld1q_u16(&parser->ops_lhs->types.items[i]);
        // uint16x8_t lhs_cast = vld1q_u16(&parser->ops_lhs->types_cast.items[i]);
        // uint16x8_t lhs_implicit_cast = vld1q_u16(&parser->ops_lhs->types_implicit_cast.items[i]);
        // lhs_type = vshlq_u16(ones, lhs_type);

        // uint16x8_t mask = vld1q_u16(&parser->ops_rhs->types_implicit_cast.items[i]);
        // uint16x8_t good = vandq_u16(lhs_type, mask);

        // uint16x8_t rhs_type = vld1q_u16(&parser->ops_rhs->types.items[i]);
        // uint16x8_t rhs_cast = vld1q_u16(&parser->ops_rhs->types_cast.items[i]);

        // lhs_type = vbslq_u16(good, lhs_type, rhs_type);
        // lhs_cast = vbslq_u16(good, lhs_cast, rhs_cast);
        // lhs_implicit_cast = vbslq_u16(good, lhs_implicit_cast, mask);

        // vst1q_u16(&parser->ops->types.items[i], lhs_type);
        // vst1q_u16(&parser->ops->types_cast.items[i], lhs_cast);
        // vst1q_u16(&parser->ops->types_implicit_cast.items[i], lhs_implicit_cast);
    }

    // for (uword i = 0; i < oc_len(parser->linear_grid); ++i) {
    //     parser->linear_grid[i].types.count = old_counts[i];
    //     parser->linear_grid[i].types_cast.count = old_counts[i];
    //     parser->linear_grid[i].types_implicit_cast.count = old_counts[i];
    // }
}



