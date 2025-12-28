
#include "typer2.h"
#include "parser.h"


LL_Typer ll_typer_create(Compiler_Context* cc) {
    LL_Typer result = { 0 };
    LL_Typer* typer = &result;

	oc_array_reserve(&cc->arena, &result.types, 128);
	result.types.count = 0;

#define INSERT_BUILTIN_TYPE(_kind, _group) oc_array_append(&cc->arena, &result.types, ((LL_Type) { .kind = _kind, .group = _group }))

	INSERT_BUILTIN_TYPE(LL_TYPE_UNKNOWN, LL_TYPE_GROUP_UNKNOWN);
	INSERT_BUILTIN_TYPE(LL_TYPE_INT8, LL_TYPE_GROUP_INT);
	INSERT_BUILTIN_TYPE(LL_TYPE_INT16, LL_TYPE_GROUP_INT);
	INSERT_BUILTIN_TYPE(LL_TYPE_INT32, LL_TYPE_GROUP_INT);
	INSERT_BUILTIN_TYPE(LL_TYPE_INT64, LL_TYPE_GROUP_INT);
	INSERT_BUILTIN_TYPE(LL_TYPE_UINT8, LL_TYPE_GROUP_UINT);
	INSERT_BUILTIN_TYPE(LL_TYPE_UINT16, LL_TYPE_GROUP_UINT);
	INSERT_BUILTIN_TYPE(LL_TYPE_UINT32, LL_TYPE_GROUP_UINT);
	INSERT_BUILTIN_TYPE(LL_TYPE_UINT64, LL_TYPE_GROUP_UINT);
	INSERT_BUILTIN_TYPE(LL_TYPE_CHAR8, LL_TYPE_GROUP_CHAR);
	INSERT_BUILTIN_TYPE(LL_TYPE_CHAR16, LL_TYPE_GROUP_CHAR);
	INSERT_BUILTIN_TYPE(LL_TYPE_CHAR32, LL_TYPE_GROUP_CHAR);
	INSERT_BUILTIN_TYPE(LL_TYPE_CHAR64, LL_TYPE_GROUP_CHAR);
	INSERT_BUILTIN_TYPE(LL_TYPE_BOOL1, LL_TYPE_GROUP_BOOL);
	INSERT_BUILTIN_TYPE(LL_TYPE_BOOL8, LL_TYPE_GROUP_BOOL);
	INSERT_BUILTIN_TYPE(LL_TYPE_BOOL16, LL_TYPE_GROUP_BOOL);
	INSERT_BUILTIN_TYPE(LL_TYPE_BOOL32, LL_TYPE_GROUP_BOOL);
	INSERT_BUILTIN_TYPE(LL_TYPE_BOOL64, LL_TYPE_GROUP_BOOL);
	INSERT_BUILTIN_TYPE(LL_TYPE_FLOAT16, LL_TYPE_GROUP_FLOAT);
	INSERT_BUILTIN_TYPE(LL_TYPE_FLOAT32, LL_TYPE_GROUP_FLOAT);
	INSERT_BUILTIN_TYPE(LL_TYPE_FLOAT64, LL_TYPE_GROUP_FLOAT);

#undef INSERT_BUILTIN_TYPE
	return result;
}

void ll_typer_run(Compiler_Context* cc, LL_Typer* typer, LL_Parser* parser, Code* root) {
}



void ll_print_type_raw(LL_Type* type, Oc_Writer* w) {
    uint32_t i;
    switch (type->kind) {
    case LL_TYPE_UNKNOWN:     wprint(w, "unknown"); break;
    case LL_TYPE_INT8:     wprint(w, "int8"); break;
	case LL_TYPE_INT16:    wprint(w, "int16"); break;
	case LL_TYPE_INT32:    wprint(w, "int32"); break;
	case LL_TYPE_INT64:    wprint(w, "int64"); break;
    case LL_TYPE_UINT8:    wprint(w, "uint8"); break;
	case LL_TYPE_UINT16:   wprint(w, "uint16"); break;
	case LL_TYPE_UINT32:   wprint(w, "uint32"); break;
	case LL_TYPE_UINT64:   wprint(w, "uint64"); break;
    case LL_TYPE_BOOL1:    wprint(w, "bool1"); break;
    case LL_TYPE_BOOL8:    wprint(w, "bool8"); break;
    case LL_TYPE_BOOL16:   wprint(w, "bool16"); break;
    case LL_TYPE_BOOL32:   wprint(w, "bool32"); break;
    case LL_TYPE_BOOL64:   wprint(w, "bool64"); break;
    case LL_TYPE_CHAR8:    wprint(w, "char8"); break;
    case LL_TYPE_CHAR16:   wprint(w, "char16"); break;
    case LL_TYPE_CHAR32:   wprint(w, "char32"); break;
    case LL_TYPE_CHAR64:   wprint(w, "char64"); break;

    case LL_TYPE_FLOAT16:  wprint(w, "float16"); break;
    case LL_TYPE_FLOAT32:  wprint(w, "float32"); break;
    case LL_TYPE_FLOAT64:  wprint(w, "float64"); break;

    case LL_TYPE_STRING:   wprint(w, "string"); break;
    case LL_TYPE_POINTER: {
        LL_Type_Pointer* ptr_type = (LL_Type_Pointer*)type;
        ll_print_type_raw(ptr_type->element_type, w);
        wprint(w, "*");
        break;
    }
#if 0
    case LL_TYPE_ARRAY: {
        LL_Type_Array* array_type = (LL_Type_Array*)type;
        ll_print_type_raw(array_type->element_type, w);
        wprint(w, "[{}]", array_type->base.width);
        break;
    }
#endif
    case LL_TYPE_SLICE: {
        LL_Type_Array* array_type = (LL_Type_Array*)type;
        ll_print_type_raw(array_type->element_type, w);
        wprint(w, "[:]");
        break;
    }
    case LL_TYPE_FUNCTION: {
        LL_Type_Function* fn_type = (LL_Type_Function*)type;
        if (fn_type->return_type) ll_print_type_raw(fn_type->return_type, w);
        wprint(w, " (");
        for (i = 0; i < fn_type->parameter_count; ++i) {
            if (i > 0) wprint(w, ", ");
            if (fn_type->parameters[i]) ll_print_type_raw(fn_type->parameters[i], w);
        }
        wprint(w, ")");
        break;
    }
    case LL_TYPE_STRUCT: {
        LL_Type_Struct* struct_type = (LL_Type_Struct*)type;
        wprint(w, "struct {} {{ ", struct_type->field_count);
        for (i = 0; i < struct_type->field_count; ++i) {
            if (struct_type->fields[i]) {
                ll_print_type_raw(struct_type->fields[i], w);
                if (struct_type->offsets) {
                    wprint(w, "({})", struct_type->offsets[i]);
                }
                wprint(w, "; ");
            }
        }
        wprint(w, "}");
    } break;
    case LL_TYPE_NAMED:
        wprint(w, "named {}", ((LL_Type_Named*)type)->scope->ident->str);
        // wprint(w, "named {} (", ((LL_Type_Named*)type)->scope->ident->str);
        // ll_print_type_raw(((LL_Type_Named*)type)->actual_type, w);
        // wprint(w, ")");
        break;
    case LL_TYPE_TYPE:
        wprint(w, "type");
        break;
    default: oc_assert(false); break;
    }
}

void ll_print_type(LL_Type* type) {
    ll_print_type_raw(type, &stdout_writer);
    print("\n");
}


