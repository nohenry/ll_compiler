native void write_int(int64 a);

void macro foobar() {
    int local = 123;
    $a = local + $add_amount; // refers to 'a' and 'add_amount' in outer scope
    write_int(68);
}

void macro create_hoisted_var() {
    int $b = 45; // this variable is created in caller's scope
}

void macro member(%T a, int b) {
    T $res = a + b;
}

void macro stmt(void ins_arg) {
    for int i = 0; i < 5; i += 1 {
        ins_arg;
    }
}

void macro foreach(int[:] array, void code) {
    for uint64 i = 0; i < array.length; i += 1 {
        int element = array[i];
        code;
    }
}

void macro test_ptr_arg(%T* a) {
    write_int(*a);
}

void main() {
    int add_amount = 2;
    int a;
    foobar();
    write_int(a);

    foobar(); // notice we call foobar twice and 'local' variable is not conflicting
    write_int(a);

    create_hoisted_var();
    write_int(b);

    int number = 123;
    member(number, 4);
    write_int(res);

    number.member(7);
    write_int(res);

    stmt(do {
        write_int(i);
    });

    int[5] a = [1, 2, 4, 8, 16];

    foreach(a, do {
        write_int(element);
    });

    int64 aa = 123;
    test_ptr_arg(&aa);
}