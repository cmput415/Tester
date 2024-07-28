#include <stdio.h>

extern int foo(int x, int y);

int main() {

    int x = 4, y = 7; 
    printf("%d", foo(x, y)); //CHECK:11
    
    return 0;
}