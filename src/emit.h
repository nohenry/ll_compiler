#pragma once

#include "common.h"
#include "../core/core1.h"
#include "ast.h"

typedef struct {
    Oc_String_Builder sb;
} LL_Emitter;

LL_Emitter ll_emitter_create(Compiler_Context* cc);
void ll_emitter_run(Compiler_Context* cc, LL_Emitter *e, Code_Scope* code);
void ll_emitter_emit_statement(Compiler_Context* cc, LL_Emitter *e, Oc_Writer* w, int indent, bool do_indent, Code* code);
void ll_emitter_emit_expression(Compiler_Context* cc, LL_Emitter *e, Oc_Writer* w, int indent, Code* code);

