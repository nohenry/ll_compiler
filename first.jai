test :: (a: int, b: int, c: int, d: int) #c_call {
    // print("% % % %\n", a, b, c, d);
}

main :: () {
    // #asm { int 69; };
    a := 1;
    b := 2;
    c := 3;
    d := 4;
    e := 5;
    f := 6;
    g := 7;
    // test(a, b, c, f * g);
    h := a + b + c + d + e + f + g;
    #asm { int 69; };
}

#run {
    main();
}