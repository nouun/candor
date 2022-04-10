#ifndef CANDOR_PARSER_H
#define CANDOR_PARSER_H

#include "candor.h"
#include "cval.h"

#include <mpc.h>

extern mpc_parser_t* candor_parser;

/// Parse an mpc_ast to a cval
cval* cval_read(mpc_ast_t* tree);

#endif /* CANDOR_PARSER_H */
