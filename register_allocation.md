
int a;
int b;
int c;
int d;


int e = (a + b) / (c + d);

mov rax, [c]
mov rbx, [d]
add rax, rbx
push rax
mov rax, [a]
mov rbx, [b]
add rax, rbx
pop rbx
div rbx
mov [e], rax

int e = foobar() + a;

mov rbx, [a]
call foobar