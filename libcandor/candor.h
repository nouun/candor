#ifndef CANDOR_H
#define CANDOR_H

#include "builtins.h"
#include "cval.h"

#include <mpc.h>

extern mpc_parser_t* candor_parser;

void  candor_init(void);
void  candor_deinit(void);
cval* candor_load(const char* filename, const char* str);
cval* candor_load_file(const char* filename);

#endif /* CANDOR_H */
