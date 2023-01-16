#include "ast.h"
#include "lexer.h"

using namespace ast;

int main() {
  binary_oper['<'] = 10;
  binary_oper['+'] = 20;
  binary_oper['-'] = 20;
  binary_oper['*'] = 40;

  fprintf(stderr, "ready> ");
  get_next_token();
  main_loop();
  return 0;
}