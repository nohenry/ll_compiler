native void write_int(int64 v);
native void write_string(string s);

void print_string(string* s) {
    write_string(*s);
    // *s = 
}

string string_trim(string s) {
    while s.length > 0 && s[0] == 0x20 {
        s = s[1:];
    }
    while s.length > 0 && s[s.length - 1] == 0xa {
        s = s[:s.length - 1];
    }
    return s;
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

    string s = "   sdf   \n";
    s = string_trim(s);
    write_string(s);
}