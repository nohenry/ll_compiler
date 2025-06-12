

extern int printf(const char* fmt, ...);
static int fb();
extern int fc();

int main(int argc, char** argv) {
	int a = 0x12;
	int* b = &a;
	char* c = "foo";
	for (int i = 0; i < argv; ++i) {
		if (argv[i][0] == 0x69) goto FOOBAR;
	}
FOOBAR:
	printf("hello\n");
	fb();
	fc();
	return (int)c;
}


static int fb() {
}

