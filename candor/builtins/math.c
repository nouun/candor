#include "../builtins.h"

#include <stdio.h>
#include <string.h>

cval* builtin_op(cenv* env, char op, cval* args) {
  char opstr[2] = { op, '\0' };
  CASSERT_MIN(opstr, 2);
  
  for (int i = 0; i < args->sexpr->count; i++) {
    CASSERT_TYPE(opstr, i, CVAL_NUM);
  }

  cval* out = cval_pop(args, 0);

  if ((op == '-') && (args->sexpr->count == 0)) { out->num = -out->num; }

  while (args->sexpr->count) {
    cval* y = cval_pop(args, 0);

    if (op == '+') { out->num += y->num; }
    if (op == '-') { out->num -= y->num; }
    if (op == '*') { out->num *= y->num; }
    if (op == '/') {
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

  cval_del(args);

  return out;
}

cval* builtin_add(cenv* env, cval* arg) {
  return builtin_op(env, '+', arg);
}

cval* builtin_sub(cenv* env, cval* arg) {
  return builtin_op(env, '-', arg);
}

cval* builtin_div(cenv* env, cval* arg) {
  return builtin_op(env, '/', arg);
}

cval* builtin_mult(cenv* env, cval* arg) {
  return builtin_op(env, '*', arg);
}

void cenv_add_builtins_math(cenv* env) {
  cenv_add_builtin(env, "+", builtin_add);
  cenv_add_builtin(env, "-", builtin_sub);
  cenv_add_builtin(env, "*", builtin_mult);
  cenv_add_builtin(env, "/", builtin_div);
}
