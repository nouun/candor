#include "../builtins.h"

#include <stdio.h>
#include <string.h>

cval* builtin_op(cenv* env, char op, cval* args) {
  (void)env;
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

cval* builtin_add(cenv* env, cval* args) {
  return builtin_op(env, '+', args);
}

cval* builtin_sub(cenv* env, cval* args) {
  return builtin_op(env, '-', args);
}

cval* builtin_div(cenv* env, cval* args) {
  return builtin_op(env, '/', args);
}

cval* builtin_mult(cenv* env, cval* args) {
  return builtin_op(env, '*', args);
}

cval* builtin_mod(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("%", 2);
  CASSERT_TYPE("%", 0, CVAL_NUM);
  CASSERT_TYPE("%", 1, CVAL_NUM);

  cval* val = cval_pop(args, 0);
  cval* base = cval_take(args, 0);

  int out = val->num % base->num;
  
  cval_del(val);
  cval_del(base);

  return cval_num(out);
}

void builtins_add_math(cenv* env) {
  builtin_add_fun(env, "+", builtin_add);
  builtin_add_fun(env, "-", builtin_sub);
  builtin_add_fun(env, "*", builtin_mult);
  builtin_add_fun(env, "/", builtin_div);
  builtin_add_fun(env, "%", builtin_mod);
}
