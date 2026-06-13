#include "types.h"
#include "lexer.h"
#include "utils.h"
#include "parser.h"

#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// operator_t operators[] = {
// 	{.kind = TOKEN_MINUS, .type = OP_TYPE_UNARY, .precedence = 0}, // zero means high precedence
// 	{ 0 },
// };

static bool token_is_op(tokens_kind_e kind) {
	return kind >= TOKEN_PLUS && kind <= TOKEN_WORD;
}

expression_t* new_expression(interneds_t *filenames, const char *filename, expression_t *left, tokens_kind_e op, expression_t *right) {
	expression_t *result = calloc(1, sizeof(expression_t));

	result->filename = get_interned(filenames, filename);

	result->left = left;
	result->op = op;
	result->right = right;

	return result;
}

// TODO: Переписать для таблицы с операторами, их типа (унарные/бинарные/тернарные) и его приорететом с ассоциативностью (левая или правая)

errors_t parse_semicolon(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr) {
	if (tokens->kind == TOKEN_EOF) {
		return errs;
	}

	expression_t* result = nullptr;

	expression_t* left = nullptr;

	size_t column = tokens->column; size_t row = tokens->row;
	
	errs = parse_assignment(errs, &left, filenames, tokens, &tokens);

	if (!left) goto exit;

	if (tokens->kind != TOKEN_SEMICOLON) {
		result = left;

		goto exit;
	}

	tokens++;

	result = new_expression(filenames, tokens->filename, left, TOKEN_SEMICOLON, nullptr);

	result->column = column; result->row = row;

	errs = parse_semicolon(errs, &result->right, filenames, tokens, &tokens);

	exit:

	if (endptr) *endptr = tokens;

	if (!result) return errs;

	result->end_column = tokens->end_column; result->end_row = tokens->end_row;

	if (_result) {
		*_result = result;
	}

	else {
		free_expresion(result);
	}
	
	return errs;
}

errors_t parse_assignment(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr) {
	if (tokens[1].kind != TOKEN_ASSIGNMENT &&
		tokens[1].kind != TOKEN_PLUS_ASSIGNMENT &&
		tokens[1].kind != TOKEN_MINUS_ASSIGNMENT &&
		tokens[1].kind != TOKEN_MULTIPLY_ASSIGNMENT &&
		tokens[1].kind != TOKEN_DIVIDE_ASSIGNMENT &&
		tokens[1].kind != TOKEN_REMAINDER_ASSIGNMENT &&
		tokens[1].kind != TOKEN_POW_ASSIGNMENT) {
		errs = parse_ternary(errs, _result, filenames, tokens, endptr);

		return errs;
	}

	size_t column = tokens->column; size_t row = tokens->row;
	
	if (tokens[0].kind != TOKEN_WORD) {
		char buf[64] = { 0 };
		get_token_name(buf, 64, tokens[0]);

		errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, tokens->filename, column, row, tokens[1].column, tokens[1].row, nullptr, nullptr, "expected %s got %s", get_token_kind_name(TOKEN_WORD), buf);

		return errs;
	}

	expression_t* result = nullptr;

	const char* var_name = tokens[0].word;

	tokens += 1; // WORD

	tokens_kind_e assigment_type = tokens->kind;

	tokens += 1;

	expression_t* right = nullptr;

	errs = parse_assignment(errs, &right, filenames, tokens, &tokens);

	if (!right) goto exit;

	result = new_expression(filenames, tokens->filename, nullptr, assigment_type, right);

	result->word = get_interned(filenames, var_name);

	result->column = column; result->row = row;

	exit:

	if (endptr) *endptr = tokens;

	if (!result) return errs;
	
	result->end_column = tokens->end_column; result->end_row = tokens->end_row;

	if (_result) {
		*_result = result;
	}

	else {
		free_expresion(result); result = nullptr;
	}

	return errs;
}

errors_t parse_ternary(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr) {
	expression_t* result = nullptr;
	
	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
	
	errs = parse_logical(errs, &left, filenames, tokens, &tokens);

	if (!left) goto exit;
		
	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return errs;
	// }

	if (tokens->kind != TOKEN_QUESTION_MARK) {
		result = left;

		goto exit;
	}

	result = new_expression(filenames, tokens->filename, left, tokens->kind, nullptr);

	result->column = column; result->row = row;

	tokens++;

	errs = parse_logical(errs, &result->center, filenames, tokens, &tokens);

	// if (err > 0) {
	// 	free_expresion(cur);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (tokens->kind != TOKEN_COLON) {
		errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, tokens->filename, tokens->column, tokens->row, tokens->end_column, tokens->end_row, nullptr, nullptr, "expected %s, got %s", get_token_kind_name(TOKEN_COLON), get_token_kind_name(tokens->kind));

		goto exit;
	}

	tokens++;

	errs = parse_ternary(errs, &result->right, filenames, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(cur);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	exit:

	if (endptr) *endptr = tokens;

	if (!result) return errs;

	result->end_column = tokens->end_column; result->end_row = tokens->end_row;

	if (_result) {
		*_result = result;
	}

	else {
		free_expresion(result); result = nullptr;
	}

	return errs;
}

errors_t parse_logical(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr) {
	expression_t* result = nullptr;
	
	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
		
	errs = parse_compare(errs, &left, filenames, tokens, &tokens);

	if (!left) goto exit;
		
	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return errs;
	// }

	if (tokens->kind != TOKEN_LOGIC_AND &&
		tokens->kind != TOKEN_LOGIC_OR) {
		result = left;
		
		goto exit;
	}

	result = new_expression(filenames, tokens->filename, left, tokens->kind, nullptr);

	result->column = column; result->row = row;

	tokens++;
		
	errs = parse_logical(errs, &result->right, filenames, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(result);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	exit:

	if (endptr) *endptr = tokens;

	if (!result) return errs;

	result->end_column = tokens->end_column; result->end_row = tokens->end_row;

	if (_result) {
		*_result = result;
	}

	else {
		free_expresion(result); result = nullptr;
	}

	return errs;
}

errors_t parse_compare(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr) {
	expression_t* result = nullptr;

	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
		
	errs = parse_add(errs, &left, filenames, tokens, &tokens);

	if (!left) goto exit;
		
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
		result = left;

		goto exit;
	}

	result = new_expression(filenames, tokens->filename, left, tokens->kind, nullptr);

	result->column = column; result->row = row;

	tokens++;
		
	errs = parse_compare(errs, &result->right, filenames, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(result);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	exit:

	if (endptr) *endptr = tokens;

	if (!result) return errs;

	result->end_column = tokens->end_column; result->end_row = tokens->end_row;

	if (_result) {
		*_result = result;
	}

	else {
		free_expresion(result); result = nullptr;
	}

	return errs;
}

errors_t parse_add(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr) {
	expression_t* cur = nullptr;

	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
		
	errs = parse_mult(errs, &left, filenames, tokens, &tokens);

	if (!left) goto exit;
		
	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (tokens->kind != TOKEN_PLUS && 
		tokens->kind != TOKEN_MINUS) {
		cur = left;

		goto exit;
	}

	cur = new_expression(filenames, tokens->filename, left, tokens->kind, nullptr);

	cur->column = column; cur->row = row;

	while (tokens->kind == TOKEN_PLUS || tokens->kind == TOKEN_MINUS) {
		tokens++;

		errs = parse_mult(errs, &cur->right, filenames, tokens, &tokens);

		// if (err > 0) {
		// 	free_expresion(cur);

		// 	if (endptr) *endptr = tokens;

		// 	return err;
		// }

		if (tokens->kind != TOKEN_EOF && 
			(tokens->kind == TOKEN_PLUS || tokens->kind == TOKEN_MINUS)) {
			cur = new_expression(filenames, tokens->filename, cur, tokens->kind, nullptr);
		}
	}

	exit:

	if (endptr) *endptr = tokens;

	if (!cur) return errs;

	cur->end_column = tokens->end_column; cur->end_row = tokens->end_row;

	if (_result) {
		*_result = cur;
	}

	else {
		free_expresion(cur); cur = nullptr;
	}

	return errs;
}

errors_t parse_mult(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr) {
	expression_t* cur = nullptr;

	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
		
	errs = parse_pow(errs, &left, filenames, tokens, &tokens);

	if (!left) goto exit;

	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (tokens->kind != TOKEN_MULTIPLY &&
		tokens->kind != TOKEN_DIVIDE &&
		tokens->kind != TOKEN_REMAINDER) {
		cur = left;

		goto exit;
	}

	cur = new_expression(filenames, tokens->filename, left, tokens->kind, nullptr);

	cur->column = column; cur->row = row;
		
	while (tokens->kind == TOKEN_MULTIPLY ||
		tokens->kind == TOKEN_DIVIDE ||
		tokens->kind == TOKEN_REMAINDER) {
		tokens++;

		errs = parse_pow(errs, &cur->right, filenames, tokens, &tokens);

		if (!cur->right) goto exit;

		if (tokens->kind != TOKEN_EOF && 
			(tokens->kind == TOKEN_MULTIPLY ||
			tokens->kind == TOKEN_DIVIDE ||
			tokens->kind == TOKEN_REMAINDER)) {
			cur = new_expression(filenames, tokens->filename, cur, tokens->kind, nullptr);
		}
	}

	if (cur->op == TOKEN_DIVIDE ||
		cur->op == TOKEN_REMAINDER) {
		if (cur->right && 
			cur->right->op == TOKEN_NUMBER &&
			cur->right->val == 0) {

			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_POSSIBLE_UB, ERROR_LEVEL_WARN, tokens->filename, column, row, tokens->end_column, tokens->end_row, nullptr, nullptr, "%s by zero is ambigious", cur->op == TOKEN_DIVIDE ? "division" : "modulo");
		}
	}

	exit:
	
	if (endptr) *endptr = tokens;

	if (!cur) return errs;
	
	cur->end_column = tokens->end_column; cur->end_row = tokens->end_row;

	if (_result) {
		*_result = cur;
	}

	else {
		free_expresion(cur); cur = nullptr;
	}

	return errs;
}

errors_t parse_pow(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr) {
	expression_t* result = nullptr;

	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
		
	errs = parse_unar(errs, &left, filenames, tokens, &tokens);

	if (!left) goto exit;

	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (tokens->kind != TOKEN_POW) {
		result = left;

		goto exit;
	}
	
	result = new_expression(filenames, tokens->filename, left, TOKEN_POW, nullptr);

	result->column = column; result->row = row;

	tokens++;
		
	errs = parse_pow(errs, &result->right, filenames, tokens, &tokens);
	
	// if (err > 0) {
	// 	free_expresion(result);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (result->op == TOKEN_POW && 
		result->left && 
		result->left->op == TOKEN_NUMBER && result->left->val == 0) {
		if (result->right && 
			result->right->op == TOKEN_NUMBER &&
			result->right->val == 0) {
			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_POSSIBLE_UB, ERROR_LEVEL_WARN, tokens->filename, column, row, tokens->end_column, tokens->row  - row, nullptr, nullptr, "0 ** 0 is ambigious (any number in 0 power equals 1, but zero in any number power equals 0)");
		}
	}

	exit:

	if (endptr) *endptr = tokens;

	if (!result) return errs;
	
	result->end_column = tokens->end_column; result->end_row = tokens->end_row;

	if (_result) {
		*_result = result;
	}

	else {
		free_expresion(result); result = nullptr;
	}

	return errs;
}

errors_t parse_unar(errors_t errs, expression_t **_result, interneds_t *filenames, const token_t* tokens, const token_t** endptr) {
	if (tokens->kind == TOKEN_EOF) {
		errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_EOF, ERROR_LEVEL_ERROR, tokens->filename, tokens->column, tokens->row, tokens->end_column, tokens->end_row, nullptr, nullptr, "expected expression or number, got EOF");

		return errs;
	}

	expression_t* result = nullptr;

	size_t column = tokens->column, row = tokens->row;

	bool increment = false, decrement = false;

	// в оптимизаторе когда expr->unary_op == TOKEN_PREINCREMENT/TOKEN_POSTINCREMENT и в expr->right есть переменная из expr->left выдавать предупреждение

	if (tokens->kind == TOKEN_INCREMENT) {
		tokens++;

		increment = true;
	}

	else if (tokens->kind == TOKEN_DECREMENT) {
		tokens++;

		decrement = true;
	}

	if (tokens->kind == TOKEN_NUMBER) {
		result = new_expression(filenames, tokens->filename, nullptr, TOKEN_NUMBER, nullptr);
	
		result->column = column; result->row = row;

		result->val = tokens->val;

		tokens++;

		if (increment) {
			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INCOMPATIBLE_OPERATOR_TYPE, ERROR_LEVEL_ERROR, tokens->filename, column, row, tokens->column, tokens->row, nullptr, nullptr, "incorrect combination of operator (++) and operand");
		}

		else if (decrement) {
			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INCOMPATIBLE_OPERATOR_TYPE, ERROR_LEVEL_ERROR, tokens->filename, column, row, tokens->column, tokens->row, nullptr, nullptr, "incorrect combination of operator (--) and operand");
		}
		
		if (tokens->kind == TOKEN_INCREMENT) {
			tokens++;

			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INCOMPATIBLE_OPERATOR_TYPE, ERROR_LEVEL_ERROR, tokens->filename, column, row, tokens->column, tokens->row, nullptr, nullptr, "incorrect combination of operator (++) and operand");
		}

		else if (tokens->kind == TOKEN_DECREMENT) {
			tokens++;

			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INCOMPATIBLE_OPERATOR_TYPE, ERROR_LEVEL_ERROR, tokens->filename, column, row, tokens->column, tokens->row, nullptr, nullptr, "incorrect combination of operator (--) and operand");
		}
	}

	else if (tokens->kind == TOKEN_WORD) {
		result = new_expression(filenames, tokens->filename, nullptr, TOKEN_WORD, nullptr);
	
		result->column = column; result->row = row;

		result->word = get_interned(filenames, tokens->word);

		tokens++;

		if (increment) {
			result->unary_op = TOKEN_PREINCREMENT;
		}

		else if (decrement) {
			result->unary_op = TOKEN_PREDECREMENT;
		}
		
		if (tokens->kind == TOKEN_INCREMENT) {
			tokens++;

			result->unary_op = TOKEN_POSTINCREMENT;
		}

		else if (tokens->kind == TOKEN_DECREMENT) {
			tokens++;

			result->unary_op = TOKEN_POSTDECREMENT;
		}
	}

	else if (tokens->kind == TOKEN_LEFT_PARENT) {
		tokens++;

		errs = parse_semicolon(errs, &result, filenames, tokens, &tokens);

		if (result) {
			result->column = column; result->row = row;
		}

		if (tokens->kind == TOKEN_RIGHT_PARENT) {
			tokens++;
		}

		else {
			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_UNCLOSED_PARENT, ERROR_LEVEL_ERROR, tokens->filename, tokens->column, tokens->row, 1, 0, ")", nullptr, "'(' wasn't closed here");
		}
		
		// if (err > 0) {
		// 	free_expresion(result);

		// 	if (endptr) *endptr = tokens;

		// 	return errs;
		// }
	}

	else if (tokens->kind == TOKEN_PLUS) {
		tokens++;
		
		errs = parse_unar(errs, &result, filenames, tokens, &tokens);
	}

	else if (tokens->kind == TOKEN_MINUS ||
		tokens->kind == TOKEN_EXCLAMATION_MARK ||
		tokens->kind == TOKEN_TILDE) {
		result = new_expression(filenames, tokens->filename, nullptr, TOKEN_PARSER_UNARY_ONLY, nullptr);
	
		result->column = column; result->row = row;

		result->unary_op = tokens->kind;

		tokens++;

		errs = parse_unar(errs, &result->left, filenames, tokens, &tokens);
	}

	else {
		if (endptr) *endptr = tokens;

		char buf[64] = { 0 };
		get_token_name(buf, 64, *tokens);

		errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, tokens->filename, tokens->column, tokens->row, tokens->end_column, tokens->end_row, nullptr, nullptr, "expected expression or number, got %s", buf);

		goto exit;
	}

	exit:

	if (endptr) *endptr = tokens;

	if (!result) return errs;

	result->end_column = tokens->end_column; result->end_row = tokens->end_row;

	if (_result) {
		*_result = result;
	}

	else {
		free_expresion(result); result = nullptr;
	}

	return errs;
}

ssize_t view_expresion(char* _buf, ssize_t buf_size, expression_t* expr) {
	if (_buf && buf_size < 1) return 0;

	char* buf = _buf;

	ssize_t index = 0;

	if (!expr) {
		index += snprintf(buf, buf_size, "(null)");
		
		return index;
	}

	if (!token_is_op(expr->op) && expr->op != TOKEN_PARSER_UNARY_ONLY) {
		printf("shit %i\n\r", expr->op);

		return 0;
	}

	if (buf && index < (buf_size - 1)) buf[index] = '(';

	index++;

	typedef struct unary_op_t {
		tokens_kind_e op; const char* text;
	} unary_op_t;

	const unary_op_t pre_unary_ops[] = {
		{TOKEN_MINUS, "-"},
		{TOKEN_EXCLAMATION_MARK, "!"},
		{TOKEN_TILDE, "~"},
		{TOKEN_PREINCREMENT, "++"},
		{TOKEN_PREDECREMENT, "--"},
		{0, 0}
	};

	const unary_op_t post_unary_ops[] = {
		{TOKEN_POSTINCREMENT, "++"},
		{TOKEN_POSTDECREMENT, "--"},
		{0, 0}
	};

	for (size_t i = 0; pre_unary_ops[i].op != 0 && pre_unary_ops[i].text != 0; i++) {
		if (expr->unary_op == pre_unary_ops[i].op) {
			index += snprintf(_buf ? (buf + index) : 0, _buf ? (buf_size - index) : 0, "%s", pre_unary_ops[i].text);

			break;
		}
	}

	if (expr->op == TOKEN_NUMBER) {
		index += snprintf(_buf ? (buf + index) : 0, _buf ? (buf_size - index) : 0, "%i", expr->val);
	}

	else if (expr->op == TOKEN_WORD ||
			expr->op == TOKEN_ASSIGNMENT ||
			expr->op == TOKEN_PLUS_ASSIGNMENT ||
			expr->op == TOKEN_MINUS_ASSIGNMENT ||
			expr->op == TOKEN_MULTIPLY_ASSIGNMENT ||
			expr->op == TOKEN_DIVIDE_ASSIGNMENT ||
			expr->op == TOKEN_REMAINDER_ASSIGNMENT) {
		index += snprintf(_buf ? (buf + index) : 0, _buf ? (buf_size - index) : 0, "%s", expr->word);
	}

	else {
		index += view_expresion(_buf ? (buf + index) : 0, _buf ? (buf_size - index) : 0, expr->left);
	}
	
	if (expr->right) {
		switch (expr->op) {
			case TOKEN_PLUS:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " + "); break;
			case TOKEN_MINUS:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " - "); break;
			case TOKEN_POW:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " ** "); break;
			case TOKEN_PLUS_ASSIGNMENT:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " += "); break;
			case TOKEN_MINUS_ASSIGNMENT:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " -= "); break;
			case TOKEN_MULTIPLY_ASSIGNMENT:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " *= "); break;
			case TOKEN_DIVIDE_ASSIGNMENT:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " /= "); break;
			case TOKEN_REMAINDER_ASSIGNMENT:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " %%= "); break;
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
			case TOKEN_LOGIC_AND:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " && "); break;
			case TOKEN_LOGIC_OR:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " || "); break;
			case TOKEN_GREATER:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " > "); break;
			case TOKEN_LESS:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " < "); break;
			case TOKEN_QUESTION_MARK:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " ? "); break;
			case TOKEN_ASSIGNMENT:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " = "); break;
			case TOKEN_SEMICOLON:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, "; "); break;
			default:
				index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, "st: %i %i %i or %i\n\r", expr->left ? expr->left->val : 0, expr->op, expr->right ? expr->right->val : 0, expr->val); break;
		}
	}

	if (expr->op != TOKEN_NUMBER &&
		expr->op != TOKEN_WORD) {
		if (expr->op == TOKEN_QUESTION_MARK) {
			index += view_expresion(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, expr->center);

			index += snprintf(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, " : ");

			index += view_expresion(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, expr->right);
		}

		else if (expr->right) {
			index += view_expresion(_buf ? (buf + index) : nullptr, _buf ? (buf_size - index) : 0, expr->right);
		}
	}

	for (size_t i = 0; post_unary_ops[i].op != 0 && post_unary_ops[i].text != 0; i++) {
		if (expr->unary_op == post_unary_ops[i].op) {
			index += snprintf(_buf ? (buf + index) : 0, _buf ? (buf_size - index) : 0, "%s", post_unary_ops[i].text);

			break;
		}
	}

	if (buf && index < (buf_size - 1)) buf[index] = ')';

	index++;

	if (buf && index < (buf_size - 1)) buf[index] = 0;
		
	return index;
}

void free_expresion(expression_t* expr) {
	if (!expr) return;

	if (expr->op != TOKEN_UNEXPECTED &&
		expr->op != TOKEN_UNDEFINED &&
		expr->op != TOKEN_NUMBER) {
		free_expresion(expr->left); expr->left = nullptr;

		free_expresion(expr->center); expr->center = nullptr;

		free_expresion(expr->right); expr->right = nullptr;

		expr->word = nullptr;

		expr->filename = nullptr;
	}

	expr->op = TOKEN_UNDEFINED;

	free(expr);
}
