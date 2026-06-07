#ifndef REVAL_PARSER_H
#define REVAL_PARSER_H

#include "types.h"

#include "lexer_fwd.h"
#include "parser_fwd.h"

struct expression_t {
	expression_t *left, *right;

	expression_op_e op;
};

typedef struct expressions_t {
	expression_t* exprs; size_t exprs_cnt;
} expressions_t;

int parse(expressions_t *_result, tokens_t tokens);

#endif
