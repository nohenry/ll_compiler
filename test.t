void main() {
	extern void printf(string fmt, ..args);
    void test_fun(int a, int b, int c, int d) {
		int aa = d * 2;
		printf("Hello test_fun %d\n", aa);
    }
	int[3][4] array = [[0, 1, 2], [3, 4, 5], [6, 7, 8], [9, 10, 11]];

	int[3] aa = array[0];
	printf("\x1b[33;1mAA is\x1b[0m %d\n", aa[3]);

	// int b = 23 + 45;
	// int d = 10;
	// int c = b + d;
	// int a = c + 45;
	// int* pa = &a;
	// int aa = *pa;
	// string s = "fdsf";

    // uint8 a = 2;
    // int c = a * b;

	// for int i = 0; i <= 20; i+=1 {
	// 	printf("\x1b[31;1mfoobar\x1b[0m %d\n", i);
	// }

	// int f = const do {
	// 	if true {
	// 		break 6;
	// 	} else {
	// 		break 9;
	// 	}
	// };

	test_fun(3, 4, 5, 6);

	// int d = 56;
	// int* pa = &a;
	// int8 b = *pa;
	// int* a;
	// int[10] array;
	// int[:] slice;
	// int[:uint8] slice_with_sized_length_after;
	// int[int32:] slice_with_sized_length_before;
	// int64 length = 120;
	// int[length:] slice_with_variable_specified_sized_length_before;
	

	// printf("\x1b[31;1mfoobar\x1b[0m %d\n", 1);
    // test_fun(1, 2, 3, 4);

	// return aa;
	// int bb = printf("hello", 1, 2, 3);
	// int aa = b(123, 1);
}

