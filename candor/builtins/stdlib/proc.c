#include "../stdlib.h"

#include <stdlib.h>


cval* stdlib_env_get(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("env/get", 1);
  CASSERT_TYPE("env/get", 0, CVAL_STR);

  cval* key = cval_take(args, 0);
  char* var = getenv(key->str);
  cval_del(key);

  if (!var) {
    var    = malloc(1);
    var[0] = '\0';
  }

  return cval_str(var);
}

cval* stdlib_env_set(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("env/set", 2);
  CASSERT_TYPE("env/set", 0, CVAL_STR);
  CASSERT_TYPE("env/set", 1, CVAL_STR);

  cval* key = cval_pop(args, 0);
  cval* val = cval_take(args, 0);

  cval* out = cval_num(setenv(key->str, val->str, 1));
  cval_del(key);
  cval_del(val);
  return out;
}

cval* stdlib_env_unset(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("env/unset", 1);
  CASSERT_TYPE("env/unset", 0, CVAL_STR);

  cval* key = cval_take(args, 0);
  cval* out = cval_num(unsetenv(key->str));
  cval_del(key);
  return out;
}

void stdlib_add_proc(cenv* env) {
  builtin_add_fun(env, "env/get", stdlib_env_get);
  builtin_add_fun(env, "env/set", stdlib_env_set);
  builtin_add_fun(env, "env/unset", stdlib_env_unset);
}
