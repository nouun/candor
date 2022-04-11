#include "parser.h"

#include "cval.h"

#include <errno.h>
#include <mpc.h>
#include <stdlib.h>

static cval* cval_read_num(mpc_ast_t* tree) {
  errno    = 0;
  long out = strtol(tree->contents, NULL, 10);
  return errno != ERANGE ? cval_num(out) : cval_err("invalid number");
}

static cval* cval_read_str(mpc_ast_t* tree) {
  // Remove trailing "
  tree->contents[strlen(tree->contents) - 1] = '\0';
  // Copy without leading "
  char* unesc = strdup(tree->contents + 1);
  unesc     = mpcf_unescape(unesc);
  cval* str = cval_str(unesc);

  free(unesc);
  return str;
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
