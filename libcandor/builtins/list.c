#include "../builtins.h"

#include <stdio.h>

cval* builtin_head(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "head", 1);
  CASSERT_TYPE(arg, "head", arg->sexpr->cell[0], CVAL_SEXPR);
  CASSERT(arg, arg->sexpr->cell[0]->sexpr->count != 0,
          "func(head): index out of bounds, empty sexpr");

  cval* val = cval_take(arg, 0);
  return cval_take(val, 0);
}

cval* builtin_tail(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "tail", 1);
  CASSERT_TYPE(arg, "tail", arg->sexpr->cell[0], CVAL_SEXPR);
  CASSERT(arg, arg->sexpr->cell[0]->sexpr->count != 0,
          "func(tail): Index out of bounds, empty sexpr");

  cval* val = cval_take(arg, 0);
  cval_del(cval_pop(val, 0));
  return val;
}

cval* builtin_join(cenv* env, cval* arg) {
  CASSERT_MIN(arg, "join", 1);
  for (int i = 0; i < arg->sexpr->count; i++) {
    CASSERT_TYPE(arg, "join", arg->sexpr->cell[i], CVAL_SEXPR);
  }

  cval* out = cval_pop(arg, 0);

  while (arg->sexpr->count) {
    cval* other = cval_pop(arg, 0);

    while (other->sexpr->count) { out = cval_add(out, cval_pop(other, 0)); }

    cval_del(other);
  }

  cval_del(arg);
  return out;
}

cval* builtin_list(cenv* env, cval* arg) {
  return arg;
}

void cenv_add_builtins_list(cenv* env) {
  cenv_add_builtin(env, "list", builtin_list);
  cenv_add_builtin(env, "join", builtin_join);
  cenv_add_builtin(env, "head", builtin_head);
  cenv_add_builtin(env, "tail", builtin_tail);
}
