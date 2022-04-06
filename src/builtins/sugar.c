#include "../builtins.h"

#include <stdio.h>

void cenv_add_builtin_var(cenv* env, char* name, cval* val) {
  cval* key = cval_kywd(name);
  cenv_put(env, key, val);
  cval_del(key);
  cval_del(val);
}

void cenv_add_builtins_sugar(cenv* env) {
  cenv_add_builtin_var(env, "nil", cval_sexpr());
  cenv_add_builtin_var(env, "true", cval_num(1));
  cenv_add_builtin_var(env, "t", cval_num(1));
  cenv_add_builtin_var(env, "false", cval_num(0));
  cenv_add_builtin_var(env, "f", cval_num(0));

/*   cenv_add_builtin_str(env, "<std-lib-sugar>", "            \ */
/* (defun (err? val)     (eq? \"<error>\"       (typeof val))) \ */
/* (defun (fun? val)     (eq? \"<function>\"    (typeof val))) \ */
/* (defun (mcr? val)     (eq? \"<macro>\"       (typeof val))) \ */
/* (defun (num? val)     (eq? \"<number>\"      (typeof val))) \ */
/* (defun (str? val)     (eq? \"<string>\"      (typeof val))) \ */
/* (defun (sexpr? val)   (eq? \"<sexpression>\" (typeof val))) \ */
/* (defun (keyword? val) (eq? \"<keyword>\"     (typeof val))) \ */
/* "); */
}
