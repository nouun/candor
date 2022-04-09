#ifndef CANDOR_CVAL_H
#define CANDOR_CVAL_H

#include <mpc.h>

#include <stdbool.h>

// TODO: Handle malloc returning NULL
// NOTE: Use prealloced error or something?
#define CVAL_ALLOC(ptr, cval_type)                                             \
  ptr       = malloc(sizeof(cval));                                            \
  ptr->type = cval_type;

#define CVAL_ALLOC_SET(ptr, cval_type, prop, val)                              \
  CVAL_ALLOC(ptr, cval_type);                                                  \
  ptr->prop = val;

#define CENV_SIZE_BASE 16
#define CENV_SIZE_INCR 16

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
    char*          str;
    char*          kywd;
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

/// Create a new cenv
cenv* cenv_new(void);
/// Free all keys and values in a cenv
void  cenv_del(cenv* env);

/// Puts a key and value in the top level cenv, copies val
void  cenv_def(cenv* env, cval* key, cval* val);
/// Keys a key a value in the cenv, copies val
void  cenv_put(cenv* env, cval* key, cval* val);
/// Gets a value from cenv by key
cval* cenv_get(const cenv* env, cval* key);

/// Copy cval to new cval
cval* cval_copy(const cval* val);
/// Free cval and all assoc values
void  cval_del(cval* val);
/// Compare two cvals
bool  cval_cmp(cval* lhs, cval* rhs);

/// Load a file into current cenv
cval* cval_load_file(cenv* env, const char* filename);

cval* cval_err(char* fmt, ...);
/// Create a new cval keyword, copies kywd
cval* cval_kywd(char* kywd);
cval* cval_num(long num);
/// Create a new cval string, copies str
cval* cval_str(char* str);
/// Create a new cval quote, does not copy val
cval* cval_quot(cval* val);
/// Create a new cval quasiquote, does not copy val
cval* cval_qquot(cval* val);
cval* cval_sexpr(void);
cval* cval_bfun(cbuiltin fun);
cval* cval_bmcr(cbuiltin fun);
/// Create a new cval function, does not copy params or body
cval* cval_fun(cval* params, cval* body);
/// Create a new cval macro, does not copy params or body
cval* cval_mcr(cval* params, cval* body);

/// Parse an mpc_ast to a cval
cval* cval_read(mpc_ast_t* tree);
/// Evaluate and free val
cval* cval_eval(cenv* env, cval* val);

/// Add child to val's sexpr cells
cval* cval_add(cval* val, cval* child);
/// Take the value at idx and free all other values
cval* cval_take(cval* val, int idx);
/// Take the value at idx and remove from val's cells
cval* cval_pop(cval* val, int idx);

/// Print val
void cval_print(cval* val);
/// Print val followed by a newline
void cval_println(cval* val);

#endif /* CANDOR_CVAL_H */
