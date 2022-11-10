#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include "utils.h"
#include "syntax.h"
#include "help.h"

static int compare_keys_pair(const void* va, const void* vb)
{
	const symbol_t* a = (symbol_t *)va, * b = (symbol_t*)vb;
	return strcmp(a->key, b->key);
}

syms_t get_symbol(symbol_t* table, char* key, unsigned int size)
{
	symbol_t key_pair[1] = { {key} };
	syms_t null = { NAN, 0 };
	symbol_t* p = (symbol_t *)bsearch(key_pair, table, size, sizeof(*table), compare_keys_pair);
	return p ? p->val : null;
}

unsigned int set_symbol_double(symbol_t** table, char* key, double val, unsigned int *size)
{
	unsigned int ret = SUCCESS;
	unsigned int i = 0;

	if (isnan(get_symbol(*table, key, *size).data.definite)) {
		MEM_(*table, (*size)+1, symbol_t);
		MEM((*table)[*size].key, 80, char);
		sprintf((*table)[*size].key, "%s", key);
		(*table)[*size].val.flag = 0;
		(*table)[*size].val.data.definite = val;
		(*size)++;
		qsort((*table), (*size), sizeof(**table), compare_keys_pair);
		return ret;
	}

	while (i < (int)(*size)) {
		if (!strcmp((*table)[i].key, key)) {
			(* table)[i].val.flag = 0;
			(* table)[i].val.data.definite = val;
			return ret;
		}
		i++;
	}

exit:
	return ret;
}

unsigned int set_symbol_expression(symbol_t** table, char* key, char* val, unsigned int* size)
{
	unsigned int ret = SUCCESS;
	unsigned int i = 0;

	if (isnan(get_symbol(*table, key, *size).data.definite)) {
		MEM_(*table, (*size) + 1, symbol_t);
		MEM((*table)[*size].key, 80, char);
		sprintf((*table)[*size].key, "%s", key);
		(*table)[*size].val.flag = 1;
		sprintf((*table)[*size].val.data.expression, "%s", val);
		(*size)++;
		qsort((*table), (*size), sizeof(**table), compare_keys_pair);
		return ret;
	}

	while (i < (int)(*size)) {
		if (!strcmp((*table)[i].key, key)) {
			(*table)[i].val.flag = 1;
			sprintf((*table)[i].val.data.expression, "%s", val);
			return ret;
		}
		i++;
	}

exit:
	return ret;
}

unsigned int create_map(map_t* map)
{
	unsigned int ret = SUCCESS;

	map->size = 1;
	MEM(map->table, map->size, symbol_t);
	MEM(map->table[0].key, 80, char);
	sprintf(map->table[0].key, "ans");
	map->table->val.flag = 0;
	map->table->val.data.definite = 0;

exit:

	return ret;
}

void destroy_map(map_t* map)
{
	if (map) {
		if (map->table) {
			for (unsigned int i = 0; i < map->size; i++) {
				if (map->table[i].key) {
					free(map->table[i].key);
					map->table[i].key = NULL;
				}
			}
			free(map->table);
			map->table = NULL;
		}
	}
}

bool isfloat(char* val)
{
	bool isF = 1;
	unsigned int i = 0;

	if (val[0] == '-')
		i = 1;

	for (i; i < strlen(val); i++) {
		if (isdigit(val[i])) {
		}
		else if (isalpha(val[i])) {
			isF = 0;
			break;
		}
		else if (val[i] != '.') {
			isF = 0;
			break;
		}
	}


	return isF;
}

void get_hStr(State_t* state)
{
	memset(state->hStr, '\0', 8000);
	for (unsigned int i = state->hmax - 1; i > 0; i--)
	{
		strcat(state->hStr, state->history[i]);
		strcat(state->hStr, "\n");
	}
}

void get_cursor(State_t* state)
{
	unsigned int i = 0;

	memset(state->cursor, '\0', 80);
	for (i = 0; i < state->index; i++) {
		state->cursor[i] = ' ';
	}
	state->cursor[i] = '|';
}

void get_mStr(State_t* state)
{
	memset(state->mStr, '\0', 8000);
	for (unsigned int i = 0; i < state->map.size; i++) {
		if (state->map.table[i].val.flag) { // String
			sprintf(state->tmp1, "%s: %s\n", state->map.table[i].key, state->map.table[i].val.data.expression);
			strcat(state->mStr, state->tmp1);
			memset(state->tmp1, '\0', 80);
		}
		else { // Double
			sprintf(state->tmp1, "%s: %lf\n", state->map.table[i].key, state->map.table[i].val.data.definite);
			strcat(state->mStr, state->tmp1);
			memset(state->tmp1, '\0', 80);
		}
	}
}

void print_table(symbol_t* table, unsigned int size) {

	for (unsigned int i = 0; i < size; i++) {
		if (table[i].val.flag) { // String
			printf("table[%s]: %s\n", table[i].key, table[i].val.data.expression);
		} else { // Double
			printf("table[%s]: %lf\n", table[i].key, table[i].val.data.definite);
		}
	}
}

void print_error(char* expression, const char *flag)
{
	std::ofstream fd;
	fd.open("err.txt", std::ios_base::app);
	fd << "Expression: " << expression << std::endl;
	fd << "Error: " << flag << std::endl;
	fd.close();
}