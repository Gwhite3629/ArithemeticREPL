#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include "utils.h"

unsigned int resolve_p(char** rhs, map_t m);

unsigned int resolve_e(char** rhs, map_t m);

unsigned int resolve_m(char** rhs, map_t m);

unsigned int resolve_d(char** rhs, map_t m);

unsigned int resolve_a(char** rhs, map_t m);

unsigned int resolve_s(char** rhs, map_t m);

#endif