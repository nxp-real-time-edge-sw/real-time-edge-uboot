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
#include <asm/gic-v3.h>
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


static u64 gic_mpidr_to_affinity(unsigned long mpidr)
{
	u64 aff;

	aff = ((u64)MPIDR_AFFINITY_LEVEL(mpidr, 3) << 32 |
			MPIDR_AFFINITY_LEVEL(mpidr, 2) << 16 |
			MPIDR_AFFINITY_LEVEL(mpidr, 1) << 8  |
			MPIDR_AFFINITY_LEVEL(mpidr, 0));

	return aff;
}

static int gic_set_type(struct udevice *dev, uint irq, bool active_low)
{
	int ret = 0;
	u32 confmask = 0x2 << ((irq % 16) * 2);
	u32 confoff = (irq / 16) * 4;
	u32 val, oldval;

	gic_mask_irq(irq);
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
	u32 val;
	u32 confoff;
	unsigned int phy_core;
	int i;

	confoff = irq->id * 8;

    /* convert core mask to physical core */
	for (i = 0; i < CONFIG_MAX_CPUS; i++) {
		if ((core_mask >> i) & 0x1) {
			phy_core = i;
			break;
		}
	}
	gic_mask_irq(irq->id);
	val = gic_mpidr_to_affinity(phy_core);
	writel(val, priv->gicd_base + GICD_IROUTERn + confoff);
	gic_unmask_irq(irq->id);
	return 0;
}

static void gic_set_pri_per_cpu(void)
{
	int i;
	u32 val = 0x80808080;

	for (i = 0; i < 32; i += 4)
		writel(val, priv->gicd_base + GICD_IPRIORITYRn + (i / 4) * 4);
}

static void gic_set_pri_common(void)
{
	unsigned long val;
	int i;

	/* set the SGI interrupts for this core to group 1 */
	val = 0xffffffff;
	writel(val, priv->gicd_base + GICD_IGROUPRn);
	writel(GICD_ENABLE, priv->gicd_base + GICD_CTLR);

	val = 0x70707070;
	for (i = 32; i < 256; i += 4)
		writel(val, priv->gicd_base + GICD_IPRIORITYRn + (i / 4) * 4);

#ifdef CONFIG_ARM64
	/* route interrupt to EL2 */
	if (current_el() == 2) {
		asm volatile("mrs %0, HCR_EL2" : "=r" (val));
		val |= (1 << 4);
		asm volatile("msr HCR_EL2, %0" : : "r" (val));
	}
#endif

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

static const struct irq_ops gicv3_chip = {
	.set_polarity   = gic_set_type,
	.set_affinity   = gic_set_affinity,
};

static int gicv3_probe(struct udevice *dev)
{
	priv = dev_get_priv(dev);

	priv->gicd_base = (void __iomem *)dev_read_addr_index(dev, 0);
	if (!priv->gicd_base) {
		pr_err("%s: failed to get GICD address\n", __func__);
		return -ENOENT;
	}

	priv->gicr_base = (void __iomem *)dev_read_addr_index(dev, 1);
	if (!priv->gicr_base) {
		pr_err("%s: failed to get GICR address\n", __func__);
		return -ENOENT;
	}

	return 0;
}

static const struct udevice_id gicv3_ids[] = {
	{ .compatible = "arm,gic-v3" },
};

U_BOOT_DRIVER(gicv3) = {
	.name   = "gicv3",
	.id = UCLASS_IRQ,
	.ops    = &gicv3_chip,
	.priv_auto  = sizeof(struct gic_chip_data),
	.of_match = gicv3_ids,
	.probe  = gicv3_probe,
};
