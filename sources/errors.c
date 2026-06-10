#include "types.h"
#include "utils.h"
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>

#include "errors.h"

errors_t emit_error(errors_t errors, subsystem_e subsystem, error_e type, error_level_e level, ssize_t st_column, ssize_t st_row, ssize_t columns, ssize_t rows, const char* msg, ...) {
	va_list ptr;

	va_start(ptr, msg);

	if (!msg || strlen(msg) <= 0) {
		msg = get_err_description(type);
	}

	va_list ptr_size;

	va_copy(ptr_size, ptr);

	size_t len = vsnprintf(nullptr, 0, msg, ptr_size) + 1;

	va_end(ptr_size);

	if (!errors.errors) {
		errors = (errors_t){ 0 };
	}

	if (errors.errors_cnt >= errors.errors_size) {
		size_t new_size = align_up(errors.errors_cnt + 1, ERRORS_ALLOC_STEP);

		errors.errors = realloc(errors.errors, new_size * sizeof(error_t));

		memset(errors.errors + errors.errors_size, 0, (new_size - errors.errors_size) * sizeof(error_t));

		errors.errors_size = new_size;
	}

	error_t *cur = errors.errors + errors.errors_cnt;

	cur->msg = malloc(len);

	vsnprintf(cur->msg, len, msg, ptr);

	cur->subsystem = subsystem;
	cur->type = type;
	cur->level = level;
	cur->st_column = st_column;
	cur->st_row = st_row;
	cur->columns = columns;
	cur->rows = rows;

	va_end(ptr);

	errors.errors_cnt++;

	if (errors.level < cur->level) {
		errors.level = cur->level;
	}

	// printf("%zu/%zu\n\r", st_column, columns);

	return errors;
}

void free_errors(errors_t errs) {
	for (size_t i = 0; i < errs.errors_cnt; i++) {
		error_t *cur = errs.errors + i;

		if (cur->msg) free(cur->msg);
		
		cur->msg = nullptr;
	}

	free(errs.errors);

	errs.errors_cnt = 0; errs.errors_size = 0;
}

void print_errors(errors_t errs, const char* expr) {
	if (errs.errors && errs.level >= ERROR_LEVEL_NOTE) {
		for (size_t i = 0; i < errs.errors_cnt; i++) {
			error_t *cur = errs.errors + i;

			fprintf(stderr, "%zu: %s %s: %s\n\r", cur->st_row, get_subsystem_name(cur->subsystem), get_level_name(cur->level), cur->msg);

			fprintf(stderr, "%6zu" SEPERATOR " ", cur->st_column);
			for (size_t j = 0; expr[j]; j++) {
				char c = expr[j];

				if (c == '\t') {
					fprintf(stderr, "    ");
				}

				else {
					fputc(c, stderr);
				}
			}
			fprintf(stderr, "\n\r");

			fprintf(stderr, "%6s" SEPERATOR " %*s", "", (int)cur->st_column, "");

			fprintf(stderr, "^");

			if (cur->columns >= 1) {
				for (ssize_t j = 0; j < cur->columns - 1; j++) {
					fprintf(stderr, "~");
				}
			}

			fprintf(stderr, "\n\r");

			fprintf(stderr, "%zu/%zu\n\r", cur->st_column, cur->columns);
		}
	}
} 

const char* get_level_name(error_level_e level) {
	switch (level) {
		case ERROR_LEVEL_OK:
			return "ok";
		case ERROR_LEVEL_NOTE:
			return "note";
		case ERROR_LEVEL_WARN:
			return "warning";
		case ERROR_LEVEL_ERROR:
			return "error";
		case ERROR_LEVEL_FATAL:
			return "fatal error";
	}

	return "";
}

const char* get_subsystem_name(subsystem_e subsystem) {
	switch (subsystem) {
		case SUBSYSTEM_LEXER:
			return "lexer";
		case SUBSYSTEM_PARSER:
			return "parser";
		case SUBSYSTEM_EVAL:
			return "eval";
	}

	return "";
}

const char* get_err_description(error_e type) {
	switch (type) {
		// warnings:

		// lexer
		case ERROR_LEXER_LEADING_ZEROES_IN_NUMBER:
			return "leading zeros in decimal integer literals is an obsolete way for octal; remove leading zero or use 0o for octal";
		
		// parser
		case ERROR_PARSER_POSSIBLE_UB:
			return "undefined behaviour possible";
		
			
		case ERROR_OK:
			return "ok";

		// errors:

		// lexer
		case ERROR_LEXER_INVALID_SYNTAX:
			return "invalid syntax";
		case ERROR_LEXER_INVALID_NUMBER:
			return "invalid number literal";
		case ERROR_LEXER_INVALID_NUMBER_RADIX:
			return "invalid number literal radix";
		
		// parser
		case ERROR_PARSER_UNCLOSED_PARENT:
			return "parent wasn't closed";
		case ERROR_PARSER_INVALID_SYNTAX:
			return "invalid syntax";
		case ERROR_PARSER_EOF:
			return "end of file";
		
		// eval
		case ERROR_EVAL_DIVISION_BY_ZERO:
			return "division by zero";
		case ERROR_EVAL_UNKNOWN_OPERATOR:
			return "unknown operation";
		case ERROR_EVAL_EXPR_ZERO_PTR:
			return "zero pointer";
	}

	return "";
}
