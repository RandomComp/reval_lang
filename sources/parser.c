#include "types.h"
#include "lexer.h"
#include "utils.h"
#include "parser.h"

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

// TODO: Переписать для таблицы с операторами, их типа (унарные/бинарные/тернарные) и его приорететом с ассоциативностью (левая или правая)

parser_err_e parse_compare(expression_t **_result, const token_t* tokens, const token_t** endptr) {
	parser_err_e err = 0;

	expression_t* left = nullptr;
	
	err = parse_left(&left, tokens, &tokens);
		
	if (err > 0) {
		free_expresion(left);

		if (endptr) *endptr = tokens;

		return err;
	}

	if (tokens->kind != TOKEN_EQUAL) {
		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return err;
	}

	expression_t* result = calloc(1, sizeof(expression_t));

	result->op = tokens->kind;

	tokens++;
	
	result->left = left;
		
	err = parse_compare(&result->right, tokens, &tokens);
		
	if (err > 0) {
		free_expresion(result);

		if (endptr) *endptr = tokens;

		return err;
	}

	if (endptr) *endptr = tokens;

	if (_result) *_result = result;

	return err;
}

parser_err_e parse_left(expression_t **_result, const token_t* tokens, const token_t** endptr) {
	parser_err_e err = 0;

	expression_t* left = nullptr;
	
	err = parse_mult(&left, tokens, &tokens);
		
	if (err > 0) {
		free_expresion(left);

		if (endptr) *endptr = tokens;

		return err;
	}

	if (tokens->kind != TOKEN_PLUS && 
		tokens->kind != TOKEN_MINUS) {
		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return err;
	}

	expression_t* cur = calloc(1, sizeof(expression_t));

	cur->op = tokens->kind;

	cur->left = left;

	while (tokens->kind == TOKEN_PLUS || tokens->kind == TOKEN_MINUS) {
		tokens_kind_e kind = tokens->kind;

		tokens++;

		err = parse_mult(&cur->right, tokens, &tokens);

		if (err > 0) {
			free_expresion(cur);

			if (endptr) *endptr = tokens;

			return err;
		}

		if (tokens->kind != TOKEN_EOF) {
			kind = tokens->kind;
		}

		expression_t* temp = cur;

		cur = calloc(1, sizeof(expression_t));

		cur->op = kind;

		cur->left = temp;
	}

	printf("cur->left = %p\n\r", cur->left);

	printf("cur->right = %p\n\r", cur->right);

	if (endptr) *endptr = tokens;

	if (_result) *_result = cur;

	return err;
}

parser_err_e parse_mult(expression_t **_result, const token_t* tokens, const token_t** endptr) {
	parser_err_e err = 0;

	expression_t* left = nullptr;
	
	err = parse_unar(&left, tokens, &tokens);
		
	if (err > 0) {
		free_expresion(left);

		if (endptr) *endptr = tokens;

		return err;
	}

	if (tokens->kind != TOKEN_POW &&
		tokens->kind != TOKEN_MULTIPLY &&
		tokens->kind != TOKEN_DIVIDE &&
		tokens->kind != TOKEN_REMAINDER) {
		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return err;
	}

	expression_t* result = calloc(1, sizeof(expression_t));

	result->op = tokens->kind;

	tokens++;
	
	result->left = left;
		
	err = parse_mult(&result->right, tokens, &tokens);
		
	if (err > 0) {
		free_expresion(result);

		if (endptr) *endptr = tokens;

		return err;
	}

	if (result->op == TOKEN_DIVIDE ||
		(result->op == TOKEN_POW &&
		result->left->val == 0)) {
		if (result->right && 
			result->right->op == TOKEN_NUMBER &&
			result->right->val == 0) {
			err = PARSER_ERR_POSSIBLE_UB;
		}
	}

	if (endptr) *endptr = tokens;

	if (_result) *_result = result;

	return err;
}

parser_err_e parse_unar(expression_t **_result, const token_t* tokens, const token_t** endptr) {
	parser_err_e err = 0;

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

		err = parse_compare(&result, tokens, &tokens);

		if (tokens->kind == TOKEN_RIGHT_PARENT) {
			tokens++;
		}

		else err = PARSER_ERR_UNCLOSED_PARENT;
		
		if (err > 0) {
			free_expresion(result);

			if (endptr) *endptr = tokens;

			return err;
		}
	}

	else {
		return PARSER_ERR_INVALID_SYNTAX;
	}

	if (src_tokens->kind == TOKEN_MINUS) {
		result->val = -result->val;
	}

	if (endptr) *endptr = tokens;

	if (_result) *_result = result;

	return err;
}

void show_expresion(expression_t* expr) {
	if (!expr) {
		printf("(null)"); return;
	}

	if (expr->op < TOKEN_PLUS || expr->op > TOKEN_NUMBER) {
		printf("shit %i\n\r", expr->op);

		return;
	}

	printf("(");

	if (expr->op != TOKEN_NUMBER) {
		show_expresion(expr->left);
	}
	
	switch (expr->op) {
		case TOKEN_NUMBER:
			printf("%i", expr->val); break;
		case TOKEN_PLUS:
			printf(" + "); break;
		case TOKEN_MINUS:
			printf(" - "); break;
		case TOKEN_POW:
			printf(" ** "); break;
		case TOKEN_MULTIPLY:
			printf(" * "); break;
		case TOKEN_DIVIDE:
			printf(" / "); break;
		case TOKEN_REMAINDER:
			printf(" %% "); break;
		case TOKEN_EQUAL:
			printf(" == "); break;
		default:
			printf("st: %i %i %i or %i\n\r", expr->left ? expr->left->val : 0, expr->op, expr->right ? expr->right->val : 0, expr->val); break;
	}

	if (expr->op != TOKEN_NUMBER) {
		show_expresion(expr->right);
	}

	printf(")");
}

eval_err_e eval_expresion(int* _result, expression_t* expr) {
	if (!expr) return 0;

	if (expr->op == TOKEN_NUMBER) {
		if (_result) *_result = expr->val;

		return 0;
	}

	int left = 0, right = 0;
	
	eval_err_e err = eval_expresion(&left, expr->left);

	if ((int)err < 0) return err;

	err = MAX(err, eval_expresion(&right, expr->right));

	if ((int)err < 0) return err;

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
				return EVAL_ERR_DIVISION_BY_ZERO;
			}
		
			result = left / right; break;
		case TOKEN_REMAINDER:
			if (right == 0) {
				return EVAL_ERR_DIVISION_BY_ZERO;
			}
		
			result = left % right; break;
		case TOKEN_EQUAL:
			result = left == right; break;
		default:
			err = EVAL_ERR_UNKNOWN_OPERATOR; break;
	}

	if (_result) *_result = result;

	return err;
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

const char* get_parser_err_description(parser_err_e err) {
	switch (err) {
		case PARSER_ERR_POSSIBLE_UB:
			return "undefined behaviour possible";
		case PARSER_ERR_OK:
			return "ok";
		case PARSER_ERR_UNCLOSED_PARENT:
			return "unclosed parent";
		case PARSER_ERR_INVALID_SYNTAX:
			return "invalid syntax";
	}

	return "";
}

const char* get_eval_err_description(eval_err_e err) {
	switch (err) {
		case EVAL_ERR_OK:
			return "ok";
		case EVAL_ERR_DIVISION_BY_ZERO:
			return "division by zero";
		case EVAL_ERR_UNKNOWN_OPERATOR:
			return "unknown operation";
	}

	return "";
}
