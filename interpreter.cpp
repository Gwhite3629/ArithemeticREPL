#include "interpreter.h"
#include "help.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

unsigned int resolve_p(char** rhs, map_t m)
{
	unsigned int ret = SUCCESS;
	unsigned int o_count = 0;
	unsigned int c_count = 0;
	unsigned int i = 0;
	char* temp = NULL;
	char* ths = NULL;
	char* phs = NULL;
	char* farleft = NULL;
	char* farright = NULL;

	MEM(farleft, 80, char);
	MEM(farright, 80, char);
	MEM(ths, 80, char);
	MEM(phs, 80, char);

	// Simple test for valid syntax

	for (i = 0; i < strlen((*rhs)); i++) {
		if ((*rhs)[i] == '(')
			o_count++;
	}

	for (i = 0; i < strlen((*rhs)); i++) {
		if ((*rhs)[i] == ')')
			c_count++;
	}

	if (o_count != c_count) {
		print_error(*rhs, "Mis-matched Parenthesis\n");
		goto exit;
	}

	while (c_count > 0) {
		temp = strpbrk((*rhs), ")");
		memcpy(farright, temp + 1, strlen(temp) - 1);
		while (temp && (temp[0] != '(')) {
			temp--;
		}
		if (temp == NULL) {
			print_error(*rhs, "Mismatched Parentheses\n");
			goto exit;
		}
		memcpy(farleft, (*rhs), strlen(*rhs) - strlen(temp));
		memcpy(ths, temp + 1, strlen(temp) - strlen(farright) - 2);
		while (strcmp(ths, phs) != 0) {
			syms_t C;

			C = get_symbol(m.table, ths, m.size);

			if (!C.flag) {
				if (isnan(C.data.definite)) {
					if (isfloat(ths)) {
						C.data.definite = strtod(ths, NULL);
						C.flag = 0;
						break;
					}
					else {
						sprintf(C.data.expression, "%s", ths);
						C.flag = 1;
					}
				}
				else {
					break;
				}
			}
			else {
				sprintf(ths, "%s", C.data.expression);
			}

			CHECK(ret = resolve_e(&ths, m));
			CHECK(ret = resolve_m(&ths, m));
			CHECK(ret = resolve_d(&ths, m));
			CHECK(ret = resolve_a(&ths, m));
			CHECK(ret = resolve_s(&ths, m));
			memcpy(phs, ths, strlen(ths));
		}
		sprintf((*rhs), "%s%s%s", farleft, ths, farright);
		memset(farleft, '\0', 80);
		memset(farright, '\0', 80);
		memset(ths, '\0', 80);
		c_count--;
	}

exit:

	if (farleft)
		free(farleft);
	if (farright)
		free(farright);
	if (ths)
		free(ths);
	if (phs)
		free(phs);

	return ret;
}

unsigned int resolve_e(char** rhs, map_t m)
{
	unsigned int ret = SUCCESS;
	unsigned int count = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int index = 0;
	char* temp;
	char* ltemp = NULL;
	char* rtemp = NULL;
	char symbol[80] = "\0";
	syms_t L = {NAN, 0};
	syms_t R = {NAN, 0};
	bool Ldigit = 1;
	bool Rdigit = 1;
	unsigned int max_lower = 0;
	unsigned int max_upper = 0;
	char* farleft = NULL;
	char* farright = NULL;
	unsigned int offset = 0;

	MEM(farleft, 80, char);
	MEM(farright, 80, char);
	MEM(ltemp, 80, char);
	MEM(rtemp, 80, char);

	for (i = 0; i < strlen(*rhs); i++) {
		if ((*rhs)[i] == '^')
			count++;
	}
	if (count == 0)
		return ret;
	while (j < count) {
		// Checks if the character before or after the exponent operator is another operator.
		temp = strchr((* rhs) + offset, '^');
		if (strrchr(operators, (temp - 1)[0]) != NULL) {
			print_error(*rhs, "Operator without LHS argument\n");
			goto exit;
		}
		else if (strrchr(operators, (temp + 1)[0]) != NULL) {
			print_error(*rhs, "Operator without RHS argument\n");
			goto exit;
		}

		max_lower = (unsigned int)(strlen(*rhs) - strlen(temp));

		Ldigit = 1;
		// Get lefthand symbol.
		unsigned int index = 0;
		for (i = 1; i <= max_lower; i++) {
			if (strrchr(operators, (temp - i)[0]) == NULL) {
				memcpy(ltemp, symbol, strlen(symbol));
				sprintf(symbol, "%c%s", (temp - i)[0], ltemp);
				if (isdigit((temp - i)[0]) == 0) {
					Ldigit = 0;
				}
			}
			else {
				break;
			}
		}
		memcpy(farleft, *rhs, max_lower - strlen(symbol));
		if (Ldigit) {
			L.data.definite = strtod(symbol, NULL);
			L.flag = 0;
		}
		else {
			L = get_symbol(m.table, symbol, m.size);
			if (!L.flag) {
				Ldigit = 1;
				if (isnan(L.data.definite)) {
					if (isfloat(symbol)) {
						L.data.definite = strtod(symbol, NULL);
						L.flag = 0;
						Ldigit = 1;
					}
					else {
						sprintf(L.data.expression, "%s", symbol);
						L.flag = 1;
						Ldigit = 0;
					}
				}
			}
		}

		max_upper = (unsigned int)strlen(temp);

		sprintf(symbol, "\0");

		Rdigit = 1;
		// Get righthand symbol.
		for (i = 1; i < max_upper; i++) {
			if (strrchr(operators, (temp + i)[0]) == NULL) {
				memcpy(rtemp, symbol, strlen(symbol));
				sprintf(symbol, "%s%c", rtemp, (temp + i)[0]);
				if (isdigit((temp + i)[0]) == 0) {
					Rdigit = 0;
				}
			}
			else {
				break;
			}
		}
		memcpy(farright, temp + strlen(symbol) + 1, strlen(temp) - 1 - strlen(symbol));
		if (Rdigit) {
			R.data.definite = strtod(symbol, NULL);
			R.flag = 0;
		}
		else {
			R = get_symbol(m.table, symbol, m.size);
			if (!R.flag) {
				Rdigit = 1;
				if (isnan(R.data.definite)) {
					if (isfloat(symbol)) {
						R.data.definite = strtod(symbol, NULL);
						R.flag = 0;
						Rdigit = 1;
					}
					else {
						sprintf(R.data.expression, "%s", symbol);
						R.flag = 1;
						Rdigit = 0;
					}
				}
			}
		}

		if (Ldigit && Rdigit) {
			double r = pow(L.data.definite, R.data.definite);
			sprintf(*rhs, "%s%lf%s", farleft, r, farright);
		}
		else if (Ldigit && !Rdigit) {
			sprintf(*rhs, "%s%lf^(%s)%s", farleft, L.data.definite, R.data.expression, farright);
		}
		else if (!Ldigit && Rdigit) {
			sprintf(*rhs, "%s(%s)^%lf%s", farleft, L.data.expression, R.data.definite, farright);
		}
		else {
			sprintf(*rhs, "%s(%s)^(%s)%s", farleft, L.data.expression, R.data.expression, farright);
		}
		offset = (unsigned int)(strlen(*rhs) - strlen(farright));
		sprintf(symbol, "\0");
		memset(farright, '\0', 80);
		memset(farleft, '\0', 80);
		j++;
	}

exit:
	if (farleft)
		free(farleft);
	if (farright)
		free(farright);
	if (ltemp)
		free(ltemp);
	if (rtemp)
		free(rtemp);

	return ret;
}

unsigned int resolve_m(char** rhs, map_t m)
{
	unsigned int ret = SUCCESS;
	unsigned int count = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int index = 0;
	char* temp;
	char* ltemp = NULL;
	char* rtemp = NULL;
	char symbol[80] = "\0";
	syms_t L = { NAN, 0 };
	syms_t R = { NAN, 0 };
	bool Ldigit = 1;
	bool Rdigit = 1;
	unsigned int max_lower = 0;
	unsigned int max_upper = 0;
	char* farleft = NULL;
	char* farright = NULL;
	unsigned int offset = 0;

	MEM(farleft, 80, char);
	MEM(farright, 80, char);
	MEM(ltemp, 80, char);
	MEM(rtemp, 80, char);

	for (i = 0; i < strlen(*rhs); i++) {
		if ((*rhs)[i] == '*')
			count++;
	}
	if (count == 0)
		return ret;
	while (j < count) {
		// Checks if the character before or after the exponent operator is another operator.
		temp = strchr((*rhs) + offset, '*');
		if (strrchr(operators, (temp - 1)[0]) != NULL) {
			print_error(*rhs, "Operator without LHS argument\n");
			goto exit;
		}
		else if (strrchr(operators, (temp + 1)[0]) != NULL) {
			print_error(*rhs, "Operator without RHS argument\n");
			goto exit;
		}

		max_lower = (unsigned int)(strlen(*rhs) - strlen(temp));

		Ldigit = 1;
		// Get lefthand symbol.
		unsigned int index = 0;
		for (i = 1; i <= max_lower; i++) {
			if (strrchr(operators, (temp - i)[0]) == NULL) {
				memcpy(ltemp, symbol, strlen(symbol));
				sprintf(symbol, "%c%s", (temp - i)[0], ltemp);
				if (isdigit((temp - i)[0]) == 0) {
					Ldigit = 0;
				}
			}
			else {
				break;
			}
		}
		memcpy(farleft, *rhs, max_lower - strlen(symbol));
		if (Ldigit) {
			L.data.definite = strtod(symbol, NULL);
			L.flag = 0;
		}
		else {
			L = get_symbol(m.table, symbol, m.size);
			if (!L.flag) {
				Ldigit = 1;
				if (isnan(L.data.definite)) {
					if (isfloat(symbol)) {
						L.data.definite = strtod(symbol, NULL);
						L.flag = 0;
						Ldigit = 1;
					}
					else {
						sprintf(L.data.expression, "%s", symbol);
						L.flag = 1;
						Ldigit = 0;
					}
				}
			}
		}

		max_upper = (unsigned int)strlen(temp);

		sprintf(symbol, "\0");

		Rdigit = 1;
		// Get righthand symbol.
		for (i = 1; i < max_upper; i++) {
			if (strrchr(operators, (temp + i)[0]) == NULL) {
				memcpy(rtemp, symbol, strlen(symbol));
				sprintf(symbol, "%s%c", rtemp, (temp + i)[0]);
				if (isdigit((temp + i)[0]) == 0) {
					Rdigit = 0;
				}
			}
			else {
				break;
			}
		}
		memcpy(farright, temp + strlen(symbol) + 1, strlen(temp) - 1 - strlen(symbol));
		if (Rdigit) {
			R.data.definite = strtod(symbol, NULL);
			R.flag = 0;
		}
		else {
			R = get_symbol(m.table, symbol, m.size);
			if (!R.flag) {
				Rdigit = 1;
				if (isnan(R.data.definite)) {
					if (isfloat(symbol)) {
						R.data.definite = strtod(symbol, NULL);
						R.flag = 0;
						Rdigit = 1;
					}
					else {
						sprintf(R.data.expression, "%s", symbol);
						R.flag = 1;
						Rdigit = 0;
					}
				}
			}
		}

		if (Ldigit && Rdigit) {
			double r = L.data.definite * R.data.definite;
			sprintf(*rhs, "%s%lf%s", farleft, r, farright);
		}
		else if (Ldigit && !Rdigit) {
			sprintf(*rhs, "%s%lf*(%s)%s", farleft, L.data.definite, R.data.expression, farright);
		}
		else if (!Ldigit && Rdigit) {
			sprintf(*rhs, "%s(%s)*%lf%s", farleft, L.data.expression, R.data.definite, farright);
		}
		else {
			sprintf(*rhs, "%s(%s)*(%s)%s", farleft, L.data.expression, R.data.expression, farright);
		}
		offset = (unsigned int)(strlen(*rhs) - strlen(farright));
		sprintf(symbol, "\0");
		memset(farright, '\0', 80);
		memset(farleft, '\0', 80);
		j++;
	}

exit:
	if (farleft)
		free(farleft);
	if (farright)
		free(farright);
	if (ltemp)
		free(ltemp);
	if (rtemp)
		free(rtemp);

	return ret;
}

unsigned int resolve_d(char** rhs, map_t m)
{
	unsigned int ret = SUCCESS;
	unsigned int count = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int index = 0;
	char* temp;
	char* ltemp = NULL;
	char* rtemp = NULL;
	char symbol[80] = "\0";
	syms_t L = { NAN, 0 };
	syms_t R = { NAN, 0 };
	bool Ldigit = 1;
	bool Rdigit = 1;
	unsigned int max_lower = 0;
	unsigned int max_upper = 0;
	char* farleft = NULL;
	char* farright = NULL;
	unsigned int offset = 0;

	MEM(farleft, 80, char);
	MEM(farright, 80, char);
	MEM(ltemp, 80, char);
	MEM(rtemp, 80, char);

	for (i = 0; i < strlen(*rhs); i++) {
		if ((*rhs)[i] == '/')
			count++;
	}
	if (count == 0)
		return ret;
	while (j < count) {
		// Checks if the character before or after the exponent operator is another operator.
		temp = strchr((*rhs) + offset, '/');
		if (strrchr(operators, (temp - 1)[0]) != NULL) {
			print_error(*rhs, "Operator without LHS argument\n");
			goto exit;
		}
		else if (strrchr(operators, (temp + 1)[0]) != NULL) {
			print_error(*rhs, "Operator without RHS argument\n");
			goto exit;
		}

		max_lower = (unsigned int)(strlen(*rhs) - strlen(temp));

		Ldigit = 1;
		// Get lefthand symbol.
		unsigned int index = 0;
		for (i = 1; i <= max_lower; i++) {
			if (strrchr(operators, (temp - i)[0]) == NULL) {
				memcpy(ltemp, symbol, strlen(symbol));
				sprintf(symbol, "%c%s", (temp - i)[0], ltemp);
				if (isdigit((temp - i)[0]) == 0) {
					Ldigit = 0;
				}
			}
			else {
				break;
			}
		}
		memcpy(farleft, *rhs, max_lower - strlen(symbol));
		if (Ldigit) {
			L.data.definite = strtod(symbol, NULL);
			L.flag = 0;
		}
		else {
			L = get_symbol(m.table, symbol, m.size);
			if (!L.flag) {
				Ldigit = 1;
				if (isnan(L.data.definite)) {
					if (isfloat(symbol)) {
						L.data.definite = strtod(symbol, NULL);
						L.flag = 0;
						Ldigit = 1;
					}
					else {
						sprintf(L.data.expression, "%s", symbol);
						L.flag = 1;
						Ldigit = 0;
					}
				}
			}
		}

		max_upper = (unsigned int)strlen(temp);

		sprintf(symbol, "\0");

		Rdigit = 1;
		// Get righthand symbol.
		for (i = 1; i < max_upper; i++) {
			if (strrchr(operators, (temp + i)[0]) == NULL) {
				memcpy(rtemp, symbol, strlen(symbol));
				sprintf(symbol, "%s%c", rtemp, (temp + i)[0]);
				if (isdigit((temp + i)[0]) == 0) {
					Rdigit = 0;
				}
			}
			else {
				break;
			}
		}
		memcpy(farright, temp + strlen(symbol) + 1, strlen(temp) - 1 - strlen(symbol));
		if (Rdigit) {
			R.data.definite = strtod(symbol, NULL);
			R.flag = 0;
		}
		else {
			R = get_symbol(m.table, symbol, m.size);
			if (!R.flag) {
				Rdigit = 1;
				if (isnan(R.data.definite)) {
					if (isfloat(symbol)) {
						R.data.definite = strtod(symbol, NULL);
						R.flag = 0;
						Rdigit = 1;
					}
					else {
						sprintf(R.data.expression, "%s", symbol);
						R.flag = 1;
						Rdigit = 0;
					}
				}
			}
		}

		if (Ldigit && Rdigit) {
			double r = L.data.definite / R.data.definite;
			sprintf(*rhs, "%s%lf%s", farleft, r, farright);
		}
		else if (Ldigit && !Rdigit) {
			sprintf(*rhs, "%s%lf/(%s)%s", farleft, L.data.definite, R.data.expression, farright);
		}
		else if (!Ldigit && Rdigit) {
			sprintf(*rhs, "%s(%s)/%lf%s", farleft, L.data.expression, R.data.definite, farright);
		}
		else {
			sprintf(*rhs, "%s(%s)/(%s)%s", farleft, L.data.expression, R.data.expression, farright);
		}
		offset = (unsigned int)(strlen(*rhs) - strlen(farright));
		sprintf(symbol, "\0");
		memset(farright, '\0', 80);
		memset(farleft, '\0', 80);
		j++;
	}

exit:
	if (farleft)
		free(farleft);
	if (farright)
		free(farright);
	if (ltemp)
		free(ltemp);
	if (rtemp)
		free(rtemp);

	return ret;
}

unsigned int resolve_a(char** rhs, map_t m)
{
	unsigned int ret = SUCCESS;
	unsigned int count = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int index = 0;
	char* temp;
	char* ltemp = NULL;
	char* rtemp = NULL;
	char symbol[80] = "\0";
	syms_t L = { NAN, 0 };
	syms_t R = { NAN, 0 };
	bool Ldigit = 1;
	bool Rdigit = 1;
	unsigned int max_lower = 0;
	unsigned int max_upper = 0;
	char* farleft = NULL;
	char* farright = NULL;
	unsigned int offset = 0;

	MEM(farleft, 80, char);
	MEM(farright, 80, char);
	MEM(ltemp, 80, char);
	MEM(rtemp, 80, char);

	for (i = 0; i < strlen(*rhs); i++) {
		if ((*rhs)[i] == '+')
			count++;
	}
	if (count == 0)
		return ret;
	while (j < count) {
		// Checks if the character before or after the exponent operator is another operator.
		temp = strchr((*rhs) + offset, '+');
		if (strrchr(operators, (temp - 1)[0]) != NULL) {
			print_error(*rhs, "Operator without LHS argument\n");
			goto exit;
		}
		else if (strrchr(operators, (temp + 1)[0]) != NULL) {
			print_error(*rhs, "Operator without RHS argument\n");
			goto exit;
		}

		max_lower = (unsigned int)(strlen(*rhs) - strlen(temp));

		Ldigit = 1;
		// Get lefthand symbol.
		unsigned int index = 0;
		for (i = 1; i <= max_lower; i++) {
			if (strrchr(operators, (temp - i)[0]) == NULL) {
				memcpy(ltemp, symbol, strlen(symbol));
				sprintf(symbol, "%c%s", (temp - i)[0], ltemp);
				if (isdigit((temp - i)[0]) == 0) {
					Ldigit = 0;
				}
			}
			else {
				break;
			}
		}
		memcpy(farleft, *rhs, max_lower - strlen(symbol));
		if (Ldigit) {
			L.data.definite = strtod(symbol, NULL);
			L.flag = 0;
		}
		else {
			L = get_symbol(m.table, symbol, m.size);
			if (!L.flag) {
				Ldigit = 1;
				if (isnan(L.data.definite)) {
					if (isfloat(symbol)) {
						L.data.definite = strtod(symbol, NULL);
						L.flag = 0;
						Ldigit = 1;
					}
					else {
						sprintf(L.data.expression, "%s", symbol);
						L.flag = 1;
						Ldigit = 0;
					}
				}
			}
		}

		max_upper = (unsigned int)strlen(temp);

		sprintf(symbol, "\0");

		Rdigit = 1;
		// Get righthand symbol.
		for (i = 1; i < max_upper; i++) {
			if (strrchr(operators, (temp + i)[0]) == NULL) {
				memcpy(rtemp, symbol, strlen(symbol));
				sprintf(symbol, "%s%c", rtemp, (temp + i)[0]);
				if (isdigit((temp + i)[0]) == 0) {
					Rdigit = 0;
				}
			}
			else {
				break;
			}
		}
		memcpy(farright, temp + strlen(symbol) + 1, strlen(temp) - 1 - strlen(symbol));
		if (Rdigit) {
			R.data.definite = strtod(symbol, NULL);
			R.flag = 0;
		}
		else {
			R = get_symbol(m.table, symbol, m.size);
			if (!R.flag) {
				Rdigit = 1;
				if (isnan(R.data.definite)) {
					if (isfloat(symbol)) {
						R.data.definite = strtod(symbol, NULL);
						R.flag = 0;
						Rdigit = 1;
					}
					else {
						sprintf(R.data.expression, "%s", symbol);
						R.flag = 1;
						Rdigit = 0;
					}
				}
			}
		}

		if (Ldigit && Rdigit) {
			double r = L.data.definite + R.data.definite;
			sprintf(*rhs, "%s%lf%s", farleft, r, farright);
		}
		else if (Ldigit && !Rdigit) {
			sprintf(*rhs, "%s%lf+(%s)%s", farleft, L.data.definite, R.data.expression, farright);
		}
		else if (!Ldigit && Rdigit) {
			sprintf(*rhs, "%s(%s)+%lf%s", farleft, L.data.expression, R.data.definite, farright);
		}
		else {
			sprintf(*rhs, "%s(%s)+(%s)%s", farleft, L.data.expression, R.data.expression, farright);
		}
		offset = (unsigned int)(strlen(*rhs) - strlen(farright));
		sprintf(symbol, "\0");
		memset(farright, '\0', 80);
		memset(farleft, '\0', 80);
		j++;
	}

exit:
	if (farleft)
		free(farleft);
	if (farright)
		free(farright);
	if (ltemp)
		free(ltemp);
	if (rtemp)
		free(rtemp);

	return ret;
}

unsigned int resolve_s(char** rhs, map_t m)
{
	unsigned int ret = SUCCESS;
	unsigned int count = 0;
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int index = 0;
	char* temp;
	char* ltemp = NULL;
	char* rtemp = NULL;
	char symbol[80] = "\0";
	syms_t L = { NAN, 0 };
	syms_t R = { NAN, 0 };
	bool Ldigit = 1;
	bool Rdigit = 1;
	unsigned int max_lower = 0;
	unsigned int max_upper = 0;
	char* farleft = NULL;
	char* farright = NULL;
	unsigned int offset = 0;

	MEM(farleft, 80, char);
	MEM(farright, 80, char);
	MEM(ltemp, 80, char);
	MEM(rtemp, 80, char);

	for (i = 0; i < strlen(*rhs); i++) {
		if ((*rhs)[i] == '-')
			count++;
	}
	if (count == 0)
		return ret;
	while (j < count) {
		// Checks if the character before or after the exponent operator is another operator.
		temp = strchr((*rhs) + offset, '-');
		if (strrchr(operators, (temp - 1)[0]) != NULL) {
			print_error(*rhs, "Operator without LHS argument\n");
			goto exit;
		}
		else if (strrchr(operators, (temp + 1)[0]) != NULL) {
			print_error(*rhs, "Operator without RHS argument\n");
			goto exit;
		}

		max_lower = (unsigned int)(strlen(*rhs) - strlen(temp));

		Ldigit = 1;
		// Get lefthand symbol.
		unsigned int index = 0;
		for (i = 1; i <= max_lower; i++) {
			if (strrchr(operators, (temp - i)[0]) == NULL) {
				memcpy(ltemp, symbol, strlen(symbol));
				sprintf(symbol, "%c%s", (temp - i)[0], ltemp);
				if (isdigit((temp - i)[0]) == 0) {
					Ldigit = 0;
				}
			}
			else {
				break;
			}
		}
		memcpy(farleft, *rhs, max_lower - strlen(symbol));
		if (Ldigit) {
			L.data.definite = strtod(symbol, NULL);
			L.flag = 0;
		}
		else {
			L = get_symbol(m.table, symbol, m.size);
			if (!L.flag) {
				Ldigit = 1;
				if (isnan(L.data.definite)) {
					if (isfloat(symbol)) {
						L.data.definite = strtod(symbol, NULL);
						L.flag = 0;
						Ldigit = 1;
					}
					else {
						sprintf(L.data.expression, "%s", symbol);
						L.flag = 1;
						Ldigit = 0;
					}
				}
			}
		}

		max_upper = (unsigned int)strlen(temp);

		sprintf(symbol, "\0");

		Rdigit = 1;
		// Get righthand symbol.
		for (i = 1; i < max_upper; i++) {
			if (strrchr(operators, (temp + i)[0]) == NULL) {
				memcpy(rtemp, symbol, strlen(symbol));
				sprintf(symbol, "%s%c", rtemp, (temp + i)[0]);
				if (isdigit((temp + i)[0]) == 0) {
					Rdigit = 0;
				}
			}
			else {
				break;
			}
		}
		memcpy(farright, temp + strlen(symbol) + 1, strlen(temp) - 1 - strlen(symbol));
		if (Rdigit) {
			R.data.definite = strtod(symbol, NULL);
			R.flag = 0;
		}
		else {
			R = get_symbol(m.table, symbol, m.size);
			if (!R.flag) {
				Rdigit = 1;
				if (isnan(R.data.definite)) {
					if (isfloat(symbol)) {
						R.data.definite = strtod(symbol, NULL);
						R.flag = 0;
						Rdigit = 1;
					}
					else {
						sprintf(R.data.expression, "%s", symbol);
						R.flag = 1;
						Rdigit = 0;
					}
				}
			}
		}

		if (Ldigit && Rdigit) {
			double r = L.data.definite - R.data.definite;
			sprintf(*rhs, "%s%lf%s", farleft, r, farright);
		}
		else if (Ldigit && !Rdigit) {
			sprintf(*rhs, "%s%lf-(%s)%s", farleft, L.data.definite, R.data.expression, farright);
		}
		else if (!Ldigit && Rdigit) {
			sprintf(*rhs, "%s(%s)-%lf%s", farleft, L.data.expression, R.data.definite, farright);
		}
		else {
			sprintf(*rhs, "%s(%s)-(%s)%s", farleft, L.data.expression, R.data.expression, farright);
		}
		offset = (unsigned int)(strlen(*rhs) - strlen(farright));
		sprintf(symbol, "\0");
		memset(farright, '\0', 80);
		memset(farleft, '\0', 80);
		j++;
	}

exit:
	if (farleft)
		free(farleft);
	if (farright)
		free(farright);
	if (ltemp)
		free(ltemp);
	if (rtemp)
		free(rtemp);

	return ret;
}