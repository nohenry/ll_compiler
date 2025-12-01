native void write_int(int64 d);

void main() {
    if true write_int(10);
    if true {
        write_int(27);
    }

    if true {
        write_int(12);
    } else {
        write_int(400);
    }

    if false {
        write_int(12);
    } else {
        write_int(400);
    }

    int a = 123;
    if a == 40 write_int(40) else write_int(-1);
    if a == 123 write_int(123) else write_int(-1);

    for int i = 0; i < 4; i += 1 {
        write_int(i);
    }

    int sum = 0;
    for int i = 0; i < 40; i += 1 sum += i;
    
    write_int(sum);
}