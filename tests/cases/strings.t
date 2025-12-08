native void write_int(int64 v);
native void write_string(string s);

void print_string(string* s) {
    write_string(*s);
    // *s = 
}

void main() {
    string a = "fosbar";
    char c1 = a[0];
    write_int(v = cast(int64)c1);

    char[:] cslice = a;
    cslice = cslice[1:];
    write_int(v = cast(int64)cslice.length);
    write_int(v = cast(int64)cslice[0]);

    cslice = cslice[1:3];
    write_int(v = cast(int64)cslice.length);
    write_int(v = cast(int64)cslice[0]);

    print_string(&a);
    write_string(a);
    write_string(a[:]);
    write_string(a[3:]);
    write_string(a[:2]);
}