native void write_int(int64 v);

void polymorphic(%T foo) {
    write_int(cast(int64)foo);
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
}