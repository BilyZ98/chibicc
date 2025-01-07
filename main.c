#include "chibicc.h"


int main(int argc, char** argv) {
  if (argc !=2) {
    error("%s: invalid number of arguments", argv[0]);
  }
  Token* tok = tokenize(argv[1]);
  Function* func = parse(tok);
  codegen(func);

  return 0;
} 


