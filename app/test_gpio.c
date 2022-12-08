// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2023 NXP
 *
 */

#include <common.h>
#include <asm-generic/gpio.h>
#include <dm.h>

int test_set_gpio(int value)
{
	struct gpio_desc desc;

	desc.flags = GPIOD_IS_OUT;
#ifdef CONFIG_ARCH_IMX8M
	dm_gpio_lookup_name("GPIO5_7", &desc);		/* GPIO5_7 is the pin 8 of J1003 */
	char label[] = "output7";
#else
	dm_gpio_lookup_name("MPC@023100001", &desc);
	char label[] = "output1";
#endif

	dm_gpio_request(&desc, label);
	dm_gpio_set_dir_flags(&desc, GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	dm_gpio_set_value(&desc, value);
	dm_gpio_free(desc.dev, &desc);

	return 0;
}

int test_get_gpio(void)
{
	struct gpio_desc desc;

	desc.flags = GPIOD_IS_IN;
#ifdef CONFIG_ARCH_IMX8M
	dm_gpio_lookup_name("GPIO5_8", &desc);	/* GPIO5_8 is the pin 7 of J1003 */
	char label[] = "input8";
#else
	dm_gpio_lookup_name("MPC@023100002", &desc);
	char label[] = "input2";
#endif
	int value;

	dm_gpio_request(&desc, label);
	value = dm_gpio_get_value(&desc);
	dm_gpio_free(desc.dev, &desc);

	return value;
}

/* before run this func, need connect J502.3 and J502.5 */
int test_gpio(void)
{
	int value;
	int ret = 0;

	test_set_gpio(1);
	udelay(1000000);
	value = test_get_gpio();
	if (value == 1) {
		test_set_gpio(0);
		udelay(1000000);
		value = test_get_gpio();
		if (value == 0)
			ret = 1;
	}
	if (ret == 0)
		printf("[error]GPIO test failed\n");
	else
		printf("[ok]GPIO test ok\n");

	return ret;
}
