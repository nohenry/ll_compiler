native void write_int(int64 a);

void polymorphic(%T foo) {
    write_int(foo);
}

void main() {
    float a = 123.0;
    polymorphic(a);
}