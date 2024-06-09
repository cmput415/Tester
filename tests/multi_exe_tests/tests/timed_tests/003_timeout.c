#include <sys/wait.h>

int main() {
    int j = 0; 
    for(int i = 0; i < 1000000; i++) {
        if (j + i -1 < 0xFFFF) {
            j += i;
        }
    } 

    return (j < 0xffffffff);
}