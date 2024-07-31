// compile with clang -shared -fPIC -o foo.so foo.c

//clang -fPIC -c foo.c -o foo.o
//clang -shared -o libfoo.so foo.o

int foo(int x, int y) {
    return x + y;
}
