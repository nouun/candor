#include "../builtins.h"

#include <stdio.h>
#include <string.h>

cval* builtin_if(cenv* env, cval* args) {
  CASSERT_COUNT("if", 3);
  CEVAL_MACRO_ARG(0);
  CASSERT_TYPE("if", 0, CVAL_NUM);

  int idx = args->sexpr->cell[0]->num == 0 ? 2 : 1;
  return cval_eval(env, cval_take(args, idx));
}

cval* builtin_eq(cenv* env, cval* args) {
  CASSERT_MIN("=", 2);

  CEVAL_MACRO_ARG(0);
  cval* fst = cval_pop(args, 0);

  int out = 1;
  while (args->sexpr->count) {
    CEVAL_MACRO_ARG(0, cval_del(fst));
    cval* cmp = cval_pop(args, 0);
    
    if (!cval_cmp(fst, cmp)) {
      out = 0;
      cval_del(cmp);
      break;
    }

    cval_del(cmp);
  }

  cval_del(fst);
  cval_del(args);

  return cval_num(out);
}

cval* builtin_ord(cenv* env, char* op, cval* args) {
  CASSERT_MIN(op, 2);
  CEVAL_MACRO_ARG(0);
  CASSERT_TYPE(op, 0, CVAL_NUM);

  int out = 1;
  cval* cur = cval_pop(args, 0);
  while (args->sexpr->count) {
    CEVAL_MACRO_ARG(0, cval_del(cur));
    CASSERT_TYPE(op, 0, CVAL_NUM, cval_del(cur));
    
    cval* val = cval_pop(args, 0);
    
    if ((strcmp(op, "lt?") && !(cur->num < val->num))
        || (strcmp(op, "gt?") && !(cur->num > val->num))) {
      out = 0;
      break;
    }

    cval_del(cur);
    cur = val;
  }

  cval_del(cur);
  cval_del(args);

  return cval_num(out);
}

cval* builtin_lt(cenv* env, cval* arg) {
  return builtin_ord(env, "lt?", arg);
}

cval* builtin_gt(cenv* env, cval* arg) {
  return builtin_ord(env, "gt?", arg);
}

void builtins_add_conditional(cenv* env) {
  builtin_add_mcr(env, "if", builtin_if);
  builtin_add_mcr(env, "eq?", builtin_eq);
  
  builtin_add_mcr(env, "lt?", builtin_lt);
  builtin_add_mcr(env, "gt?", builtin_gt);
}
