#include "emit.h"

LL_Emitter ll_emitter_create(Compiler_Context* cc) {
    LL_Emitter emitter = { 0 };
    oc_sb_init(&emitter.sb, &cc->arena);
    return emitter;
}

void ll_emitter_run(Compiler_Context* cc, LL_Emitter *e, Code_Scope* file) {
    ll_emitter_emit_statement(cc, e, &e->sb.writer, 0, true, (Code*)file);
}

void ll_emitter_emit_statement(Compiler_Context* cc, LL_Emitter *e, Oc_Writer* w, int indent, bool do_indent, Code* stmt) {
    #define INDENT() do { if (do_indent) { for (int iii = 0 ; iii < indent; ++iii) wprint(w, "    "); } else { do_indent = true; } } while (0)

    switch (stmt->kind) {
    case CODE_KIND_FOR_SCOPE: {
        Code_Scope* scp = CODE_AS(stmt, Code_Scope);
        for (uint32 i = 0; i < scp->statements.count; ++i) {
            if (scp->statements.items[i]->kind != CODE_KIND_FOR) continue;
            ll_emitter_emit_statement(cc, e, &e->sb.writer, indent, true, scp->statements.items[i]);
        }
    } break;
    case CODE_KIND_BLOCK: {
        Code_Scope* scp = CODE_AS(stmt, Code_Scope);
        INDENT();
        wprint(w, "{{\n");
        for (uint32 i = 0; i < scp->declarations.capacity; ++i) {
            if (scp->declarations.entries[i].filled) {
                // if (scp->declarations.entries[i]._value->base.kind == ODE_) {
                // if (scp->declarations.entries[i]._value->base.kind == CODE_KIND_VARIABLE_DECLARATION) {
                //     if (CODE_AS(scp->declarations.entries[i]._value, Code_Variable_Declaration)->initializer) {
                //     } else {
                //         ll_emitter_emit_statement(cc, e, &e->sb.writer, indent + 1, true, (Code*)scp->declarations.entries[i]._value);
                //         wprint(w, "\n");
                //     }
                // } else {
                //     ll_emitter_emit_statement(cc, e, &e->sb.writer, indent + 1, true, (Code*)scp->declarations.entries[i]._value);
                //     wprint(w, "\n");
                // }
            }
        }
        for (uint32 i = 0; i < scp->statements.count; ++i) {
            if (scp->statements.items[i]->kind == CODE_KIND_PARAMETER) {
                Code_Parameter* param = CODE_AS(scp->statements.items[i], Code_Parameter);
                if (param->flags & LL_STORAGE_CLASS_STATIC) {}
                else continue;
            }

            ll_emitter_emit_statement(cc, e, &e->sb.writer, indent + 1, true, scp->statements.items[i]);
            wprint(w, "\n");
        }
        INDENT();
        wprint(w, "}");
    } break;
    case CODE_KIND_VARIABLE_DECLARATION: {
        Code_Variable_Declaration* var_decl = CODE_AS(stmt, Code_Variable_Declaration);
        INDENT();
        if (var_decl->storage_class & LL_STORAGE_CLASS_CONST) {
            wprint(w, "const {}", var_decl->base.ident->str);
        } else {
            wprint(w, "let {}", var_decl->base.ident->str);
        }
        if (var_decl->initializer) {
            wprint(w, "=");
            ll_emitter_emit_expression(cc, e, w, indent, var_decl->initializer);
        }
        wprint(w, ";");
    } break;
    case CODE_KIND_PARAMETER: {
        Code_Parameter* var_decl = CODE_AS(stmt, Code_Parameter);
        INDENT();
        if (var_decl->flags & LL_STORAGE_CLASS_STATIC) {
            wprint(w, "static {}", var_decl->base.ident->str);
        }
        wprint(w, ";");
    } break;
    case CODE_KIND_FUNCTION_DECLARATION: {
        Code_Function_Declaration* fn_decl = CODE_AS(stmt, Code_Function_Declaration);
        INDENT();
        if (fn_decl->storage_class & LL_STORAGE_CLASS_ARROW_FUNC) {
        } else {
            if (!fn_decl->base.within_scope->decl || fn_decl->base.within_scope->decl->base.kind != CODE_KIND_CLASS_DECLARATION) {
                wprint(w, "function");
                if (fn_decl->base.ident) {
                    wprint(w, " {}", fn_decl->base.ident->str);
                }
            } else {
                wprint(w, "{}", fn_decl->base.ident->str);
            }
        }

        wprint(w, "(");
        for (uint32 i = 0; i < fn_decl->parameters.count; ++i) {
            if (i) wprint(w, ",");
            wprint(w, "{}", fn_decl->parameters.items[i]->base.ident->str);
        }
        wprint(w, ")");

        if (fn_decl->storage_class & LL_STORAGE_CLASS_ARROW_FUNC) {
            wprint(w, "=>");
            if (fn_decl->body) {
                if (fn_decl->body->base.kind == CODE_KIND_BLOCK) {
                    ll_emitter_emit_statement(cc, e, w, indent, false, (Code*)fn_decl->body);
                } else {
                    ll_emitter_emit_expression(cc, e, w, indent, (Code*)fn_decl->body);
                }
            }
        } else {
            if (fn_decl->body) {
                ll_emitter_emit_statement(cc, e, w, indent, false, (Code*)fn_decl->body);
            }
        }

    } break;
    case CODE_KIND_CLASS_DECLARATION: {
        INDENT();
        Code_Class_Declaration* class_decl = CODE_AS(stmt, Code_Class_Declaration);
        wprint(w, "class");
        if (class_decl->base.ident) {
            wprint(w, " {}", class_decl->base.ident->str);
        }
        if (class_decl->extends) {
            ll_emitter_emit_statement(cc, e, w, indent, false, (Code*)class_decl->extends);
        }
        oc_assert(class_decl->block);
        ll_emitter_emit_statement(cc, e, w, indent, false, (Code*)class_decl->block);
    } break;
    case CODE_KIND_INTERFACE_DECLARATION: {
    } break;
    case CODE_KIND_IF: {
        INDENT();
        Code_If* iff = CODE_AS(stmt, Code_If);
        wprint(w, "if(");
        ll_emitter_emit_expression(cc, e, w, indent, iff->cond);
        wprint(w, ")");
        ll_emitter_emit_statement(cc, e, w, indent, false, iff->body);
        if (iff->else_clause) {
            wprint(w, "else ");
            ll_emitter_emit_statement(cc, e, w, indent, false, iff->else_clause);
        }
    } break;
    case CODE_KIND_FOR: {
        INDENT();
        Code_Loop* loop = CODE_AS(stmt, Code_Loop);
        wprint(w, "for(");
        if (loop->init) {
            ll_emitter_emit_statement(cc, e, w, indent, false, loop->init);
        } else wprint(w, ";");

        if (loop->cond) {
            ll_emitter_emit_expression(cc, e, w, indent, loop->cond);
        }
        wprint(w, ";");

        if (loop->update) {
            ll_emitter_emit_expression(cc, e, w, indent, loop->update);
        }
        wprint(w, ")");

        ll_emitter_emit_statement(cc, e, w, indent, false, loop->body);
    } break;
    case CODE_KIND_WHILE: {
        INDENT();
        Code_Loop* loop = CODE_AS(stmt, Code_Loop);
        wprint(w, "while(");
        oc_assert(loop->cond);
        ll_emitter_emit_expression(cc, e, w, indent, loop->cond);
        wprint(w, ")");
        ll_emitter_emit_statement(cc, e, w, indent, false, loop->body);
    } break;
    case CODE_KIND_RETURN: {
        INDENT();
        Code_Control_Flow* cf = CODE_AS(stmt, Code_Control_Flow);
        wprint(w, "return");
        if (cf->expr) {
            wprint(w, " ");
            ll_emitter_emit_expression(cc, e, w, indent, cf->expr);
        }
        wprint(w, ";");
    } break;
    case CODE_KIND_BREAK: {
        INDENT();
        wprint(w, "break;");
    } break;
    case CODE_KIND_CONTINUE: {
        INDENT();
        wprint(w, "continue;");
    } break;
    default:
        INDENT();
        ll_emitter_emit_expression(cc, e, w, indent, stmt);
        wprint(w, ";");
    }
}

void ll_emitter_emit_expression(Compiler_Context* cc, LL_Emitter *e, Oc_Writer* w, int indent, Code* expr) {
    switch (expr->kind) {
    case CODE_KIND_LITERAL_INT: {
        wprint(w, "{}", CODE_AS(expr, Code_Literal)->u64);
    } break;
    case CODE_KIND_LITERAL_FLOAT: {
        wprint(w, "{}", CODE_AS(expr, Code_Literal)->f64);
    } break;
    case CODE_KIND_LITERAL_STRING: {
        if (expr->token_info.flag & LL_TOKEN_FLAG_STRING_SINGLE_QUOTE) {
            wprint(w, "'{}'", CODE_AS(expr, Code_Literal)->str);
        } else {
            wprint(w, "\"{}\"", CODE_AS(expr, Code_Literal)->str);
        }
    } break;
    case CODE_KIND_IDENT: {
        wprint(w, "{}", CODE_AS(expr, Code_Ident)->str);
    } break;
    case CODE_KIND_GROUPED: {
        wprint(w, "(");
        ll_emitter_emit_expression(cc, e, w, indent, CODE_AS(expr, Code_Grouped)->expr);
        wprint(w, ")");
    } break;
    case CODE_KIND_PRE_OP: {
        Code_Operation* op = CODE_AS(expr, Code_Operation);
        if (op->op.kind == LL_TOKEN_KIND_IDENT) {
            wprint(w, "{} ", op->op.str);
        } else {
            lexer_print_token_raw_to_writer(&op->op, w);
        }
        ll_emitter_emit_expression(cc, e, w, indent, op->right);
    } break;
    case CODE_KIND_BINARY_OP: {
        Code_Operation* op = CODE_AS(expr, Code_Operation);
        if (op->from_initializer) {
            oc_assert(op->op.kind == '=');
            wprint(w, "let {}", op->from_initializer->base.ident->str);
            wprint(w, "=");
            ll_emitter_emit_expression(cc, e, w, indent, op->right);
        } else {
            ll_emitter_emit_expression(cc, e, w, indent, op->left);
            if (op->op.kind == LL_TOKEN_KIND_IDENT) {
                wprint(w, " {} ", op->op.str);
            } else {
                lexer_print_token_raw_to_writer(&op->op, w);
            }
            ll_emitter_emit_expression(cc, e, w, indent, op->right);
        }
    } break;
    case CODE_KIND_INDEX: {
        Code_Index* idx = CODE_AS(expr, Code_Index);
        ll_emitter_emit_expression(cc, e, w, indent, idx->ptr);
        wprint(w, "(");
        ll_emitter_emit_expression(cc, e, w, indent, idx->index);
        wprint(w, ")");
    } break;
    case CODE_KIND_INVOKE: {
        Code_Invoke* inv = CODE_AS(expr, Code_Invoke);
        ll_emitter_emit_expression(cc, e, w, indent, inv->expr);

        wprint(w, "(");
        for (uint32 i = 0; i < inv->arguments.count; ++i) {
            if (i) wprint(w, ",");
            ll_emitter_emit_expression(cc, e, w, indent, inv->arguments.items[i]);
        }
        wprint(w, ")");
    } break;
    case CODE_KIND_OBJECT_INITIALIZER: {
        Code_Initializer* init = CODE_AS(expr, Code_Initializer);
        wprint(w, "{{\n");
        for (uint32 i = 0; i < init->count; ++i) {
            oc_assert(init->items[i]->kind == CODE_KIND_KEY_VALUE);
            Code_Key_Value* kv = CODE_AS(init->items[i]->kind, Code_Key_Value);
            
            ll_emitter_emit_expression(cc, e, w, indent, kv->key);
            wprint(w, ": ");
            ll_emitter_emit_expression(cc, e, w, indent, kv->value);
        }
        wprint(w, "}");
    } break;
    case CODE_KIND_ARRAY_INITIALIZER: {
        Code_Initializer* init = CODE_AS(expr, Code_Initializer);
        wprint(w, "[");
        for (uint32 i = 0; i < init->count; ++i) {
            if (i) wprint(w, ",");
            oc_assert(init->items[i]->kind != CODE_KIND_KEY_VALUE);
            ll_emitter_emit_expression(cc, e, w, indent, init->items[i]);
        }
        wprint(w, "]");
    } break;
    case CODE_KIND_TERNARY: {
        Code_If* iff = CODE_AS(expr, Code_If);
        ll_emitter_emit_expression(cc, e, w, indent, iff->cond);
        wprint(w, "?");
        ll_emitter_emit_expression(cc, e, w, indent, iff->body);
        wprint(w, ":");
        ll_emitter_emit_expression(cc, e, w, indent, iff->else_clause);
    } break;
    case CODE_KIND_FUNCTION_DECLARATION:
        ll_emitter_emit_statement(cc, e, w, indent, false, expr);
        break;
    case CODE_KIND_CLASS_DECLARATION:
        ll_emitter_emit_statement(cc, e, w, indent, false, expr);
        break;
    default: print("Unhandled expression: {}\n", expr->kind);
    }
}
