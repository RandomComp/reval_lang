#ifndef REVAL_PARSER_H
#define REVAL_PARSER_H

#include "types.h"

#include "lexer_fwd.h"
#include "parser_fwd.h"

// TODO: Добавить поддержку double унифировав результат выражений eval_expresion и val

struct expression_t {
	expression_t *left, *right;

	int val;

	tokens_kind_e op;
};

typedef enum parser_err_e {
	PARSER_ERR_POSSIBLE_UB 		= -1,
	PARSER_ERR_OK 				= 0,
	PARSER_ERR_UNCLOSED_PARENT 	= 1,
	PARSER_ERR_INVALID_SYNTAX 	= 2,
} parser_err_e;

typedef enum eval_err_e {
	EVAL_ERR_OK 				= 0,
	EVAL_ERR_DIVISION_BY_ZERO 	= 1,
	EVAL_ERR_UNKNOWN_OPERATOR 	= 2
} eval_err_e;

parser_err_e parse_compare(expression_t **_result, const token_t* tokens, const token_t** endptr);

parser_err_e parse_left(expression_t **_result, const token_t* tokens, const token_t** endptr);

parser_err_e parse_mult(expression_t **_result, const token_t* tokens, const token_t** endptr);

parser_err_e parse_unar(expression_t **_result, const token_t* tokens, const token_t** endptr);

void show_expresion(expression_t* expr);

eval_err_e eval_expresion(int* result, expression_t* expr);

void free_expresion(expression_t* expr);

const char* get_parser_err_description(parser_err_e err);

const char* get_eval_err_description(eval_err_e err);

#endif
