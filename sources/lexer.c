#include "types.h"
#include "utils.h"
#include "lexer.h"

#include <ctype.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

error_e recognize_token_len(size_t* _result, const char* c, const char** _endptr) {
	size_t result = 0;

	error_e err = 0;

	c += skip_spaces(c);

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

errors_t recognize_token(errors_t errs, interneds_t *interneds, const char* filename, size_t column, size_t row, size_t* end_column, size_t* end_row, token_t* _result, const char* c, const char** endptr) {
	token_t result = { .kind = TOKEN_UNDEFINED, .val = 0 };

	result.filename = get_interned(interneds, filename);

	bool recognized = false;

	size_t _end_columns = 0, _end_rows = 0;

	if (*c == 0) {
		result.kind = TOKEN_EOF;

		recognized = true;

		_end_columns = 1;
	}

	if (!recognized && isdigit(*c)) {
		const char* endc = c;

		uint64 temp = 0;
		parse_num_err_e parse_num_err = parse_num(&temp, c, &endc);

		size_t len = endc - c;

		if (parse_num_err == PARSER_NUM_ERR_LEADING_ZEROES) {
			size_t leading_zeroes_len = strcspn(c, "1234567" ALPHABET);

			errs = emit_error(errs, SUBSYSTEM_LEXER, ERROR_LEXER_LEADING_ZEROES_IN_NUMBER, ERROR_LEVEL_WARN, filename, column, row, _end_columns + leading_zeroes_len, _end_rows, nullptr, nullptr, nullptr);
		}

		size_t expected_len = strcspn(c, SPACES ALPHABET);

		radix_e radix = get_num_radix(c);
		const char* radix_name = get_radix_name(radix);
		const char* c_part = get_number_part(c);

		size_t c_part_expected_len = strcspn(c_part, ALPHABET);

		const char* alphabet = get_radix_alphabet(radix);

		radix_e may_be_radix = radix;

		if (parse_num_err == PARSER_NUM_ERR_MAY_DECIMAL) {
			may_be_radix = RADIX_DECIMAL;
		}

		else if (parse_num_err == PARSER_NUM_ERR_MAY_HEXADECIMAL) {
			may_be_radix = RADIX_HEXADECIMAL;
		}

		const char* may_be_radix_name = get_radix_name(may_be_radix);

		_end_columns += expected_len;

		if (parse_num_err == PARSER_NUM_ERR_MAY_DECIMAL ||
			parse_num_err == PARSER_NUM_ERR_MAY_HEXADECIMAL) {
			size_t first_valid_digit = len;

			first_valid_digit = strlcspn(c + len, alphabet);

			size_t add_str_len = snprintf(nullptr, 0, "%s%.*s", get_radix_prefix(may_be_radix), (int)c_part_expected_len, c_part) + 1;
			char* add_str = malloc(add_str_len);

			snprintf(add_str, add_str_len, "%s%.*s", get_radix_prefix(may_be_radix), (int)c_part_expected_len, c_part);
		
			size_t del_str_len = snprintf(nullptr, 0, "%.*s", (int)expected_len, c) + 1;
			char* del_str = malloc(del_str_len);

			snprintf(del_str, del_str_len, "%.*s", (int)expected_len, c);
		
			errs = emit_error(errs, SUBSYSTEM_LEXER, ERROR_LEXER_INVALID_NUMBER_RADIX, ERROR_LEVEL_ERROR, filename, column, row, _end_columns, _end_rows, add_str, del_str, "invalid \"%.*s\" digits in %s literal, did you mean %s \"%s\"?", first_valid_digit - len + 1, c + len, radix_name, may_be_radix_name, add_str);
			
			free(add_str);
			free(del_str);
		}

		const char* expected_endc = c + expected_len;

		if (parse_num_err == 0 || parse_num_err == PARSER_NUM_ERR_INVALID_LITERAL) {
			if (expected_endc != endc)  {
				errs = emit_error(errs, SUBSYSTEM_LEXER, ERROR_LEXER_INVALID_NUMBER, ERROR_LEVEL_ERROR, filename, column, row, _end_columns, _end_rows, nullptr, nullptr, "\"%.*s\" is not valid %s number", expected_len, c, radix_name);
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
		size_t index = 0;
		
		for (size_t i = 0; TWO_C_OPS[i]; i += 2) {
			if (TWO_C_OPS[i] == c[0] &&
				TWO_C_OPS[i + 1] == c[1]) {
				result.kind = index + TOKEN_INCREMENT;

				recognized = true;
				
				_end_columns += 2;

				c += 2;

				break;
			}

			index++;
		}
	}
	
	if (!recognized) {
		if (*c == 0) goto exit;

		const char* one_c_op = strchr(ONE_C_OPS, *c);

		if (one_c_op != nullptr) {
			tokens_kind_e kind = (one_c_op - ONE_C_OPS) + TOKEN_PLUS;

			result.kind = kind;

			size_t times = *c == ';' ? strspn(c, " " SPACES ";") : 1; // skip multiple semicolon delimeters

			c += times;

			recognized = true;

			_end_columns += times;
		}

		else if (*c == '\"') {
			const char *str_endptr = c;

			result.kind = TOKEN_STRING;

			size_t length = parse_str(nullptr, 0, c, &str_endptr) + 1;

			char *temp = malloc(length);

			parse_str(temp, length, c, &str_endptr);

			if (!temp) {
				errs = emit_error(errs, SUBSYSTEM_LEXER, ERROR_LEXER_DOUBLE_QUOTE_NEVER_CLOSED, ERROR_LEVEL_ERROR, filename, column, row, 1, _end_rows, nullptr, nullptr, "quote '\"' was never closed");
			}

			result.word = get_interned(interneds, temp);

			// else if (str_endptr >= (c + strlen(c))) {
			// 	errs = emit_error(errs, SUBSYSTEM_LEXER, ERROR_LEXER_DOUBLE_QUOTE_NEVER_CLOSED, ERROR_LEVEL_ERROR, filename, column, row, 1, _end_rows, nullptr, nullptr, "quote '\"' was never closed");
			// }

			recognized = true;

			c = str_endptr;

			_end_columns += str_endptr - c;
		}

		else if (strchr(ALPHABET "0123456789", *c) == nullptr) {
			size_t expected_len = strcspn(c, SPACES ALPHABET);

			result.kind = TOKEN_WORD;
			char *temp = malloc(expected_len + 1);

			snprintf(temp, expected_len + 1, "%.*s", (int)expected_len, c);

			result.word = get_interned(interneds, temp);

			free(temp);

			recognized = true;

			_end_columns += expected_len;

			c += expected_len;
		}
	}

	if (recognized) {
		result.column = column;
		result.row = row;

		result.end_column = column + _end_columns;
		result.end_row = row + _end_rows;
	}

	if (!recognized) {
		size_t expected_len = strcspn(c, SPACES ONE_C_OPS);

		errs = emit_error(errs, SUBSYSTEM_LEXER, ERROR_LEXER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, filename, column, row, expected_len, _end_rows, nullptr, nullptr, "unexpected symbols \"%.*s\"", expected_len, c);

		result.kind = TOKEN_UNEXPECTED;
	}

	if (_result) *_result = result;

	exit:

	if (end_column) *end_column = _end_columns;
	if (end_row) *end_row = _end_rows;

	if (endptr) *endptr = c;

	return errs;
}

errors_t tokenize(errors_t errs, tokens_t *_result, interneds_t *interneds, const char* filename, const char* code, const char** endptr) {
	tokens_t result = { 0 };

	if (_result) {
		result.tokens = calloc(TOKENS_ALLOC_STEP, sizeof(token_t));
	}

	size_t size = TOKENS_ALLOC_STEP;

	size_t columns = 0, rows = 0;

	while (result.tokens_cnt < 1000) {
		count_spaces(&columns, &rows, code);

		code += skip_spaces(code);
		
		// if (*code == 0) return errs;

		if (code[0] == '/' && code[1] == '/') {
			code += 2;

			while (*code && strchr(SPACES, *code) == nullptr) {
				code++;
			}

			rows++;
		}

		if (_result && result.tokens_cnt >= size) {
			result.tokens = realloc(result.tokens, (size + TOKENS_ALLOC_STEP) * sizeof(token_t));

			memset(result.tokens + size, 0, TOKENS_ALLOC_STEP);

			size += TOKENS_ALLOC_STEP;
		}

		token_t* cur = result.tokens + result.tokens_cnt;
		
		size_t end_columns = 0, end_rows = 0;

		if (_result) {
			errs = recognize_token(errs, interneds, filename, columns, rows, &end_columns, &end_rows, cur, code, &code);

			if (result.tokens[result.tokens_cnt].kind == TOKEN_UNEXPECTED ||
				result.tokens[result.tokens_cnt].kind == TOKEN_EOF) break;
		}

		result.tokens_cnt++;

		columns += end_columns; rows += end_rows;
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

void free_token(token_t *token) {
	if (!token) return;

	token->word = nullptr;

	free(token);
}

void free_tokens(tokens_t tokens) {
	for (size_t i = 0; i < tokens.tokens_cnt; i++) {
		tokens.tokens[i].word = nullptr;

		tokens.tokens[i].filename = nullptr;
	}

	free(tokens.tokens);
}

const char* get_token_kind_name(tokens_kind_e kind) {
	switch (kind) {
		case TOKEN_UNDEFINED:
			return "UNDEFINED";
		case TOKEN_ONE_CHAR_OPERATORS_START:
			return "ONE CHAR OPERATORS START";
		case TOKEN_TWO_CHAR_OPERATORS_START:
			return "TWO CHAR OPERATORS START";
		case TOKEN_THREE_CHAR_OPERATORS_START:
			return "THREE CHAR OPERATORS START";
		case TOKEN_PLUS:
			return "PLUS";
		case TOKEN_MINUS:
			return "MINUS";
		case TOKEN_INCREMENT:
			return "INCREMENT";
		case TOKEN_DECREMENT:
			return "DECREMENT";
		case TOKEN_POW:
			return "POW";
		case TOKEN_PLUS_ASSIGNMENT:
			return "PLUS ASSIGNMENT";
		case TOKEN_MINUS_ASSIGNMENT:
			return "MINUS ASSIGNMENT";
		case TOKEN_MULTIPLY_ASSIGNMENT:
			return "MULTIPLY ASSIGNMENT";
		case TOKEN_DIVIDE_ASSIGNMENT:
			return "DIVIDE ASSIGNMENT";
		case TOKEN_REMAINDER_ASSIGNMENT:
			return "REMAINDER ASSIGNMENT";
		case TOKEN_POW_ASSIGNMENT:
			return "POW ASSIGNMENT";
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
		case TOKEN_LOGIC_AND:
			return "LOGIC AND";
		case TOKEN_LOGIC_OR:
			return "LOGIC OR";
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
		case TOKEN_WORD:
			return "WORD";
		case TOKEN_STRING:
			return "STRING";
		case TOKEN_LEFT_PARENT:
			return "LPAR";
		case TOKEN_RIGHT_PARENT:
			return "RPAR";
		case TOKEN_ASSIGNMENT:
			return "ASSIGNMENT";
		case TOKEN_SEMICOLON:
			return "SEMICOLON";
		case TOKEN_EXCLAMATION_MARK:
			return "EXCLAMATION MARK";
		case TOKEN_TILDE:
			return "TILDE";
		case TOKEN_UNEXPECTED:
			return "UNEXPECTED";
		case TOKEN_EOF:
			return "EOF";
		case TOKEN_PARSER_UNARY_ONLY:
			return "PARSER UNARY ONLY";
		case TOKEN_PREINCREMENT:
			return "PREINCREMENT";
		case TOKEN_PREDECREMENT:
			return "PREDECREMENT";
		case TOKEN_POSTINCREMENT:
			return "POSTINCREMENT";
		case TOKEN_POSTDECREMENT:
			return "POSTDECREMENT";
	}

	return "";
}

ssize_t get_token_name(char* buf, size_t buf_size, token_t token) {
	if (!buf) return -1;

	size_t result = 0;

	if (token.kind == TOKEN_NUMBER) {
		result += snprintf(buf, buf_size, "NUMBER %i", token.val);
	}

	else if (token.kind == TOKEN_WORD) {
		result += snprintf(buf, buf_size, "WORD %s", token.word);
	}

	else if (token.kind == TOKEN_STRING) {
		result += snprintf(buf, buf_size, "STRING \"%s\"", token.word);
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
			break;
		case TOKEN_ONE_CHAR_OPERATORS_START:
			break;
		case TOKEN_TWO_CHAR_OPERATORS_START:
			break;
		case TOKEN_THREE_CHAR_OPERATORS_START:
			break;
		case TOKEN_PREINCREMENT:
			break;
		case TOKEN_PREDECREMENT:
			break;
		case TOKEN_POSTINCREMENT:
			break;
		case TOKEN_POSTDECREMENT:
			break;
		case TOKEN_PLUS:
			result += snprintf(buf, buf_size, "+"); break;
		case TOKEN_MINUS:
			result += snprintf(buf, buf_size, "-"); break;
		case TOKEN_INCREMENT:
			result += snprintf(buf, buf_size, "++"); break;
		case TOKEN_DECREMENT:
			result += snprintf(buf, buf_size, "--"); break;
		case TOKEN_POW:
			result += snprintf(buf, buf_size, "**"); break;
		case TOKEN_PLUS_ASSIGNMENT:
			result += snprintf(buf, buf_size, "+="); break;
		case TOKEN_MINUS_ASSIGNMENT:
			result += snprintf(buf, buf_size, "-="); break;
		case TOKEN_MULTIPLY_ASSIGNMENT:
			result += snprintf(buf, buf_size, "*="); break;
		case TOKEN_DIVIDE_ASSIGNMENT:
			result += snprintf(buf, buf_size, "/="); break;
		case TOKEN_REMAINDER_ASSIGNMENT:
			result += snprintf(buf, buf_size, "%%="); break;
		case TOKEN_POW_ASSIGNMENT:
			result += snprintf(buf, buf_size, "**="); break;
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
		case TOKEN_LOGIC_AND:
			result += snprintf(buf, buf_size, "&&"); break;
		case TOKEN_LOGIC_OR:
			result += snprintf(buf, buf_size, "||"); break;
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
		case TOKEN_WORD:
			result += snprintf(buf, buf_size, "%s", token.word); break;
		case TOKEN_STRING:
			result += snprintf(buf, buf_size, "\"%s\"", token.word); break;
		case TOKEN_LEFT_PARENT:
			result += snprintf(buf, buf_size, "("); break;
		case TOKEN_RIGHT_PARENT:
			result += snprintf(buf, buf_size, ")"); break;
		case TOKEN_ASSIGNMENT:
			result += snprintf(buf, buf_size, "="); break;
		case TOKEN_SEMICOLON:
			result += snprintf(buf, buf_size, ";"); break;
		case TOKEN_EXCLAMATION_MARK:
			result += snprintf(buf, buf_size, "!"); break;
		case TOKEN_TILDE:
			result += snprintf(buf, buf_size, "~"); break;
		case TOKEN_UNEXPECTED:
			break;
		case TOKEN_EOF:
			break;
		case TOKEN_PARSER_UNARY_ONLY:
			break;
	}

	return result;
}
