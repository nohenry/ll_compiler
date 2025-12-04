native void write_int(int64 a);

void macro foobar() {
    int local = 123;
    $a = local + $add_amount; // refers to 'a' and 'add_amount' in outer scope
    write_int(68);
}

void macro create_hoisted_var() {
    int b = 45; // this variable is created in caller's scope
}

void macro member(int a) {
    int $res = 69;
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

    123.member();
    write_int(res);
}