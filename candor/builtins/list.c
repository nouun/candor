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

  cval* fn  = cval_pop(args, 0);
  cval* lst = cval_take(args, 0);

  for (int i = 0; i < lst->sexpr->count; i++) {
    cval* args          = cval_add(cval_sexpr(), lst->sexpr->cell[i]);
    lst->sexpr->cell[i] = cval_call(env, cval_copy(fn), args);
  }

  cval_del(fn);

  return lst;
}

cval* builtin_reduce(cenv* env, cval* args) {
  (void)env;
  (void)args;
  // TODO: Implement reduce
  return cval_err("func(reduce): not implemented");
}

cval* builtin_filter(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("filter", 2);
  CASSERT_TYPE4("filter", 0, CVAL_FUN, CVAL_BFUN, CVAL_MCR, CVAL_BMCR);
  CASSERT_TYPE("filter", 1, CVAL_SEXPR);

  cval* fn  = cval_pop(args, 0);
  cval* lst = cval_take(args, 0);

  cval* out = cval_sexpr();

  while (lst->sexpr->count) {
    cval* val = cval_pop(lst, 0);
    cval* args = cval_add(cval_sexpr(), cval_copy(val));
    cval* ret  = cval_call(env, cval_copy(fn), args);
    if (ret->type != CVAL_NUM) {
      cval* err = cval_err("func(filter): Expected type '%s', got type '%s'",
                           cval_type_str[CVAL_NUM], cval_type_str[ret->type]);
      cval_del(fn);
      cval_del(lst);
      cval_del(ret);
      cval_del(args);
      return err;
    }

    if(ret->num) {
      out = cval_add(out, val);
    } else {
      cval_del(val);
    }

    cval_del(ret);
  }

  cval_del(fn);
  cval_del(lst);

  return out;
}

void builtins_add_list(cenv* env) {
  builtin_add_fun(env, "tail", builtin_tail);
  builtin_add_fun(env, "head", builtin_head);
  builtin_add_fun(env, "join", builtin_join);
  builtin_add_fun(env, "list", builtin_list);
  builtin_add_fun(env, "map", builtin_map);
  builtin_add_fun(env, "reduce", builtin_reduce);
  builtin_add_fun(env, "filter", builtin_filter);
}
