#include "builtins.h"

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cval* builtin_import(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "import", 1);
  cval* val = cval_take(arg, 0);
  CASSERT_TYPE2(arg, "import", val, CVAL_STR, CVAL_KYWD);

  char* orig_name;
  if (val->type == CVAL_KYWD) {
    orig_name = val->kywd;
  } else {
    orig_name = val->str;
  }

  char* stdlib_dir = getenv("CANDOR_STDLIB_DIR");
  if (!stdlib_dir) { stdlib_dir = STDLIB_DIR; }

  char* name = malloc(strlen(stdlib_dir) + strlen(orig_name) + 7);
  sprintf(name, "%s/%s.cndr", stdlib_dir, orig_name);

  cval_del(val);

  return cval_load_file(env, name);
}

cval* builtin_load(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "load", 1);
  cval* val = cval_take(arg, 0);
  CASSERT_TYPE2(arg, "load", val, CVAL_STR, CVAL_KYWD);

  char* name;
  if (val->type == CVAL_KYWD) {
    name = malloc(strlen(val->kywd) + 1);
    strcpy(name, val->kywd);
  } else {
    name = malloc(strlen(val->str) + 1);
    strcpy(name, val->str);
  }
  cval_del(val);

  size_t len = strlen(name);
  if ((5 > len) || strncmp(name + len - 5, ".cndr", 5) != 0) {
    char* bak = malloc(len + 1);
    strcpy(bak, name);
    free(name);

    name = malloc(len + 6);
    sprintf(name, "%s.cndr", bak);

    free(bak);
  }

  return cval_load_file(env, name);
}

cval* builtin_dump(cenv* env, cval* arg) {
  printf("Dumping environment:\n");
  for (int i = 0; i < env->count; i++) {
    printf("%s: ", env->keys[i]);
    cval_println(env->vals[i]);
  }

  cval_del(arg);
  return cval_sexpr();
}

cval* builtin_exit(cenv* env, cval* arg) {
  CASSERT(arg, "exit", 0, 1);

  long status = 0;

  if (arg->sexpr->count == 1) {
    CASSERT_TYPE(arg, "exit", arg->sexpr->cell[0], CVAL_NUM);

    status = arg->sexpr->cell[0]->num;
  }

  cval_del(arg);
  exit(status);
}

cval* builtin_eval(cenv* env, cval* arg) {
  if (arg->sexpr->count == 0) {
    cval_del(arg);
    return cval_sexpr();
  }

  cval* out = cval_eval(env, cval_copy(arg->sexpr->cell[0]));
  cval_del(arg);
  return out;
}

cval* builtin_def(cenv* env, cval* arg, bool local) {
  CASSERT_RANGE(arg, "def", 1, 2);
  CASSERT_TYPE(arg, "def", arg->sexpr->cell[0], CVAL_KYWD);

  // TODO: Implement docs
  /* if (arg->count == 3) { */
  /*   char  msg[ 256 ]; */
  /*   char* type = cval_type_str[ arg->cell[ 1 ]->type ]; */
  /*   snprintf(msg, sizeof(msg), "func(define): expected list, got %s", type);
   */
  /*   CASSERT(arg, arg->cell[ 1 ]->type == CVAL_STRING, msg); */
  /* } */

  cval* val;
  if (arg->sexpr->count == 2) {
    val = arg->sexpr->cell[1];
  } else {
    val = arg->sexpr->cell[2];
  }

  if (local) {
    cenv_put(env, arg->sexpr->cell[0], val);
  } else {
    cenv_def(env, arg->sexpr->cell[0], val);
  }

  cval_del(arg);

  return cval_sexpr();
}

cval* builtin_def_local(cenv* env, cval* arg) {
  return builtin_def(env, arg, true);
}

cval* builtin_def_global(cenv* env, cval* arg) {
  return builtin_def(env, arg, false);
}


cval* builtin_lambda(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "lambda", 2);
  CASSERT_TYPE(arg, "lambda", arg->sexpr->cell[0], CVAL_SEXPR);

  return cval_fun(arg->sexpr->cell[0], arg->sexpr->cell[1]);
}

cval* builtin_defun(cenv* env, cval* arg, bool local) {
  CASSERT_RANGE(arg, "defun", 2, 3);
  CASSERT_TYPE2(arg, "defun", arg->sexpr->cell[0], CVAL_KYWD, CVAL_SEXPR);

  cval* def = cval_pop(arg, 0);
  cval* key;
  cval* params;
  if (def->type == CVAL_KYWD) {
    key    = def;
    params = cval_sexpr();
  } else {
    key    = cval_pop(def, 0);
    params = cval_copy(def);
    cval_del(def);
  }

  cval* fun;
  if (arg->sexpr->count == 1) {
    fun = cval_copy(arg->sexpr->cell[0]);
  } else {
    fun = cval_copy(arg->sexpr->cell[1]);
  }

  cval* lambda = cval_fun(params, fun);

  if (local) {
    cenv_put(env, key, lambda);
  } else {
    cenv_def(env, key, lambda);
  }
  cval_del(lambda);
  cval_del(key);
  cval_del(arg);

  return cval_sexpr();
}

cval* builtin_defun_local(cenv* env, cval* arg) {
  return builtin_defun(env, arg, true);
}

cval* builtin_defun_global(cenv* env, cval* arg) {
  return builtin_defun(env, arg, false);
}

cval* builtin_defmacro(cenv* env, cval* arg) {
  CASSERT_RANGE(arg, "defmcr", 2, 3);
  CASSERT_TYPE2(arg, "defmcr", arg->sexpr->cell[0], CVAL_KYWD, CVAL_SEXPR);

  cval* def = cval_pop(arg, 0);
  cval* key;
  cval* params;
  if (def->type == CVAL_KYWD) {
    key    = def;
    params = cval_sexpr();
  } else {
    key    = cval_pop(def, 0);
    params = cval_copy(def);
    cval_del(def);
  }

  cval* fun;
  if (arg->sexpr->count == 1) {
    fun = cval_copy(arg->sexpr->cell[0]);

  } else {
    fun = cval_copy(arg->sexpr->cell[1]);
  }

  cval* lambda = cval_fun(params, fun);

  cenv_def(env, key, lambda);
  cval_del(lambda);
  cval_del(key);
  cval_del(arg);

  return cval_sexpr();
}

cval* builtin_print(cenv* env, cval* arg) {
  for (int i = 0; i < arg->sexpr->count; i++) {
    if (arg->sexpr->cell[i]->type == CVAL_STR) {
      printf("%s", arg->sexpr->cell[i]->str);
    } else {
      cval_print(arg->sexpr->cell[i]);
    }

    if (i != arg->sexpr->count - 1) { putchar(' '); }
  }

  cval_del(arg);
  return cval_sexpr();
}

cval* builtin_println(cenv* env, cval* arg) {
  cval* out = builtin_print(env, arg);
  putchar('\n');
  return out;
}

cval* builtin_do(cenv* env, cval* arg) {
  if (arg->sexpr->count == 0) {
    cval_del(arg);
    return cval_sexpr();
  }

  cval* out = cval_copy(arg->sexpr->cell[arg->sexpr->count - 1]);
  cval_del(arg);

  return out;
}

cval* builtin_typeof(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "typeof", 1);
  int type = arg->sexpr->cell[0]->type;
  cval_del(arg);

  // Hijack cval_err to use formatting
  cval* res = cval_err("<%s>", cval_type_str[type]);
  res->type = CVAL_STR;
  return res;
}

void cenv_add_builtin_str(cenv* env, char* name, char* str) {
  cval* vals = candor_load(name, str);
  while (vals->sexpr->count) {
    cval* val = cval_pop(vals, 0);
    cval* res = cval_eval(env, val);
    if (res->type == CVAL_ERR) { cval_println(res); }
    cval_del(res);
    cval_del(val);
  }
  cval_del(vals);
}

void cenv_add_builtin_macro(cenv* env, char* name, cbuiltin func) {
  cval* val = cval_bmcr(func);
  cval* key = cval_kywd(name);
  cenv_put(env, key, val);
  cval_del(key);
  cval_del(val);
}

void cenv_add_builtin(cenv* env, char* name, cbuiltin func) {
  cval* val = cval_bfun(func);
  cval* key = cval_kywd(name);
  cenv_put(env, key, val);
  cval_del(key);
  cval_del(val);
}

void cenv_add_builtins(cenv* env) {
  cenv_add_builtin(env, "import", builtin_import);
  cenv_add_builtin(env, "load", builtin_load);
  cenv_add_builtin(env, "exit", builtin_exit);
  cenv_add_builtin(env, "dump", builtin_dump);
  cenv_add_builtin(env, "typeof", builtin_typeof);

  cenv_add_builtin_macro(env, "defmcr", builtin_defmacro);
  cenv_add_builtin_macro(env, "def", builtin_def_local);
  cenv_add_builtin_macro(env, "def!", builtin_def_global);
  cenv_add_builtin_macro(env, "defun", builtin_defun_local);
  cenv_add_builtin_macro(env, "defun!", builtin_defun_global);
  cenv_add_builtin_macro(env, "lambda", builtin_lambda);

  cenv_add_builtin(env, "eval", builtin_eval);
  cenv_add_builtin(env, "do", builtin_do);
  cenv_add_builtin(env, "print", builtin_print);
  cenv_add_builtin(env, "println", builtin_println);

  cenv_add_builtins_conditional(env);
  cenv_add_builtins_list(env);
  cenv_add_builtins_math(env);
}
