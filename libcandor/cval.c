#include "candor.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static cval* cval_alloc(CvalType type) {
  cval* v = malloc(sizeof(cval));
  v->type = type;
  return v;
}

cval* cval_num(long num) {
  cval* v = cval_alloc(CVAL_NUM);
  v->num  = num;
  return v;
}

cval* cval_str(char* str) {
  cval* v = cval_alloc(CVAL_STR);
  v->str  = strdup(str);
  return v;
}

cval* cval_err(char* fmt, ...) {
  cval* v = cval_alloc(CVAL_ERR);

  va_list va;
  va_start(va, fmt);

  v->err = malloc(1024);
  vsnprintf(v->err, 1023, fmt, va);
  v->err = realloc(v->err, strlen(v->err) + 1);

  va_end(va);

  return v;
}

cval* cval_kywd(char* kywd) {
  cval* v = cval_alloc(CVAL_KYWD);
  v->kywd = strdup(kywd);
  return v;
}

cval* cval_sexpr(void) {
  cval* v            = cval_alloc(CVAL_SEXPR);
  v->sexpr           = malloc(sizeof(sexpr));
  v->sexpr->capacity = 4;
  v->sexpr->count    = 0;
  v->sexpr->cell     = malloc(sizeof(char*) * v->sexpr->capacity);
  return v;
}

cval* cval_qquot(cval* val) {
  cval* v = cval_alloc(CVAL_QQUOT);
  v->quot = val;
  return v;
}

cval* cval_quot(cval* val) {
  cval* v = cval_alloc(CVAL_QUOT);
  v->quot = val;
  return v;
}

cval* cval_bfun(const cbuiltin func) {
  cval* v    = cval_alloc(CVAL_BFUN);
  v->builtin = func;
  return v;
}

cval* cval_bmcr(const cbuiltin func) {
  cval* v    = cval_alloc(CVAL_BMCR);
  v->builtin = func;
  return v;
}

cval* cval_fun(cval* params, cval* body) {
  cval* v             = cval_alloc(CVAL_FUN);
  v->function         = malloc(sizeof(user_function));
  v->function->env    = cenv_new(NULL);
  v->function->params = params;
  v->function->body   = body;
  return v;
}

cval* cval_mcr(cval* params, cval* body) {
  cval* v = cval_fun(params, body);
  v->type = CVAL_MCR;
  return v;
}

void user_function_del(user_function* val) {
  cenv_del(val->env);
  cval_del(val->params);
  cval_del(val->body);
}

void cval_del(cval* val) {
  switch (val->type) {
  case CVAL_MCR:
  case CVAL_FUN: user_function_del(val->function); break;
  case CVAL_NUM: break;
  case CVAL_STR: free(val->str); break;
  case CVAL_KYWD: free(val->kywd); break;
  case CVAL_ERR: free(val->err); break;
  case CVAL_QQUOT:
  case CVAL_QUOT: cval_del(val->quot); break;
  case CVAL_SEXPR:
    for (int i = 0; i < val->sexpr->count; i++) {
      cval_del(val->sexpr->cell[i]);
    }
    free(val->sexpr);
  }

  free(val);
}

int sexpr_cmp(sexpr* lhs, sexpr* rhs) {
  if (lhs->count != rhs->count) {
    return 0;
  } else {
    for (int i = 0; i < lhs->count; i++) {
      if (!cval_cmp(lhs->cell[i], rhs->cell[i])) { return 0; }
    }
    return 1;
  }
}

int cval_cmp(cval* lhs, cval* rhs) {
  if (lhs->type != rhs->type) { return 0; }

  switch (lhs->type) {
  case CVAL_NUM: return lhs->num == rhs->num;
  case CVAL_ERR: return strcmp(lhs->err, rhs->err) == 0;
  case CVAL_STR: return strcmp(lhs->str, rhs->str) == 0;
  case CVAL_KYWD: return strcmp(lhs->kywd, rhs->kywd) == 0;
  case CVAL_SEXPR: return sexpr_cmp(lhs->sexpr, rhs->sexpr);
  case CVAL_QQUOT:
  case CVAL_QUOT: return cval_cmp(lhs->quot, rhs->quot);
  case CVAL_BMCR:
  case CVAL_BFUN: return lhs->builtin == rhs->builtin;
  case CVAL_MCR:
  case CVAL_FUN:
    return cval_cmp(lhs->function->body, rhs->function->body)
           && cval_cmp(lhs->function->params, rhs->function->params);
  }

  // How tf
  return 0;
}

void cval_print_expr(cval* val, char open, char close) {
  putchar(open);
  for (int i = 0; i < val->sexpr->count; i++) {
    cval_print(val->sexpr->cell[i]);

    if (i != (val->sexpr->count - 1)) { putchar(' '); }
  }

  putchar(close);
}

void cval_str_print(cval* val) {
  char* esc = malloc(strlen(val->str) + 1);
  strcpy(esc, val->str);
  esc = mpcf_escape(esc);
  printf("\"%s\"", esc);
  free(esc);
}

void cval_print(cval* val) {
  switch (val->type) {
  case CVAL_NUM: printf("%li", val->num); break;
  case CVAL_STR: cval_str_print(val); break;
  case CVAL_ERR: printf("error: %s", val->err); break;
  case CVAL_MCR: printf("<macro>"); break;
  case CVAL_BMCR: printf("<builtin-macro>"); break;
  case CVAL_FUN: printf("<function>"); break;
  case CVAL_BFUN: printf("<builtin>"); break;
  case CVAL_KYWD: printf("%s", val->kywd); break;
  case CVAL_SEXPR: cval_print_expr(val, '(', ')'); break;
  case CVAL_QQUOT:
  case CVAL_QUOT:
    putchar('(');
    cval_print(val->quot);
    putchar(')');
  }
}

void cval_println(cval* val) {
  cval_print(val);
  putchar('\n');
}

cenv* cenv_copy(const cenv* env);

cval* cval_copy(const cval* val) {
  cval* out = malloc(sizeof(cval));
  out->type = val->type;

  switch (out->type) {
  case CVAL_NUM: out->num = val->num; break;
  case CVAL_STR: out->str = strdup(val->str); break;
  case CVAL_ERR: out->err = strdup(val->err); break;
  case CVAL_KYWD: out->kywd = strdup(val->kywd); break;

  case CVAL_QQUOT:
  case CVAL_QUOT: out->quot = cval_copy(val->quot); break;

  case CVAL_BMCR:
  case CVAL_BFUN: out->builtin = val->builtin; break;

  case CVAL_MCR:
  case CVAL_FUN:
    out->function         = malloc(sizeof(user_function));
    out->function->env    = cenv_copy(val->function->env);
    out->function->params = cval_copy(val->function->params);
    out->function->body   = cval_copy(val->function->body);
    break;

  case CVAL_SEXPR:
    out->sexpr        = malloc(sizeof(sexpr));
    out->sexpr->count = val->sexpr->count;
    out->sexpr->cell  = malloc(sizeof(cval*) * out->sexpr->count);
    for (int i = 0; i < out->sexpr->count; i++) {
      out->sexpr->cell[i] = cval_copy(val->sexpr->cell[i]);
    }
    break;
  }

  return out;
}

cenv* cenv_new(cenv* parent) {
  cenv* env     = malloc(sizeof(cenv));
  env->par      = parent;
  env->capacity = CENV_SIZE_BASE;
  env->count    = 0;
  env->keys     = malloc(sizeof(cval*) * env->count);
  env->vals     = malloc(sizeof(cval*) * env->count);
  return env;
}

void cenv_del(cenv* env) {
  for (int i = 0; i < env->count; i++) {
    free(env->keys[i]);
    cval_del(env->vals[i]);
  }

  free(env->keys);
  free(env->vals);
  free((cenv*)env);
}

cenv* cenv_copy(const cenv* env) {
  cenv* e     = malloc(sizeof(cenv));
  e->par      = env->par;
  e->count    = env->count;
  e->capacity = env->capacity;
  e->keys     = malloc(sizeof(char*) * env->capacity);
  e->vals     = malloc(sizeof(cval*) * env->capacity);
  for (int i = 0; i < env->count; i++) {
    e->keys[i] = malloc(strlen(env->keys[i]) + 1);
    strcpy(e->keys[i], env->keys[i]);
    e->vals[i] = cval_copy(env->vals[i]);
  }
  return e;
}

cval* cenv_get(const cenv* env, cval* key) {
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
      env->vals[i] = val;
      return;
    }
  }

  env->count++;
  while (env->count >= env->capacity) { env->capacity += CENV_SIZE_INCR; }

  env->vals = realloc(env->vals, sizeof(cval*) * env->capacity);
  env->keys = realloc(env->keys, sizeof(char*) * env->capacity);

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
  val->sexpr->count++;

  while (val->sexpr->count >= val->sexpr->capacity) {
    val->sexpr->capacity *= 2;
  }

  val->sexpr->cell
    = realloc(val->sexpr->cell, sizeof(cval*) * val->sexpr->capacity);
  val->sexpr->cell[val->sexpr->count - 1] = child;

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
  cval* out = val->sexpr->cell[idx];

  memmove(&val->sexpr->cell[idx], &val->sexpr->cell[idx + 1],
          sizeof(cval*) * (val->sexpr->count - idx - 1));
  val->sexpr->count--;

  return out;
}

cval* cval_take(cval* val, int idx) {
  cval* out = cval_pop(val, idx);
  cval_del(val);

  return out;
}

cval* cval_call(cenv* env, cval* fun, cval* args) {
  if (fun->type == CVAL_BFUN || fun->type == CVAL_BMCR) {
    return fun->builtin(env, args);
  }

  if (args->sexpr->count > fun->function->params->sexpr->count) {
    cval* err
      = cval_err("Expected %li args, got %li",
                 fun->function->params->sexpr->count, args->sexpr->count);
    cval_del(args);
    return err;
  }

  for (int i = 0; i < fun->function->params->sexpr->count; i++) {
    if (i >= args->sexpr->count) {
      cenv_put(fun->function->env, fun->function->params->sexpr->cell[i],
               cval_sexpr());
    } else {
      cenv_put(fun->function->env, fun->function->params->sexpr->cell[i],
               args->sexpr->cell[i]);
    }
  }

  cval_del(args);

  fun->function->env->par = env;
  cval* res = cval_eval(fun->function->env, cval_copy(fun->function->body));
  return res;
}

cval* cval_eval_sexpr(cenv* env, cval* val) {
  for (int i = 0; i < val->sexpr->count; i++) {
    if (val->sexpr->cell[0]->type == CVAL_MCR
        || val->sexpr->cell[0]->type == CVAL_BMCR) {
      continue;
    }
    val->sexpr->cell[i] = cval_eval(env, val->sexpr->cell[i]);
  }

  for (int i = 0; i < val->sexpr->count; i++) {
    if (val->sexpr->cell[i]->type == CVAL_ERR) { return cval_take(val, i); }
  }

  if (val->sexpr->count == 0) { return val; }

  cval* fun = cval_pop(val, 0);
  if (fun->type != CVAL_FUN && fun->type != CVAL_BFUN && fun->type != CVAL_MCR
      && fun->type != CVAL_BMCR) {
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

cval* cval_load_file(cenv* env, const char* filename) {
  mpc_result_t res;
  if (mpc_parse_contents(filename, candor_parser, &res)) {
    cval* file = cval_read(res.output);
    mpc_ast_delete(res.output);

    while (file->sexpr->count) {
      cval* cell = cval_pop(file, 0);
      cval* expr = cval_eval(env, cell);
      if (expr->type == CVAL_ERR) { cval_println(expr); }
      cval_del(expr);
    }

    cval_del(file);

    return cval_sexpr();
  }

  char* msg = mpc_err_string(res.error);
  cval* err = cval_err("unable to load file:\n%s", msg);

  mpc_err_delete(res.error);
  free(msg);

  return err;
}
