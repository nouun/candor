#include "../builtins.h"

#include <stdio.h>

cval* builtin_head(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("head", 1);
  CASSERT_TYPE("head", 0, CVAL_SEXPR);
  CASSERT("head", args->sexpr->cell[0]->sexpr->count != 0,
          "index out of bounds, empty sexpr");

  return cval_take(cval_take(args, 0), 0);
}

cval* builtin_tail(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("tail", 1);
  CASSERT_TYPE("tail", 0, CVAL_SEXPR);
  CASSERT("tail", args->sexpr->cell[0]->sexpr->count > 1,
          "index out of bounds, expected sexpr with more than 1 element");

  cval* val = cval_take(args, 0);
  cval_del(cval_pop(val, 0));
  return val;
}

// TODO: Flatten
cval* builtin_join(cenv* env, cval* args) {
  (void)env;
  CASSERT_MIN("join", 1);
  for (int i = 0; i < args->sexpr->count; i++) {
    CASSERT_TYPE("join", i, CVAL_SEXPR);
  }

  cval* out = cval_pop(args, 0);
  while (args->sexpr->count) {
    cval* other = cval_pop(args, 0);

    while (other->sexpr->count) {
      cval* new = cval_pop(other, 0);
      out       = cval_add(out, new);
    }

    cval_del(other);
  }

  cval_del(args);
  return out;
}

cval* builtin_list(cenv* env, cval* args) {
  (void)env;
  return args;
}

cval* builtin_map(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("map", 2);
  CASSERT_TYPE4("map", 0, CVAL_FUN, CVAL_BFUN, CVAL_MCR, CVAL_BMCR);
  CASSERT_TYPE("map", 1, CVAL_SEXPR);
  /* cval* fn  = cval_pop(args, 0); */
  /* cval* lst = cval_take(args, 0); */

  // TODO: Implement map
  return cval_err("func(map): not implemented");
}

cval* builtin_reduce(cenv* env, cval* args) {
  (void)env;
  (void)args;
  // TODO: Implement reduce
  return cval_err("func(reduce): not implemented");
}

cval* builtin_filter(cenv* env, cval* args) {
  (void)env;
  (void)args;
  // TODO: Implement filter
  return cval_err("func(filter): not implemented");
}

void cenv_add_builtins_list(cenv* env) {
  cenv_add_builtin(env, "tail", builtin_tail);
  cenv_add_builtin(env, "head", builtin_head);
  cenv_add_builtin(env, "join", builtin_join);
  cenv_add_builtin(env, "list", builtin_list);

  cenv_add_builtin(env, "map", builtin_map);
  cenv_add_builtin(env, "reduce", builtin_reduce);
  cenv_add_builtin(env, "filter", builtin_filter);
}
