/**
 * Check for a different type of error than is rasied at runtime.
 * This will still pass because the error parsing is lenient. 
 */

#include <stdio.h>
#include <stdlib.h>

#define TYPE_ERROR 1

int main() {

    fprintf(stderr, "RandomRuntimeError on line 12: runtime error"); 
    
    exit(1);
    
    return 0;
}

//CHECK:OtherRuntimeError on line 12