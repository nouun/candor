#include "candor.h"

#include "builtins.h"
#include "cenv.h"
#include "cval.h"
#include "parser.h"

#include <mpc.h>

static cenv* candor_env;

static mpc_parser_t* cndr_number;
static mpc_parser_t* cndr_string;
static mpc_parser_t* cndr_keyword;
static mpc_parser_t* cndr_quot;
static mpc_parser_t* cndr_qquot;
static mpc_parser_t* cndr_nonquot;
static mpc_parser_t* cndr_sexpr;
static mpc_parser_t* cndr_expr;
static mpc_parser_t* cndr_comment;
mpc_parser_t*        candor_parser;

void candor_init(void) {
  cndr_number   = mpc_new("number");
  cndr_string   = mpc_new("string");
  cndr_keyword  = mpc_new("keyword");
  cndr_quot     = mpc_new("quot");
  cndr_qquot    = mpc_new("qquot");
  cndr_nonquot  = mpc_new("nonquot");
  cndr_sexpr    = mpc_new("sexpr");
  cndr_expr     = mpc_new("expr");
  cndr_comment  = mpc_new("comment");
  candor_parser = mpc_new("candor");


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
            cndr_number, cndr_string, cndr_keyword, cndr_quot, cndr_qquot,
            cndr_nonquot, cndr_sexpr, cndr_expr, cndr_comment, candor_parser);

  candor_env = cenv_new();
  cenv_add_builtins(candor_env);
}

void candor_deinit(void) {
  cenv_del(candor_env);

  mpc_cleanup(10, cndr_number, cndr_string, cndr_keyword, cndr_nonquot,
              cndr_qquot, cndr_quot, cndr_sexpr, cndr_expr, cndr_comment,
              candor_parser);
}

cval* candor_load(const char* filename, const char* str) {
  mpc_result_t res;
  if (mpc_parse(filename, str, candor_parser, &res)) {
    cval* val = cval_read(res.output);

    cval* out = cval_sexpr();
    while (val->sexpr->count) {
      cval_del(out);
      out = cval_eval(candor_env, cval_pop(val, 0));

      if (out->type == CVAL_ERR) {
        mpc_ast_delete(res.output);
        cval_del(val);
        return out;
      }
    }

    cval_del(val);
    mpc_ast_delete(res.output);
    return out;
  }

  char* msg = mpc_err_string(res.error);
  cval* err = cval_err("unable to parse %s:\n%s", filename, msg);

  free(msg);
  mpc_err_delete(res.error);

  return err;
}

cval* candor_load_file(const char* filename) {
  return cval_load_file(candor_env, filename);
}
