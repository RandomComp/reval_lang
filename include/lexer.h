#ifndef REVAL_LEXER_H
#define REVAL_LEXER_H

#include "types.h"

#include "lexer_fwd.h"

struct token_t {
	tokens_kind_e kind;

	int val;
};

struct tokens_t {
	token_t* tokens; size_t tokens_cnt;
};

// typedef enum tokenizer_err_e {
	
// } tokenizer_err_e;

#define TOKENS_KIND_ALLOC_STEP 4

const char* skip_spaces(const char* text);

int recognize_token_len(size_t* _result, const char* c, const char** endptr);

int recognize_token(token_t* _result, const char* c, const char** endptr);

int tokenize(tokens_t *_result, const char *code, const char** endptr);

ssize_t get_token_name(char* buf, size_t buf_size, token_t token);
ssize_t get_token_c(char* buf, size_t buf_size, token_t token);

#endif
