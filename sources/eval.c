#include "eval.h"

#include "errors.h"
#include "parser.h"
#include "utils.h"
#include <string.h>
#include <malloc.h>

var_t* get_var(vars_t vars, const char *name) {
	var_t* result = nullptr;

	for (size_t i = 0; i < vars.vars_cnt; i++) {
		if (!vars.vars[i].used) continue;

		if (strcmp(vars.vars[i].name, name) == 0) {
			result = vars.vars + i; break;
		}
	}

	return result;
}

vars_t set_var(vars_t vars, const char *name, int val) {
	if (!vars.vars) {
		vars.vars_size = VAR_ALLOC_STEP;

		vars.vars = calloc(vars.vars_size, sizeof(var_t));
	}

	if (vars.vars_cnt >= vars.vars_size) {
		size_t new_size = align_up(vars.vars_size, VAR_ALLOC_STEP);

		vars.vars = recalloc(vars.vars, vars.vars_size * sizeof(var_t), new_size * sizeof(var_t));

		vars.vars_size = new_size;
	}
	
	var_t *var = get_var(vars, name);

	if (!var) {
		var = vars.vars + vars.vars_cnt;

		var->name = strdup(name);
		var->used = true;

		vars.vars_cnt++;
	}
	
	var->val = val;

	return vars;
}

vars_t del_var(vars_t vars, const char *name) {
	return vars;
}

void free_vars(vars_t vars) {
	if (!vars.vars) return;

	for (size_t i = 0; i < vars.vars_cnt; i++) {
		var_t *cur = vars.vars + i;

		if (cur->name) free(cur->name);
		cur->name = nullptr;

		cur->used = false;
	}

	free(vars.vars);
}

errors_t eval_expresion(errors_t errs, vars_t *_vars, int* _result, expression_t* expr) {
	if (!expr) {
		errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_EXPR_ZERO_PTR, ERROR_LEVEL_ERROR, expr->filename, 0, 0, 0, 0, nullptr, nullptr, nullptr);
		
		return errs;
	}

	vars_t vars = _vars ? *_vars : (vars_t){ 0 };

	int val = 0;

	if (expr->op == TOKEN_NUMBER) {
		val = expr->val;
	}

	else if (expr->op == TOKEN_WORD) {
		var_t *var = get_var(vars, expr->word);

		if (var != nullptr) {
			switch (expr->unary_op) {
				case TOKEN_PREINCREMENT:
					var->val = var->val + 1; break;
				case TOKEN_PREDECREMENT:
					var->val = var->val - 1; break;
				
				default: break;
			}

			val = var->val;

			switch (expr->unary_op) {
				case TOKEN_POSTINCREMENT:
					var->val = var->val + 1; break;
				case TOKEN_POSTDECREMENT:
					var->val = var->val - 1; break;
				
				default: break;
			}
		}
		
		else {
			errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_UNKNOWN_VARIABLE, ERROR_LEVEL_ERROR, expr->filename, expr->column, expr->row, expr->end_column, expr->end_row, nullptr, nullptr, "unknown variable \"%s\"", expr->word);

			goto exit;
		}
	}

	if (expr->op == TOKEN_NUMBER || expr->op == TOKEN_WORD) {
		switch (expr->unary_op) {
			case TOKEN_MINUS:
				val = -val; break;
			case TOKEN_EXCLAMATION_MARK:
				val = !val; break;
			case TOKEN_TILDE:
				val = ~val; break;
			
			default: break;
		}

		if (_result) *_result = val;

		goto exit;
	}

	int left = 0, right = 0, center = 0;
	
	if (expr->left) {
		errs = eval_expresion(errs, &vars, &left, expr->left);

		if (errs.level >= ERROR_LEVEL_ERROR) goto exit;
	}

	// TODO: сделать поддержку инкремента/декрмента и присваивания вида уравнения

	if (expr->op == TOKEN_PARSER_UNARY_ONLY) {
		switch (expr->unary_op) {
			case TOKEN_MINUS:
				left = -left; break;
			case TOKEN_EXCLAMATION_MARK:
				left = !left; break;
			case TOKEN_TILDE:
				left = ~left; break;
			case TOKEN_PREINCREMENT:
				left = left + 1; break;
			case TOKEN_PREDECREMENT:
				left = left - 1; break;
			
			default: break;
		}

		if (_result) *_result = left;

		return errs;
	}

	if (expr->op == TOKEN_QUESTION_MARK) {
		errs = eval_expresion(errs, &vars, &center, expr->center);

		if (errs.level >= ERROR_LEVEL_ERROR) goto exit;
	}

	if (expr->right) {
		errs = eval_expresion(errs, &vars, &right, expr->right);

		if (errs.level >= ERROR_LEVEL_ERROR) goto exit;
	}

	int result = 0;

	var_t *var = nullptr;

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
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_DIVISION_BY_ZERO, ERROR_LEVEL_ERROR, expr->filename, expr->column, 0, expr->end_column, 0, nullptr, nullptr, "division by zero (%i / %i)", left, right);

				break;
			}
		
			result = left / right; break;
		case TOKEN_REMAINDER:
			if (right == 0) {
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_DIVISION_BY_ZERO, ERROR_LEVEL_ERROR, expr->filename, expr->column, 0, expr->end_column, 0, nullptr, nullptr, "modulo by zero (%i %% %i)", left, right);

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
			result = right;
			vars = set_var(vars, expr->word, right);
			break;
		case TOKEN_PLUS_ASSIGNMENT:
			var = get_var(vars, expr->word);

			if (var) {
				result = var->val + right;

				set_var(vars, expr->word, result);
			}

			else {
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_UNKNOWN_VARIABLE, ERROR_LEVEL_ERROR, expr->filename, expr->column, expr->row, expr->end_column, expr->end_row, nullptr, nullptr, "unknown variable \"%s\"", expr->word);
			}
		
			break;
		case TOKEN_MINUS_ASSIGNMENT:
			var = get_var(vars, expr->word);

			if (var) {
				result = var->val - right;

				set_var(vars, expr->word, result);
			}

			else {
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_UNKNOWN_VARIABLE, ERROR_LEVEL_ERROR, expr->filename, expr->column, expr->row, expr->end_column, expr->end_row, nullptr, nullptr, "unknown variable \"%s\"", expr->word);
			}
		
			break;
		case TOKEN_MULTIPLY_ASSIGNMENT:
			var = get_var(vars, expr->word);

			if (var) {
				result = var->val * right;

				set_var(vars, expr->word, result);
			}

			else {
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_UNKNOWN_VARIABLE, ERROR_LEVEL_ERROR, expr->filename, expr->column, expr->row, expr->end_column, expr->end_row, nullptr, nullptr, "unknown variable \"%s\"", expr->word);
			}
		
			break;
		case TOKEN_DIVIDE_ASSIGNMENT:
			if (right == 0) {
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_DIVISION_BY_ZERO, ERROR_LEVEL_ERROR, expr->filename, expr->column, expr->row, expr->end_column, expr->end_row, nullptr, nullptr, "division by zero (%i / %i)", left, right);

				break;
			}
		
			var = get_var(vars, expr->word);

			if (var) {
				result = var->val / right;

				set_var(vars, expr->word, result);
			}

			else {
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_UNKNOWN_VARIABLE, ERROR_LEVEL_ERROR, expr->filename, expr->column, expr->row, expr->end_column, expr->end_row, nullptr, nullptr, "unknown variable \"%s\"", expr->word);
			}
		
			break;
		case TOKEN_REMAINDER_ASSIGNMENT:
			if (right == 0) {
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_DIVISION_BY_ZERO, ERROR_LEVEL_ERROR, expr->filename, expr->column, expr->row, expr->end_column, expr->end_row, nullptr, nullptr, "modulo by zero (%i %% %i)", left, right);

				break;
			}
		
			var = get_var(vars, expr->word);

			if (var) {
				result = var->val % right;

				set_var(vars, expr->word, result);
			}

			else {
				errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_UNKNOWN_VARIABLE, ERROR_LEVEL_ERROR, expr->filename, expr->column, expr->row, expr->end_column, expr->end_row, nullptr, nullptr, "unknown variable \"%s\"", expr->word);
			}
		
			break;
		case TOKEN_SEMICOLON:
			result = expr->right ? right : left; break;
		default:
			errs = emit_error(errs, SUBSYSTEM_EVAL, ERROR_EVAL_UNKNOWN_OPERATOR, ERROR_LEVEL_ERROR, expr->filename, expr->column, 0, expr->end_column, 0, nullptr, nullptr, "unknown operator %i", expr->op);

			break;
	}

	if (_result) *_result = result;

	exit:

	if (_vars) *_vars = vars;

	else free_vars(vars);

	return errs;
}
