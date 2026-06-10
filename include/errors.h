#ifndef REVAL_ERRORS_H
#define REVAL_ERRORS_H

#include "types.h"

typedef enum subsystem_e {
	SUBSYSTEM_LEXER,
	SUBSYSTEM_PARSER,
	SUBSYSTEM_EVAL,
} subsystem_e;

typedef enum error_e {
	// warnings:
	
	// lexer
	ERROR_LEXER_LEADING_ZEROES_IN_NUMBER = -2,
	
	// parser
	ERROR_PARSER_POSSIBLE_UB,
	
	ERROR_OK = 0,
	
	// errors:
	
	// lexer
	ERROR_LEXER_INVALID_SYNTAX,
	ERROR_LEXER_INVALID_NUMBER,
	ERROR_LEXER_INVALID_NUMBER_RADIX,
	// ERROR_LEXER_MAXIMUM_TOKENS_EXCEEDED,

	// parser
	ERROR_PARSER_UNCLOSED_PARENT,
	ERROR_PARSER_INVALID_SYNTAX,
	ERROR_PARSER_EOF,

	// eval
	ERROR_EVAL_DIVISION_BY_ZERO,
	ERROR_EVAL_UNKNOWN_OPERATOR,
	ERROR_EVAL_EXPR_ZERO_PTR,
} error_e;

typedef enum error_level_e {
	ERROR_LEVEL_OK,
	ERROR_LEVEL_NOTE,
	ERROR_LEVEL_WARN,
	ERROR_LEVEL_ERROR,
	ERROR_LEVEL_FATAL,
} error_level_e;

typedef struct error_t {
	subsystem_e subsystem;
	error_e type;
	char* msg;

	error_level_e level;

	ssize_t st_column, st_row;
	ssize_t columns, rows;
} error_t;

#define ERRORS_ALLOC_STEP 4
#define MAX_ERRORS 32

typedef struct errors_t {
	error_t* errors; size_t errors_cnt, errors_size;

	error_level_e level;
} errors_t;

errors_t emit_error(errors_t errors, subsystem_e subsystem, error_e type, error_level_e level, ssize_t st_column, ssize_t st_row, ssize_t columns, ssize_t rows, const char* msg, ...);

void free_errors(errors_t errs);

void print_errors(errors_t errs, const char* expr);

const char* get_level_name(error_level_e level);
const char* get_subsystem_name(subsystem_e subsystem);

const char* get_err_description(error_e type);

#endif
