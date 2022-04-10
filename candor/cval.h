#ifndef CANDOR_CVAL_H
#define CANDOR_CVAL_H

#include "candor.h"

#include <stdbool.h>

// TODO: Handle malloc returning NULL
// NOTE: Use prealloced error or something?
#define CVAL_ALLOC(ptr, cval_type)                                             \
  ptr       = malloc(sizeof(cval));                                            \
  ptr->type = cval_type;

#define CVAL_ALLOC_SET(ptr, cval_type, prop, val)                              \
  CVAL_ALLOC(ptr, cval_type);                                                  \
  ptr->prop = val;

#define CVAL_SIZE_BASE 4

static char* cval_type_str[]
  = { [CVAL_NUM] = "number",        [CVAL_STR] = "string",
      [CVAL_ERR] = "error",         [CVAL_KYWD] = "keyword",
      [CVAL_SEXPR] = "sexpr",       [CVAL_QUOT] = "quot",
      [CVAL_QQUOT] = "quasiquot",   [CVAL_FUN] = "function",
      [CVAL_BFUN] = "builtin",      [CVAL_MCR] = "macro",
      [CVAL_BMCR] = "builtin-macro" };

/// Copy cval to new cval
cval* cval_copy(const cval* val);
/// Free cval and all assoc values
void cval_del(cval* val);
/// Compare two cvals
bool cval_cmp(cval* lhs, cval* rhs);

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

/// Evaluate and free val
cval* cval_eval(cenv* env, cval* val);

/// Add child to val's sexpr cells
cval* cval_add(cval* val, cval* child);
/// Take the value at idx and free all other values
cval* cval_take(cval* val, int idx);
/// Take the value at idx and remove from val's cells
cval* cval_pop(cval* val, int idx);


#endif /* CANDOR_CVAL_H */
