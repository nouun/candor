#include "builtins.h"

#include <stdio.h>


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

  mpc_result_t res;
  if (mpc_parse_contents(name, Candor, &res)) {
    cval* file = cval_read(res.output);
    mpc_ast_delete(res.output);

    while (file->count) {
      cval* cell = cval_pop(file, 0);
      cval* expr = cval_eval(env, cell);
      if (expr->type == CVAL_ERR) { cval_println(expr); }
      cval_del(expr);
    }

    cval_del(file);
    free(name);

    return cval_sexpr();
  }

  char* msg = mpc_err_string(res.error);
  cval* err = cval_err("func(load): unable to load file:\n%s", msg);

  mpc_err_delete(res.error);
  free(name);
  free(msg);

  return err;
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

  if (arg->count == 1) {
    CASSERT_TYPE(arg, "exit", arg->cell[0], CVAL_NUM);

    status = arg->cell[0]->num;
  }

  cval_del(arg);
  exit(status);
}

cval* builtin_eval(cenv* env, cval* arg) {
  if (arg->count == 0) {
    cval_del(arg);
    return cval_sexpr();
  }

  cval* out = cval_eval(env, cval_copy(arg->cell[0]));
  cval_del(arg);
  return out;
}

cval* builtin_def(cenv* env, cval* arg) {
  CASSERT_RANGE(arg, "def", 1, 2);
  CASSERT_TYPE(arg, "def", arg->cell[0], CVAL_KYWD);

  // TODO: Implement docs
  /* if (arg->count == 3) { */
  /*   char  msg[ 256 ]; */
  /*   char* type = cval_type_str[ arg->cell[ 1 ]->type ]; */
  /*   snprintf(msg, sizeof(msg), "func(define): expected list, got %s", type);
   */
  /*   CASSERT(arg, arg->cell[ 1 ]->type == CVAL_STRING, msg); */
  /* } */

  cval* val;
  if (arg->count == 2) {
    val = arg->cell[1];
  } else {
    val = arg->cell[2];
  }

  cenv_def(env, arg->cell[0], val);
  cval_del(arg);

  return cval_sexpr();
}

cval* builtin_lambda(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "lambda", 2);
  CASSERT_TYPE(arg, "lambda", arg->cell[0], CVAL_SEXPR);

  cval* lambda = cval_lambda(arg->cell[0], arg->cell[1]);

  return lambda;
}

cval* builtin_defun(cenv* env, cval* arg) {
  CASSERT_RANGE(arg, "defun", 2, 3);
  CASSERT_TYPE2(arg, "defun", arg->cell[0], CVAL_KYWD, CVAL_SEXPR);

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
  if (arg->count == 1) {
    fun = cval_copy(arg->cell[0]);
  } else {
    fun = cval_copy(arg->cell[1]);
  }

  cval* lambda = cval_lambda(params, fun);

  cenv_def(env, key, lambda);
  cval_del(lambda);
  cval_del(key);
  cval_del(arg);

  return cval_sexpr();
}

cval* builtin_defmacro(cenv* env, cval* arg) {
  CASSERT_RANGE(arg, "defmcr", 2, 3);
  CASSERT_TYPE2(arg, "defmcr", arg->cell[0], CVAL_KYWD, CVAL_SEXPR);

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
  if (arg->count == 1) {
    fun = cval_copy(arg->cell[0]);
  } else {
    fun = cval_copy(arg->cell[1]);
  }

  cval* lambda = cval_lambda(params, fun);

  cenv_def(env, key, lambda);
  cval_del(lambda);
  cval_del(key);
  cval_del(arg);

  return cval_sexpr();
}

cval* builtin_print(cenv* env, cval* arg) {
  for (int i = 0; i < arg->count; i++) {
    if (arg->cell[i]->type == CVAL_STR) {
      printf("%s", arg->cell[i]->str);
    } else {
      cval_print(arg->cell[i]);
    }

    if (i != arg->count - 1) { putchar(' '); }
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
  if (arg->count == 0) {
    cval_del(arg);
    return cval_sexpr();
  }

  cval* out = cval_copy(arg->cell[arg->count - 1]);
  cval_del(arg);

  return out;
}

cval* builtin_typeof(cenv* env, cval* arg) {
  CASSERT_COUNT(arg, "typeof", 1);
  int type = arg->cell[0]->type;
  cval_del(arg);

  switch (type) {
  case CVAL_ERR: return cval_str("<error>");
  case CVAL_FUN: return cval_str("<function>");
  case CVAL_MCR: return cval_str("<macro>");
  case CVAL_NUM: return cval_str("<number>");
  case CVAL_STR: return cval_str("<string>");
  case CVAL_KYWD: return cval_str("<keyword>");
  case CVAL_QUOT: return cval_str("<quote>");
  case CVAL_QQUOT: return cval_str("<quasiquote>");
  case CVAL_SEXPR: return cval_str("<sexpression>");
  }

  return cval_str("<unknown type>");
}

void cenv_add_builtin_str(cenv* env, char* name, char* str) {
  cval* vals = candor_parse(name, str);
  while (vals->count) {
    cval* val = cval_pop(vals, 0);
    cval* res = cval_eval(env, val);
    if (res->type == CVAL_ERR) { cval_println(res); }
    cval_del(res);
    cval_del(val);
  }
  cval_del(vals);
}

void cenv_add_builtin_macro(cenv* env, char* name, cbuiltin func) {
  cval* val = cval_mcr(func);
  cval* key = cval_kywd(name);
  cenv_put(env, key, val);
  cval_del(key);
  cval_del(val);
}

void cenv_add_builtin(cenv* env, char* name, cbuiltin func) {
  cval* val = cval_fun(func);
  cval* key = cval_kywd(name);
  cenv_put(env, key, val);
  cval_del(key);
  cval_del(val);
}

void cenv_add_builtins(cenv* env) {
  cenv_add_builtin(env, "load", builtin_load);
  cenv_add_builtin(env, "exit", builtin_exit);
  cenv_add_builtin(env, "dump", builtin_dump);
  cenv_add_builtin(env, "typeof", builtin_typeof);

  cenv_add_builtin_macro(env, "defmcr", builtin_defmacro);
  cenv_add_builtin_macro(env, "def", builtin_def);
  cenv_add_builtin_macro(env, "defun", builtin_defun);
  cenv_add_builtin_macro(env, "lambda", builtin_lambda);

  cenv_add_builtin(env, "eval", builtin_eval);
  cenv_add_builtin(env, "do", builtin_do);
  cenv_add_builtin(env, "print", builtin_print);
  cenv_add_builtin(env, "println", builtin_println);

  cenv_add_builtins_conditional(env);
  cenv_add_builtins_list(env);
  cenv_add_builtins_math(env);
  cenv_add_builtins_sugar(env);
}
