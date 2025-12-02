native void write_int(int64 v);

void main() {
    string a = "foobar";
    uint8 c1 = a[0];
    write_int(v = c1);
}