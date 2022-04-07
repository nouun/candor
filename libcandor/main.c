#include <mpc.h>
#include <src/candor.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer) + 1);
  strcopy(cpy, buffer);
  cpy[strlen(cpy) + 1] = '\0';
  return cpy;
}

#else
#include <editline.h>
#endif

static mpc_result_t mpc_result;
static cenv*        candor_env;


void shutdown(void) {
  cenv_del(candor_env);

  if (mpc_result.output) {
    mpc_ast_delete(mpc_result.output);
  } else if (mpc_result.error) {
    mpc_err_delete(mpc_result.error);
  }


  candor_deinit();
  rl_uninitialize();
}

int main(int argc, char** argv) {
  candor_env = cenv_new();
  candor_init(candor_env);
  atexit(shutdown);

  if (argc >= 2) {
    for (int i = 1; i < argc; i++) {
      cval* args = cval_add(cval_sexpr(), cval_str(argv[i]));
      cval* res  = builtin_load(candor_env, args);
      if (res->type == CVAL_ERR) { cval_println(res); }
      cval_del(res);
    }

    return 0;
  }

  puts("candor v0.1.0");

  while (1) {
    char* input = readline("candor > ");
    add_history(input);

    if (!input) { break; }

    if (mpc_parse("<stdin>", input, Candor, &mpc_result)) {
      cval* result = cval_read(mpc_result.output);
      while (result->count) {
        cval* out = cval_eval(candor_env, cval_pop(result, 0));

        if (!result->count) {
          cval_println(out);
          cval_del(out);
        }
      }

      cval_del(result);
      mpc_ast_delete(mpc_result.output);
    } else if (mpc_result.error) {
      mpc_err_print(mpc_result.error);
      mpc_err_delete(mpc_result.error);
    }

    free(input);
  }

  return 0;
}
