#ifndef REVAL_LEXER_FWD_H
#define REVAL_LEXER_FWD_H

typedef enum tokens_kind_e {
	TOKEN_UNDEFINED,
	TOKEN_PLUS,
	TOKEN_NUMBER,
	TOKEN_EOF
} tokens_kind_e;

typedef struct token_t token_t;

typedef struct tokens_t tokens_t;

#endif
