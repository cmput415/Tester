#include <sys/wait.h>

#define ITER 100000000

int main() {
    int j = 0; 
    for(int i = 0; i < ITER; i++) {
        if (j + i -1 < 0xFFFF) {
            j += i;
        }
    } 

    return (j < 0xffffffff);
}