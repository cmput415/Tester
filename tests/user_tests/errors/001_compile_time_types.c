typedef struct Apple{
  char a[5];
} Apple;

typedef struct Banana {
  char b[6];
} Banana ;

int main() {
  
  Apple a = { "apple" }; 
  Banana b = { "banana" };
  a = b;

  return 0;
}

//CHECK:error: 