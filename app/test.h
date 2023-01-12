// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2023 NXP
 *
 */

#ifndef _BAREMETAL_APP_TEST_H
#define _BAREMETAL_APP_TEST_H

#ifndef __ASSEMBLY__

int test_gpio(void);

void test_i2c(void);

void test_irq_init(void);

void test_icc_func_init(void);

int test_qspi(void);

#endif

#endif /* _BAREMETAL_APP_TEST_H */
