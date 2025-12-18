native void write_int(int64 a);

void polymorphic(%T* foo) {
    write_int(foo);
}

void main() {
    int64 a = 123;
    polymorphic(a);
}