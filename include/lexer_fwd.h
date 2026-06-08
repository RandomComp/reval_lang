#ifndef REVAL_LEXER_FWD_H
#define REVAL_LEXER_FWD_H

typedef enum tokens_kind_e {
	TOKEN_UNDEFINED,
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_POW,
	TOKEN_MULTIPLY,
	TOKEN_DIVIDE,
	TOKEN_REMAINDER,
	TOKEN_EQUAL,
	TOKEN_NUMBER,
	TOKEN_LEFT_PARENT,
	TOKEN_RIGHT_PARENT,
	TOKEN_EOF
} tokens_kind_e;

typedef struct token_t token_t;

typedef struct tokens_t tokens_t;

#endif
