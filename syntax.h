#ifndef _SYNTAX_H_
#define _SYNTAX_H_

#include "utils.h"

typedef struct command {
	const char* key;
	unsigned int val;
} command_t;

unsigned int tokenize(char* filename, map_t* m);

unsigned int eval(char* line, map_t* m);

unsigned int trim(char* line);

void organize(char* line, char* lhs, char* rhs, char c, map_t* map);

unsigned int evaluate(map_t* m, char* lhs, char *rhs);

unsigned int get_command(char* key);

void run(State_t* state);

#endif