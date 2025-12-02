native void* malloc(uint64 n);
native void write_int(int64 n);

void main() {
    int* ns = cast(int*)malloc(6);
    ns[0] = 10;
    write_int(ns[0]);
}