#ifndef REVAL_EVAL_H
#define REVAL_EVAL_H

#include "errors.h"
#include "parser_fwd.h"

typedef struct var_t {
	char *name; int val; bool used;
} var_t;

typedef struct vars_t {
	var_t *vars; size_t vars_cnt, vars_size;
} vars_t;

#define VAR_ALLOC_STEP 4

vars_t set_var(vars_t vars, const char *name, int val);
vars_t del_var(vars_t vars, const char *name);

errors_t eval_expresion(errors_t errs, vars_t *vars, int* _result, expression_t* expr);

#endif
