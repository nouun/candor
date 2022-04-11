#include "../builtins.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cval* builtin_str_split(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("str/split", 2);
  CASSERT_TYPE("str/split", 0, CVAL_STR);
  CASSERT_TYPE("str/split", 1, CVAL_STR);

  cval* str = cval_pop(args, 0);
  cval* dlm = cval_take(args, 0);
  cval* out = cval_sexpr();

  char* ptr = strtok(str->str, dlm->str);
  while (ptr) {
    out = cval_add(out, cval_str(ptr));
    ptr = strtok(NULL, dlm->str);
  }

  free(ptr);
  cval_del(str);
  cval_del(dlm);

  return out;
}

cval* builtin_str_to_num(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("str->num", 1);
  CASSERT_TYPE("str->num", 0, CVAL_STR);

  cval* val = cval_take(args, 0);

  char* end;
  long  value = strtol(val->str, &end, 10);
  if (end == val->str || *end != '\0' || errno == ERANGE) {
    cval* err = cval_err(
      "func(str->num): unable to parse string '%s' to number", val->str);
    cval_del(val);
    return err;
  }

  cval_del(val);

  return cval_num(value);
}

void cenv_add_builtins_string(cenv* env) {
  cenv_add_builtin(env, "str/split", builtin_str_split);
  cenv_add_builtin(env, "str->num", builtin_str_to_num);
}
