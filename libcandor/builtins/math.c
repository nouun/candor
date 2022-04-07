#include "../builtins.h"

#include <stdio.h>

cval* builtin_op(cenv* env, char* op, cval* arg) {
  CASSERT_MIN(arg, op, 2)
  for (int i = 0; i < arg->count; i++) {
    CASSERT_TYPE(arg, op, arg->cell[i], CVAL_NUM);
  }

  cval* out = cval_pop(arg, 0);

  if ((strcmp(op, "-") == 0) && arg->count == 0) { out->num = -out->num; }

  while (arg->count) {
    cval* y = cval_pop(arg, 0);

    if (strcmp(op, "+") == 0) { out->num += y->num; }
    if (strcmp(op, "-") == 0) { out->num -= y->num; }
    if (strcmp(op, "*") == 0) { out->num *= y->num; }
    if (strcmp(op, "/") == 0) {
      if (y->num == 0) {
        cval_del(out);
        cval_del(y);
        out = cval_err("division by zero");
        break;
      }

      out->num /= y->num;
    }

    cval_del(y);
  }

  cval_del(arg);

  return out;
}

cval* builtin_add(cenv* env, cval* arg) {
  return builtin_op(env, "+", arg);
}

cval* builtin_sub(cenv* env, cval* arg) {
  return builtin_op(env, "-", arg);
}

cval* builtin_div(cenv* env, cval* arg) {
  return builtin_op(env, "/", arg);
}

cval* builtin_mult(cenv* env, cval* arg) {
  return builtin_op(env, "*", arg);
}

void cenv_add_builtins_math(cenv* env) {
  cenv_add_builtin(env, "+", builtin_add);
  cenv_add_builtin(env, "-", builtin_sub);
  cenv_add_builtin(env, "*", builtin_mult);
  cenv_add_builtin(env, "/", builtin_div);
}
