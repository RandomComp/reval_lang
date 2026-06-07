#include "types.h"
#include "lexer.h"
#include "parser.h"

#include <stdio.h>

void show_usage(const char* exec_name) {
	printf("Usage: %s [expression]\n\r", exec_name);
}

int main(int argc, char** argv) {
	if (argc <= 1) {
		show_usage(argv[0]);

		return 1;
	}
	
	const char* expr = argv[1];

	tokens_t tokens = { 0 };

	tokenize(&tokens, expr, nullptr);

	for (size_t i = 0; i < tokens.tokens_cnt; i++) {
		char buf[32] = { 0 };

		get_token_name(buf, 32, tokens.tokens[i]);

		printf("%s\n\r", buf);
	}

	return 0;
}
