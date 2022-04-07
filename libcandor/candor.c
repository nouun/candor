#include "candor.h"

#include <mpc.h>

static mpc_parser_t* Number;
static mpc_parser_t* String;
static mpc_parser_t* Keyword;
static mpc_parser_t* Quot;
static mpc_parser_t* Qquot;
static mpc_parser_t* Nonquot;
static mpc_parser_t* Sexpr;
static mpc_parser_t* Expr;
static mpc_parser_t* Comment;
mpc_parser_t* Candor;

void candor_init(cenv* env) {
  Number  = mpc_new("number");
  String  = mpc_new("string");
  Keyword = mpc_new("keyword");
  Quot    = mpc_new("quot");
  Qquot   = mpc_new("qquot");
  Nonquot = mpc_new("nonquot");
  Sexpr   = mpc_new("sexpr");
  Expr    = mpc_new("expr");
  Comment = mpc_new("comment");
  Candor  = mpc_new("candor");


  mpca_lang(MPCA_LANG_DEFAULT,
            "number  \"number\"      : /-?[0-9]+/                        ; "
            "string  \"string\"      : /\"(\\\\.|[^\"])*\"/              ; "
            "keyword \"keyword\"     : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!?&]+/ ; "
            "quot    \"quote\"       : \"'\" <nonquot>                   ; "
            "qquot   \"quasiquote\"  : '`' <nonquot>                     ; "
            "nonquot                 : <number> | <keyword> | <sexpr>      "
            "                          | <string> | <comment>            ; "
            "sexpr   \"sexpression\" : '(' <expr>* ')'                   ; "
            "expr                    : <quot> | <qquot> | <nonquot>      ; "
            "comment \"comment\"     : /;[^\\r\\n]*/                     ; "
            "candor                  : /^/ <expr>* /$/                   ; ",
            Number, String, Keyword, Quot, Qquot, Nonquot, Sexpr, Expr, Comment, Candor);


  cenv_add_builtins(env);
}

void candor_deinit(void) {  
  mpc_cleanup(10, Number, String, Keyword, Nonquot, Qquot, Quot, Sexpr, Expr,
              Comment, Candor);
}

cval* candor_parse(const char* filename, char* str) {
  mpc_result_t res;
  if(mpc_parse(filename, str, Candor, &res)){
    cval* val = cval_read(res.output);

    mpc_ast_delete(res.output);

    return val;
  } else {
    char* msg = mpc_err_string(res.error);
    mpc_err_delete(res.error);

    cval* err = cval_err("unable to parse string:\n%s", msg);

    free(msg);

    return err;
  }
}

