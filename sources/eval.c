#include "errors.h"
#include "parser.h"
#include "utils.h"
#include <string.h>

static int x = 0, y = 0, z = 0;

errors_t eval_expresion(errors_t errs, int* _result, expression_t* expr) {
	if (!expr) {
		errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_EXPR_ZERO_PTR, ERROR_LEVEL_ERROR, 0, 0, 0, 0, nullptr, nullptr, nullptr);
		
		return errs;
	}

	bool negative = expr->unary_op == TOKEN_MINUS;
	bool logical_not = expr->unary_op == TOKEN_EXCLAMATION_MARK;
	bool bit_not = expr->unary_op == TOKEN_TILDE;

	int val = 0;

	if (expr->op == TOKEN_NUMBER) {
		val = expr->val;
	}

	else if (expr->op == TOKEN_WORD) {
		if (strcmp(expr->word, "x") == 0) {
			val = x;
		}

		else if (strcmp(expr->word, "y") == 0) {
			val = y;
		}

		else if (strcmp(expr->word, "z") == 0) {
			val = z;
		}
	}

	if (expr->op == TOKEN_NUMBER || expr->op == TOKEN_WORD) {
		if (negative) val = -val;

		else if (logical_not) val = !val;

		else if (bit_not) val = ~val;

		if (_result) *_result = val;

		return errs;
	}

	int left = 0, right = 0, center = 0;
	
	if (expr->left) {
		errs = eval_expresion(errs, &left, expr->left);

		if (errs.level >= ERROR_LEVEL_ERROR) return errs;
	}

	if (expr->op == TOKEN_PARSER_UNARY_ONLY) {
		if (negative) left = -left;

		else if (logical_not) left = !left;

		else if (bit_not) left = ~left;

		if (_result) *_result = left;

		return errs;
	}

	if (expr->op == TOKEN_QUESTION_MARK) {
		errs = eval_expresion(errs, &center, expr->center);

		if (errs.level >= ERROR_LEVEL_ERROR) return errs;
	}

	if (expr->right) {
		errs = eval_expresion(errs, &right, expr->right);

		if (errs.level >= ERROR_LEVEL_ERROR) return errs;
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
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_DIVISION_BY_ZERO, ERROR_LEVEL_ERROR, expr->column, 0, expr->end_column, 0, nullptr, nullptr, "division by zero (%i / %i)", left, right);

				break;
			}
		
			result = left / right; break;
		case TOKEN_REMAINDER:
			if (right == 0) {
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_DIVISION_BY_ZERO, ERROR_LEVEL_ERROR, expr->column, 0, expr->end_column, 0, nullptr, nullptr, "modulo by zero (%i %% %i)", left, right);

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
		case TOKEN_LOGIC_AND:
			result = left && right; break;
		case TOKEN_LOGIC_OR:
			result = left || right; break;
		case TOKEN_GREATER:
			result = left > right; break;
		case TOKEN_LESS:
			result = left < right; break;
		case TOKEN_QUESTION_MARK:
			result = left ? center : right; break;
		case TOKEN_ASSIGNMENT:
			if (strcmp(expr->word, "x") == 0) {
				result = x = right;
			}

			else if (strcmp(expr->word, "y") == 0) {
				result = y = right;
			}

			else if (strcmp(expr->word, "z") == 0) {
				result = z = right;
			}
			break;
		case TOKEN_SEMICOLON:
			result = expr->right ? right : left; break;
		default:
			errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_UNKNOWN_OPERATOR, ERROR_LEVEL_ERROR, expr->column, 0, expr->end_column, 0, nullptr, nullptr, "unknown operator %i", expr->op);

			break;
	}

	if (_result) *_result = result;

	return errs;
}
