// INPUT:a
// INPUT:b

#include <stdio.h>

int main(int argc, char **argv) {
  
  char line1[25];
  char line2[25];
  
  // read line 1
  if (fgets(line1, sizeof(line1), stdin) != NULL) {
    printf("%s", line1);
  }

  // read line 2
  if (fgets(line2, sizeof(line2), stdin) != NULL) {
    printf("%s", line2);
  }
  
  printf("c\n");
  return 0;
}

// CHECK:a
// CHECK:b
// CHECK:c
// CHECK:
