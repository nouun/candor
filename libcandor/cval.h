#ifndef CANDOR_CVAL_H
#define CANDOR_CVAL_H

#include <mpc.h>

struct cval;
struct cenv;
typedef struct cval cval;
typedef struct cenv cenv;

struct cenv {
  cenv*  par;
  int    count;
  char** keys;
  cval** vals;
};

typedef cval* (*cbuiltin)(cenv*, cval*);

typedef struct cval {
  int   type;
  long  num;
  char* str;
  char* err;
  char* kywd;

  // Builtin functions
  cbuiltin builtin;

  // User defined functions
  cenv* env;
  cval* formals;
  cval* body;

  struct cval* quot;

  int           count;
  struct cval** cell;
} cval;

enum {
  CVAL_NUM,
  CVAL_STR,
  CVAL_ERR,
  CVAL_KYWD,
  CVAL_SEXPR,
  CVAL_QUOT,
  CVAL_QQUOT,
  CVAL_FUN,
  CVAL_MCR,
};
static char* cval_type_str[]
  = { "number", "string",    "error",    "keyword", "sexpr",
      "quot",   "quasiquot", "function", "macro" };

cenv* cenv_new(void);
void  cenv_del(cenv* env);

void  cenv_def(cenv* env, cval* key, cval* fun);
void  cenv_put(cenv* env, cval* key, cval* fun);
cval* cenv_get(cenv* env, cval* key);

cval* cval_copy(cval* val);
void  cval_del(cval* val);
int   cval_cmp(cval* lhs, cval* rhs);

cval* cval_err(char* fmt, ...);
cval* cval_kywd(char* kywd);
cval* cval_num(long num);
cval* cval_str(char* str);
cval* cval_sexpr(void);
cval* cval_lambda(cval* formals, cval* body);
cval* cval_fun(cbuiltin fun);
cval* cval_mcr(cbuiltin fun);

cval* cval_read(mpc_ast_t* tree);
cval* cval_eval(cenv* env, cval* val);

cval* cval_add(cval* val, cval* child);
cval* cval_take(cval* val, int idx);
cval* cval_pop(cval* val, int idx);

void cval_print(cval* val);
void cval_println(cval* val);

#endif /* CANDOR_CVAL_H */
