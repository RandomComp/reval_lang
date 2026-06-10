#include "types.h"
#include "utils.h"
#include "lexer.h"

#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const char* skip_spaces(const char* text) {
	if (strchr(" \f\n\r\t\v", *text) != nullptr) {
		text += strspn(text, " \f\n\r\t\v");
	}

	return text;
}

void count_spaces(size_t* column, size_t* row, const char* text) {
	while (*text && strchr(" \t\n", *text) != nullptr) {
		switch (*text) {
			case ' ':
				if (column) *column += 1;
				break;
			case '\t':
				if (column) *column += 1;
				break;
			case '\n':
				if (column) *column = 0;
				if (row) *row += 1;
				break;
		}

		text++;
	}
}

error_e recognize_token_len(size_t* _result, const char* c, const char** _endptr) {
	size_t result = 0;

	error_e err = 0;

	c = skip_spaces(c);

	if (*c == 0) {
		result = 1;
	}

	if (isdigit(*c)) {
		const char* endptr = nullptr;
		
		parse_num(nullptr, c, &endptr);

		result = endptr - c;

		if (_endptr) *_endptr = c;

		if (_result) *_result = result;

		return err;
	}
	
	const char* two_c_ops = "**==!=>=<=";

	bool ok = false;
	
	for (size_t i = 0; two_c_ops[i]; i += 2) {
		if (two_c_ops[i] == c[0] &&
			two_c_ops[i + 1] == c[1]) {
			ok = true;

			result = 2;

			break;
		}
	}

	if (!ok) {
		const char* one_c_ops = "+-*/%><?:()";

		const char* one_c_op = strchr(one_c_ops, *c);

		if (one_c_op != nullptr) {
			result = 1;
		}

		else err = 1;
	}

	if (_endptr) *_endptr = c;

	if (_result) *_result = result;

	return err;
}

#define ALPHABET " +-*/%><?:()=!"

errors_t recognize_token(errors_t errs, size_t* column, size_t* row, size_t* end_column, size_t* end_row, token_t* _result, const char* c, const char** endptr) {
	token_t result = { .kind = TOKEN_UNDEFINED, .val = 0 };

	bool recognized = false;

	size_t _columns = 0, _rows = 0;
	size_t _end_columns = 0, _end_rows = 0;

	// printf("st c = %.2x '%c'\n\r", *c, *c);

	count_spaces(&_columns, &_rows, c);

	c = skip_spaces(c);

	if (*c == 0) {
		result.kind = TOKEN_EOF;

		recognized = true;
	}

	if (!recognized && isdigit(*c)) {
		const char* endc = c;

		uint64 temp = 0;
		parse_num_err_e parse_num_err = parse_num(&temp, c, &endc);

		size_t len = endc - c;

		if (parse_num_err == PARSER_NUM_ERR_LEADING_ZEROES) {
			size_t leading_zeroes_len = strcspn(c, "1234567" ALPHABET);

			errs = emit_error(errs, SUBSYSTEM_LEXER, ERROR_LEXER_LEADING_ZEROES_IN_NUMBER, ERROR_LEVEL_WARN, _columns, _rows, _end_columns + leading_zeroes_len - 1, _end_rows, nullptr);
		}

		size_t expected_len = strcspn(c, ALPHABET);

		const char* radix_name = get_radix_name(get_num_radix(c));
		const char* c_part = get_number_part(c);

		size_t c_part_expected_len = strcspn(c_part, ALPHABET);

		const char* expected_endc = c + expected_len;

		switch (parse_num_err) {
			case PARSER_NUM_ERR_MAY_DECIMAL:
				errs = emit_error(errs, SUBSYSTEM_LEXER, ERROR_LEXER_INVALID_NUMBER_RADIX, ERROR_LEVEL_ERROR, _columns + len, _rows, _end_columns + expected_len - len, _end_rows, "invalid %s number \"%.*s\", did you mean decimal \"%.*s\"?", radix_name, expected_len, c, c_part_expected_len, c_part);
		
				break;
			case PARSER_NUM_ERR_MAY_HEXADECIMAL:
				errs = emit_error(errs, SUBSYSTEM_LEXER, ERROR_LEXER_INVALID_NUMBER_RADIX, ERROR_LEVEL_ERROR, _columns + len, _rows, _end_columns + expected_len - len, _end_rows, "invalid %s number \"%.*s\", did you mean hexadecimal \"0x%.*s\"?", radix_name, expected_len, c, c_part_expected_len, c_part);
		
				break;
			default: break;
		}

		_end_columns += expected_len;

		if (parse_num_err == 0 || parse_num_err == PARSER_NUM_ERR_INVALID_LITERAL) {
			if (expected_endc != endc)  {
				errs = emit_error(errs, SUBSYSTEM_LEXER, ERROR_LEXER_INVALID_NUMBER, ERROR_LEVEL_ERROR, _columns, _rows, _end_columns, _end_rows, "\"%.*s\" is not valid %s number", expected_len, c, radix_name);
			}
		}

		if (expected_endc == endc)  {
			result.kind = TOKEN_NUMBER;
			result.val = temp;
		}

		c = expected_endc;

		recognized = true;
	}

	if (!recognized) {
		const char* two_c_ops = "**==!=>=<=";

		size_t index = 0;
		
		for (size_t i = 0; two_c_ops[i]; i += 2) {
			if (two_c_ops[i] == c[0] &&
				two_c_ops[i + 1] == c[1]) {
				result.kind = index + TOKEN_POW;

				recognized = true;
				
				_columns += 2;
				
				_end_columns += 2;

				c += 2;

				break;
			}

			index++;
		}
	}
	
	if (!recognized) {
		const char* one_c_ops = "+-*/%><?:()";

		const char* one_c_op = strchr(one_c_ops, *c);

		if (one_c_op != nullptr) {
			tokens_kind_e kind = (one_c_op - one_c_ops) + TOKEN_PLUS;

			result.kind = kind;

			c++;

			recognized = true;

			_columns += 1;
			_end_columns += 1;
		}

		else {
			size_t expected_len = strcspn(c, "0123456789" ALPHABET);

			_end_columns += expected_len;

			errs = emit_error(errs, SUBSYSTEM_LEXER, ERROR_LEXER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, _columns, _rows, _end_columns, _end_rows, "unknown operator \"%.*s\"", expected_len, c);

			c += expected_len;
		}
	}

	// printf("recognized = %i; %.2x\n\r", recognized, *c);

	if (recognized) {
		result.column = _columns;
		result.row = _rows;

		result.end_column = _end_columns;
		result.end_row = _end_rows;
	}

	if (_result) *_result = result;

	if (column) *column += _columns;
	if (row) *row += _rows;

	if (end_column) *end_column += _end_columns;
	if (end_row) *end_row += _end_rows;

	if (endptr) *endptr = c;

	return errs;
}

errors_t tokenize(errors_t errs, tokens_t *_result, const char* code, const char** endptr) {
	tokens_t result = { 0 };

	if (_result) {
		result.tokens = calloc(TOKENS_KIND_ALLOC_STEP, sizeof(token_t));
	}

	size_t size = TOKENS_KIND_ALLOC_STEP;

	size_t columns = 0, rows = 0;
	size_t end_columns = 0, end_rows = 0;

	while (result.tokens_cnt < 1000) {
		count_spaces(&columns, &rows, code);

		code = skip_spaces(code);
		
		// if (*code == 0) return errs;

		if (code[0] == '/' && code[1] == '/') {
			code += 2;

			while (*code && strchr("\f\n\r\v", *code) == nullptr) {
				code++;
			}

			rows++;
		}

		if (_result) {
			// printf("%zu/%zu; %.*s\n\r", columns, end_columns, (int)(end_columns - columns), code);
			
			errs = recognize_token(errs, &columns, &rows, &end_columns, &end_rows, result.tokens + result.tokens_cnt, code, &code);

			// char buf[64] = { 0 };
			// get_token_name(buf, 64, result.tokens[result.tokens_cnt]);

			// printf("\"%s\"\n\r", buf);

			if (result.tokens[result.tokens_cnt].kind == TOKEN_EOF) break;
		}

		result.tokens_cnt++;

		if (_result && result.tokens_cnt >= size) {
			result.tokens = realloc(result.tokens, (size + TOKENS_KIND_ALLOC_STEP) * sizeof(token_t));

			memset(result.tokens + size, 0, TOKENS_KIND_ALLOC_STEP);

			size += TOKENS_KIND_ALLOC_STEP;
		}

		// columns = end_columns; rows = end_rows;
	}

	if (result.tokens_cnt >= 1000) {
		printf("Maximum tokens count exceed (%zu >= 1000)\n\r", result.tokens_cnt);

		free(result.tokens);

		abort();

		return errs;
	} 

	result.tokens_cnt++;

	if (_result) *_result = result;

	if (endptr) *endptr = code;

	return errs;
}

const char* get_token_kind_name(tokens_kind_e kind) {
	switch (kind) {
		case TOKEN_UNDEFINED:
			return "UNDEFINED";
		case TOKEN_PLUS:
			return "PLUS";
		case TOKEN_MINUS:
			return "MINUS";
		case TOKEN_POW:
			return "POW";
		case TOKEN_MULTIPLY:
			return "MULTIPLY";
		case TOKEN_DIVIDE:
			return "DIVIDE";
		case TOKEN_REMAINDER:
			return "REMAINDER";
		case TOKEN_EQUALS:
			return "EQUALS";
		case TOKEN_NOT_EQUALS:
			return "NOT EQUALS";
		case TOKEN_GREATER_EQUALS:
			return "GREATER EQUALS";
		case TOKEN_LESS_EQUALS:
			return "LESS EQUALS";
		case TOKEN_GREATER:
			return "GREATER";
		case TOKEN_LESS:
			return "LESS";
		case TOKEN_QUESTION_MARK:
			return "QUESTION MARK";
		case TOKEN_COLON:
			return "COLON";
		case TOKEN_NUMBER:
			return "NUMBER";
		case TOKEN_LEFT_PARENT:
			return "LPAR";
		case TOKEN_RIGHT_PARENT:
			return "RPAR";
		case TOKEN_EOF:
			return "EOF";
	}

	return "";
}

ssize_t get_token_name(char* buf, size_t buf_size, token_t token) {
	if (!buf) return -1;

	size_t result = 0;

	if (token.kind == TOKEN_NUMBER) {
		result += snprintf(buf, buf_size, "NUMBER %i", token.val);
	}

	else {
		result += snprintf(buf, buf_size, "%s", get_token_kind_name(token.kind));
	}

	return result;
}

ssize_t get_token_c(char* buf, size_t buf_size, token_t token) {
	if (!buf) return -1;

	ssize_t result = 0;

	switch (token.kind) {
		case TOKEN_UNDEFINED:
			result += snprintf(buf, buf_size, ""); break;
		case TOKEN_PLUS:
			result += snprintf(buf, buf_size, "+"); break;
		case TOKEN_MINUS:
			result += snprintf(buf, buf_size, "-"); break;
		case TOKEN_POW:
			result += snprintf(buf, buf_size, "**"); break;
		case TOKEN_MULTIPLY:
			result += snprintf(buf, buf_size, "*"); break;
		case TOKEN_DIVIDE:
			result += snprintf(buf, buf_size, "/"); break;
		case TOKEN_REMAINDER:
			result += snprintf(buf, buf_size, "%%"); break;
		case TOKEN_EQUALS:
			result += snprintf(buf, buf_size, "=="); break;
		case TOKEN_NOT_EQUALS:
			result += snprintf(buf, buf_size, "!="); break;
		case TOKEN_GREATER_EQUALS:
			result += snprintf(buf, buf_size, ">="); break;
		case TOKEN_LESS_EQUALS:
			result += snprintf(buf, buf_size, "<="); break;
		case TOKEN_GREATER:
			result += snprintf(buf, buf_size, ">"); break;
		case TOKEN_LESS:
			result += snprintf(buf, buf_size, "<"); break;
		case TOKEN_QUESTION_MARK:
			result += snprintf(buf, buf_size, "?"); break;
		case TOKEN_COLON:
			result += snprintf(buf, buf_size, ":"); break;
		case TOKEN_NUMBER:
			result += snprintf(buf, buf_size, "%i", token.val); break;
		case TOKEN_LEFT_PARENT:
			result += snprintf(buf, buf_size, "("); break;
		case TOKEN_RIGHT_PARENT:
			result += snprintf(buf, buf_size, ")"); break;
		case TOKEN_EOF:
			result += snprintf(buf, buf_size, ""); break;
	}

	return result;
}
