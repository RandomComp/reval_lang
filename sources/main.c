#include "types.h"
#include "lexer.h"
#include "parser.h"
#include "eval.h"

#include <stdio.h>
#include <stdlib.h>

void show_usage(const char* exec_name) {
	printf("Usage: %s [expression]\n\r", exec_name);
}

#ifdef IS_WIN
#include <windows.h>

LONG WINAPI sigsegv_handler(PEXCEPTION_POINTERS pExceptionInfo) {
	if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
		printf("Segmentation fault\n\r");
		
		return EXCEPTION_EXECUTE_HANDLER;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}
#else
#include <signal.h>

void sigsegv_handler(int sig) {
	printf("critical error: segmentation fault\n\r");

	exit(sig);
}
#endif

int main(int argc, char** argv) {
	#ifdef IS_WIN
	SetUnhandledExceptionFilter(sigsegv_handler);
	#else
	signal(SIGSEGV, sigsegv_handler);
	#endif

	if (argc <= 1) {
		show_usage(argv[0]);

		return 1;
	}
	
	const char* expr_str = argv[1];

	tokens_t tokens = { 0 };

	const char* expr_endptr = expr_str;

	errors_t errs = { 0 };
	
	errs = tokenize(errs, &tokens, expr_str, &expr_endptr);

	// errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_EOF, ERROR_LEVEL_WARN, 0, 0, 5, 0, "test add", "test del", "hello, wass'up, bro");

	for (size_t i = 0; i < tokens.tokens_cnt; i++) {
		char buf[32] = { 0 };

		get_token_name(buf, 32, tokens.tokens[i]);

		printf("%s\n\r", buf);
	}

	if (errs.level >= ERROR_LEVEL_ERROR) {
		print_errors(errs, expr_str);

		free_errors(errs);

		free_tokens(tokens);

		return 0;
	}

	expression_t* expr = nullptr;

	const token_t* endptr = tokens.tokens;

	int result = 0;

	errs = parse_semicolon(errs, &expr, endptr, &endptr);

	if (endptr->kind == TOKEN_UNDEFINED ||
		endptr != (tokens.tokens + tokens.tokens_cnt - 1)) {
		errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, endptr->column, endptr->row, endptr->end_column, endptr->end_row, nullptr, nullptr, nullptr);
	}

	if (errs.level >= ERROR_LEVEL_ERROR) {
		print_errors(errs, expr_str);

		free_errors(errs);

		free_expresion(expr); expr = nullptr;

		free_tokens(tokens);

		return 0;
	}

	char buf[128] = { 0 };

	view_expresion(buf, 128, expr);

	printf("%s\n\r", buf);

	errs = eval_expresion(errs, &result, expr);

	print_errors(errs, expr_str);

	printf("%i\n\r", result);

	free_errors(errs);

	free_expresion(expr); expr = nullptr;

	free_tokens(tokens);

	return 0;
}
