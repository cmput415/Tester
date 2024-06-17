#include <stdio.h>

#define CUSTOM_COMPILE_TIME_ERROR 1

int main() {
    #if CUSTOM_COMPILE_TIME_ERROR
        printf("CompileTimeError: this is some text after."); 
        // NOTE:
        // Unless a program exits with non-zero status code, it can not
        // be considered an error test. 
        exit(0);
    #else
        printf("%d", undefined_variable);
    #endif
    return 0;
}

//CHECK:CompileTimeError: