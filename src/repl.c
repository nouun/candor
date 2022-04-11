#include "repl.h"
#include <mpc.h>

void run_repl(void) {
  while (1) {
    char* input = readline("candor > ");
    add_history(input);

    if (!input) { break; }

    cval* res = candor_load("<stdin>", input);
    cval_println(res);
    cval_del(res);

    free(input);
  }
}
