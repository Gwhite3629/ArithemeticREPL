#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdbool.h>

#include "help.h"

typedef union sym {
	double definite;
	char expression[80];
} sym;

typedef struct syms {
	sym data;
	bool flag; // Default 0, double
} syms_t;

typedef struct symbol {
	char* key;
	syms_t val;
} symbol_t;

typedef struct map {
	symbol_t* table;
	unsigned int size;
} map_t;

typedef struct state {
	map_t map;
	char history[100][80];
	char hStr[8000];
	char cursor[80];
	char mStr[8000];
	char userin[80];
	char tmp1[80];
	char tmp2[80];
	unsigned int index;
	unsigned int hindex;
	unsigned int hmax;
	bool q;
} State_t;

#define operators "-+=*^/()"

syms_t get_symbol(symbol_t* table, char* key, unsigned int size);

#define set_symbol(table, key, val, size) \
	isfloat(val) ?	set_symbol_double(table, key, strtod(val, NULL),size) :\
						set_symbol_expression(table, key, val, size);

unsigned int set_symbol_double(symbol_t** table, char* key, double val, unsigned int *size);

unsigned int set_symbol_expression(symbol_t** table, char* key, char* val, unsigned int* size);

unsigned int create_map(map_t* map);

void destroy_map(map_t* map);

bool isfloat(char* val);

void get_hStr(State_t* state);

void get_cursor(State_t* state);

void get_mStr(State_t* state);

void print_table(symbol_t* table, unsigned int size);

void print_error(char* expression, const char *flag);

#endif // _UTILS_H_