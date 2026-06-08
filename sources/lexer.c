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

int recognize_token_len(size_t* _result, const char* c, const char** endptr) {
	size_t result = 0;

	int err = 0;

	c = skip_spaces(c);

	if (*c == 0) {
		result = 1;
	}

	if (*c == '+') {
		result = 1;
		
		c++;
	}

	else if (*c == '-') {
		result = 1;
		
		c++;
	}

	else if (strncmp(c, "**", 2) == 0) {
		result = 2;
		
		c += 2;
	}

	else if (*c == '*') {
		result = 1;
		
		c++;
	}

	else if (*c == '/') {
		result = 1;
		
		c++;
	}

	else if (*c == '%') {
		result = 1;
		
		c++;
	}

	else if (strncmp(c, "==", 2) == 0) {
		result = 2;
		
		c += 2;
	}

	else if (*c == '(') {
		result = 1;
		
		c++;
	}

	else if (*c == ')') {
		result = 1;
		
		c++;
	}

	else if (isdigit(*c)) {
		const char *_endptr = nullptr;
		
		parse_num(c, &_endptr);
		
		result = (size_t)(_endptr - c);
	}
	
	else err = 1;

	if (endptr) *endptr = c;

	if (_result) *_result = result;

	return err;
}

int recognize_token(token_t* _result, const char* c, const char** endptr) {
	token_t result = { .kind = TOKEN_UNDEFINED, .val = 0 };

	int err = 0;

	c = skip_spaces(c);

	if (*c == 0) {
		result.kind = TOKEN_EOF;
	}

	if (*c == '+') {
		result.kind = TOKEN_PLUS;
		
		c++;
	}

	else if (*c == '-') {
		result.kind = TOKEN_MINUS;
		
		c++;
	}

	else if (strncmp(c, "**", 2) == 0) {
		result.kind = TOKEN_POW;
		
		c += 2;
	}

	else if (*c == '*') {
		result.kind = TOKEN_MULTIPLY;
		
		c++;
	}

	else if (*c == '/') {
		result.kind = TOKEN_DIVIDE;
		
		c++;
	}

	else if (*c == '%') {
		result.kind = TOKEN_REMAINDER;
		
		c++;
	}

	else if (strncmp(c, "==", 2) == 0) {
		result.kind = TOKEN_EQUAL;
		
		c += 2;
	}

	else if (*c == '(') {
		result.kind = TOKEN_LEFT_PARENT;
		
		c++;
	}

	else if (*c == ')') {
		result.kind = TOKEN_RIGHT_PARENT;
		
		c++;
	}

	else if (isdigit(*c)) {
		result.kind = TOKEN_NUMBER;
		result.val = parse_num(c, &c);
	}
	
	else err = 1;

	if (endptr) *endptr = c;

	if (_result) *_result = result;

	return err;
}

int tokenize(tokens_t *_result, const char* code, const char** endptr) {
	tokens_t result = { 0 };

	if (_result) {
		result.tokens = calloc(TOKENS_KIND_ALLOC_STEP, sizeof(token_t));
	}

	size_t size = TOKENS_KIND_ALLOC_STEP;

	int err = 0;

	while (result.tokens_cnt < 1000) {
		if (_result) {
			int _err = recognize_token(result.tokens + result.tokens_cnt, code, &code);

			err = _err;

			if (_err != 0 || result.tokens[result.tokens_cnt].kind == TOKEN_EOF) break;
		}

		result.tokens_cnt++;

		if (_result && result.tokens_cnt >= size) {
			result.tokens = realloc(result.tokens, (size + TOKENS_KIND_ALLOC_STEP) * sizeof(token_t));

			memset(result.tokens + size, 0, TOKENS_KIND_ALLOC_STEP);

			size += TOKENS_KIND_ALLOC_STEP;
		}
	}

	if (result.tokens_cnt >= 1000) {
		printf("Maximum tokens count exceed (%zu >= 1000)\n\r", result.tokens_cnt);

		free(result.tokens);

		abort();

		return 1;
	} 

	result.tokens_cnt++;

	if (_result) *_result = result;

	return err;
}

ssize_t get_token_name(char* buf, size_t buf_size, token_t token) {
	if (!buf) return -1;

	size_t result = 0;

	switch (token.kind) {
		case TOKEN_UNDEFINED:
			result += snprintf(buf, buf_size, "UNDEFINED"); break;
		case TOKEN_PLUS:
			result += snprintf(buf, buf_size, "PLUS"); break;
		case TOKEN_MINUS:
			result += snprintf(buf, buf_size, "MINUS"); break;
		case TOKEN_POW:
			result += snprintf(buf, buf_size, "POW"); break;
		case TOKEN_MULTIPLY:
			result += snprintf(buf, buf_size, "MULTIPLY"); break;
		case TOKEN_DIVIDE:
			result += snprintf(buf, buf_size, "DIVIDE"); break;
		case TOKEN_REMAINDER:
			result += snprintf(buf, buf_size, "REMAINDER"); break;
		case TOKEN_EQUAL:
			result += snprintf(buf, buf_size, "EQUAL"); break;
		case TOKEN_NUMBER:
			result += snprintf(buf, buf_size, "NUMBER %i", token.val); break;
		case TOKEN_LEFT_PARENT:
			result += snprintf(buf, buf_size, "LPAR"); break;
		case TOKEN_RIGHT_PARENT:
			result += snprintf(buf, buf_size, "RPAR"); break;
		case TOKEN_EOF:
			result += snprintf(buf, buf_size, "EOF"); break;
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
		case TOKEN_EQUAL:
			result += snprintf(buf, buf_size, "=="); break;
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
