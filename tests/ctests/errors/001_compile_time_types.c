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

//CHECK:error: assigning to 'Apple' (aka 'struct Apple') from incompatible type 'Banana' (aka 'struct Banana')
//CHECK:  a = b;
//CHECK:    ^ ~
//CHECK:1 error generated.
//CHECK: