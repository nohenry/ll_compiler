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
    for int i = 0; i < 40; i += 1   sum += i;
    write_int(sum);

    int j = for int i = 0; i < 4; i += 1 if i == 2 break i;
    write_int(j);

    int a = do {
        break 10;
    };
    write_int(a);

    int b = do {
        break do 30;
    };
    write_int(b);

    int c = for int i = 0; i < 40; i += 1 {
        int asd1 = do {
            break do 34;
        };
        write_int(asd1);
        int asd = do {
            break for 37;
        };
    };
    write_int(c);
}