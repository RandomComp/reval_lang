#ifndef REVAL_LEXER_FWD_H
#define REVAL_LEXER_FWD_H

typedef enum tokens_kind_e {
	TOKEN_UNEXPECTED,
	TOKEN_UNDEFINED,

	// one char operators
	TOKEN_ONE_CHAR_OPERATORS_START, // pseudo token

	TOKEN_PLUS, // +
	TOKEN_MINUS, // -
	TOKEN_MULTIPLY, // *
	TOKEN_DIVIDE, // /
	TOKEN_REMAINDER, // %
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
	TOKEN_TWO_CHAR_OPERATORS_START, // pseudo token

	TOKEN_INCREMENT, // ++x
	TOKEN_DECREMENT, // --x

	TOKEN_POW, // **

	TOKEN_PLUS_ASSIGNMENT, // +=
	TOKEN_MINUS_ASSIGNMENT, // -=
	TOKEN_MULTIPLY_ASSIGNMENT, // *=
	TOKEN_DIVIDE_ASSIGNMENT, // /=
	TOKEN_REMAINDER_ASSIGNMENT, // %=

	TOKEN_EQUALS, // ==
	TOKEN_NOT_EQUALS, // !=
	TOKEN_GREATER_EQUALS, // >=
	TOKEN_LESS_EQUALS, // <=
	TOKEN_LOGIC_AND, // &&
	TOKEN_LOGIC_OR, // ||

	// three char operators

	TOKEN_THREE_CHAR_OPERATORS_START, // pseudo token

	TOKEN_POW_ASSIGNMENT, // **=

	TOKEN_NUMBER,
	TOKEN_WORD,
	TOKEN_STRING,
	
	TOKEN_EOF,

	// for parser (pseudo tokens)
	TOKEN_PARSER_UNARY_ONLY,
	TOKEN_PREINCREMENT, // ++x
	TOKEN_PREDECREMENT, // --x
	TOKEN_POSTINCREMENT, // x++
	TOKEN_POSTDECREMENT, // x--
} tokens_kind_e;

typedef struct token_t token_t;

typedef struct tokens_t tokens_t;

#define SPACES "\f\n\r\v"

#define ALPHABET " +-*/%><?:()=;!~\""

#define ONE_C_OPS "+-*/%><?:()=;!~"
#define TWO_C_OPS "++--**+=-=*=/=%===!=>=<=&&||"
#define THREE_C_OPS "**="

#endif
