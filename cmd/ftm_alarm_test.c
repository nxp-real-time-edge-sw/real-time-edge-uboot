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

unsigned long pre_time;
unsigned long avg_time;
unsigned long max_time;
unsigned long min_time;
unsigned long irq_count;
struct udevice *dev;


char * compatible_name = "fsl,ls1046a-ftm-alarm";

static void do_ftm_show(struct udevice *dev)
{
	printf("%-15s | %-15s | %-15s | %-15s\n", "irq count", "average (us)", "max (us)", "min (us)");
	printf("%-15lu | %-15lu | %-15lu | %-15lu\n", irq_count, avg_time, max_time, min_time);
	return;
}

void timer_isr_handler(void *arg)
{
	unsigned long curr_time = timer_get_us();
	unsigned long interval_time;

	if (pre_time != 0) {
		interval_time = curr_time - pre_time;
		if (max_time < interval_time)
			max_time = interval_time;

		if (min_time > interval_time || min_time == 0)
			min_time = interval_time;

		avg_time = (avg_time * (irq_count - 1) + interval_time) / (irq_count);
	}
	pre_time = curr_time;

	irq_count++;
	return;
}

void do_ftm_start(struct udevice *dev)
{
	if (irq_count != 0) {
		printf("FTM test already started.\n");
		return;
	}

	ftm_rtc_set_alarm(dev, 2000, timer_isr_handler);

	printf("FTM test start.\n");
    return;
}

void do_ftm_stop(struct udevice *dev)
{
	ftm_rtc_alarm_stop(dev);

	avg_time = 0;
	max_time = 0;
	min_time = 0;
	irq_count = 0;
	pre_time = 0;

	printf("FTM test stop.\n");
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
		do_ftm_show(dev);
	} else if (!strcmp(argv[1], "start")) {
		do_ftm_start(dev);
	} else if (!strcmp(argv[1], "stop")) {
		do_ftm_stop(dev);
	} else
		return CMD_RET_USAGE;

	return 0;
}

static char ftm_help_text[] =
	"test ftm alarm\n"
	"show		- show FTM test result\n"
	"start		- start FTM test\n"
	"stop		- stop FTM test\n"
	"";


U_BOOT_CMD(ftm, CONFIG_SYS_MAXARGS, 0, ftm_cmd,
	"ftm alarm test", ftm_help_text
);
