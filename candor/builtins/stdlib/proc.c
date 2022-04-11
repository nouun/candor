#include "../stdlib.h"

#include <stdlib.h>

cval* stdlib_env_get(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "env/get", 1);
  cval* key = cval_take(arg, 0);
  CASSERT_TYPE(arg, "env/get", key, CVAL_STR);

  char* var = getenv(key->str);
  cval_del(key);

  if (!var) {
    var    = malloc(1);
    var[0] = '\0';
  }

  return cval_str(var);
}

cval* stdlib_env_set(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "env/set", 2);
  cval* key = cval_pop(arg, 0);
  cval* val = cval_take(arg, 0);
  CASSERT_TYPE(arg, "env/set", key, CVAL_STR);
  CASSERT_TYPE(arg, "env/set", val, CVAL_STR);

  cval* out = cval_num(setenv(key->str, val->str, 1));
  cval_del(key);
  cval_del(val);
  return out;
}

cval* stdlib_env_unset(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "env/unset", 1);
  cval* key = cval_take(arg, 0);
  CASSERT_TYPE(arg, "env/unset", key, CVAL_STR);

  cval* out = cval_num(unsetenv(key->str));
  cval_del(key);
  return out;
}

void cenv_add_stdlib_proc(cenv* env) {
  cenv_add_builtin(env, "env/get", stdlib_env_get);
  cenv_add_builtin(env, "env/set", stdlib_env_set);
  cenv_add_builtin(env, "env/unset", stdlib_env_unset);
}
