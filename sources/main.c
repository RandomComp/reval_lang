#include "types.h"
#include "lexer.h"
#include "parser.h"
#include "eval.h"

#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void show_usage(const char* exec_name) {
	printf("Usage: %s [file]\n\r", exec_name);
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

int run_code(errors_t *errs, interneds_t *interneds, const char *filename, const char *code) {
	tokens_t tokens = { 0 };

	const char* expr_endptr = code;
	
	*errs = tokenize(*errs, &tokens, interneds, filename, code, &expr_endptr);

	// errs = emit_error(errs, SUBSYSTEM_PARSER, ERROR_PARSER_EOF, ERROR_LEVEL_WARN, 0, 0, 5, 0, "test add", "test del", "hello, wass'up, bro");

	for (size_t i = 0; i < tokens.tokens_cnt; i++) {
		char buf[32] = { 0 };

		get_token_name(buf, 32, tokens.tokens[i]);

		printf("%zi:%zi...%zi:%zi -- %s\n\r", tokens.tokens[i].column, tokens.tokens[i].row, tokens.tokens[i].end_column, tokens.tokens[i].end_row, buf);
	}

	if (errs->level >= ERROR_LEVEL_ERROR) {
		free_tokens(tokens);

		return 1;
	}

	expression_t* expr = nullptr;

	const token_t* endptr = tokens.tokens;

	int result = 0;

	*errs = parse_semicolon(*errs, &expr, interneds, endptr, &endptr);

	if (endptr->kind == TOKEN_UNDEFINED ||
		endptr != (tokens.tokens + tokens.tokens_cnt - 1)) {
		*errs = emit_error(*errs, SUBSYSTEM_PARSER, ERROR_PARSER_INVALID_SYNTAX, ERROR_LEVEL_ERROR, endptr->filename, endptr->column, endptr->row, endptr->end_column, endptr->end_row, nullptr, nullptr, nullptr);
	}

	if (errs->level >= ERROR_LEVEL_ERROR) {
		print_errors(*errs, code);

		free_expresion(expr); expr = nullptr;

		free_tokens(tokens);

		return 1;
	}

	// char buf[128] = { 0 };

	// view_expresion(buf, 128, expr);

	// printf("%s\n\r", buf);

	free_tokens(tokens);

	*errs = eval_expresion(*errs, nullptr, &result, expr);

	printf("%i\n\r", result);

	free_expresion(expr); expr = nullptr;

	return 0;
}

int main(int argc, char** argv) {
	#ifdef IS_WIN
	SetUnhandledExceptionFilter(sigsegv_handler);
	#else
	signal(SIGSEGV, sigsegv_handler);
	#endif

	setlocale(LC_ALL, "");

	if (argc <= 1) {
		show_usage(argv[0]);

		return 1;
	}

	const char *filename = argv[1];

	FILE* file = fopen(filename, "r");

	if (!file) {
		fprintf(stderr, "%s: \"%s\": %s\n\r", argv[0], filename, strerror(errno));

		return ENOENT;
	}

	fseek(file, 0, SEEK_END);

	long file_size = ftell(file);

	fseek(file, 0, SEEK_SET);

	char* code = malloc(file_size + 1);

	if (!code) {
		fclose(file);

		return ENOMEM;
	}

	size_t bytes = fread(code, 1, file_size, file);

	code[bytes] = 0;

	fclose(file);

	errors_t errs = { 0 };

	interneds_t interneds = { 0 };

	int err = run_code(&errs, &interneds, filename, code);

	// printf("interned strings:\n\r");

	// for (size_t i = 0; i < interneds.cnt; i++) {
	// 	printf("    %s\n\r", interneds.interneds[i]);
	// }

	print_errors(errs, code);

	free_interneds(&interneds);

	free_errors(errs);

	free(code);

	return err;
}
