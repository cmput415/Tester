#include <sys/wait.h>

#define iter 100000000

int main() {
    int j = 0; 
    for(int i = 0; i < iter; i++) {
        if (j + i -1 < 0xffff) {
            j += i;
        }
    } 

    sleep(1);

    int i = 0;
    for (int i = 0; i < 1000000; i++) {
        i++;
    }

    return (j < 0xffffffff);
}