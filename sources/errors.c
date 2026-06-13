#include "types.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <string.h>

#include "errors.h"

errors_t emit_error(errors_t errors, subsystem_e subsystem, error_e type, error_level_e level, const char *filename, ssize_t st_column, ssize_t st_row, ssize_t end_column, ssize_t end_row, const char* add_hint, const char* del_hint, const char* msg, ...) {
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
	cur->filename = strdup(filename);
	cur->st_column = st_column;
	cur->st_row = st_row;
	cur->end_column = end_column;
	cur->end_row = end_row;

	if (add_hint) {
		cur->add_hint = strdup(add_hint);
	}

	if (del_hint) {
		cur->del_hint = strdup(del_hint);
	}

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

		if (cur->add_hint) free(cur->add_hint);
		
		cur->add_hint = nullptr;

		if (cur->del_hint) free(cur->del_hint);
		
		cur->del_hint = nullptr;

		if (cur->filename) free(cur->filename);
		
		cur->filename = nullptr;
	}

	free(errs.errors);

	errs.errors_cnt = 0; errs.errors_size = 0;
}

static const term_colors_e levels_color[] = {
	[ERROR_LEVEL_NOTE] = TERM_COLOR_AQUA,
	[ERROR_LEVEL_WARN] = TERM_COLOR_MAGENTA,
	[ERROR_LEVEL_ERROR] = TERM_COLOR_BRIGHT_RED,
	[ERROR_LEVEL_FATAL] = TERM_COLOR_BRIGHT_RED,
};

void print_errors(errors_t errs, const char* expr) {
	if (errs.errors && errs.level != ERROR_LEVEL_OK) {
		for (size_t i = 0; i < errs.errors_cnt; i++) {
			error_t *cur = errs.errors + i;

			term_colors_e level_color = levels_color[cur->level];
			
			set_fg_color(stderr, TERM_COLOR_BRIGHT_WHITE);
			fprintf(stderr, "%s:%zu:%zu: ", cur->filename, cur->st_row + 1, cur->st_column + 1);
			set_fg_color(stderr, level_color);
			fprintf(stderr, "%s %s", get_subsystem_name(cur->subsystem), get_level_name(cur->level));
			set_default(stderr);
			set_fg_color(stderr, TERM_COLOR_BRIGHT_WHITE);
			fprintf(stderr, ": %s\n\r", cur->msg);
			set_default(stderr);

			ssize_t expr_j = skip_newlines(expr, cur->st_row);

			fprintf(stderr, "%6zu" SEPERATOR " ", cur->st_row + 1);
			for (; expr[expr_j] && expr[expr_j] != '\n' && expr[expr_j] != '\r'; expr_j++) {
				if (expr_j == cur->st_column) {
					set_fg_color(stderr, level_color);
				}

				if (expr_j >= cur->end_column) {
					set_default(stderr);
				}

				char c = expr[expr_j];

				if (c == '\t') {
					fprintf(stderr, "    ");
				}

				else if (isascii(c)) {
					fputc(c, stderr);
				}

				else {
					fprintf(stderr, "<0x%.2x>", c);
				}
			}
			set_default(stderr);
			fprintf(stderr, "\n\r");

			fprintf(stderr, "%6s" SEPERATOR " %*s", "", (int)cur->st_column, "");

			fprintf(stderr, "^");

			ssize_t columns = cur->end_column - cur->st_column;

			if (columns >= 1) {
				for (ssize_t j = 0; j < columns - 1; j++) {
					fprintf(stderr, "~");
				}
			}

			fprintf(stderr, "\n\r");

			if (cur->del_hint) {
				fprintf(stderr, "%6s %*s -", "", (int)cur->st_column - 1, "");
				set_fg_color(stderr, TERM_COLOR_RED);
				fprintf(stderr, "%s\n\r", cur->del_hint);
				set_default(stderr);
			}

			if (cur->add_hint) {
				fprintf(stderr, "%6s %*s +", "", (int)cur->st_column - 1, "");
				set_fg_color(stderr, TERM_COLOR_GREEN);
				fprintf(stderr, "%s\n\r", cur->add_hint);
				set_default(stderr);
			}

			// printf("%zu/%zu\n\r", cur->st_column, cur->columns);
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
		case ERROR_LEXER_DOUBLE_QUOTE_NEVER_CLOSED:
			return "double '\"' was never closed";
		
		// parser
		case ERROR_PARSER_UNCLOSED_PARENT:
			return "parent wasn't closed";
		case ERROR_PARSER_INVALID_SYNTAX:
			return "invalid syntax";
		case ERROR_PARSER_INCOMPATIBLE_OPERATOR_TYPE:
			return "incorrect combination of operator and operands";
		case ERROR_PARSER_EOF:
			return "end of file";
		
		// eval
		case ERROR_EVAL_DIVISION_BY_ZERO:
			return "division by zero";
		case ERROR_EVAL_UNKNOWN_OPERATOR:
			return "unknown operation";
		case ERROR_EVAL_UNKNOWN_VARIABLE:
			return "unknown variable";
		case ERROR_EVAL_EXPR_ZERO_PTR:
			return "zero pointer";
	}

	return "";
}
