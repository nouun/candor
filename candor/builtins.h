#ifndef BUILTINS_H
#define BUILTINS_H

#include "candor.h"
#include "cenv.h"
#include "cval.h"

#include <stdarg.h>
#include <stdbool.h>


#define CEVAL_MACRO_ARG(idx, ...)                                              \
  args->sexpr->cell[idx] = cval_eval(env, args->sexpr->cell[idx]);             \
  if (args->sexpr->cell[idx]->type == CVAL_ERR) {                              \
    cval* err = cval_pop(args, idx);                                           \
    cval_del(args);                                                            \
    __VA_ARGS__;                                                               \
    return err;                                                                \
  }

#define CASSERT_COUNT(fn, cnt, ...)                                            \
  if (args->sexpr->count != cnt) {                                             \
    cval* err = cval_err("func(%s): Expected %li args, got %li", fn, cnt,      \
                         args->sexpr->count);                                  \
    cval_del(args);                                                            \
    __VA_ARGS__;                                                               \
    return err;                                                                \
  }

#define CASSERT_RANGE(fn, min, max, ...)                                       \
  if (args->sexpr->count > max || args->sexpr->count < min) {                  \
    cval* err = cval_err("func(%s): Expected %li-%li args, got %li", fn, min,  \
                         max, args->sexpr->count);                             \
    cval_del(args);                                                            \
    __VA_ARGS__;                                                               \
    return err;                                                                \
  }

#define CASSERT_MIN(fn, min, ...)                                              \
  if (args->sexpr->count < min) {                                              \
    cval* err = cval_err("func(%s): Expected %li or more args, got %s", fn,    \
                         min, args->sexpr->count);                             \
    cval_del(args);                                                            \
    __VA_ARGS__;                                                               \
    return err;                                                                \
  }

#define CASSERT_MAX(fn, max, ...)                                              \
  if (args->sexpr->count > max) {                                              \
    cval* err = cval_err("func(%s): Expected %li or less args, got %s", fn,    \
                         min, args->sexpr->count);                             \
    cval_del(args);                                                            \
    __VA_ARGS__;                                                               \
    return err;                                                                \
  }

#define CASSERT_TYPE(fn, idx, expected_type, ...)                              \
  if (args->sexpr->cell[idx]->type != expected_type) {                         \
    cval* err = cval_err("func(%s): Expected type '%s', got type '%s'", fn,    \
                         cval_type_str[expected_type],                         \
                         cval_type_str[args->sexpr->cell[idx]->type]);         \
    cval_del(args);                                                            \
    __VA_ARGS__;                                                               \
    return err;                                                                \
  }

#define CASSERT_TYPE2(fn, idx, type1, type2, ...)                              \
  if (args->sexpr->cell[idx]->type != type1                                    \
      && args->sexpr->cell[idx]->type != type2) {                              \
    cval* err                                                                  \
      = cval_err("func(%s): Expected type '%s' or '%s', got type '%s'", fn,    \
                 cval_type_str[type1], cval_type_str[type2],                   \
                 cval_type_str[args->sexpr->cell[idx]->type]);                 \
    cval_del(args);                                                            \
    __VA_ARGS__;                                                               \
    return err;                                                                \
  }

#define CASSERT_TYPE3(fn, idx, type1, type2, type3, ...)                       \
  if (args->sexpr->cell[idx]->type != type1                                    \
      && args->sexpr->cell[idx]->type != type2                                 \
      && args->sexpr->cell[idx]->type != type3) {                              \
    cval* err = cval_err(                                                      \
      "func(%s): Expected type '%s', '%s', or '%s', got type '%s'", fn,        \
      cval_type_str[type1], cval_type_str[type2], cval_type_str[type3],        \
      cval_type_str[args->sexpr->cell[idx]->type]);                            \
    cval_del(args);                                                            \
    __VA_ARGS__;                                                               \
    return err;                                                                \
  }

#define CASSERT_TYPE4(fn, idx, type1, type2, type3, type4, ...)                \
  if (args->sexpr->cell[idx]->type != type1                                    \
      && args->sexpr->cell[idx]->type != type2                                 \
      && args->sexpr->cell[idx]->type != type3                                 \
      && args->sexpr->cell[idx]->type != type4) {                              \
    cval* err = cval_err(                                                      \
      "func(%s): Expected type '%s', '%s', '%s', or '%s', got type '%s'", fn,  \
      cval_type_str[type1], cval_type_str[type2], cval_type_str[type3],        \
      cval_type_str[type4], cval_type_str[args->sexpr->cell[idx]->type]);      \
    cval_del(args);                                                            \
    __VA_ARGS__;                                                               \
    return err;                                                                \
  }

#define CASSERT(fn, cond, msg, ...)                                            \
  if (!(cond)) {                                                               \
    cval_del(args);                                                            \
    __VA_ARGS__;                                                               \
    return cval_err("func(%s): %s", fn, msg);                                  \
  }


cval* builtin_def(cenv* env, cval* args, bool local);
cval* builtin_load(cenv* env, cval* args);
void  cenv_add_builtins(cenv* env);

void cenv_add_builtins_conditional(cenv* env);
void cenv_add_builtins_list(cenv* env);
void cenv_add_builtins_math(cenv* env);
void cenv_add_builtins_string(cenv* env);

void cenv_add_builtin(cenv* env, char* name, cbuiltin func);
void cenv_add_builtin_macro(cenv* env, char* name, cbuiltin func);

#endif
