#include "emit.h"

LL_Emitter ll_emitter_create(Compiler_Context* cc) {
    LL_Emitter emitter = { 0 };
    oc_sb_init(&emitter.sb, &cc->arena);
    return emitter;
}

void ll_emitter_run(Compiler_Context* cc, LL_Emitter *e, Code_Scope* file) {
    for (uint32 i = 0; i < file->declarations.capacity; ++i) {
        if (file->declarations.entries[i].filled) {
            if (file->declarations.entries[i]._value->base.kind == CODE_KIND_VARIABLE_DECLARATION) {
                if (CODE_AS(file->declarations.entries[i]._value->base.kind, Code_Variable_Declaration)->initializer) {
                } else {
                    ll_emitter_emit_statement(cc, e, &e->sb.writer, 0, true, (Code*)file->declarations.entries[i]._value);
                }
            } else {
                ll_emitter_emit_statement(cc, e, &e->sb.writer, 0, true, (Code*)file->declarations.entries[i]._value);
            }
        }
    }
    for (uint32 i = 0; i < file->statements.count; ++i) {
        ll_emitter_emit_statement(cc, e, &e->sb.writer, 0, true, file->statements.items[i]);
    }
}

void ll_emitter_emit_statement(Compiler_Context* cc, LL_Emitter *e, Oc_Writer* w, int indent, bool do_indent, Code* stmt) {
    #define INDENT() do { if (do_indent) { for (int iii = 0 ; iii < indent; ++iii) wprint(w, "    "); } else { do_indent = true; } } while (0)

    switch (stmt->kind) {
    case CODE_KIND_FOR_SCOPE: {
        Code_Scope* scp = CODE_AS(stmt, Code_Scope);
        for (uint32 i = 0; i < scp->statements.count; ++i) {
            if (scp->statements.items[i]->kind != CODE_KIND_FOR) continue;
            ll_emitter_emit_statement(cc, e, &e->sb.writer, indent, true, scp->statements.items[i]);
            wprint(w, "\n");
        }
    } break;
    case CODE_KIND_BLOCK: {
        Code_Scope* scp = CODE_AS(stmt, Code_Scope);
        INDENT();
        wprint(w, "{{\n");
        for (uint32 i = 0; i < scp->declarations.capacity; ++i) {
            if (scp->declarations.entries[i].filled) {
                if (scp->declarations.entries[i]._value->base.kind == CODE_KIND_VARIABLE_DECLARATION) {
                    if (CODE_AS(scp->declarations.entries[i]._value, Code_Variable_Declaration)->initializer) {
                        print("init\n");
                    } else {
                        INDENT();
                        ll_emitter_emit_statement(cc, e, &e->sb.writer, indent + 1, true, (Code*)scp->declarations.entries[i]._value);
                        wprint(w, "\n");
                    }
                } else {
                    INDENT();
                    ll_emitter_emit_statement(cc, e, &e->sb.writer, indent + 1, true, (Code*)scp->declarations.entries[i]._value);
                    wprint(w, "\n");
                }
            }
        }
        for (uint32 i = 0; i < scp->statements.count; ++i) {
            INDENT();
            ll_emitter_emit_statement(cc, e, &e->sb.writer, indent + 1, true, scp->statements.items[i]);
            wprint(w, "\n");
        }
        INDENT();
        wprint(w, "}");
    } break;
    case CODE_KIND_VARIABLE_DECLARATION: {
        Code_Variable_Declaration* var_decl = CODE_AS(stmt, Code_Variable_Declaration);
        INDENT();
        wprint(w, "let {}", var_decl->base.ident->str);
        if (var_decl->initializer) {
            wprint(w, "=");
            ll_emitter_emit_expression(cc, e, w, indent, var_decl->initializer);
        }
        wprint(w, ";");
    } break;
    case CODE_KIND_FUNCTION_DECLARATION: {
        Code_Function_Declaration* fn_decl = CODE_AS(stmt, Code_Function_Declaration);
        INDENT();
        wprint(w, "function");
        if (fn_decl->base.ident) {
            wprint(w, " {}", fn_decl->base.ident->str);
        }
        wprint(w, "(");
        for (uint32 i = 0; i < fn_decl->parameters.count; ++i) {
            if (i) wprint(w, ",");
            wprint(w, "{}", fn_decl->parameters.items[i].ident->str);
        }
        wprint(w, ")");
        if (fn_decl->body) {
            ll_emitter_emit_statement(cc, e, w, indent, false, (Code*)fn_decl->body);
        }
    } break;
    case CODE_KIND_FOR: {
        INDENT();
        Code_Loop* loop = CODE_AS(stmt, Code_Loop);
        wprint(w, "for (");
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
        wprint(w, "{}", CODE_AS(expr, Code_Literal)->str);
    } break;
    case CODE_KIND_IDENT: {
        wprint(w, "{}", CODE_AS(expr, Code_Ident)->str);
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
    case CODE_KIND_FUNCTION_DECLARATION:
        ll_emitter_emit_statement(cc, e, w, indent, false, expr);
        break;
    default: print("Unhandled expression: {}\n", expr->kind);
    }
}
