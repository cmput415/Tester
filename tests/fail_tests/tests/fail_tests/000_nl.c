#include <stdio.h>


int main() {
    
    printf("a\nb\nc\n");

    return 0;
}

// This will fail since there is an extra newline emitted.
// The expected file will have 5 bytes, the generated will have 6

//CHECK:a
//CHECK:b
//CHECK:c