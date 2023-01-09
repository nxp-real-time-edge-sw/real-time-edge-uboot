#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <time.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <dm/lists.h>
#include <asm/io.h>
#include <dt-structs.h>
#include <mapmem.h>
#include <linux/err.h>
#include <asm/interrupt-gic.h>

struct gpio_desc gpio_out;	/*Get from dts*/
struct gpio_desc gpio_in;	/*Get from dts*/
int gpio_irq_number;	/*Get from dts*/
void *g_gic_gpio_irq_cb[32];

void gpio_irq_register(int offset, void (*gpio_irq)(void))
{
	g_gic_gpio_irq_cb[offset] = (void *)gpio_irq;
}

static void gpio_irq(void)
{
	printk("get irq success\n");
	dm_gpio_clr_interrupt(&gpio_in);
}


static void gpio_bank_irq(int hw_irq, int src_coreid, void *data)
{
	if (g_gic_gpio_irq_cb[gpio_in.offset])
		gpio_irq();
}


static void gpio_register_irq(u32 coreid, u32 hw_irq, int offset)
{
	gic_irq_register(hw_irq, gpio_bank_irq, NULL);
	gpio_irq_register(offset, gpio_irq);
	gic_set_type(hw_irq);
	gic_set_target(1 << coreid, hw_irq);
	enable_interrupts();
}

void gpio_int_enable(void)
{
	u32 coreid = 1;

	/* Set GPIO_IN interrupt */
	dm_gpio_set_interrupt(&gpio_in);
    /* register GPIO2 interrupt */
	gpio_register_irq(coreid, gpio_irq_number, gpio_in.offset);
}

void gpio_int_start(void)
{
	dm_gpio_set_value(&gpio_out, 1);
}

void gpio_int_stop(void)
{
	dm_gpio_set_value(&gpio_out, 0);
}


#if CONFIG_IS_ENABLED(DM_GPIO)
static int gpio_int_probe(struct udevice *dev)
{
	int ret;

	ret = gpio_request_by_name(dev, "reg-names", 0, &gpio_out, GPIOD_IS_OUT);
	if (ret)
		printk("gpio_out_request_by_name_failed\n");
	ret = gpio_request_by_name(dev, "reg-names", 1, &gpio_in, GPIOD_IS_IN);
	if (ret)
		printk("gpio_in_request_by_name_failed\n");
	dev_read_u32_index(dev, "interrupts", 1, &gpio_irq_number);

	return 0;

}
static const struct udevice_id gpio_int_ids[] = {
	{ .compatible = "fsl,gpio-int" },
	{ }
};

U_BOOT_DRIVER(gpio_int) = {
	.name   = "gpio_int",
	.id = UCLASS_GPIO,
	.of_match = gpio_int_ids,
	.probe  = gpio_int_probe,
};
#endif
