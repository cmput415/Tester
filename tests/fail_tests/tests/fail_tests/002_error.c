#include <stdio.h>

#define CUSTOM_COMPILE_TIME_ERROR 1

int main() {
    #if CUSTOM_COMPILE_TIME_ERROR
        printf("CompileTimeError on line 7: this is some text after."); 
        exit(1);
    #endif
    return 0;
}

//CHECK:CompileTimeError on line 6: