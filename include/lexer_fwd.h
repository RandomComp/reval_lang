#ifndef REVAL_LEXER_FWD_H
#define REVAL_LEXER_FWD_H

typedef enum tokens_kind_e {
	TOKEN_UNEXPECTED,
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
	TOKEN_ASSIGNMENT, // =
	TOKEN_SEMICOLON, // ;
	TOKEN_EXCLAMATION_MARK, // !
	TOKEN_TILDE, // ~

	// two char operators
	TOKEN_POW, // **
	TOKEN_EQUALS, // ==
	TOKEN_NOT_EQUALS, // !=
	TOKEN_GREATER_EQUALS, // >=
	TOKEN_LESS_EQUALS, // <=
	TOKEN_LOGIC_AND, // &&
	TOKEN_LOGIC_OR, // ||

	TOKEN_NUMBER,
	TOKEN_WORD,
	
	TOKEN_EOF,

	// for parser
	TOKEN_PARSER_UNARY_ONLY,
} tokens_kind_e;

typedef struct token_t token_t;

typedef struct tokens_t tokens_t;

#define ALPHABET " +-*/%><?:()=;!~"

#define ONE_C_OPS "+-*/%><?:()=;!~"
#define TWO_C_OPS "**==!=>=<=&&||"

#endif
