

extern int printf(const char* fmt, ...);
static int fb();
extern int fc();

int main(int argc, char** argv) {

    register volatile unsigned long e = 0xFFFFFFFFF000;
    register volatile unsigned int a = e;
    register volatile unsigned long d = a;
    printf("e: %lx\n", e);
    printf("a: %x\n", a);
    printf("d: %lx\n", d);

	return (int)d;
}


static int fb() {
}

