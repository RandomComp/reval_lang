#include "types.h"
#include "lexer.h"
#include "utils.h"
#include "parser.h"

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

// operator_t operators[] = {
// 	{.kind = TOKEN_MINUS, .type = OP_TYPE_UNARY, .precedence = 0}, // zero means high precedence
// 	{ 0 },
// };

static bool token_is_op(tokens_kind_e kind) {
	return kind >= TOKEN_PLUS && kind <= TOKEN_NUMBER;
}

// TODO: Переписать для таблицы с операторами, их типа (унарные/бинарные/тернарные) и его приорететом с ассоциативностью (левая или правая)

errors_t parse_ternary(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	expression_t* left = nullptr;
	
	errs = parse_compare(errs, &left, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return errs;
	// }

	if (tokens->kind != TOKEN_QUESTION_MARK) {
		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return errs;
	}

	expression_t* cur = calloc(1, sizeof(expression_t));

	cur->op = tokens->kind;

	cur->left = left;

	tokens++;

	errs = parse_compare(errs, &cur->center, tokens, &tokens);

	// if (err > 0) {
	// 	free_expresion(cur);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (tokens->kind != TOKEN_COLON) {
		errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, tokens->column, tokens->row, tokens->end_column, tokens->end_row, "expected %s, got %s", get_token_kind_name(TOKEN_QUESTION_MARK), get_token_kind_name(tokens->kind));

		return errs;
	}

	tokens++;

	errs = parse_ternary(errs, &cur->right, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(cur);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (endptr) *endptr = tokens;

	if (_result) *_result = cur;

	return errs;
}

errors_t parse_compare(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	expression_t* left = nullptr;
	
	errs = parse_add(errs, &left, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return errs;
	// }

	if (tokens->kind != TOKEN_EQUALS &&
		tokens->kind != TOKEN_NOT_EQUALS &&
		tokens->kind != TOKEN_GREATER_EQUALS &&
		tokens->kind != TOKEN_LESS_EQUALS &&
		tokens->kind != TOKEN_GREATER &&
		tokens->kind != TOKEN_LESS) {
		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return errs;
	}

	expression_t* result = calloc(1, sizeof(expression_t));

	result->op = tokens->kind;

	tokens++;
	
	result->left = left;
		
	errs = parse_compare(errs, &result->right, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(result);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (endptr) *endptr = tokens;

	if (_result) *_result = result;

	return errs;
}

errors_t parse_add(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	expression_t* left = nullptr;
	
	errs = parse_mult(errs, &left, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (tokens->kind != TOKEN_PLUS && 
		tokens->kind != TOKEN_MINUS) {
		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return errs;
	}

	expression_t* cur = calloc(1, sizeof(expression_t));

	cur->op = tokens->kind;

	cur->left = left;

	while (tokens->kind == TOKEN_PLUS || tokens->kind == TOKEN_MINUS) {
		tokens++;

		errs = parse_mult(errs, &cur->right, tokens, &tokens);

		// if (err > 0) {
		// 	free_expresion(cur);

		// 	if (endptr) *endptr = tokens;

		// 	return err;
		// }

		if (tokens->kind != TOKEN_EOF && 
			(tokens->kind == TOKEN_PLUS || tokens->kind == TOKEN_MINUS)) {
			expression_t* temp = cur;

			cur = calloc(1, sizeof(expression_t));

			cur->op = tokens->kind;

			cur->left = temp;
		}
	}

	if (endptr) *endptr = tokens;

	if (_result) *_result = cur;

	return errs;
}

errors_t parse_mult(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	expression_t* left = nullptr;
	
	errs = parse_unar(errs, &left, tokens, &tokens);

	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (tokens->kind != TOKEN_POW &&
		tokens->kind != TOKEN_MULTIPLY &&
		tokens->kind != TOKEN_DIVIDE &&
		tokens->kind != TOKEN_REMAINDER) {
		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return errs;
	}

	expression_t* result = calloc(1, sizeof(expression_t));

	result->op = tokens->kind;

	tokens++;
	
	result->left = left;
		
	errs = parse_mult(errs, &result->right, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(result);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (result->op == TOKEN_DIVIDE) {
		if (result->right && 
			result->right->op == TOKEN_NUMBER &&
			result->right->val == 0) {

			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_POSSIBLE_UB, ERROR_LEVEL_WARN, tokens->column, tokens->row, tokens->end_column, tokens->end_row, "zero division is ambigious");
		}
	}

	if (result->op == TOKEN_POW && result->left->val == 0) {
		if (result->right && 
			result->right->op == TOKEN_NUMBER &&
			result->right->val == 0) {
			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_POSSIBLE_UB, ERROR_LEVEL_WARN, tokens->column, tokens->row, tokens->end_column, tokens->end_row, "0 ** 0 is ambigious (any number in 0 power equals 1, but zero in any number power equals 0)");
		}
	}

	if (endptr) *endptr = tokens;

	if (_result) *_result = result;

	return errs;
}

errors_t parse_unar(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	if (tokens->kind == TOKEN_EOF) {
		errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_EOF, ERROR_LEVEL_ERROR, tokens->column, tokens->row, tokens->end_column, tokens->end_row, "expected expression or number, got EOF");

		return errs;
	}

	expression_t* result = nullptr;

	const token_t* src_tokens = tokens;

	if (tokens->kind == TOKEN_PLUS) {
		tokens++;
	}

	if (tokens->kind == TOKEN_MINUS) {
		tokens++;
	}

	if (tokens->kind == TOKEN_NUMBER) {
		result = calloc(1, sizeof(expression_t));

		result->val = tokens->val;
		result->op = TOKEN_NUMBER;

		tokens++;
	}

	else if (tokens->kind == TOKEN_LEFT_PARENT) {
		tokens++;

		errs = parse_ternary(errs, &result, tokens, &tokens);

		if (tokens->kind == TOKEN_RIGHT_PARENT) {
			tokens++;
		}

		else {
			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_UNCLOSED_PARENT, ERROR_LEVEL_ERROR, tokens->column, tokens->row, tokens->end_column, tokens->end_row, "'(' wasn't closed here");
		}
		
		// if (err > 0) {
		// 	free_expresion(result);

		// 	if (endptr) *endptr = tokens;

		// 	return errs;
		// }
	}

	else {
		if (endptr) *endptr = tokens;

		char buf[64] = { 0 };
		get_token_name(buf, 64, *tokens);

		errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, tokens->column, tokens->row, tokens->end_column, tokens->end_row, "expected expression or number, got %s", buf);

		return errs;
	}

	if (src_tokens->kind == TOKEN_MINUS) {
		result->unary_op = TOKEN_MINUS;
	}

	if (endptr) *endptr = tokens;

	if (_result) *_result = result;

	return errs;
}

size_t view_expresion(char* _buf, size_t buf_size, expression_t* expr) {
	char* buf = _buf;

	size_t index = 0;

	if (!expr) {
		index += snprintf(buf, buf_size, "(null)");
		
		return index;
	}

	if (!token_is_op(expr->op)) {
		printf("shit %i\n\r", expr->op);

		return 0;
	}

	if (buf) buf[index] = '(';

	index++;

	if (expr->unary_op == TOKEN_MINUS) {
		if (buf) buf[index] = '-';

		index++;
	}

	if (expr->op != TOKEN_NUMBER) {
		index += view_expresion(_buf ? (buf + index) : 0, _buf ? (buf_size - index) : 0, expr->left);
	}

	if (expr->op == TOKEN_NUMBER) {
		index += snprintf(_buf ? (buf + index) : 0, _buf ? (buf_size - index) : 0, "%i", expr->val);
	}
	
	if (expr->right) {
		switch (expr->op) {
			case TOKEN_PLUS:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " + "); break;
			case TOKEN_MINUS:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " - "); break;
			case TOKEN_POW:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " ** "); break;
			case TOKEN_MULTIPLY:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " * "); break;
			case TOKEN_DIVIDE:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " / "); break;
			case TOKEN_REMAINDER:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " %% "); break;
			case TOKEN_EQUALS:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " == "); break;
			case TOKEN_NOT_EQUALS:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " != "); break;
			case TOKEN_GREATER_EQUALS:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " >= "); break;
			case TOKEN_LESS_EQUALS:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " <= "); break;
			case TOKEN_GREATER:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " > "); break;
			case TOKEN_LESS:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " < "); break;
			case TOKEN_QUESTION_MARK:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " ? "); break;
			default:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, "st: %i %i %i or %i\n\r", expr->left ? expr->left->val : 0, expr->op, expr->right ? expr->right->val : 0, expr->val); break;
		}
	}

	if (expr->op != TOKEN_NUMBER) {
		if (expr->op == TOKEN_QUESTION_MARK) {
			index += view_expresion(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, expr->center);

			index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " : ");

			index += view_expresion(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, expr->right);
		}

		else if (expr->right) {
			index += view_expresion(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, expr->right);
		}
	}

	if (buf) buf[index] = ')';

	index++;
		
	return index;
}

errors_t eval_expresion(errors_t errs, int* _result, expression_t* expr) {
	if (!expr) {
		emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_EXPR_ZERO_PTR, ERROR_LEVEL_ERROR, 0, 0, 1, 0, nullptr);
		
		return errs;
	}

	bool negative = expr->unary_op == TOKEN_MINUS;

	if (expr->op == TOKEN_NUMBER) {
		if (_result) *_result = negative ? -expr->val : expr->val;

		return errs;
	}

	int left = 0, right = 0, center = 0;
	
	errs = eval_expresion(errs, &left, expr->left);

	if (expr->op == TOKEN_QUESTION_MARK) {
		errs = eval_expresion(errs, &center, expr->center);
	}

	if (expr->right) {
		errs = eval_expresion(errs, &right, expr->right);
	}

	int result = 0;

	switch (expr->op) {
		case TOKEN_PLUS:
			result = left + right; break;
		case TOKEN_MINUS:
			result = left - right; break;
		case TOKEN_POW:
			result = powi(left, right); break;
		case TOKEN_MULTIPLY:
			result = left * right; break;
		case TOKEN_DIVIDE:
			if (right == 0) {
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_DIVISION_BY_ZERO, ERROR_LEVEL_ERROR, 0, 0, 1, 0, "division by zero (%i / %i)", left, right);

				break;
			}
		
			result = left / right; break;
		case TOKEN_REMAINDER:
			if (right == 0) {
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_DIVISION_BY_ZERO, ERROR_LEVEL_ERROR, 0, 0, 1, 0, "division by zero (%i / %i)", left, right);

				break;
			}
		
			result = left % right; break;
		case TOKEN_EQUALS:
			result = left == right; break;
		case TOKEN_NOT_EQUALS:
			result = left != right; break;
		case TOKEN_GREATER_EQUALS:
			result = left >= right; break;
		case TOKEN_LESS_EQUALS:
			result = left <= right; break;
		case TOKEN_GREATER:
			result = left > right; break;
		case TOKEN_LESS:
			result = left < right; break;
		case TOKEN_QUESTION_MARK:
			result = left ? center : right; break;
		default:
			emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_UNKNOWN_OPERATOR, ERROR_LEVEL_ERROR, 0, 0, 1, 0, "unknown operator %i", expr->op);

			break;
	}

	if (_result) *_result = result;

	return errs;
}

void free_expresion(expression_t* expr) {
	if (!expr) return;

	if (expr->op != TOKEN_UNDEFINED &&
		expr->op != TOKEN_NUMBER) {
		free_expresion(expr->left); expr->left = nullptr;

		free_expresion(expr->right); expr->right = nullptr;
	}

	expr->op = TOKEN_UNDEFINED;

	free(expr);
}
