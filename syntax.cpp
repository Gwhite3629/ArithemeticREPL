#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <ctype.h>
#include "syntax.h"
#include "interpreter.h"
#include "help.h"
#include "utils.h"

#define COMMAND_L 3

command_t COMMANDS[COMMAND_L] = {
	{"CLEAR", 1},
	{"QUIT", 2},
	{"RUN", 3}
};

unsigned int tokenize(char* filename, map_t* map)
{
	unsigned int ret = SUCCESS;

	char* buf = NULL;
	char c;
	int counter = 0;

	std::ifstream fd;

	fd.open(filename);

	MEM(buf, 80, char);

	while (fd.getline(buf, 80)) {
		CHECK(ret = eval(buf, map));
	}

exit:
	fd.close();
	if (buf)
		free(buf);

	return ret;
}

unsigned int eval(char* line, map_t* map)
{
	unsigned int ret = SUCCESS;

	char* rhs = NULL;
	char* lhs = NULL;
	char* temp = NULL;
	syms_t ts = { NAN, 0 };
	MEM(rhs, 80, char);
	MEM(lhs, 80, char);

	if (strpbrk(line, "=") == NULL) {
		MEM(temp, 80, char);
		sprintf(temp, "ans = ");
		ts = get_symbol(map->table, line, map->size);
		if (!ts.flag) {
			if (isnan(ts.data.definite)) {
				strcat(temp, line);
			}
			else {
				sprintf(line, "%lf\n", ts.data.definite);
				strcat(temp, line);
			}
		}
		else {
			strcat(temp, ts.data.expression);
		}
		memcpy(line, temp, 80);
	}

	CHECK(ret = trim(line));
	organize(line, lhs, rhs, '=', map);
	/*if (strcmp(lhs, "Invalid"))
		set_symbol(&(map->table), lhs, rhs, &(map->size));*/
	//CHECK(ret = syntax_check(lhs));
	CHECK(ret = evaluate(map, lhs, rhs));

exit:
	if (temp)
		free(temp);
	if (rhs)
		free(rhs);
	if (lhs)
		free(lhs);

	return ret;
}

unsigned int trim(char* line)
{
	unsigned int ret = SUCCESS;

	size_t l = strlen(line);
	unsigned int i = 0;
	unsigned int j = 0;

	char* temp = NULL;

	MEM(temp, 80, char);
	memcpy(temp, line, l);

	while (j < (l - i)) {
		j = strcspn(temp, " \n");
		strncpy(temp + j, temp + j + 1, l - j);
		i++;
		temp[79] = '\0';
	}
	memcpy(line, temp, strlen(temp) + 1);

exit:
	if (temp)
		free(temp);

	return ret;
}

void organize(char* line, char* lhs, char* rhs, char c, map_t *map)
{
	unsigned int count = 0;
	char* temp = NULL;

	for (unsigned int i = 0; i < strlen(line); i++) {
		if (line[i] == c)
			count++;
	}

	switch (count) {
	case 1:
		temp = strchr(line, c);
		memcpy(lhs, line, strlen(line) - strlen(temp));
		memcpy(rhs, temp + 1, strlen(temp) - 1);
		break;
	default:
		memcpy(lhs, "Invalid", 8);
		memcpy(rhs, "Invalid", 8);
		break;
	}

	if (strpbrk(lhs, operators) != NULL) {
		memcpy(lhs, "Invalid", 8);
		memcpy(rhs, "Invalid", 8);
	}
}

unsigned int evaluate(map_t* map, char* var, char *rhs)
{
	unsigned int ret = SUCCESS;

	char* phs = NULL;
	MEM(phs, 80, char);
	syms_t C;

	//memcpy(rhs, get_symbol(map->table, var, map->size).data.expression, strlen(get_symbol(map->table, var, map->size).data.expression));
	// TODO: Evaluation of expressions with operator precedence.
	while (strcmp(rhs, phs) != 0) {
		C = get_symbol(map->table, rhs, map->size);

		if (!C.flag) {
			if (isnan(C.data.definite)) {
				if (isfloat(rhs)) {
					C.data.definite = strtod(rhs, NULL);
					C.flag = 0;
					set_symbol(&map->table, var, rhs, &map->size);
					goto exit;
				}
				else {
					sprintf(C.data.expression, "%s", rhs);
					C.flag = 1;
				}
			}
			else {
				set_symbol_double(&map->table, var, C.data.definite, &map->size);
				goto exit;
			}
		}
		else {
			sprintf(rhs, "%s", C.data.expression);
		}
		CHECK(ret = resolve_p(&rhs, *map));
		CHECK(ret = resolve_e(&rhs, *map));
		CHECK(ret = resolve_m(&rhs, *map));
		CHECK(ret = resolve_d(&rhs, *map));
		CHECK(ret = resolve_a(&rhs, *map));
		CHECK(ret = resolve_s(&rhs, *map));
		C = get_symbol(map->table, rhs, map->size);

		if (!C.flag) {
			if (isnan(C.data.definite)) {
				if (isfloat(rhs)) {
					C.data.definite = strtod(rhs, NULL);
					C.flag = 0;
					set_symbol(&map->table, var, rhs, &map->size);
					goto exit;
				}
				else {
					sprintf(C.data.expression, "%s", rhs);
					C.flag = 1;
				}
			}
			else {
				set_symbol_double(&map->table, var, C.data.definite, &map->size);
				goto exit;
			}
		}
		else {
			sprintf(rhs, "%s", C.data.expression);
		}
		memcpy(phs, rhs, strlen(rhs));
		set_symbol(&map->table, var, rhs, &map->size);
	}
	set_symbol(&map->table, var, rhs, &map->size);

exit:
	if (phs)
		free(phs);

	return ret;
}

static int compare_keys_pair(const void* va, const void* vb)
{
	const command_t* a = (command_t*)va, * b = (command_t*)vb;
	return strcmp(a->key, b->key);
}

unsigned int get_command(char* key)
{
	command_t key_pair[1] = { {key} };
	command_t* p = (command_t *)bsearch(key_pair, COMMANDS, COMMAND_L, sizeof(*COMMANDS), compare_keys_pair);
	return p ? p->val : 0;
}

void run(State_t* state)
{
	memcpy(state->tmp1, state->userin, strcspn(state->userin, " "));
	switch (get_command(state->tmp1)) {
		case 1:
			memcpy(state->tmp2, state->userin + strlen(state->tmp1) + 1, strlen(state->userin) - strlen(state->tmp1));
			if (!strcmp(state->tmp2, "h")) {
				for (unsigned int i = state->hmax - 1; i > 0; i--) {
					memset(state->history[i], '\0', 80);
				}
				state->hmax = 1;
				state->hindex = 0;
			}
			else if (!strcmp(state->tmp2, "t")) {
				destroy_map(&state->map);
				create_map(&state->map);
			}
			else {
				for (unsigned int i = state->hmax - 1; i > 0; i--) {
					memset(state->history[i], '\0', 80);
				}
				state->hmax = 1;
				state->hindex = 0;
				destroy_map(&state->map);
				create_map(&state->map);
			}
			memset(state->tmp1, '\0', 80);
			memset(state->tmp2, '\0', 80);
			return;
		case 2:
			state->q = 1;
			memset(state->tmp1, '\0', 80);
			return;
		case 3:
			memcpy(state->tmp2, state->userin + strlen(state->tmp1) + 1, strlen(state->userin) - strlen(state->tmp1));
			tokenize(state->tmp2, &state->map);
			memset(state->tmp1, '\0', 80);
			memset(state->tmp2, '\0', 80);
			return;
		default:
			eval(state->userin, &state->map);
			memset(state->tmp1, '\0', 80);
			return;
	}
}