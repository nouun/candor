#include "../builtins.h"

#include <stdio.h>
#include <string.h>

cval* builtin_if(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "if", 3);
  arg->sexpr->cell[0] = cval_eval(env, arg->sexpr->cell[0]);
  CASSERT_NOERR(arg);
  CASSERT_TYPE(arg, "if", arg->sexpr->cell[0], CVAL_NUM);

  int idx = arg->sexpr->cell[0]->num == 0 ? 2 : 1;
  return cval_eval(env, cval_take(arg, idx));
}

cval* builtin_eq(cenv* env, cval* arg) {
  CASSERT_MIN(arg, "=", 2);

  cval* fst = cval_eval(env, cval_pop(arg, 0));
  if (fst->type == CVAL_ERR) {
    cval_del(arg);
    return fst;
  }

  while (arg->sexpr->count) {
    cval* cell = cval_pop(arg, 0);
    cval* cmp  = cval_eval(env, cell);
    if (cmp->type == CVAL_ERR) {
      cval* err = cval_copy(cmp);

      cval_del(fst);
      cval_del(arg);

      return err;
    }

    if (cval_cmp(fst, cmp)) {
      cval_del(fst);
      cval_del(arg);

      return cval_num(1);
    }
  }

  cval_del(fst);
  cval_del(arg);

  return cval_num(0);
}

cval* builtin_ord(cenv* env, char* op, cval* arg) {
  CASSERT_MIN(arg, op, 2);

  cval* out = cval_eval(env, cval_pop(arg, 0));
  CASSERT_NOERR(out);
  CASSERT_TYPE(arg, op, out, CVAL_NUM);

  while (arg->sexpr->count) {
    cval* val = cval_eval(env, cval_pop(arg, 0));
    if (val->type == CVAL_ERR) {
      cval_del(out);
      cval_del(arg);

      return val;
    }
    CASSERT_TYPE(arg, op, val, CVAL_NUM);

    if ((strcmp(op, "lt?") && !(out->num < val->num))
        || (strcmp(op, "gt?") && !(out->num > val->num))) {
      cval_del(val);
      cval_del(out);
      cval_del(arg);
      return cval_num(0);
    }

    cval_del(out);
    out = val;
  }

  cval_del(out);
  cval_del(arg);

  return cval_num(1);
}

cval* builtin_lt(cenv* env, cval* arg) {
  return builtin_ord(env, "lt?", arg);
}

cval* builtin_gt(cenv* env, cval* arg) {
  return builtin_ord(env, "gt?", arg);
}

void cenv_add_builtins_conditional(cenv* env) {
  cenv_add_builtin_macro(env, "if", builtin_if);
  cenv_add_builtin_macro(env, "eq?", builtin_eq);

  cenv_add_builtin_macro(env, "lt?", builtin_lt);
  cenv_add_builtin_macro(env, "gt?", builtin_gt);
}
