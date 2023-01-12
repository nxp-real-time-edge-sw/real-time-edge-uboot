// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 NXP
 */


#include <math.h>
#include <stdlib.h>
#include <common.h>
#include <command.h>
#undef log

int __errno;

typedef enum
{
	MATH_ACOS = 0,
	MATH_ASIN,
	MATH_ATAN,
	MATH_ATAN2,
	MATH_COS,
	MATH_COSH,
	MATH_SIN,
	MATH_SINH,
	MATH_TANH,
	MATH_EXP,
	MATH_LDEXP,
	MATH_LOG,
	MATH_LOG10,
	MATH_POW,
	MATH_SQRT,
	MATH_CEIL,
	MATH_FABS,
	MATH_FLOOR,
	MATH_FMOD
} e_math;

static double my_strtod(const char *str, char **ptr)
{
	int integer = 0, decimal = 0, bits;
	char *dotptr, *endptr;
	double num;

	integer = simple_strtol(str, &dotptr, 10);
	if (*dotptr == '.') {
		dotptr++;
		decimal = simple_strtol(dotptr, &endptr, 10);
		bits = endptr - dotptr;
		num = integer + decimal * pow(0.1, bits);
		if (!ptr)
			*ptr = endptr;
	} else {
		num = integer;
		if (!ptr)
			*ptr = dotptr;
	}
	return num;
}

static int math_func(e_math math_num, int num, int argc, char * const argv[])
{
	double x, y, res = 0;
	int integer, decimal, expo, flag;

	if (argc < num)
		return CMD_RET_FAILURE;

	x = my_strtod(argv[1], NULL);

	switch (math_num) {
	case MATH_ACOS:
		if ((x <= -1) || (x >= 1))
			return CMD_RET_FAILURE;
		res = acos(x);
		break;
	case MATH_ASIN:
		if ((x <= -1) || (x >= 1))
			return CMD_RET_FAILURE;
		res = asin(x);
		break;
	case MATH_ATAN:
		res = atan(x);
		break;
	case MATH_ATAN2:
		y = my_strtod(argv[2], NULL);
		res = atan2(x, y);
		break;
	case MATH_COS:
		res = cos(x);
		break;
	case MATH_COSH:
		res = cosh(x);
		break;
	case MATH_SIN:
		res = sin(x);
		break;
	case MATH_SINH:
		res = sinh(x);
		break;
	case MATH_TANH:
		res = tanh(x);
		break;
	case MATH_EXP:
		res = exp(x);
		break;
	case MATH_LDEXP:
		expo = simple_strtol(argv[2], NULL, 10);
		res = ldexp(x, expo);
		break;
	case MATH_LOG:
		res = log(x);
		break;
	case MATH_LOG10:
		res = log10(x);
		break;
	case MATH_POW:
		y = my_strtod(argv[2], NULL);
		res = pow(x, y);
		break;
	case MATH_SQRT:
		res = sqrt(x);
		break;
	case MATH_CEIL:
		res = ceil(x);
		break;
	case MATH_FABS:
		res = fabs(x);
		break;
	case MATH_FLOOR:
		res = floor(x);
		break;
	case MATH_FMOD:
		y = my_strtod(argv[2], NULL);
		res = fmod(x, y);
		break;
	}

	if (res < 0) {
		flag = -1;
		res = -res;
	} else {
		flag = 1;
	}

	integer = (int)res;
	decimal = (int)((res - integer) * 1000);
	if (flag == -1)
		printf("= -%d.%03d\n", integer, decimal);
	else
		printf("= %d.%03d\n", integer, decimal);

	return CMD_RET_SUCCESS;
}

static int do_acos(struct cmd_tbl *cmdtp, int flag,
		   int argc, char * const argv[])
{
	return math_func(MATH_ACOS, 2, argc, argv);
}

static int do_asin(struct cmd_tbl *cmdtp, int flag,
		   int argc, char * const argv[])
{
	return math_func(MATH_ASIN, 2, argc, argv);
}

static int do_atan(struct cmd_tbl *cmdtp, int flag,
		   int argc, char * const argv[])
{
	return math_func(MATH_ATAN, 2, argc, argv);
}

static int do_atan2(struct cmd_tbl *cmdtp, int flag,
		    int argc, char * const argv[])
{
	return math_func(MATH_ATAN2, 3, argc, argv);
}

static int do_cos(struct cmd_tbl *cmdtp, int flag,
		  int argc, char * const argv[])
{
	return math_func(MATH_COS, 2, argc, argv);
}

static int do_cosh(struct cmd_tbl *cmdtp, int flag,
		   int argc, char * const argv[])
{
	return math_func(MATH_COSH, 2, argc, argv);
}

static int do_sin(struct cmd_tbl *cmdtp, int flag,
		  int argc, char * const argv[])
{
	return math_func(MATH_SIN, 2, argc, argv);
}

static int do_sinh(struct cmd_tbl *cmdtp, int flag,
		   int argc, char * const argv[])
{
	return math_func(MATH_SINH, 2, argc, argv);
}

static int do_tanh(struct cmd_tbl *cmdtp, int flag,
		   int argc, char * const argv[])
{
	return math_func(MATH_TANH, 2, argc, argv);
}

static int do_exp(struct cmd_tbl *cmdtp, int flag,
		  int argc, char * const argv[])
{
	return math_func(MATH_EXP, 2, argc, argv);
}

static int do_ldexp(struct cmd_tbl *cmdtp, int flag,
		    int argc, char * const argv[])
{
	return math_func(MATH_LDEXP, 3, argc, argv);
}

static int do_log(struct cmd_tbl *cmdtp, int flag,
		  int argc, char * const argv[])
{
	return math_func(MATH_LOG, 2, argc, argv);
}

static int do_log10(struct cmd_tbl *cmdtp, int flag,
		    int argc, char * const argv[])
{
	return math_func(MATH_LOG10, 2, argc, argv);
}

static int do_pow(struct cmd_tbl *cmdtp, int flag,
		  int argc, char * const argv[])
{
	return math_func(MATH_POW, 3, argc, argv);
}

static int do_sqrt(struct cmd_tbl *cmdtp, int flag,
		   int argc, char * const argv[])
{
	return math_func(MATH_SQRT, 2, argc, argv);
}

static int do_ceil(struct cmd_tbl *cmdtp, int flag,
		  int argc, char * const argv[])
{
	return math_func(MATH_CEIL, 2, argc, argv);
}

static int do_fabs(struct cmd_tbl *cmdtp, int flag,
		   int argc, char * const argv[])
{
	return math_func(MATH_FABS, 2, argc, argv);
}

static int do_floor(struct cmd_tbl *cmdtp, int flag,
		    int argc, char * const argv[])
{
	return math_func(MATH_FLOOR, 2, argc, argv);
}

static int do_fmod(struct cmd_tbl *cmdtp, int flag,
		   int argc, char * const argv[])
{
	return math_func(MATH_FMOD, 3, argc, argv);
}

static struct cmd_tbl cmd_math[] = {
	U_BOOT_CMD_MKENT(acos, 2, 1, do_acos, "", ""),
	U_BOOT_CMD_MKENT(asin, 2, 1, do_asin, "", ""),
	U_BOOT_CMD_MKENT(atan, 2, 1, do_atan, "", ""),
	U_BOOT_CMD_MKENT(atan2, 3, 1, do_atan2, "", ""),
	U_BOOT_CMD_MKENT(cos, 2, 1, do_cos, "", ""),
	U_BOOT_CMD_MKENT(cosh, 2, 1, do_cosh, "", ""),
	U_BOOT_CMD_MKENT(sin, 2, 1, do_sin, "", ""),
	U_BOOT_CMD_MKENT(sinh, 2, 1, do_sinh, "", ""),
	U_BOOT_CMD_MKENT(tanh, 2, 1, do_tanh, "", ""),
	U_BOOT_CMD_MKENT(exp, 2, 1, do_exp, "", ""),
	U_BOOT_CMD_MKENT(ldexp, 3, 1, do_ldexp, "", ""),
	U_BOOT_CMD_MKENT(log, 2, 1, do_log, "", ""),
	U_BOOT_CMD_MKENT(log10, 2, 1, do_log10, "", ""),
	U_BOOT_CMD_MKENT(pow, 3, 1, do_pow, "", ""),
	U_BOOT_CMD_MKENT(sqrt, 2, 1, do_sqrt, "", ""),
	U_BOOT_CMD_MKENT(ceil, 2, 1, do_ceil, "", ""),
	U_BOOT_CMD_MKENT(fabs, 2, 1, do_fabs, "", ""),
	U_BOOT_CMD_MKENT(floor, 2, 1, do_floor, "", ""),
	U_BOOT_CMD_MKENT(fmod, 3, 1, do_fmod, "", ""),
};

static int do_math(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_math, ARRAY_SIZE(cmd_math));

	/* Drop the math command */
	argc--;
	argv++;

	if (cp == NULL || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	math, 4, 1, do_math,
	"Test Math Functions",
	"- Only test some simple math functions:\n"
	"  math acos x(double)\n"
	"  math asin x(double)\n"
	"  math atan x(double)\n"
	"  math atan2 y x(double)\n"
	"  math cos x(double)\n"
	"  math cosh x(double)\n"
	"  math sin x(double)\n"
	"  math sinh x(double)\n"
	"  math tanh x(double)\n"
	"  math exp x(double)\n"
	"  math ldexp x(double) exp(int)\n"
	"  math log x(double)\n"
	"  math log10 x(double)\n"
	"  math pow x(double) y(double)\n"
	"  math sqrt x(double)\n"
	"  math ceil x(double)\n"
	"  math fabs x(double)\n"
	"  math floor x(double)\n"
	"  math fmod x(double) y(double)\n"
);
