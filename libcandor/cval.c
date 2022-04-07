#include "cval.h"

#include "builtins.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

cval* cval_num(long num) {
  cval* v = malloc(sizeof(cval));
  v->type = CVAL_NUM;
  v->num  = num;
  return v;
}

cval* cval_str(char* str) {
  cval* v = malloc(sizeof(cval));
  v->type = CVAL_STR;
  v->str  = malloc(strlen(str) + 1);
  strcpy(v->str, str);
  return v;
}

cval* cval_err(char* fmt, ...) {
  cval* v = malloc(sizeof(cval));
  v->type = CVAL_ERR;

  va_list va;
  va_start(va, fmt);

  v->err = malloc(1024);
  vsnprintf(v->err, 1023, fmt, va);
  v->err = realloc(v->err, strlen(v->err) + 1);

  va_end(va);

  return v;
}

cval* cval_kywd(char* kywd) {
  cval* v = malloc(sizeof(cval));
  v->type = CVAL_KYWD;
  v->kywd = malloc(sizeof(strlen(kywd) + 1));
  strcpy(v->kywd, kywd);
  return v;
}

cval* cval_sexpr(void) {
  cval* v  = malloc(sizeof(cval));
  v->type  = CVAL_SEXPR;
  v->count = 0;
  v->cell  = NULL;
  return v;
}

cval* cval_qquot(cval* val) {
  cval* v = malloc(sizeof(cval));
  v->type = CVAL_QQUOT;
  v->quot = val;
  return v;
}

cval* cval_quot(cval* val) {
  cval* v = malloc(sizeof(cval));
  v->type = CVAL_QUOT;
  v->quot = val;
  return v;
}

cval* cval_fun(cbuiltin func) {
  cval* v    = malloc(sizeof(cval));
  v->type    = CVAL_FUN;
  v->builtin = func;
  return v;
}

cval* cval_mcr(cbuiltin func) {
  cval* v    = malloc(sizeof(cval));
  v->type    = CVAL_MCR;
  v->builtin = func;
  return v;
}

cval* cval_macro(cval* formals, cval* body) {
  cval* v = cval_lambda(formals, body);
  v->type = CVAL_MCR;
  return v;
}

cval* cval_lambda(cval* formals, cval* body) {
  cval* v = malloc(sizeof(cval));
  v->type = CVAL_FUN;

  v->builtin = NULL;
  v->env     = cenv_new();

  v->formals = formals;
  v->body    = body;

  return v;
}

void cval_del(cval* val) {
  switch (val->type) {
  case CVAL_MCR:
  case CVAL_FUN:
    if (!val->builtin) {
      cenv_del(val->env);
      cval_del(val->formals);
      cval_del(val->body);
    }
    break;
  case CVAL_NUM: break;
  case CVAL_STR: free(val->str); break;
  case CVAL_KYWD: free(val->kywd); break;
  case CVAL_ERR: free(val->err); break;
  case CVAL_QQUOT:
  case CVAL_QUOT: cval_del(val->quot); break;
  case CVAL_SEXPR:
    for (int i = 0; i < val->count; i++) { cval_del(val->cell[i]); }
    free(val->cell);
    break;
  }

  free(val);
}

int cval_cmp(cval* lhs, cval* rhs) {
  if (lhs->type != rhs->type) { return 0; }

  switch (lhs->type) {
  case CVAL_NUM: return lhs->num == rhs->num;
  case CVAL_ERR: return strcmp(lhs->err, rhs->err) == 0;
  case CVAL_STR: return strcmp(lhs->str, rhs->str) == 0;
  case CVAL_KYWD: return strcmp(lhs->kywd, rhs->kywd) == 0;
  case CVAL_SEXPR: {
    if (lhs->count != rhs->count) {
      return 0;
    } else {
      for (int i = 0; i < lhs->count; i++) {
        if (!cval_cmp(lhs->cell[i], rhs->cell[i])) { return 0; }
      }
      return 1;
    }
  }
  case CVAL_QQUOT:
  case CVAL_QUOT: return cval_cmp(lhs->quot, rhs->quot);
  case CVAL_MCR:
  case CVAL_FUN:
    return (lhs->builtin) ? lhs->builtin == rhs->builtin
                          : cval_cmp(lhs->body, rhs->body)
                              && cval_cmp(lhs->formals, rhs->formals);
  }

  // How tf
  return 0;
}

void cval_print_expr(cval* val, char open, char close) {
  putchar(open);
  for (int i = 0; i < val->count; i++) {
    cval_print(val->cell[i]);

    if (i != (val->count - 1)) { putchar(' '); }
  }
  putchar(close);
}

void cval_print(cval* val) {
  switch (val->type) {
  case CVAL_NUM: printf("%li", val->num); break;
  case CVAL_STR: {
    char* esc = malloc(strlen(val->str) + 1);
    strcpy(esc, val->str);
    esc = mpcf_escape(esc);
    printf("\"%s\"", esc);
    free(esc);
    break;
  }
  case CVAL_ERR: printf("error: %s", val->err); break;
  case CVAL_MCR:
    if (val->builtin) {
      printf("<builtin-macro>");
    } else {
      printf("<macro>");
    }
    break;
  case CVAL_FUN:
    if (val->builtin) {
      printf("<builtin>");
    } else {
      printf("<function>");
    }
    break;
  case CVAL_KYWD: printf("%s", val->kywd); break;
  case CVAL_QQUOT:
  case CVAL_QUOT: {
    putchar('(');
    cval_print(val->quot);
    putchar(')');
    break;
  }
  case CVAL_SEXPR: cval_print_expr(val, '(', ')'); break;
  }
}

void cval_println(cval* val) {
  cval_print(val);
  putchar('\n');
}

cenv* cenv_copy(cenv* env);

cval* cval_copy(cval* val) {
  cval* out = malloc(sizeof(cval));
  out->type = val->type;

  switch (out->type) {
  case CVAL_MCR:
  case CVAL_FUN:
    if (val->builtin) {
      out->builtin = val->builtin;
    } else {
      out->builtin = NULL;
      out->env     = cenv_copy(val->env);
      out->formals = cval_copy(val->formals);
      out->body    = cval_copy(val->body);
    }
    break;
  case CVAL_NUM: out->num = val->num; break;
  case CVAL_STR:
    out->str = malloc(sizeof(val->str) + 1);
    strcpy(out->str, val->str);
    break;
  case CVAL_ERR:
    out->err = malloc(sizeof(val->err) + 1);
    strcpy(out->err, val->err);
    break;

  case CVAL_KYWD:
    out->kywd = malloc(sizeof(val->kywd) + 1);
    strcpy(out->kywd, val->kywd);
    break;

  case CVAL_QQUOT:
  case CVAL_QUOT: out->quot = cval_copy(val->quot); break;

  case CVAL_SEXPR:
    out->count = val->count;
    out->cell  = malloc(sizeof(cval*) * out->count);
    for (int i = 0; i < out->count; i++) {
      out->cell[i] = cval_copy(val->cell[i]);
    }
    break;
  }

  return out;
}

cenv* cenv_new(void) {
  cenv* env  = malloc(sizeof(cenv));
  env->par   = NULL;
  env->count = 0;
  env->keys  = NULL;
  env->vals  = NULL;
  return env;
}

void cenv_del(cenv* env) {
  for (int i = 0; i < env->count; i++) {
    free(env->keys[i]);
    cval_del(env->vals[i]);
  }

  free(env->keys);
  free(env->vals);
  free(env);
}

cenv* cenv_copy(cenv* env) {
  cenv* e  = malloc(sizeof(cenv));
  e->par   = env->par;
  e->count = env->count;
  e->keys  = malloc(sizeof(char*) * env->count);
  e->vals  = malloc(sizeof(cval*) * env->count);
  for (int i = 0; i < env->count; i++) {
    e->keys[i] = malloc(strlen(env->keys[i]) + 1);
    strcpy(e->keys[i], env->keys[i]);
    e->vals[i] = cval_copy(env->vals[i]);
  }
  return e;
}

cval* cenv_get(cenv* env, cval* key) {
  for (int i = 0; i < env->count; i++) {
    if (strcmp(env->keys[i], key->kywd) == 0) {
      return cval_copy(env->vals[i]);
    }
  }

  if (env->par) { return cenv_get(env->par, key); }

  return cval_err("unbound keyword '%s'", key->kywd);
}

void cenv_def(cenv* env, cval* key, cval* val) {
  while (env->par) { env = env->par; }
  cenv_put(env, key, val);
}

void cenv_put(cenv* env, cval* key, cval* val) {
  for (int i = 0; i < env->count; i++) {
    if (strcmp(env->keys[i], key->kywd) == 0) {
      cval_del(env->vals[i]);
      env->vals[i] = cval_copy(val);
      return;
    }
  }


  env->count++;
  env->vals = realloc(env->vals, sizeof(cval*) * env->count);
  env->keys = realloc(env->keys, sizeof(char*) * env->count);

  env->vals[env->count - 1] = cval_copy(val);
  env->keys[env->count - 1] = malloc(strlen(key->kywd) + 1);
  strcpy(env->keys[env->count - 1], key->kywd);
}

cval* cval_read_num(mpc_ast_t* tree) {
  errno    = 0;
  long out = strtol(tree->contents, NULL, 10);
  return errno != ERANGE ? cval_num(out) : cval_err("invalid number");
}

cval* cval_read_str(mpc_ast_t* tree) {
  // Remove trailing "
  tree->contents[strlen(tree->contents) - 1] = '\0';
  char* unesc = malloc(strlen(tree->contents) + 1);
  // Copy without leading "
  strcpy(unesc, tree->contents + 1);
  unesc     = mpcf_unescape(unesc);
  cval* str = cval_str(unesc);

  free(unesc);
  return str;
}

cval* cval_add(cval* val, cval* child) {
  val->count++;
  val->cell                 = realloc(val->cell, sizeof(cval*) * val->count);
  val->cell[val->count - 1] = child;
  return val;
}

cval* cval_read(mpc_ast_t* tree) {
  if (strstr(tree->tag, "qquot")
      || (strstr(tree->tag, "quot") && !strstr(tree->tag, "nonquot"))) {
    cval* sexpr = cval_read(tree->children[1]);
    return cval_quot(sexpr);
  }

  if (strstr(tree->tag, "number")) { return cval_read_num(tree); }
  if (strstr(tree->tag, "string")) { return cval_read_str(tree); }
  if (strstr(tree->tag, "keyword")) { return cval_kywd(tree->contents); }

  cval* sexpr = NULL;
  if (strcmp(tree->tag, ">") == 0) { sexpr = cval_sexpr(); }
  if (strstr(tree->tag, "sexpr")) { sexpr = cval_sexpr(); }

  for (int i = 0; i < tree->children_num; i++) {
    if (strcmp(tree->children[i]->contents, "(") == 0) { continue; }
    if (strcmp(tree->children[i]->contents, "'(") == 0) { continue; }
    if (strcmp(tree->children[i]->contents, ")") == 0) { continue; }
    if (strcmp(tree->children[i]->tag, "regex") == 0) { continue; }
    if (strstr(tree->children[i]->tag, "comment")) { continue; }
    sexpr = cval_add(sexpr, cval_read(tree->children[i]));
  }

  return sexpr;
}

cval* cval_pop(cval* val, int idx) {
  cval* out = val->cell[idx];

  memmove(&val->cell[idx], &val->cell[idx + 1],
          sizeof(cval*) * (val->count - idx - 1));
  val->count--;
  val->cell = realloc(val->cell, sizeof(cval*) * val->count);

  return out;
}

cval* cval_take(cval* val, int idx) {
  cval* out = cval_pop(val, idx);
  cval_del(val);

  return out;
}

cval* cval_call(cenv* env, cval* fun, cval* args) {
  if (fun->builtin) { return fun->builtin(env, args); }

  if (args->count > fun->formals->count) {
    cval* err = cval_err("Expected %li args, got %li", fun->formals->count,
                         args->count);
    cval_del(args);
    return err;
  }

  for (int i = 0; i < fun->formals->count; i++) {
    if (i >= args->count) {
      cenv_put(fun->env, fun->formals->cell[i], cval_sexpr());
    } else {
      cenv_put(fun->env, fun->formals->cell[i], args->cell[i]);
    }
  }

  cval_del(args);

  fun->env->par = env;

  return cval_eval(fun->env, cval_copy(fun->body));
}

cval* cval_eval_sexpr(cenv* env, cval* val) {
  for (int i = 0; i < val->count; i++) {
    if (val->cell[0]->type == CVAL_MCR) { continue; }
    val->cell[i] = cval_eval(env, val->cell[i]);
  }

  for (int i = 0; i < val->count; i++) {
    if (val->cell[i]->type == CVAL_ERR) { return cval_take(val, i); }
  }

  if (val->count == 0) { return val; }

  cval* fun = cval_pop(val, 0);
  if (fun->type != CVAL_FUN && fun->type != CVAL_MCR) {
    cval_del(fun);
    cval_del(val);

    return cval_err("sexpr does not begin with a function");
  }

  cval* result = cval_call(env, fun, val);
  cval_del(fun);

  return result;
}

cval* cval_eval(cenv* env, cval* val) {
  if (val->type == CVAL_KYWD) {
    cval* out = cenv_get(env, val);
    cval_del(val);
    return out;
  }

  if (val->type == CVAL_QUOT) {
    cval* out = cval_copy(val->quot);
    cval_del(val);
    return out;
  }

  if (val->type == CVAL_SEXPR) { return cval_eval_sexpr(env, val); }

  return val;
}
