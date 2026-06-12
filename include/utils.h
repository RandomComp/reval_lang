#ifndef REVAL_UTILS_H
#define REVAL_UTILS_H

#include "types.h"
#include <stdio.h>

size_t skip_spaces(const char* text);

void count_spaces(size_t* column, size_t* row, const char* text);

int64 align_up(int64 num, int64 align);

int64 align_down(int64 num, int64 align);

char* parse_cli_args(char* _str);

typedef enum radix_e {
	RADIX_UNKNOWN,
	RADIX_BINARY,
	RADIX_OCTAL,
	RADIX_DECIMAL,
	RADIX_HEXADECIMAL,
} radix_e;

typedef enum parse_num_err_e {
	PARSER_NUM_ERR_OK,
	PARSER_NUM_ERR_LEADING_ZEROES,
	PARSER_NUM_ERR_MAY_DECIMAL,
	PARSER_NUM_ERR_MAY_HEXADECIMAL,
	PARSER_NUM_ERR_INVALID_LITERAL
} parse_num_err_e;

radix_e get_num_radix(const char* str);
const char* get_radix_prefix(radix_e radix);
const char* get_radix_alphbaet(radix_e radix);
const char* get_radix_name(radix_e radix);
char* get_number_part(const char* str);
parse_num_err_e parse_num(uint64* result, const char* str, const char** endptr);

bool isnum(char c);

bool ishex(char c);

unsigned int tohex(char c);

int powi(int val, int exp);

void parse_bytes(const char* str, byte* _bytes, size_t* _bytes_cnt, const char** end_ptr);

int convert_byte_to_int(byte val);

char* bytes_as_str(byte* bytes, size_t bytes_cnt);

const char* get_home_dir(void);

/* Return the length of the segment from end of S which consists entirely of characters not in REJECT */
size_t strlcspn(const char* str, const char* reject);

typedef enum term_type_e {
	TERM_DUMB,
	TERM_XTERM,
	TERM_WINDOWS,
} term_type_e;

typedef enum term_colors_e {
	TERM_COLOR_BLACK,
	TERM_COLOR_BLUE,
	TERM_COLOR_GREEN,
	TERM_COLOR_AQUA,
	TERM_COLOR_RED,
	TERM_COLOR_MAGENTA,
	TERM_COLOR_BROWN,
	TERM_COLOR_WHITE,
	TERM_COLOR_BRIGHT_BLACK,
	TERM_COLOR_BRIGHT_BLUE,
	TERM_COLOR_BRIGHT_GREEN,
	TERM_COLOR_BRIGHT_AQUA,
	TERM_COLOR_BRIGHT_RED,
	TERM_COLOR_BRIGHT_MAGENTA,
	TERM_COLOR_BRIGHT_YELLOW,
	TERM_COLOR_BRIGHT_WHITE,
} term_colors_e;

term_type_e get_term_type(void);

term_colors_e set_fg_color(FILE* out_or_err, term_colors_e color);

term_colors_e set_bg_color(FILE* out_or_err, term_colors_e color);

void set_default(FILE* out_or_err);

#endif
