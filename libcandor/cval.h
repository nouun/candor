#ifndef CANDOR_CVAL_H
#define CANDOR_CVAL_H

#include <mpc.h>

#define CENV_SIZE_BASE 16
#define CENV_SIZE_INCR  16

#define CVAL_SIZE_BASE 4

struct cval;
struct cenv;
typedef struct cval cval;
typedef struct cenv cenv;

struct cenv {
  cenv*  par;
  int    count;
  int    capacity;
  char** keys;
  cval** vals;
};

typedef cval* (*cbuiltin)(cenv*, cval*);

typedef struct {
  cenv* env;
  cval* params;
  cval* body;
} user_function;

typedef struct {
  int           count;
  int           capacity;
  struct cval** cell;
} sexpr;

typedef struct cval {
  int type;

  union {
    long           num;
    char*    str;
    char*    kywd;
    char*          err;
    cbuiltin       builtin;
    user_function* function;
    struct cval*   quot;
    sexpr*         sexpr;
  };

} cval;

typedef enum {
  CVAL_NUM,
  CVAL_STR,
  CVAL_ERR,
  CVAL_KYWD,
  CVAL_SEXPR,
  CVAL_QUOT,
  CVAL_QQUOT,
  CVAL_FUN,
  CVAL_BFUN,
  CVAL_MCR,
  CVAL_BMCR,
} CvalType;

static char* cval_type_str[]
  = { [CVAL_NUM] = "number",        [CVAL_STR] = "string",
      [CVAL_ERR] = "error",         [CVAL_KYWD] = "keyword",
      [CVAL_SEXPR] = "sexpr",       [CVAL_QUOT] = "quot",
      [CVAL_QQUOT] = "quasiquot",   [CVAL_FUN] = "function",
      [CVAL_BFUN] = "builtin",      [CVAL_MCR] = "macro",
      [CVAL_BMCR] = "builtin-macro" };

cenv* cenv_new(cenv* parent);
void  cenv_del(cenv* env);

void  cenv_def(cenv* env, cval* key, cval* val);
void  cenv_put(cenv* env, cval* key, cval* val);
cval* cenv_get(const cenv* env, cval* key);

cval* cval_copy(const cval* val);
void  cval_del(cval* val);
int   cval_cmp(cval* lhs, cval* rhs);

cval* cval_load_file(cenv* env, const char* filename);

cval* cval_err(char* fmt, ...);
cval* cval_kywd(char* kywd);
cval* cval_num(long num);
cval* cval_str(char* str);
cval* cval_sexpr(void);
cval* cval_bfun(cbuiltin fun);
cval* cval_bmcr(cbuiltin fun);
cval* cval_fun(cval* params, cval* body);
cval* cval_mcr(cval* params, cval* body);

cval* cval_read(mpc_ast_t* tree);
cval* cval_eval(cenv* env, cval* val);

cval* cval_add(cval* val, cval* child);
cval* cval_take(cval* val, int idx);
cval* cval_pop(cval* val, int idx);

void cval_print(cval* val);
void cval_println(cval* val);

#endif /* CANDOR_CVAL_H */
