native void write_int(int64 a);
native void* realloc(void* ptr, uint size);

struct Dynamic_Array {
    uint count;
    uint capacity;
    uint stride;
    void* data;
}

Dynamic_Array create() {
    Dynamic_Array dyn;
    dyn.count = 0;
    dyn.capacity = 0;
    dyn.stride = 8;
    dyn.data = null;
    return dyn;
}

void macro append(%D dyn, %T value) {
    dyn.count += 1;
    if dyn.count > dyn.capacity {
        dyn.capacity = dyn.count * 2 + 16;
        dyn.data = realloc(dyn.data, dyn.capacity * dyn.stride);
    }
    T* ptr = dyn.data;
    ptr[dyn.count - 1] = value;
}

void main() {
    Dynamic_Array d = create();
    d.append(cast(uint64)5);
    d.append(cast(uint64)10);
    d.append(cast(uint64)17);

    int64* ptr = d.data;
    write_int(ptr[0]);
    write_int(ptr[1]);
    write_int(ptr[2]);
}