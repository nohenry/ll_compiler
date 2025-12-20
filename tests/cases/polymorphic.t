native void write_int(int64 v);
native void write_string(string v);

void polymorphic(%T foo) {
    T foo_plus_1 = foo + 1;
    write_int(cast(int64)foo);
    write_int(cast(int64)foo_plus_1);
}

void polymorphic_ptr(%T* foo) {
    write_int(cast(int64)*foo);
}

void polymorphic_array(%T[7] foo) {
    write_int(cast(int64)foo[2]);
    write_int(cast(int64)foo[6]);
}

void polymorphic_ptr_to_array(%T[7]* foo) {
    write_int(cast(int64)(*foo)[2]);
    write_int(cast(int64)(*foo)[6]);
}

void polymorphic_array_count(%T[%N] foo) {
    write_int(cast(int64)foo[2]);
    write_int(cast(int64)N);
}

void just_polymorphic_type(%T) {
    T foo;

    write_int(sizeof(T));
    write_string("just polypmorh");
}

void main() {
    int a = 123;
    write_int(69);
    polymorphic(a);
    polymorphic(a);

    int b = 578;
    polymorphic_ptr(&b);

    int[7] ba = [7, 9, 11, 13, 15, 17, 19];
    polymorphic_array(ba);

    polymorphic_ptr_to_array(&ba);

    polymorphic_array_count(ba);


    just_polymorphic_type(int);
}