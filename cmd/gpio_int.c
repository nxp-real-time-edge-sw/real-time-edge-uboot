// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright 2023 NXP
 *
 * Changming Huang <jerry.huang@nxp.com>
 *
 * This file is the demo to generate GPIO interrupt on Core1,
 * And send SGI8 interrupt to Core2 and Core3 in ISR.
 * RCW:
 * RCW[382 ~ 383] = 0b'10: Enable GPIO2[0 ~ 3]
 * Hardware: LS1046ARDB:
 * 	     GPIO2_01 - Output
 * 	     GPIO2_02 - Input
 * Connect pin GPIO2_01 and GPIO2_02 together
 * TP14 - GPIO2_01
 * TP13 - GPIO2_02
 * GPIO2_02 will get the value of GPIO2_01
 * GPIO2 (INT num 99) will generate interrupt when GPIO2_02 is high to low)
 *
 * In Linux kernel file fsl-ls1046a-rdb-sdk-bm.dts, disable gpio1
 * +&gpio1 {
 * +       status = "disabled";
 * +};
 * +
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <time.h>
#include <asm/gpio.h>
#include <linux/err.h>
#include <asm/io.h>
#include <asm/interrupt-gic.h>
#include <cpu_func.h>
#include <irq_func.h>
#include <inter-core-comm.h>

#define GPIO2_ADDR	0x02310000
#define GPIO2_DIR	(GPIO2_ADDR + 0x00)
#define GPIO2_ODR	(GPIO2_ADDR + 0x04)
#define GPIO2_DATA	(GPIO2_ADDR + 0x08)
#define GPIO2_INT_EVENT	(GPIO2_ADDR + 0x0C)
#define GPIO2_INT_MASK	(GPIO2_ADDR + 0x10)
#define GPIO2_INT_CTL	(GPIO2_ADDR + 0x14)

#define GPIO2_00_OFFSET	31
#define GPIO2_01_OFFSET	30
#define GPIO2_02_OFFSET	29
#define GPIO2_01_OUTPUT	1

#define GPIO2_INT_NUM	99

#define CORE1_MASK	(1 << 1)
#define CORE2_MASK	(1 << 2)
#define CORE3_MASK	(1 << 3)
#define SGI8_NUM	8

static void gpio2_irq(int hw_irq, int src_coreid)
{
	u32 val;
	unsigned long time_us = timer_get_us();
	void *gpio2_event = (void *)GPIO2_INT_EVENT;
	u32 dest_core = (CORE2_MASK | CORE3_MASK);

	val = in_be32((u32 *)gpio2_event);
	if (val & (1 << GPIO2_02_OFFSET)) {
		out_be32(gpio2_event, val);
		icc_dump_time(dest_core);
	}
	printf("Time(us): 0x%lx, GPIO event: 0x%x\n", time_us, val);
}


static void gpio_register_irq(u32 coreid, u32 hw_irq)
{
	gic_irq_register(hw_irq, gpio2_irq);
	gic_set_type(hw_irq);
	gic_set_target(1 << coreid, hw_irq);
	enable_interrupts();
}

static void gpio2_setup_enable(void)
{
	void *gpio2_dir;
	void *gpio2_int_mask;
	void *gpio2_int_ctl;
	u32 val;
	u32 coreid = 1;

	/* Set GPIO2_01 to output */
	gpio2_dir = (void *)GPIO2_DIR;
	val = in_be32((u32 *)gpio2_dir);
	val |= (GPIO2_01_OUTPUT << GPIO2_01_OFFSET);
	out_be32(gpio2_dir, val);

	/* Set GPIO2_02 interrupt mask */
	gpio2_int_mask = (void *)GPIO2_INT_MASK;
	val = in_be32((u32 *)gpio2_int_mask);
	val |= (1 << GPIO2_02_OFFSET);
	out_be32(gpio2_int_mask, val);

	/* Set GPIO2_02 interrupt: High-to-low change */
	gpio2_int_ctl = (void *)GPIO2_INT_CTL;
	val = in_be32((u32 *)gpio2_int_ctl);
	val |= (1 << GPIO2_02_OFFSET);
	out_be32(gpio2_int_ctl, val);

	/* register GPIO2 interrupt */
	gpio_register_irq(coreid, GPIO2_INT_NUM);
}

static void gpio2_start(void)
{
	u32 val;
	void *gpio2_data = (void *)GPIO2_DATA;

	val = in_be32((u32 *)gpio2_data);
	val |= (1 << GPIO2_01_OFFSET);
	out_be32(gpio2_data, val);

	printf("data: 0x%x\n", in_be32((u32 *)gpio2_data));
	printf("event: 0x%x\n", in_be32((u32 *)GPIO2_INT_EVENT));
}

static void gpio2_stop(void)
{
	u32 val;
	void *gpio2_data = (void *)GPIO2_DATA;

	val = in_be32((u32 *)gpio2_data);
	val &= ~(1 << GPIO2_01_OFFSET);
	out_be32(gpio2_data, val);

	printf("data: 0x%x\n", in_be32((u32 *)gpio2_data));
	printf("event: 0x%x\n", in_be32((u32 *)GPIO2_INT_EVENT));
}

static int do_gpio(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	if (argc == 2 && strncmp(argv[1], "enable", 6) == 0)
		gpio2_setup_enable();
	if (argc == 2 && strncmp(argv[1], "start", 6) == 0)
		gpio2_start();
	if (argc == 2 && strncmp(argv[1], "stop", 6) == 0)
		gpio2_stop();

	return 0;
}

static char gpio_init_help[] =
	"gpio_int enable		- Enable GPIO2_02 output and interrupt\n"
	"gpio_int start			- start one loop to generate GPIO interrupt\n"
	"gpio_int stop			- generate low voltage to generate GPIO interrupt\n"
	"";
U_BOOT_CMD(gpio_int, CONFIG_SYS_MAXARGS, 0, do_gpio,
	   "enable gpio2 interrupt", gpio_init_help);
