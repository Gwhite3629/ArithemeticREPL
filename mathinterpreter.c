#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "utils.h"

int main(int argc, char* argv[])
{
	unsigned int ret = SUCCESS;
	double result = -1;

	map_t map;
	map.size = 1;

	MEM(map.table, map.size, symbol_t);

	map.table->key = "ans";
	map.table->val.flag = 0;
	map.table->val.data.definite = 0;

	if (argc < 2) {
		printf("Usage: %s filename\n", argv[0]);
		return -1;
	}

	//set_symbol_double(&map.table, "a", 5, &map.size);
	//set_symbol_expression(&map.table, "b", "1+2", &map.size);
	//set_symbol_expression(&map.table, "c", "a+b", &map.size);

	tokenize(argv[1], &map);

	print_table(map.table, map.size);

	printf("Finished successfully with size: %d\n", map.size);

exit:
	if (map.table)
		free(map.table);

	return ret;
}