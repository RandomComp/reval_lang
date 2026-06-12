#include "utils.h"

#include "ansi.h"
#include "types.h"
#include "lexer_fwd.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

size_t skip_spaces(const char* text) {
	size_t index = 0;

	if (strchr(" \f\n\r\t\v", *text) != nullptr) {
		index += strspn(text, " \f\n\r\t\v");
	}

	return index;
}

void count_spaces(size_t* column, size_t* row, const char* text) {
	while (*text && strchr(" \t\n", *text) != nullptr) {
		switch (*text) {
			case ' ':
				if (column) *column += 1;
				break;
			case '\t':
				if (column) *column += 1;
				break;
			case '\n':
				if (column) *column = 0;
				if (row) *row += 1;
				break;
		}

		text++;
	}
}

int64 align_up(int64 num, int64 align) {
	if (align == 0 || num % align == 0)
		return num;

	return ((num / align) + 1) * align;
}

int64 align_down(int64 num, int64 align) {
	if (align == 0 || num % align == 0)
		return num; 

	return ((num / align) - 1) * align;
}

static char* strtok_str = nullptr; static size_t str_len = 0;

static ssize_t strtok_index = 0;

char* parse_cli_args(char* _str) {
	if (!_str) {
		if (!strtok_str) return nullptr;

		if (strtok_index < 0) return nullptr;

		for (size_t i = strtok_index + 1; i < str_len; i++) {
			if (strtok_str[i] == 0) {
				for (; i < str_len; i++) {
					if (strtok_str[i] != 0) break;
				}

				char* result = strtok_str + i;

				strtok_index = i;

				return result;
			}
		}

		strtok_index = 0;
		
		return nullptr;
	}
	
	strtok_index = 0;

	str_len = strlen(_str);

	strtok_str = _str;

	bool quotes = false;
	bool double_quotes = false;

	bool escape = false;

	for (size_t i = 0; _str[i]; i++) {
		if (_str[i] == '"') {
			if (!escape) {
				double_quotes = !double_quotes;

				_str[i] = 0;
			}

			else escape = false;
		}

		if (_str[i] == '\'') {
			if (!escape) {
				quotes = !quotes;

				_str[i] = 0;
			}

			else escape = false;
		}

		if (_str[i] == '#' && !quotes && !double_quotes) {
			_str[i] = 0;

			str_len = i; break;
		}

		if (_str[i] == '\\') {
			escape = true;

			// _str[i] = 0;

			size_t k = i;

			for (size_t j = i; _str[j] != ' ' && j < str_len; j++) {
				if (_str[j] != '\\') {
					_str[k] = _str[j];

					k++;
				}
			}

			for (size_t j = k; _str[j] != ' ' && j < str_len; j++) {
				_str[j] = ' ';
			}
		}

		if ((_str[i] == ' ' || _str[i] == '\t') && !quotes && !double_quotes) {
			_str[i] = 0;
		}
	}

	return _str;
}

radix_e get_num_radix(const char* str) {
	if (strncmp(str, "0o", 2) == 0) {
		return RADIX_OCTAL;
	}

	else if (strncmp(str, "0b", 2) == 0) {
		return RADIX_BINARY;
	}

	else if (strncmp(str, "0x", 2) == 0) {
		return RADIX_HEXADECIMAL;
	}

	else if (str[0] == '0' && isdigit(str[1])) {
		return RADIX_OCTAL;
	}

	return RADIX_DECIMAL;
}

const char* get_radix_prefix(radix_e radix) {
	switch (radix) {
		case RADIX_BINARY: 		return "0b";
		case RADIX_OCTAL: 		return "0o";
		case RADIX_DECIMAL: 	return "";
		case RADIX_HEXADECIMAL: return "0x";
		default: break;
	}

	return nullptr;
}

const char* get_radix_alphbaet(radix_e radix) {
	switch (radix) {
		case RADIX_BINARY: 		return "01";
		case RADIX_OCTAL: 		return "01234567";
		case RADIX_DECIMAL: 	return "0123456789";
		case RADIX_HEXADECIMAL: return "0123456789abcdef";
		default: break;
	}

	return nullptr;
}

const char* get_radix_name(radix_e radix) {
	switch (radix) {
		case RADIX_BINARY: return "binary";
		case RADIX_OCTAL: return "octal";
		case RADIX_DECIMAL: return "decimal";
		case RADIX_HEXADECIMAL: return "hexadecimal";
		default: break;
	}

	return "";
}

char* get_number_part(const char* str) {
	if (strncmp(str, "0o", 2) == 0) {
		str += 2;
	}

	else if (strncmp(str, "0b", 2) == 0) {
		str += 2;
	}

	else if (strncmp(str, "0x", 2) == 0) {
		str += 2;
	}

	else if (str[0] == '0' && isdigit(str[1])) {
		str += 1;
	}

	return (char*)str;
}

parse_num_err_e parse_num(uint64* result, const char* str, const char** endptr) {
	int base = 10;

	parse_num_err_e err = 0;

	if (strncmp(str, "0o", 2) == 0) {
		base = 8; str += 2;
	}

	else if (strncmp(str, "0b", 2) == 0) {
		base = 2; str += 2;
	}

	else if (strncmp(str, "0x", 2) == 0) {
		base = 16; str += 2;
	}

	else if (str[0] == '0' && isdigit(str[1])) {
		err = PARSER_NUM_ERR_LEADING_ZEROES;

		base = 8; str += 1;
	}

	size_t expected_len = strcspn(str, ALPHABET);

	const char* expected_endc = str + expected_len;

	const char* src_str = str;

	uint64 number = strtoull(str, (char**)(&str), base);

	if (str != expected_endc) {
		radix_e radix = RADIX_UNKNOWN;

		for (size_t i = 0; i < expected_len; i++) {
			char c = tolower(src_str[i]);

			if (radix <= RADIX_DECIMAL && isdigit(c)) {
				radix = RADIX_DECIMAL;
			}

			else if (radix <= RADIX_HEXADECIMAL && ishex(c)) {
				radix = RADIX_HEXADECIMAL;
			}

			else {
				radix = RADIX_UNKNOWN; break;
			}
		}

		if (radix != RADIX_UNKNOWN) {
			if (endptr) *endptr = str;

			switch (radix) {
				case RADIX_DECIMAL:
					return PARSER_NUM_ERR_MAY_DECIMAL;
				case RADIX_HEXADECIMAL:
					return PARSER_NUM_ERR_MAY_HEXADECIMAL;
				default: break;
			}
		}

		return PARSER_NUM_ERR_INVALID_LITERAL;
	}

	if (*str == 'e') {
		str++;
		
		printf("\"%s\"\n\r", str);

		uint64 exp = 0;
		parse_num(&exp, str, &str);

		number *= powi(base, exp);
	}

	if (endptr) *endptr = str;

	if (result) *result = number;

	return err;
}

bool isnum(char c) {
	return c >= '0' && c <= '9';
}

bool ishex(char c) {
	return isnum(c) || (tolower(c) >= 'a' && tolower(c) <= 'f');
}

unsigned int tohex(char c) {
	if (isnum(c)) {
		return c - '0';
	}

	return tolower(c) - 'a' + 10;
}

int powi(int val, int exp) {
	if (exp == 0) return 1;

	if (val == 1) return 1;

	if (exp < 0) return 0;

	if (exp == 1) return val;

	if (val == 0) return 0;
	
	if (val == 2) {
		return 1 << exp;
	}

	int result = 1;

	while (exp > 0) {
		result *= val;
		
		exp--;
	}

	return result;
}

void parse_bytes(const char* str, byte* _bytes, size_t* _bytes_cnt, const char** end_ptr) {
	size_t bytes_cnt = 0;

	if (strncmp(str, "0x", 2) == 0) {
		str += 2;
	}

	while (ishex(str[0]) && ishex(str[1])) {
		byte value = (tohex(str[0]) << 4) | tohex(str[1]);

		if (_bytes)
			_bytes[bytes_cnt] = value;

		bytes_cnt++;

		str += 2;
		
		while (*str == ' ') str++;
	}

	if (_bytes_cnt) *_bytes_cnt = bytes_cnt;

	if (end_ptr) *end_ptr = str;
}

int convert_byte_to_int(byte val) {
	bool sign = val & 0x80;

	if (sign) {
		unsigned int result = 0xFFFFFF00;

		result |= val;

		return (int)result;
	}

	return (int)val;
}

char* bytes_as_str(byte* bytes, size_t bytes_cnt) {
	size_t size = 0;

	for (uint64 i = 0; i < bytes_cnt; i++) {
		byte num = bytes[i];

		if (num == '\n') {
			size += 2;
		}

		else if (num == '\r') {
			size += 2;
		}

		else if (num == '\t') {
			size += 2;
		}

		else if (num == '\b') {
			size += 2;
		}

		else if (isascii(num)) {
			size += 1;
		}

		else {
			size += 4;
		}
	}

	char* result = malloc(size);

	memset(result, 0, size);

	size_t index = 0;

	for (uint64 i = 0; i < bytes_cnt; i++) {
		byte num = bytes[i];

		if (num == '\n') {
			result[index] = '\\';
			result[index + 1] = 'n';

			index += 2;
		}

		else if (num == '\r') {
			result[index] = '\\';
			result[index + 1] = 'r';

			index += 2;
		}

		else if (num == '\t') {
			result[index] = '\\';
			result[index + 1] = 't';

			index += 2;
		}

		else if (num == '\b') {
			result[index] = '\\';
			result[index + 1] = 'b';

			index += 2;
		}

		else if (isascii(num)) {
			result[index] = num;

			index += 1;
		}

		else {
			index += snprintf(result + index, size - index, "\\x%.2X", num);
		}
	}

	return result;
}

const char* get_home_dir(void) {
	#ifdef IS_UNIX
	return getenv("HOME");
	#elif defined(IS_WIN)
	return getenv("USERPROFILE");
	#endif

	return nullptr;
}

size_t strlcspn(const char* str, const char* reject) {
	size_t len = strlen(str);

	size_t index = len;

	while (true) {
		if (strchr(reject, str[index]) == nullptr) {
			break;
		}

		index--;
	}

	return index;
}

#ifdef IS_UNIX
#include <unistd.h>
#elif defined(IS_WIN)
#include <windows.h>
#endif

term_type_e get_term_type(void) {
	#ifdef IS_WIN
	return TERM_WINDOWS;
	#elif defined(IS_UNIX)
	if (!isatty(STDOUT_FILENO) || !isatty(STDERR_FILENO)) {
		return TERM_DUMB;
	}

	const char* term_name = getenv("TERM");

	if (!term_name || 
		strstr(term_name, "dumb") != nullptr ||
		strstr(term_name, "vt100") != nullptr ||
		strstr(term_name, "ansi") != nullptr) {
		return TERM_DUMB;
	}

	if (strstr(term_name, "xterm") != nullptr ||
		strstr(term_name, "256color") != nullptr) {
		return TERM_XTERM;
	}
	#endif

	return TERM_DUMB;
}

byte color_to_ansi(term_colors_e color) {
	switch (color) {
		case TERM_COLOR_BLUE:
			return BLUE_ANSI_ID;
		case TERM_COLOR_GREEN:
			return GREEN_ANSI_ID;
		case TERM_COLOR_AQUA:
			return AQUA_ANSI_ID;
		case TERM_COLOR_RED:
			return RED_ANSI_ID;
		case TERM_COLOR_MAGENTA:
			return MAGENTA_ANSI_ID;
		case TERM_COLOR_BROWN:
			return YELLOW_ANSI_ID;
		case TERM_COLOR_WHITE:
			return WHITE_ANSI_ID;
		default:
			return 0;
	}

	return 0;
}

static term_colors_e cur_fg = TERM_COLOR_WHITE;
static term_colors_e cur_bg = TERM_COLOR_BLACK;

term_colors_e set_fg_color(FILE* out_or_err, term_colors_e color) {
	term_colors_e def = cur_fg;

	#ifdef IS_WIN
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	cur_fg = color;

	SetConsoleTextAttribute(handle, cur_bg << 8 | cur_fg);
	#else
	term_type_e type = get_term_type();

	if (type == TERM_DUMB) return TERM_COLOR_WHITE;

	if ((color & 0x08) != 0) {
		fprintf(out_or_err, "\x1B[%u;1m", color_to_ansi(color & 0x07));
	}

	else {
		fprintf(out_or_err, "\x1B[%um", color_to_ansi(color));
	}
	#endif

	return def;
}

term_colors_e set_bg_color(FILE* out_or_err, term_colors_e color) {
	term_colors_e def = cur_bg;

	#ifdef IS_WIN
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	cur_bg = color;

	SetConsoleTextAttribute(handle, cur_bg << 8 | cur_fg);
	#else
	term_type_e type = get_term_type();

	if (type == TERM_DUMB) return TERM_COLOR_BLACK;

	if ((color & 0x08) != 0) {
		fprintf(out_or_err, "\x1B[%u;1m", color_to_ansi(color & 0x07) + 10);
	}

	else {
		fprintf(out_or_err, "\x1B[%um", color_to_ansi(color) + 10);
	}
	#endif

	return def;
}

void set_default(FILE* out_or_err) {
	#ifdef IS_WIN
	set_fg_color(out_or_err, TERM_COLOR_WHITE);
	set_bg_color(out_or_err, TERM_COLOR_BLACK);
	#else
	term_type_e type = get_term_type();

	if (type == TERM_DUMB) return;

	fprintf(out_or_err, "\x1B[0m");
	#endif
}
