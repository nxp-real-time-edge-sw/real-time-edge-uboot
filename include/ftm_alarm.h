// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale FlexTimer Module (FTM) alarm device driver.
 *
 * Copyright 2023 NXP
 *
 */

#ifndef __FTM_ALARM_H
#define __FTM_ALARM_H

#include <stdbool.h>
#include <common.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <asm/interrupt-gic.h>

typedef void (*ftm_irq_func)(void *);

void ftm_counter_enable(struct udevice *dev);
void ftm_counter_disable(struct udevice *dev);
u64 ftm_get_count(struct udevice *dev);
void ftm_reset_count(struct udevice *dev);

void ftm_rtc_set_alarm(struct udevice *dev, u16 ticks, void (* func)(void *));
int ftm_rtc_set_alarm_by_us(struct udevice *dev, unsigned long us, void (* func)(void *));
void ftm_rtc_alarm_stop(struct udevice *dev);
unsigned long ftm_rtc_get_max_alarm_us(struct udevice *dev);

#endif
