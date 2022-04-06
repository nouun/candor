#include "candor.h"
#include "builtins.h"
#include "cval.h"
#include <mpc.h>

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

static mpc_parser_t* Number;
static mpc_parser_t* String;
static mpc_parser_t* Keyword;
static mpc_parser_t* Quot;
static mpc_parser_t* Qquot;
static mpc_parser_t* Nonquot;
static mpc_parser_t* Sexpr;
static mpc_parser_t* Expr;
static mpc_parser_t* Comment;
mpc_parser_t* Candor;

static mpc_result_t mpc_result;
static cenv*        candor_env;

void candor_init(cenv* env) {
  Number  = mpc_new("number");
  String  = mpc_new("string");
  Keyword = mpc_new("keyword");
  Quot    = mpc_new("quot");
  Qquot   = mpc_new("qquot");
  Nonquot = mpc_new("nonquot");
  Sexpr   = mpc_new("sexpr");
  Expr    = mpc_new("expr");
  Comment = mpc_new("comment");
  Candor  = mpc_new("candor");


  mpca_lang(MPCA_LANG_DEFAULT,
            "number  \"number\"      : /-?[0-9]+/                        ; "
            "string  \"string\"      : /\"(\\\\.|[^\"])*\"/              ; "
            "keyword \"keyword\"     : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!?&]+/ ; "
            "quot    \"quote\"       : \"'\" <nonquot>                   ; "
            "qquot   \"quasiquote\"  : '`' <nonquot>                     ; "
            "nonquot                 : <number> | <keyword> | <sexpr>      "
            "                          | <string> | <comment>            ; "
            "sexpr   \"sexpression\" : '(' <expr>* ')'                   ; "
            "expr                    : <quot> | <qquot> | <nonquot>      ; "
            "comment \"comment\"     : /;[^\\r\\n]*/                     ; "
            "candor                  : /^/ <expr>* /$/                   ; ",
            Number, String, Keyword, Quot, Qquot, Nonquot, Sexpr, Expr, Comment, Candor);


  cenv_add_builtins(env);
}

void candor_deinit(void) {
  if (mpc_result.output) {
    mpc_ast_delete(mpc_result.output);
  } else if (mpc_result.error) {
    mpc_err_delete(mpc_result.error);
  }
  
  mpc_cleanup(10, Number, String, Keyword, Nonquot, Qquot, Quot, Sexpr, Expr,
              Comment, Candor);
}

void shutdown(void) {
  cenv_del(candor_env);
  candor_deinit();
  rl_uninitialize();
}

int main(int argc, char** argv) {
  candor_env = cenv_new();
  candor_init(candor_env);
  atexit(shutdown);
  
  if (argc >= 2) {
    for(int i = 1; i < argc; i++) {
      cval* args = cval_add(cval_sexpr(), cval_str(argv[i]));
      cval* res = builtin_load(candor_env, args);
      if(res->type == CVAL_ERR) { cval_println(res); }
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
        
        if(!result->count) {
          cval_println(out);
          cval_del(out);
        }
      }

      cval_del(result);
      mpc_ast_delete(mpc_result.output);
    } else if(mpc_result.error) {
      mpc_err_print(mpc_result.error);
      mpc_err_delete(mpc_result.error);
    }

    free(input);
  }

  return 0;
}

cval* candor_parse(const char* filename, char* str) {
  mpc_result_t res;
  if(mpc_parse(filename, str, Candor, &res)){
    cval* val = cval_read(res.output);

    mpc_ast_delete(res.output);

    return val;
  } else {
    char* msg = mpc_err_string(res.error);
    mpc_err_delete(res.error);

    cval* err = cval_err("unable to parse string:\n%s", msg);

    free(msg);

    return err;
  }
}
