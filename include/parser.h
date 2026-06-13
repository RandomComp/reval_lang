#ifndef REVAL_PARSER_H
#define REVAL_PARSER_H

#include "errors.h"
#include "types.h"
#include "utils.h"

#include "lexer_fwd.h"
#include "parser_fwd.h"

// TODO: Добавить поддержку double и строк унифировав результат выражений eval_expresion и val

struct expression_t {
	tokens_kind_e unary_op;

	expression_t *left, *right, *center;

	int val; const char* word;

	tokens_kind_e op;

	// meta information
	const char *filename;
	ssize_t column, row;
	ssize_t end_column, end_row;
};

// typedef enum operator_assoc_e {
// 	OP_ASSOC_LEFT,
// 	OP_ASSOC_RIGHT,
// } operator_assoc_e;

// typedef enum operator_type_e {
// 	OP_TYPE_UNARY,
// 	OP_TYPE_BINARY,
// 	OP_TYPE_TERNARY,
// } operator_type_e;

// typedef parser_err_e (*operator_parser_t)(expression_t *left, expression_t *center, expression_t *right);

// typedef struct operator_t {
// 	tokens_kind_e kind;
// 	operator_type_e type;
// 	operator_assoc_e assoc;
// 	int precedence;

// 	operator_parser_t parser;
// } operator_t;

void add_filename(const char *filename);

expression_t* new_expression(interneds_t *filenames, const char *filename, expression_t *left, tokens_kind_e op, expression_t *right);

errors_t parse_semicolon(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr);

errors_t parse_assignment(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr);

errors_t parse_ternary(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr);

errors_t parse_logical(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr);

errors_t parse_compare(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr);

errors_t parse_add(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr);

errors_t parse_mult(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr);

errors_t parse_pow(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr);

errors_t parse_unar(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr);

ssize_t view_expresion(char* buf, ssize_t buf_size, expression_t* expr);

void free_expresion(expression_t* expr);

#endif
