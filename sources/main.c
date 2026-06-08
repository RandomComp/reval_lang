#include "types.h"
#include "lexer.h"
#include "parser.h"

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

	tokenize(&tokens, expr_str, nullptr);

	for (size_t i = 0; i < tokens.tokens_cnt; i++) {
		char buf[32] = { 0 };

		get_token_name(buf, 32, tokens.tokens[i]);

		printf("%s\n\r", buf);
	}

	expression_t* expr = nullptr;

	const token_t* endptr = tokens.tokens;

	int err = 0;

	int result = 0;

	err = parse_compare(&expr, endptr, &endptr);

	if (endptr->kind == TOKEN_UNDEFINED) {
		err = PARSER_ERR_INVALID_SYNTAX;
	}

	if (err != 0 && endptr->kind != TOKEN_EOF) {
		printf("\n\r%u: error: %s\n\r", 0, get_parser_err_description(err));

		printf("%6u" SEPERATOR "%*s", 0, 4, "");
		for (size_t i = 0; i < tokens.tokens_cnt; i++) {
			char buf[64] = { 0 };

			get_token_c(buf, 64, tokens.tokens[i]);

			printf("%.64s ", buf);
		}
		printf("\n\r");

		printf("%6s" SEPERATOR "%*s^\n\r", "", 4, "");

		printf("%td\n\r", endptr - tokens.tokens);
	}

	show_expresion(expr); printf("\n\r");

	err = eval_expresion(&result, expr);

	if (err != 0) {
		printf("%s\n\r", get_eval_err_description(err));
	}

	printf("result %i\n\r", result);

	free_expresion(expr); expr = nullptr;

	free(tokens.tokens);

	return 0;
}
