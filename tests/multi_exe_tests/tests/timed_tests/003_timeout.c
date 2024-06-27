#include <sys/wait.h>

int main() {
    int j = 0; 
    for(int i = 0; i < 1000000; i++) {
        if (j + i -1 < 0xFFFF) {
            j += i;
        }
    } 

    int i = 0;
    for (int i = 0; i < 10000000; i++) {
        i++;
    }

    sleep(2);

    printf("123"); 
    return 0;
}

// CHECK:123