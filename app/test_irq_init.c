// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2023 NXP
 *
 */

#include <common.h>
#include <asm/interrupt-gic.h>
#include <inter-core-comm.h>
#include <cpu_func.h>
#include <irq.h>

#ifdef CONFIG_TARGET_LS1021AIOT
#define CONFIG_TEST_HW_IRQ 195
#elif defined(CONFIG_TARGET_LS1043ARDB) || defined(CONFIG_TARGET_LS1046ARDB)
#define CONFIG_TEST_HW_IRQ 163
#else
#define CONFIG_TEST_HW_IRQ 163
#endif

struct irq hw_ack_irq_data;
struct irq ack_irq_data;

static void test_core_handler_ack(int hw_irq, int src_coreid, void *data)
{
	printf(
			"SGI signal: Core[%u] ack irq : %d from core : %d\r\n",
			get_core_id(), hw_irq, src_coreid);

	return;
}

static void test_core_handler_hw_ack(int hw_irq, int src_coreid, void *data)
{
	printf(
			"hardware IRQ: Core[%u] ack irq : %d\r\n",
			get_core_id(), hw_irq);

	return;
}


static void test_core_hw_irq_init(u32 coreid, u32 hw_irq)
{
	hw_ack_irq_data.dev = get_irq_udevice(0);
	hw_ack_irq_data.id  = hw_irq;
	irq_desc_register(&hw_ack_irq_data, test_core_handler_hw_ack, NULL);
	irq_set_polarity(hw_ack_irq_data.dev, hw_ack_irq_data.id, false);
	irq_set_affinity(&hw_ack_irq_data, 1 << coreid);
}

void test_irq_init(void)
{
	int coreid = 1;
	/* irq 0-15 are used for SGI, irq 8 is used for IPC */
	ack_irq_data.id = IRQ_SGI_TEST;
	irq_desc_register(&ack_irq_data, test_core_handler_ack, NULL);
	printf("IRQ %d has been registered as SGI\n", IRQ_SGI_TEST);
	/* irq 195-201 are used for hardware interrupt */
	test_core_hw_irq_init(coreid, CONFIG_TEST_HW_IRQ);
	printf("IRQ %d has been registered as HW IRQ\n",
	       CONFIG_TEST_HW_IRQ);

	/* set a SGI signal */
	asm volatile("dsb st");
	gic_send_sgi(9, 1 << coreid);
	asm volatile("sev");

	return;
}
