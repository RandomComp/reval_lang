#ifndef REVAL_PARSER_FWD_H
#define REVAL_PARSER_FWD_H

typedef enum expression_op_e {
	EXPRESSION_OP_UNDEFINED,
	EXPRESSION_OP_PLUS,
	EXPRESSION_OP_CONST
} expression_op_e;

typedef struct expression_t expression_t;

#endif
