#ifndef CANDOR_CENV_H
#define CANDOR_CENV_H

#include "candor.h"
#include "cval.h"

#define CENV_SIZE_BASE 16
#define CENV_SIZE_INCR 16

/// Create a new cenv
cenv* cenv_new(void);
cenv* cenv_copy(cenv* env);
/// Free all keys and values in a cenv
void  cenv_del(cenv* env);

/// Puts a key and value in the top level cenv, copies val
void  cenv_def(cenv* env, char* key, cval* val);
/// Keys a key a value in the cenv, copies val
void  cenv_put(cenv* env, char* key, cval* val);
/// Gets a value from cenv by key
cval* cenv_get(const cenv* env, const char* key);


#endif /* CANDOR_CENV_H */
