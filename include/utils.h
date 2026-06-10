#ifndef REVAL_UTILS_H
#define REVAL_UTILS_H

#include "types.h"

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

typedef enum term_type_e {
	TERM_DUMB,
	TERM_XTERM,
	TERM_WINDOWS,
} term_type_e;

// TODO: реализовать универсальную структуру term_t для окрашивания терминала

term_type_e get_term_type(void);

#endif
