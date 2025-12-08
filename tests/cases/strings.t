native void write_int(int64 v);

void main() {
    string a = "foobar";
    char c1 = a[0];
    write_int(v = cast(int64)c1);

    char[:] cslice = a;
    cslice = cslice[1:50];
    write_int(v = cast(int64)cslice.length);
    write_int(v = cast(int64)cslice[0]);
}