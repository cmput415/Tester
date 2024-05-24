#include <stdio.h>
#include <string.h>

const char string[] = "abc";
const char garbled[] = "ljfa;sdlkjfa;lskdjfalksdjflaksjdfla;dj";

int main() {

    char copyStr[3];
    memcpy(copyStr, string, sizeof(copyStr));

    char copyStr2[100];
    strcpy(copyStr2, copyStr);

    return 0;
}
