native void write_int(int d);

int get_data() {
    return 4;
}

void main() {
    write_int(get_data());
}