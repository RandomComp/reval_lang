#ifndef REVAL_LEXER_FWD_H
#define REVAL_LEXER_FWD_H

typedef enum tokens_kind_e {
	TOKEN_UNDEFINED,

	// one char operators
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_MULTIPLY,
	TOKEN_DIVIDE,
	TOKEN_REMAINDER,
	TOKEN_GREATER, // >
	TOKEN_LESS, // <
	TOKEN_QUESTION_MARK, // ?
	TOKEN_COLON, // :
	TOKEN_LEFT_PARENT, // (
	TOKEN_RIGHT_PARENT, // )

	// two char operators
	TOKEN_POW, // **
	TOKEN_EQUALS, // ==
	TOKEN_NOT_EQUALS, // !=
	TOKEN_GREATER_EQUALS, // >=
	TOKEN_LESS_EQUALS, // <=

	TOKEN_NUMBER,
	
	TOKEN_EOF,
} tokens_kind_e;

typedef struct token_t token_t;

typedef struct tokens_t tokens_t;

#endif
