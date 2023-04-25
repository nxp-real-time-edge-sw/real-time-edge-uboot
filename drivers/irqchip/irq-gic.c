// SPDX-License-Identifier: GPL-2.0+
/*
 *
 * Copyright 2023 NXP
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <asm/system.h>
#include <asm/ptrace.h>
#include <dm/lists.h>
#include <asm/io.h>
#include <irq.h>
#include <asm/gic.h>
#include <dt-structs.h>
#include <mapmem.h>
#include <linux/err.h>

struct gic_chip_data *priv;

void gic_mask_irq(int irq)
{
	u32 mask = 1 << (irq % 32);
	u32 confoff = (irq / 32) * 4;

	writel(mask, priv->gicd_base + GICD_ICENABLERn + confoff);
}

void gic_unmask_irq(int irq)
{
	u32 mask = 1 << (irq % 32);
	u32 confoff = (irq / 32) * 4;

	writel(mask, priv->gicd_base + GICD_ISENABLERn + confoff);
}

int gic_ack_irq(void)
{
	u32 ack = readl(priv->gicc_base + GICC_IAR);

	ack &= GICC_IAR_MASK;
	return ack;
}

void gic_end_irq(int ack)
{
	u32 val = ack & GICC_IAR_MASK;

	writel(val, priv->gicc_base + GICC_EOIR);
}

static int gic_set_type(struct udevice *dev, uint irq, bool active_low)
{
	int ret = 0;
	u32 confmask = 0x2 << ((irq % 16) * 2);
	u32 confoff = (irq / 16) * 4;
	u32 val, oldval;

	gic_mask_irq(irq);
	/* Interrupt configuration for SGIs can't be changed */
	if (irq < 16)
		return -EINVAL;

	val = oldval = readl(priv->gicd_base + GICD_ICFGR + confoff);
	if (active_low)
		val &= ~confmask;
	else
		val |= confmask;
	writel(val, priv->gicd_base + GICD_ICFGR + confoff);
	if (ret && irq < 32) {
		/* Misconfigured PPIs are usually not fatal */
		printf("GIC: PPI%d is secure or misconfigured\n", irq - 16);
	}
	gic_unmask_irq(irq);
	return 0;
}

static int gic_set_affinity(struct irq *irq, int core_mask)
{
	u32 val = core_mask << ((irq->id % 4) * 8);
	u32 mask = 0xff << ((irq->id % 4) * 8);
	u32 confoff = (irq->id / 4) * 4;
	u32 oldval = readl(priv->gicd_base + GICD_ITARGETSRn + confoff);

	writel(oldval & (~mask), priv->gicd_base + GICD_ITARGETSRn + confoff);
	oldval = readl(priv->gicd_base + GICD_ITARGETSRn + confoff);
	writel(oldval | val, priv->gicd_base + GICD_ITARGETSRn + confoff);

	return 0;
}

static void gic_set_pri_per_cpu(void)
{
	int i;
	u32 val = 0x80808080;

	for (i = 0; i < 32; i += 4)
		writel(val, priv->gicd_base + GICD_IPRIORITYRn + (i / 4) * 4);

	val = 0xf0;
	writel(val, priv->gicc_base + GICC_PMR);
}

static void gic_set_pri_common(void)
{
	unsigned long val;
	int i;

	/* set the SGI interrupts for this core to group 1 */
	val = 0xffffffff;
	writel(val, priv->gicd_base + GICD_IGROUPRn);
	writel(GICD_ENABLE, priv->gicd_base + GICD_CTLR);

	u32 bypass = readl(priv->gicc_base + GICC_CTLR);

	bypass &= GICC_DIS_BYPASS_MASK;
	bypass |= GICC_ENABLE;
	writel(bypass, priv->gicc_base + GICC_CTLR);

	val = 0x70707070;
	for (i = 32; i < 256; i += 4)
		writel(val, priv->gicd_base + GICD_IPRIORITYRn + (i / 4) * 4);

}

void gic_set_pri_irq(u32 hw_irq, u8 pri)
{
	u32 val = pri << ((hw_irq % 4) * 8);
	u32 mask = 0xff << ((hw_irq % 4) * 8);
	u32 confoff = (hw_irq / 4) * 4;
	u32 oldval = readl(priv->gicd_base + GICD_IPRIORITYRn + confoff);

	writel(oldval & (~mask), priv->gicd_base + GICD_IPRIORITYRn + confoff);
	writel(oldval | val, priv->gicd_base + GICD_IPRIORITYRn + confoff);
}


static const struct irq_ops gic_chip = {
	.set_polarity	= gic_set_type,
	.set_affinity   = gic_set_affinity,
};

static int gic_probe(struct udevice *dev)
{
	priv = dev_get_priv(dev);
#ifdef CONFIG_TARGET_LS1043ARDB
	priv = get_gic_offset();
#else
	priv->gicd_base = (void __iomem *)dev_read_addr_index(dev, 0);
	if (!priv->gicd_base) {
		pr_err("%s: failed to get GICD address\n", __func__);
		return -ENOENT;
	}

	priv->gicc_base = (void __iomem *)dev_read_addr_index(dev, 1);
	if (!priv->gicc_base) {
		pr_err("%s: failed to get GICC address\n", __func__);
		return -ENOENT;
	}
#endif

	return 0;
}

static const struct udevice_id gic_ids[] = {
	{ .compatible = "arm,gic-400" },
	{ }
};

U_BOOT_DRIVER(gic) = {
	.name   = "gic",
	.id = UCLASS_IRQ,
	.ops    = &gic_chip,
	.priv_auto  = sizeof(struct gic_chip_data),
	.of_match = gic_ids,
	.probe  = gic_probe,
};
