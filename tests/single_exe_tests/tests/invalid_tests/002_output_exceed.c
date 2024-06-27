//CHECK_FILE:./out-stream/output_exceed.out

#include <stdio.h>

int main() {
    for (int i = 0; i < 1026; i++) {
        printf("%c", 'a');
    }
}