#include "cenv.h"

#include <stdlib.h>
#include <string.h>

cenv* cenv_new(void) {
  cenv* env     = malloc(sizeof(cenv));
  env->par      = NULL;
  env->capacity = CENV_SIZE_BASE;
  env->count    = 0;
  env->keys     = malloc(sizeof(char*) * env->capacity);
  env->vals     = malloc(sizeof(cval*) * env->capacity);
  return env;
}

void cenv_del(cenv* env) {
  for (int i = 0; i < env->count; i++) {
    free(env->keys[i]);
    cval_del(env->vals[i]);
  }

  free(env->keys);
  free(env->vals);
  free((cenv*)env);
}

cenv* cenv_copy(cenv* env) {
  cenv* e     = malloc(sizeof(cenv));
  e->par      = env->par;
  e->count    = env->count;
  e->capacity = env->capacity;
  e->keys     = malloc(sizeof(char*) * env->capacity);
  e->vals     = malloc(sizeof(cval*) * env->capacity);
  for (int i = 0; i < env->count; i++) {
    e->keys[i] = malloc(strlen(env->keys[i]) + 1);
    strcpy(e->keys[i], env->keys[i]);
    e->vals[i] = cval_copy(env->vals[i]);
  }
  return e;
}

cval* cenv_get(const cenv* env, cval* key) {
  for (int i = 0; i < env->count; i++) {
    if (strcmp(env->keys[i], key->kywd) == 0) {
      return cval_copy(env->vals[i]);
    }
  }

  if (env->par) { return cenv_get(env->par, key); }

  return cval_err("unbound keyword '%s'", key->kywd);
}

void cenv_def(cenv* env, cval* key, cval* val) {
  while (env->par) { env = env->par; }
  cenv_put(env, key, val);
}

void cenv_put(cenv* env, cval* key, cval* val) {
  for (int i = 0; i < env->count; i++) {
    if (strcmp(env->keys[i], key->kywd) == 0) {
      cval_del(env->vals[i]);
      env->vals[i] = cval_copy(val);
      return;
    }
  }

  env->count++;
  while (env->count >= env->capacity) {
    env->capacity += CENV_SIZE_INCR;
    
    env->vals = realloc(env->vals, sizeof(cval*) * env->capacity);
    env->keys = realloc(env->keys, sizeof(char*) * env->capacity);
  }

  env->vals[env->count - 1] = cval_copy(val);
  env->keys[env->count - 1] = strdup(key->kywd);
}
