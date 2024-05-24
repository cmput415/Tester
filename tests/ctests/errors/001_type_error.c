// big brain C code

typedef struct Apple{
  char a[5];
} Apple;

typedef struct Banana {
  char b[6];
} Banana ;

int main() {
  
  Apple a = { "apple" }; 
  Banana b = { "banana" };
  // TODO: add type error handling to tester
  //a = b;

  return 0;
}

//CHECK_TODO:001_type_error.c:14:5: error: assigning to 'Apple' (aka 'struct Apple') from incompatible type 'Banana' (aka 'struct Banana')
//CHECK_TODO:  a = b;
//CHECK_TODO:    ^ ~
//CHECK_TODO:1 error generated.
