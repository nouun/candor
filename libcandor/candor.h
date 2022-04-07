#ifndef CANDOR_H
#define CANDOR_H

#include "builtins.h"
#include "cval.h"

#include <mpc.h>

extern mpc_parser_t* Candor;

void candor_init(cenv* env);
void candor_deinit(void);
cval* candor_parse(const char* filename, char* str);

#endif /* CANDOR_H */
