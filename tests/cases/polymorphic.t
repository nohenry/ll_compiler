native void write_int(int64 v);

void polymorphic(%T foo) {
    write_int(cast(int64)foo);
}


void main() {
    int a = 123;
    write_int(69);
    polymorphic(a);
    polymorphic(a);
    polymorphic(a);
    polymorphic(a);
    polymorphic(a);
    polymorphic(a);
}