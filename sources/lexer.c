#include "types.h"
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

int recognize_token(token_t* _result, const char* c, const char** endptr) {
	token_t result = { .kind = TOKEN_UNDEFINED, .val = 0 };

	int err = 0;

	if (*c == 0) {
		result.kind = TOKEN_EOF;
	}

	else c = skip_spaces(c);

	if (*c == '+') {
		result.kind = TOKEN_PLUS;
		
		c++;
	}

	else if (isdigit(*c)) {
		result.kind = TOKEN_NUMBER;
		result.val = strtoull(c, (char**)(&c), 10);
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

		return 1;
	} 

	result.tokens_cnt++;

	if (_result) *_result = result;

	return err;
}

void get_token_name(char* buf, size_t buf_size, token_t token) {
	if (!buf) return;

	switch (token.kind) {
		case TOKEN_UNDEFINED:
			snprintf(buf, buf_size, "UNDEFINED"); break;
		case TOKEN_PLUS:
			snprintf(buf, buf_size, "PLUS"); break;
		case TOKEN_NUMBER:
			snprintf(buf, buf_size, "NUMBER %i", token.val); break;
		case TOKEN_EOF:
			snprintf(buf, buf_size, "EOF"); break;
	}
}
