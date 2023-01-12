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
#include <irq.h>
#include <irq_func.h>
#include <inter-core-comm.h>

#define CORE0_MASK	(1 << 0)
#define CORE1_MASK	(1 << 1)
#define CORE2_MASK	(1 << 2)
#define CORE3_MASK	(1 << 3)

struct gpio_desc gpio_out;	/*Get from dts*/
struct gpio_desc gpio_in;	/*Get from dts*/
struct irq gpio_interrupt_data;
int gpio_irq_number;	/*Get from dts*/
void *g_gic_gpio_irq_cb[32];

void gpio_irq_register(int offset, void (*gpio_irq)(void))
{
	g_gic_gpio_irq_cb[offset] = (void *)gpio_irq;
}

static void gpio_irq(void)
{
	u32 dest_core = (CORE0_MASK | CORE1_MASK | CORE2_MASK | CORE3_MASK);
	unsigned long time_us = timer_get_us();

	icc_dump_time(dest_core);
	printf("Time(us): 0x%lx\n", time_us);
	dm_gpio_clr_interrupt(&gpio_in);
}


static void gpio_bank_irq(int hw_irq, int src_coreid, void *data)
{
	if (g_gic_gpio_irq_cb[gpio_in.offset])
		gpio_irq();
}


static void gpio_register_irq(u32 coreid, u32 hw_irq, int offset)
{
	gpio_interrupt_data.dev = get_irq_udevice(0);
	gpio_interrupt_data.id = hw_irq;
	irq_desc_register(&gpio_interrupt_data, gpio_bank_irq, NULL);
	gpio_irq_register(offset, gpio_irq);
	irq_set_polarity(gpio_interrupt_data.dev, gpio_interrupt_data.id, false);
	irq_set_affinity(&gpio_interrupt_data, 1 << coreid);
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
