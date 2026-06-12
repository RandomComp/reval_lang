#ifndef REVAL_EVAL_H
#define REVAL_EVAL_H

#include "errors.h"
#include "parser_fwd.h"

errors_t eval_expresion(errors_t errs, int* _result, expression_t* expr);

#endif
