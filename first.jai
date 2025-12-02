test :: (a: int, b: int, c: int, d: int, e: int, f: int) -> int #c_call {
    #asm { int 68; };
    return a + b + c + d + e + f;
}

main :: () {
    #asm { int 69; };
    a := 1;
    b := 2;
    c := 3;
    d := 4;
    e := 5;
    f := 6;
    g := 7;
    test(a, b, c, d, e, f);
    // h := a + b + c + d + e + f + g;
    #asm { int 69; };
}
