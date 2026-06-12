#ifndef REVAL_LEXER_H
#define REVAL_LEXER_H

#include "errors.h"
#include "types.h"

#include "lexer_fwd.h"

struct token_t {
	tokens_kind_e kind;

	int val; char* word;

	size_t column, row;
	size_t end_column, end_row;
};

struct tokens_t {
	token_t* tokens; size_t tokens_cnt;
};

#define TOKENS_KIND_ALLOC_STEP 4

error_e recognize_token_len(size_t* _result, const char* c, const char** endptr);

errors_t recognize_token(errors_t errs, size_t column, size_t row, size_t* end_column, size_t* end_row, token_t* _result, const char* c, const char** endptr);

errors_t tokenize(errors_t errs, tokens_t *_result, const char *code, const char** endptr);

void free_token(token_t *tokens);
void free_tokens(tokens_t tokens);

const char* get_token_kind_name(tokens_kind_e kind);
ssize_t get_token_name(char* buf, size_t buf_size, token_t token);
ssize_t get_token_c(char* buf, size_t buf_size, token_t token);

#endif
