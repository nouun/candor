#include "config.h"

#include <candor.h>
#include <mpc.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>

#define BUF_SIZE 2048

static char buffer[BUF_SIZE];

static char* readline(const char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, BUF_SIZE, stdin);
  char* cpy = malloc(BUF_SIZE);
  strcopy(cpy, buffer);
  return cpy;
}

static void add_history(const char* line) {}

#else
#include <editline.h>
#endif

static void shutdown(void) {
  candor_deinit();
  rl_uninitialize();
}

int main(int argc, char** argv) {
  candor_init();
  atexit(shutdown);

  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      cval* res = candor_load_file(argv[i]);
      if (res->type == CVAL_ERR) { cval_println(res); }
      cval_del(res);
    }
    
    return 0;
  }
  
  printf("candor v%s\n", CANDOR_VERSION);
  
  while (1) {
    char* input = readline("candor > ");
    add_history(input);

    if (!input) { break; }

    cval* res = candor_load("<stdin>", input);
    cval_println(res);
    cval_del(res);
    
    free(input);
  }

  return 0;
}
