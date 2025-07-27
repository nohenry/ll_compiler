
## Language Compiler Project
Another attempt at a compiler, this time focusing on:
- using c in a big project
- development speed (fast compilation times)
- compile time evaluation 

## The Language
- very c like
	+ let's be honest, c is pretty good
	+ c also sucks, let's improve that

```c
int main(int argc, int8** argv) {
	/* ---- types ---- */
	int d = 56;     // same as c int
	let id = d; 	// infer type
	int* pa = &a;	// pointer to int
	int8 b = *pa;	// same as c int8_t (no include needed)
	int* a = undefined; // do not initialize contents
	int[10] array;  // array of 10 ints, arrays are actual arrays
	int[10] init_array = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
	int*[10] array; // array of 10 pointer to ints.
					// types are read right to left

	int[:] slice;   // a pointer and a length, with bounds checking.
					// this is the same layout as an isize, then an int pointer
	int[:uint8] length_after; // we can explicitly define the layout.
							  // in this case, the length type is uint8 and is placed after the pointer
	int[int32:] length_before;
							  // in this case, the length type is int32 and is placed before the pointer
	int64 length = 120;
	int[length:] specified_length_before; // we can also bind a custom variable to be used as length.
										  // this might be useful in structs for ffi.

	// other builtin types:
	// char8, char16, char32, char64, char
	// int8, int16, int32, int64, int, isize
	// uint8, uint16, uint32, uint64, uint, usize
	// float16, float32, float64, float128, float
	// bool8, bool16, bool32, bool64, bool

	// strings
	char[:] this_is_a_str = "hello, world";

	/* control flow */
	if argc < 2 {
		return -1;
	} else {
		printf("Thank you for supplying the correct number of arguments\n");
	}

	for int i = 1; i < argc; i++ {
		printf("Arg %d is %s", i, argv[i]);
	}
	// also while, do-while

	// do blocks are expressions
	let calculated_value = do {
		int32 sum = 0;
		for int i = 1; i < argc; i++ {
			sum += strlen(argv[i]);
		}
		
		break sum;
	};

	// "ternary"
	let needed_value = if argv > 2 argv[2] else argv[1];
	// or
	let needed_value = if argv > 2 do { break argv[2]; } else do { break argv[1]; };
	
	/* ---- compile time evaluation ---- */
	const int const_value = 45; // this is a compile time constant.
								// a compile time constant's value must be
								// known at compile time. i.e. we can inline
								// the immediate value anywhere (unlike a c const).
	const int scale = 2;
	const int result = const_value * scale;
	int[result] array; // since result is constant, we can use it in e.g. array size
	const int[result] const_arrayarray;

	// force constant eval
	const int a = 1;
	const int b = 2;
	int rc = a + b; // runtime context, rc is not const
					// so a + b is evaluated at runtime (actually it's probably constant folded but whatever)
	int c = const a + b; // const keyword forces the expression to be evaluated at comptime.
						 // this is simple, but we can also do things like call a function or block
	int cc = const do {
		// this looks like runtime code, but the const keyword
		// on the do block, makes it evaluated at compile time.
		// all values captued from outside the block must be compile time known.
		int32 sum = 0;
		for int i = 1; i < array.len; i++ {
			sum += const_array[i];
		}
		
		break sum;
	};

	// types are compile time values
	const let size_type = int32;
	size_type[3] sizes = [1, 2, 3];

	const let size_type = if #config.arch == x86 int32 else if #config.arch == x86_64 int64 else int;
	size_type[3] sizes = [1, 2, 3];
	
	/* ---- functions ---- */
	void ima_function() { }
	int calculate(int a, int b) { return a + b; }
	int log_info(int8[:] fmt, ..args) { printf("INFO: "); return printf(fmt, ..args); }
	// this is like a template parameter:
	int const_param(const int a) {
		int[a] result;
		for int i = 0; i < a; i++ {
			result[i] = i;
		}
		return result[result.len - 1];
	}

	// function pointer
	int:(int, int) fn_ptr = calculate; // the colon is to avoid parsing ambiguities with a function call
	int result = fn_ptr(1, 2);
	
	/* ---- aggregate types ---- */
	struct Data {
		int8* data;
		isize data_len;
	}

	struct Array(const type T) {
		T* data;
		isize data_len;
	}
	int8[] datas = [1, 2, 3, 4];
	Array(int8) array = {
		data = &datas,
		len = datas.len,
	};
	Array(int8)* p_array = &array;
	int8 first = array.data[0];
	int8 first_from_p = p_array.data[0]; // auto dereference
	
	// methods
	// %T means use any value for that argument,
	// and bind the value to T symbol
	T array_sum(Array(%T)* array) {
		T result = #zero;
		for int i = 0; i < array.data_len; i++ {
			result += array.data[i];
		}
		return result;
	}
	
	int sum_result = array_sum(&array);
	// or
	int sum_result = array.array_sum(); // syntax sugar for above
	
	struct Allocator_VTable {
		void* :(Allocator_VTable* allocator, isize size) alloc;
		void  :(Allocator_VTable* allocator, void* ptr)  free;
	}
	Allocator_VTable allocator = #context.default_allocator;
	int8 data = allocator.alloc(10);

	// default values
	struct Parameters {
		bool replicate = false;
		int repeate = 10;
		int delay = 10;
		int address;
	}
	Parameters p = {
		replicate = true,
		address = 0xFE000,
	};

	// bit fields
	packed struct Segment_Access : uint8 {
		bool accessed : 1;	
		bool writable : 1;	
		bool down	  : 1;	
		bool exec 	  : 1;	
		uint8 type 	  : 1;	
		uint8 dpl 	  : 2;	
		bool present  : 1;	
	}

	// unions
	union Caster {
		int64 i64;
		float64 f64;
	}

	// enums
	enum Value_Kind {
		None,
		Int,
		Uint,
		Float,
	}

	enum Token_Kind {
		':', ';', '.', ',',
		Identifier,
	}

	// anon aggregate
	struct Vec4 {
		union {
			struct {
				float x, y, z, w;
			};
			struct {
				float r, g, b, a;
			};
		};
	}
	Vec4 v = { x = 10, y = 20, z = 30, w = 1 };
	printf("%f\n", v.r);

	struct {
		int a = 10; // local varaibles
		int b = 20;
	}
	printf("%d %d\n", a, b);

	union {
		int a = 10; // local varaibles
		float f;
	}
	printf("%f\n", f);	
	
	// other	
	float32 f = 10.2;
	int32 i = #bitcast f;

	Segment_Access sa = #bitcast 0x5A;

	isize i = #sizeof int;
	isize i1 = #sizeof i;
	isize i2 = #sizeof(array[0]);

	return 0;
}
```


