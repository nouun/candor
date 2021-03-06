#ifndef CANDOR_H
#define CANDOR_H

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

struct cval {
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
};

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


void  candor_init(void);
void  candor_deinit(void);
cval* candor_load(const char* filename, const char* str);
cval* candor_load_file(const char* filename);

/// Print val
void cval_print(cval* val);
/// Print val followed by a newline
void cval_println(cval* val);

/// Free cval and all assoc values
void cval_del(cval* val);

/* Adding Builtins */

/// Add a new builtin procedure to the global scope
void candor_add_builtin(char* name, cbuiltin proc);

#endif /* CANDOR_H */
