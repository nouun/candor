#ifndef BUILTINS_H
#define BUILTINS_H

#include "candor.h"
#include "cenv.h"
#include "cval.h"

#include <stdbool.h>

#define CASSERT_NOERR(args)                                                    \
  if (args->type == CVAL_ERR) {                                                \
    return args;                                                                \
  } else if (args->type == CVAL_SEXPR) {                                       \
    for (int i = 0; i < args->sexpr->count; i++) {                             \
      if (args->sexpr->cell[i]->type == CVAL_ERR) {                            \
        cval* err = cval_copy(args->sexpr->cell[i]);                           \
        cval_del(args);                                                        \
        return err;                                                            \
      }                                                                        \
    }                                                                          \
  }

#define CASSERT_COUNT(args, fn, cnt)                                           \
  if (arg->sexpr->count != cnt) {                                              \
    cval* err = cval_err("func(%s): Expected %li args, got %li", fn, cnt,      \
                         args->sexpr->count);                                  \
    cval_del(args);                                                            \
    return err;                                                                \
  }

#define CASSERT_RANGE(args, fn, min, max)                                      \
  if (arg->sexpr->count > max || arg->sexpr->count < min) {                    \
    cval* err = cval_err("func(%s): Expected %li-%li args, got %li", fn, min,  \
                         max, args->sexpr->count);                             \
    cval_del(args);                                                            \
    return err;                                                                \
  }

#define CASSERT_MIN(args, fn, min)                                             \
  if (arg->sexpr->count < min) {                                               \
    cval* err = cval_err("func(%s): Expected %li or more args, got %s", fn,    \
                         min, args->sexpr->count);                             \
    cval_del(args);                                                            \
    return err;                                                                \
  }

#define CASSERT_MAX(args, fn, max)                                             \
  if (arg->sexpr->count > max) {                                               \
    cval* err = cval_err("func(%s): Expected %li or less args, got %s", fn,    \
                         min, args->sexpr->count);                             \
    cval_del(args);                                                            \
    return err;                                                                \
  }

#define CASSERT_TYPE(args, fn, val, expected_type)                             \
  if (val->type != expected_type) {                                            \
    cval* err                                                                  \
      = cval_err("func(%s): Expected type '%s', got type '%s'", fn,            \
                 cval_type_str[expected_type], cval_type_str[val->type]);      \
    cval_del(args);                                                            \
    return err;                                                                \
  }

#define CASSERT_TYPE2(args, fn, val, type1, type2)                             \
  if (val->type != type1 && val->type != type2) {                              \
    cval* err = cval_err(                                                      \
      "func(%s): Expected type '%s' or '%s', got type '%s'", fn,               \
      cval_type_str[type1], cval_type_str[type2], cval_type_str[val->type]);   \
    cval_del(args);                                                            \
    return err;                                                                \
  }

#define CASSERT_TYPE3(args, fn, val, type1, type2, type3)                      \
  if (val->type != type1 && val->type != type2 && val->type != type3) {        \
    cval* err                                                                  \
      = cval_err("func(%s): Expected type '%s', '%s', or '%s', got type '%s'", \
                 fn, cval_type_str[type1], cval_type_str[type2],               \
                 cval_type_str[type3], cval_type_str[val->type]);              \
    cval_del(args);                                                            \
    return err;                                                                \
  }

#define CASSERT(args, cond, fmt, ...)                                          \
  if (!(cond)) {                                                               \
    cval* err = cval_err(fmt, ##__VA_ARGS__);                                  \
    cval_del(args);                                                            \
    return err;                                                                \
  }


cval* builtin_def(cenv* env, cval* arg, bool local);
cval* builtin_load(cenv* env, cval* arg);
void  cenv_add_builtins(cenv* env);

void cenv_add_builtins_conditional(cenv* env);
void cenv_add_builtins_list(cenv* env);
void cenv_add_builtins_math(cenv* env);
void cenv_add_builtins_string(cenv* env);

void cenv_add_builtin_str(cenv* env, char* name, char* str);
void cenv_add_builtin(cenv* env, char* name, cbuiltin func);
void cenv_add_builtin_macro(cenv* env, char* name, cbuiltin func);

#endif
