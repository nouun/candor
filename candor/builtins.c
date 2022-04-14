#include "builtins.h"

#include "builtins/stdlib.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMPORT_BUILTIN_STDLIB(libname, func, env)                              \
  if (strstr(orig_name, libname)) {                                            \
    func(env);                                                                 \
    cval_del(val);                                                             \
    return cval_sexpr();                                                       \
  }

cval* builtin_import(cenv* env, cval* args) {
  CASSERT_COUNT("import", 1);
  CASSERT_TYPE2("import", 0, CVAL_STR, CVAL_KYWD);

  cval* val = cval_take(args, 0);
  char* orig_name;

  if (val->type == CVAL_KYWD) orig_name = val->kywd;
  else
    orig_name = val->str;

  IMPORT_BUILTIN_STDLIB("proc", stdlib_add_proc, env)

  char* stdlib_dir = getenv("CANDOR_STDLIB_DIR");
  if (!stdlib_dir) { stdlib_dir = STDLIB_DIR; }

  char* name = malloc(strlen(stdlib_dir) + strlen(orig_name) + 7);
  sprintf(name, "%s/%s.cndr", stdlib_dir, orig_name);

  cval_del(val);

  cval* out = cval_load_file(env, name);
  free(name);
  return out;
}

cval* builtin_load(cenv* env, cval* args) {
  CASSERT_COUNT("load", 1);
  CASSERT_TYPE2("load", 0, CVAL_STR, CVAL_KYWD);

  cval* val = cval_take(args, 0);
  char* name;
  if (val->type == CVAL_KYWD) {
    name = val->kywd;
  } else {
    name = val->str;
  }
  free(val);

  size_t len = strlen(name);
  if (5 > len || strncmp(name + len - 5, ".cndr", 5) != 0) {
    char* bak = strdup(name);
    free(name);

    name = malloc(len + 6);
    // TODO: Test if correct len
    snprintf(name, len + 6, "%s.cndr", bak);

    free(bak);
  }

  cval* out = cval_load_file(env, name);
  free(name);
  return out;
}

cval* builtin_dump(cenv* env, cval* args) {
  CASSERT_COUNT("dump", 0);

  printf("Dumping environment:\n");
  for (int i = 0; i < env->count; i++) {
    printf("%s: ", env->keys[i]);
    cval_println(env->vals[i]);
  }

  cval_del(args);
  return cval_sexpr();
}

cval* builtin_exit(cenv* env, cval* args) {
  (void)env;
  CASSERT_RANGE("exit", 0, 1);

  long status = 0;

  if (args->sexpr->count == 1) {
    CASSERT_TYPE("exit", 0, CVAL_NUM);

    status = args->sexpr->cell[0]->num;
  }

  cval_del(args);
  exit(status);
}

cval* builtin_eval(cenv* env, cval* args) {
  CASSERT_COUNT("eval", 1);

  cval* body = cval_take(args, 0);
  return cval_eval(env, body);
}

cval* builtin_do(cenv* env, cval* args) {
  (void)env;
  if (args->sexpr->count == 0) {
    cval_del(args);
    return cval_sexpr();
  }

  return cval_take(args, args->sexpr->count - 1);
}

cval* builtin_def(cenv* env, cval* args, bool local) {
  CASSERT_RANGE("def", 1, 2);
  CASSERT_TYPE("def", 0, CVAL_KYWD);

  // TODO: Implement docs
  /* if (arg->count == 3) { */
  /*   char  msg[ 256 ]; */
  /*   char* type = cval_type_str[ arg->cell[ 1 ]->type ]; */
  /*   snprintf(msg, sizeof(msg), "func(define): expected list, got %s", type);
   */
  /*   CASSERT(arg, arg->cell[ 1 ]->type == CVAL_STRING, msg); */
  /* } */

  cval* val = cval_eval(env, cval_pop(args, args->sexpr->count == 2 ? 1 : 2));
  if (val->type == CVAL_ERR) {
    cval_del(args);
    return val;
  }

  cval* kywd = cval_take(args, 0);
  if (local) cenv_put(env, kywd->kywd, val);
  else
    cenv_def(env, kywd->kywd, val);

  free(kywd);

  return cval_sexpr();
}

cval* builtin_def_local(cenv* env, cval* arg) {
  return builtin_def(env, arg, true);
}

cval* builtin_def_global(cenv* env, cval* arg) {
  return builtin_def(env, arg, false);
}

cval* builtin_lambda(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("lambda", 2);
  CASSERT_TYPE("lambda", 0, CVAL_SEXPR);

  cval* params = args->sexpr->cell[0];
  cval* body   = args->sexpr->cell[1];

  // Free without freeing cells to avoid copying body
  free(args->sexpr->cell);
  free(args->sexpr);
  free(args);

  return cval_fun(params, body);
}

cval* builtin_define(cenv* env, cval* args, char* name, bool local, bool macro) {
  CASSERT_RANGE(name, 2, 3);
  CASSERT_TYPE2(name, 0, CVAL_KYWD, CVAL_SEXPR);

  cval* def = cval_pop(args, 0);

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
  if (args->sexpr->count == 1) {
    fun = cval_take(args, 0);
  } else {
    cval* docstr = cval_pop(args, 0);
    fun = cval_take(args, 0);

    // TODO: Docs
    cval_del(docstr);
  }

  cval* lambda = cval_fun(params, fun);
  if (macro) {
    lambda->type = CVAL_MCR;
  }

  if (local) {
    cenv_put(env, key->str, lambda);
  } else {
    cenv_def(env, key->str, lambda);
  }

  free(key);

  return cval_sexpr();
}

cval* builtin_defproc_local(cenv* env, cval* args) {
  return builtin_define(env, args, "defproc", true, false);
}

cval* builtin_defproc_global(cenv* env, cval* args) {
  return builtin_define(env, args, "defproc!", false, false);
}

cval* builtin_defmacro_local(cenv* env, cval* args) {
  return builtin_define(env, args, "defmacro", true, true);
}

cval* builtin_defmacro_global(cenv* env, cval* args) {
  return builtin_define(env, args, "defmacro!", false, true);
}

cval* builtin_let(cenv* env, cval* args) {
  CASSERT_COUNT("let", 2);
  CASSERT_TYPE("let", 0, CVAL_SEXPR);

  cenv* scope = cenv_new();
  scope->par  = env;

  cval* lets = cval_pop(args, 0);
  while (lets->sexpr->count) {
    cval* let = cval_pop(lets, 0);

    cval* kywd  = cval_pop(let, 0);
    cval* value = cval_eval(scope, cval_pop(let, 0));

    cenv_put(scope, kywd->kywd, value);

    cval_del(let);
    free(kywd);
  }
  cval_del(lets);

  cval* out = cval_eval(scope, cval_take(args, 0));
  cenv_del(scope);

  return out;
}

cval* builtin_len(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("len", 1);
  CASSERT_TYPE3("len", 0, CVAL_STR, CVAL_SEXPR, CVAL_KYWD);

  cval* val = cval_take(args, 0);
  int out;
  switch (val->type) {
  case CVAL_STR: out = strlen(val->str); break;
  case CVAL_KYWD: out = strlen(val->kywd); break;
  case CVAL_SEXPR: out = val->sexpr->count; break;
  default: out = 0;
  }

  cval* res = cval_num(out);
  cval_del(val);
  return res;
}

cval* builtin_print(cenv* env, cval* args) {
  (void)env;
  while(args->sexpr->count) {
    cval* val = cval_pop(args, 0);
    if(val->type == CVAL_STR) {
      printf("%s", val->str);
    } else {
      cval_print(val);
    }

    cval_del(val);
  }
  
  cval_del(args);
  return cval_sexpr();
}

cval* builtin_println(cenv* env, cval* arg) {
  cval* out = builtin_print(env, arg);
  putchar('\n');
  return out;
}

cval* builtin_typeof(cenv* env, cval* args) {
  (void)env;
  CASSERT_COUNT("typeof", 1);

  cval* val = cval_take(args, 0);

  // Hijack cval_err to use formatting
  cval* res = cval_err("<%s>", cval_type_str[val->type]);
  res->type = CVAL_STR;

  cval_del(val);
  return res;
}

void builtin_add_mcr(cenv* env, char* name, cbuiltin func) {
  cval* val = cval_bmcr(func);
  // Duplicate the name so that we can free it with the
  // rest of the environment keys at deinit
  cenv_put(env, strdup(name), val);
}

void builtin_add_fun(cenv* env, char* name, cbuiltin func) {
  cval* val = cval_bfun(func);
  cenv_put(env, strdup(name), val);
}

void cenv_add_builtins(cenv* env) {
  builtin_add_fun(env, "import", builtin_import);
  builtin_add_fun(env, "load", builtin_load);
  builtin_add_fun(env, "exit", builtin_exit);
  builtin_add_fun(env, "dump", builtin_dump);
  builtin_add_fun(env, "typeof", builtin_typeof);

  builtin_add_fun(env, "len", builtin_len);
  builtin_add_mcr(env, "let", builtin_let);
  
  builtin_add_mcr(env, "def", builtin_def_local);
  builtin_add_mcr(env, "def!", builtin_def_global);
  builtin_add_mcr(env, "defproc", builtin_defproc_local);
  builtin_add_mcr(env, "defproc!", builtin_defproc_global);
  builtin_add_mcr(env, "defmcr", builtin_defmacro_local);
  builtin_add_mcr(env, "defmcr!", builtin_defmacro_global);
  builtin_add_mcr(env, "lambda", builtin_lambda);

  builtin_add_fun(env, "eval", builtin_eval);
  builtin_add_fun(env, "do", builtin_do);
  builtin_add_fun(env, "print", builtin_print);
  builtin_add_fun(env, "println", builtin_println);

  builtins_add_conditional(env);
  builtins_add_list(env);
  builtins_add_math(env);
  builtins_add_string(env);
}
