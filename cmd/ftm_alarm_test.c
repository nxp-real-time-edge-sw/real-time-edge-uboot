// SPDX-License-Identifier: GPL-2.0+
/*
 * Command for Freescale FlexTimer Module (FTM) alarm device driver verification.
 *
 * Copyright 2023 NXP
 *
 */

#include <config.h>
#include <command.h>
#include <common.h>
#include <malloc.h>
#include <env.h>
#include <linux/types.h>
#include <dm.h>
#include <timer.h>
#include <linux/delay.h>

#include <ftm_alarm.h>

#define TEST_LEN    50

unsigned long max_time;
unsigned long min_time;
unsigned long irq_count;
unsigned long pre_time;
unsigned long first_time;
unsigned long last_time;
bool start_flg = false;
struct udevice *dev;


char * compatible_name = "fsl,ls1046a-ftm-alarm";

static void do_ftm_show(void)
{
	unsigned long total_time = last_time - first_time;
	unsigned long avg_time = total_time / (irq_count - 1);
	printf("%-15s | %-15s | %-15s | %-15s | %-15s |\n",
			"irq count", "total (us)", "average (us)", "max (us)", "min (us)");
	printf("%-15lu | %-15lu | %-15lu | %-15lu | %-15lu |\n",
			irq_count, total_time, avg_time, max_time, min_time);
	return;
}

void timer_isr_handler(void *arg)
{
	unsigned long interval_time;

	last_time = timer_get_us();

	if (first_time == 0)
		first_time = last_time;

	if (pre_time != 0) {
		interval_time = last_time - pre_time;
		if (max_time < interval_time)
			max_time = interval_time;

		if (min_time > interval_time || min_time == 0)
			min_time = interval_time;
	}
	pre_time = last_time;

	irq_count++;
	return;
}

static int do_ftm_start(int argc, char * const argv[])
{
	unsigned long time_us = 5;
	unsigned long input_value;
	unsigned long max_alarm_us;
	int ret;

	if (start_flg) {
		printf("FTM test already started.\n");
		return CMD_RET_SUCCESS;
	}

	if (argc == 2) {
		printf("Use default alarm time - %lu us\n", time_us);
	} else if (argc == 3) {
		input_value = simple_strtoul(argv[2], NULL, 10);
		max_alarm_us = ftm_rtc_get_max_alarm_us(dev);
		if (input_value > max_alarm_us) {
			printf("Para error, the max alarm time: %lu us\n", max_alarm_us);
			return CMD_RET_FAILURE;
		}
		time_us = input_value;
	} else {
		return CMD_RET_USAGE;
	}

	first_time = 0;
	last_time = 0;
	max_time = 0;
	min_time = 0;
	pre_time = 0;
	irq_count = 0;

	ret = ftm_rtc_set_alarm_by_us(dev, time_us, timer_isr_handler);
	if (ret) {
		printf("FTM set alarm error.\n");
		return CMD_RET_FAILURE;
	}

	start_flg = true;

	printf("FTM test start.\n");
    return CMD_RET_SUCCESS;
}

void do_ftm_stop(void)
{
	ftm_rtc_alarm_stop(dev);

	start_flg = false;

	printf("FTM test stop.\n");
	do_ftm_show();
	return;
}

static int
get_alarm_idx(void)
{
	struct uclass *uc;
	struct udevice *dev;
	int target_idx = -1, idx = 0;

	uclass_id_foreach_dev(UCLASS_RTC, dev, uc) {
		if (!strcmp(dev_read_string(dev, "compatible"), compatible_name)) {
			target_idx = idx;
			break;
		}
		idx++;
	}
	return target_idx;
}

static int
ftm_cmd(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	int ret, idx = 0;

	if (argc < 2) {
		return CMD_RET_USAGE;
	}

	idx = get_alarm_idx();
	ret = uclass_get_device(UCLASS_RTC, idx, &dev);
	if (ret) {
		printf("Cannot find RTC #%d, err=%d\n", idx, ret);
		return CMD_RET_FAILURE;
	}

	if (!strcmp(argv[1], "show")) {
		do_ftm_show();
	} else if (!strcmp(argv[1], "start")) {
		return do_ftm_start(argc, argv);
	} else if (!strcmp(argv[1], "stop")) {
		do_ftm_stop();
	} else
		return CMD_RET_USAGE;

	return CMD_RET_SUCCESS;
}

static char ftm_help_text[] =
	"test ftm alarm\n"
	"show				- show FTM test result\n"
	"start [count] us		- start FTM test\n"
	"stop				- stop FTM test\n"
	"";


U_BOOT_CMD(ftm, CONFIG_SYS_MAXARGS, 0, ftm_cmd,
	"ftm alarm test", ftm_help_text
);
