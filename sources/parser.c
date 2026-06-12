#include "types.h"
#include "lexer.h"
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

// TODO: Переписать для таблицы с операторами, их типа (унарные/бинарные/тернарные) и его приорететом с ассоциативностью (левая или правая)

errors_t parse_semicolon(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	if (tokens->kind == TOKEN_EOF) {
		if (endptr) *endptr = tokens;
		
		return errs;
	}

	expression_t* left = nullptr;

	size_t column = tokens->column; size_t row = tokens->row;
	
	errs = parse_assignment(errs, &left, tokens, &tokens);

	if (!left) return errs;

	if (tokens->kind != TOKEN_SEMICOLON) {
		left->column = column; left->row = row;
		left->end_column = tokens->end_column; left->end_row = tokens->end_row;
		
		if (endptr) *endptr = tokens;
		
		if (_result) *_result = left;
		
		return errs;
	}

	tokens++;

	expression_t* result = calloc(1, sizeof(expression_t));

	result->column = column; result->row = row;
	result->op = TOKEN_SEMICOLON;
	result->left = left;

	errs = parse_semicolon(errs, &result->right, tokens, &tokens);

	result->end_column = tokens->end_column; result->end_row = tokens->end_row;

	if (endptr) *endptr = tokens;
	
	if (_result) *_result = result;
	
	return errs;
}

errors_t parse_assignment(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	if (tokens[1].kind != TOKEN_ASSIGNMENT) {
		return parse_ternary(errs, _result, tokens, endptr);
	}

	if (tokens[0].kind != TOKEN_WORD) {
		char buf[64] = { 0 };
		get_token_name(buf, 64, tokens[0]);

		return emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, tokens[0].column, tokens[0].row, tokens[1].column, tokens[1].row, nullptr, nullptr, "expected %s got %s", get_token_kind_name(TOKEN_WORD), buf);
	}

	const char* var_name = tokens[0].word;

	tokens += 2; // WORD + ASSIGNMENT

	expression_t* right = nullptr;

	errs = parse_assignment(errs, &right, tokens, &tokens);

	if (!right) {
		goto exit;
	}

	expression_t* result = calloc(1, sizeof(expression_t));

	result->word = strdup(var_name);
	result->op = TOKEN_ASSIGNMENT;
	result->right = right;
	
	if (_result) *_result = result;

	exit:

	if (endptr) *endptr = tokens;
	
	return errs;
}

errors_t parse_ternary(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
	
	errs = parse_logical(errs, &left, tokens, &tokens);

	if (!left) return errs;
		
	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return errs;
	// }

	if (tokens->kind != TOKEN_QUESTION_MARK) {
		left->column = column; left->row = row;
		left->end_column = tokens->end_column; left->end_row = tokens->end_row;
		
		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return errs;
	}

	expression_t* result = calloc(1, sizeof(expression_t));

	result->column = column; result->row = row;

	result->op = tokens->kind;

	result->left = left;

	tokens++;

	errs = parse_logical(errs, &result->center, tokens, &tokens);

	// if (err > 0) {
	// 	free_expresion(cur);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (tokens->kind != TOKEN_COLON) {
		errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, tokens->column, tokens->row, tokens->end_column, tokens->end_row, nullptr, nullptr, "expected %s, got %s", get_token_kind_name(TOKEN_COLON), get_token_kind_name(tokens->kind));

		return errs;
	}

	tokens++;

	errs = parse_ternary(errs, &result->right, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(cur);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	result->end_column = tokens->end_column; result->end_row = tokens->end_row;

	if (endptr) *endptr = tokens;

	if (_result) *_result = result;

	return errs;
}

errors_t parse_logical(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
		
	errs = parse_compare(errs, &left, tokens, &tokens);

	if (!left) return errs;
		
	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return errs;
	// }

	if (tokens->kind != TOKEN_LOGIC_AND &&
		tokens->kind != TOKEN_LOGIC_OR) {
		left->column = column; left->row = row;
		left->end_column = tokens->end_column; left->end_row = tokens->end_row;

		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return errs;
	}

	expression_t* result = calloc(1, sizeof(expression_t));

	result->column = column; result->row = row;

	result->op = tokens->kind;

	tokens++;
	
	result->left = left;
		
	errs = parse_logical(errs, &result->right, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(result);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	result->end_column = tokens->end_column; result->end_row = tokens->end_row;

	if (endptr) *endptr = tokens;

	if (_result) *_result = result;

	return errs;
}

errors_t parse_compare(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
		
	errs = parse_add(errs, &left, tokens, &tokens);

	if (!left) return errs;
		
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
		left->column = column; left->row = row;
		left->end_column = tokens->end_column; left->end_row = tokens->end_row;

		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return errs;
	}

	expression_t* result = calloc(1, sizeof(expression_t));

	result->column = column; result->row = row;

	result->op = tokens->kind;

	tokens++;
	
	result->left = left;
		
	errs = parse_compare(errs, &result->right, tokens, &tokens);
		
	// if (err > 0) {
	// 	free_expresion(result);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	result->end_column = tokens->end_column; result->end_row = tokens->end_row;

	if (endptr) *endptr = tokens;

	if (_result) *_result = result;

	return errs;
}

errors_t parse_add(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
		
	errs = parse_mult(errs, &left, tokens, &tokens);

	if (!left) return errs;
		
	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (tokens->kind != TOKEN_PLUS && 
		tokens->kind != TOKEN_MINUS) {
		left->column = column; left->row = row;
		left->end_column = tokens->end_column; left->end_row = tokens->end_row;

		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return errs;
	}

	expression_t* cur = calloc(1, sizeof(expression_t));

	cur->column = column; cur->row = row;

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

	cur->end_column = tokens->end_column; cur->end_row = tokens->end_row;

	if (endptr) *endptr = tokens;

	if (_result) *_result = cur;

	return errs;
}

errors_t parse_mult(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
		
	errs = parse_pow(errs, &left, tokens, &tokens);

	if (!left) return errs;

	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (tokens->kind != TOKEN_MULTIPLY &&
		tokens->kind != TOKEN_DIVIDE &&
		tokens->kind != TOKEN_REMAINDER) {
		left->column = column; left->row = row;
		left->end_column = tokens->end_column; left->end_row = tokens->end_row;

		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return errs;
	}

	expression_t* cur = calloc(1, sizeof(expression_t));

	cur->column = column; cur->row = row;

	cur->op = tokens->kind;
	
	cur->left = left;
		
	while (tokens->kind == TOKEN_MULTIPLY ||
		tokens->kind == TOKEN_DIVIDE ||
		tokens->kind == TOKEN_REMAINDER) {
		tokens++;

		errs = parse_pow(errs, &cur->right, tokens, &tokens);

		if (!cur->right) return errs;

		if (tokens->kind != TOKEN_EOF && 
			(tokens->kind == TOKEN_MULTIPLY ||
			tokens->kind == TOKEN_DIVIDE ||
			tokens->kind == TOKEN_REMAINDER)) {
			expression_t* temp = cur;

			cur = calloc(1, sizeof(expression_t));

			cur->op = tokens->kind;

			cur->left = temp;
		}
	}

	if (cur->op == TOKEN_DIVIDE ||
		cur->op == TOKEN_REMAINDER) {
		if (cur->right && 
			cur->right->op == TOKEN_NUMBER &&
			cur->right->val == 0) {

			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_POSSIBLE_UB, ERROR_LEVEL_WARN, column, row, tokens->column - column, tokens->row - row, nullptr, nullptr, "%s by zero is ambigious", cur->op == TOKEN_DIVIDE ? "division" : "modulo");
		}
	}
	
	cur->end_column = tokens->column - column; cur->end_row = tokens->row - row;

	if (endptr) *endptr = tokens;

	if (_result) *_result = cur;

	return errs;
}

errors_t parse_pow(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	expression_t* left = nullptr;
	
	size_t column = tokens->column; size_t row = tokens->row;
		
	errs = parse_unar(errs, &left, tokens, &tokens);

	if (!left) return errs;

	// if (err > 0) {
	// 	free_expresion(left);

	// 	if (endptr) *endptr = tokens;

	// 	return err;
	// }

	if (tokens->kind != TOKEN_POW) {
		left->column = column; left->row = row;
		left->end_column = tokens->end_column; left->end_row = tokens->end_row;

		if (endptr) *endptr = tokens;

		if (_result) *_result = left;

		return errs;
	}

	expression_t* result = calloc(1, sizeof(expression_t));

	result->column = column; result->row = row;

	result->op = TOKEN_POW;

	tokens++;
	
	result->left = left;
		
	errs = parse_pow(errs, &result->right, tokens, &tokens);
	
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
			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_POSSIBLE_UB, ERROR_LEVEL_WARN, column, row, tokens->column - column, tokens->row  - row, nullptr, nullptr, "0 ** 0 is ambigious (any number in 0 power equals 1, but zero in any number power equals 0)");
		}
	}
	
	result->end_column = tokens->column - column; result->end_row = tokens->row - row;

	if (endptr) *endptr = tokens;

	if (_result) *_result = result;

	return errs;
}

errors_t parse_unar(errors_t errs, expression_t **_result, const token_t* tokens, const token_t** endptr) {
	if (tokens->kind == TOKEN_EOF) {
		errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_EOF, ERROR_LEVEL_ERROR, tokens->column, tokens->row, tokens->end_column, tokens->end_row, nullptr, nullptr, "expected expression or number, got EOF");

		return errs;
	}

	expression_t* result = nullptr;

	size_t column = tokens->column, row = tokens->row;

	if (tokens->kind == TOKEN_NUMBER) {
		result = calloc(1, sizeof(expression_t));
	
		result->column = column; result->row = row;

		result->val = tokens->val;
		result->op = TOKEN_NUMBER;

		tokens++;

		goto end;
	}

	else if (tokens->kind == TOKEN_WORD) {
		result = calloc(1, sizeof(expression_t));
	
		result->column = column; result->row = row;

		result->word = strdup(tokens->word);
		result->op = TOKEN_WORD;

		tokens++;

		goto end;
	}

	else if (tokens->kind == TOKEN_LEFT_PARENT) {
		tokens++;

		errs = parse_semicolon(errs, &result, tokens, &tokens);

		if (result) {
			result->column = column; result->row = row;
		}

		if (tokens->kind == TOKEN_RIGHT_PARENT) {
			tokens++;
		}

		else {
			errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_UNCLOSED_PARENT, ERROR_LEVEL_ERROR, tokens->column, tokens->row, 1, 0, nullptr, nullptr, "'(' wasn't closed here");
		}
		
		// if (err > 0) {
		// 	free_expresion(result);

		// 	if (endptr) *endptr = tokens;

		// 	return errs;
		// }

		goto end;
	}

	else if (tokens->kind == TOKEN_PLUS) {
		tokens++;
		
		errs = parse_unar(errs, &result, tokens, &tokens);
	}

	else if (tokens->kind == TOKEN_MINUS ||
		tokens->kind == TOKEN_EXCLAMATION_MARK ||
		tokens->kind == TOKEN_TILDE) {
		result = calloc(1, sizeof(expression_t));
	
		result->column = column; result->row = row;

		result->unary_op = tokens->kind;

		tokens++;

		errs = parse_unar(errs, &result->left, tokens, &tokens);

		result->op = TOKEN_PARSER_UNARY_ONLY;
	}

	else {
		if (endptr) *endptr = tokens;

		char buf[64] = { 0 };
		get_token_name(buf, 64, *tokens);

		errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, tokens->column, tokens->row, tokens->end_column, tokens->end_row, nullptr, nullptr, "expected expression or number, got %s", buf);

		return errs;
	}

	end:

	if (!result) return errs;

	result->end_column = tokens->column - column; result->end_row = tokens->row - row;

	if (endptr) *endptr = tokens;

	if (_result) *_result = result;

	return errs;
}

size_t view_expresion(char* _buf, size_t buf_size, expression_t* expr) {
	if (_buf && buf_size < 1) return 0;

	char* buf = _buf;

	size_t index = 0;

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

	if (expr->unary_op == TOKEN_MINUS) {
		if (buf && index < (buf_size - 1)) buf[index] = '-';

		index++;
	}

	else if (expr->unary_op == TOKEN_EXCLAMATION_MARK) {
		if (buf && index < (buf_size - 1)) buf[index] = '!';

		index++;
	}

	else if (expr->unary_op == TOKEN_TILDE) {
		if (buf && index < (buf_size - 1)) buf[index] = '~';

		index++;
	}

	if (expr->op == TOKEN_NUMBER) {
		index += snprintf(_buf ? (buf + index) : 0, _buf ? (buf_size - index) : 0, "%i", expr->val);
	}

	else if (expr->op == TOKEN_WORD ||
			expr->op == TOKEN_ASSIGNMENT) {
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

		if (expr->word) free(expr->word);
		expr->word = false;
	}

	expr->op = TOKEN_UNDEFINED;

	free(expr);
}
