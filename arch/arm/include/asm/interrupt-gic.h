// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2023 NXP
 *
 */
#ifndef _FSL_LAYERSCAPE_INT_GIC_H
#define _FSL_LAYERSCAPE_INT_GIC_H
#include <irq.h>

struct gic_chip_data {
	void __iomem *gicd_base;
#if defined(CONFIG_GICV2)
	void __iomem *gicc_base;
#elif defined(CONFIG_GICV3)
	void __iomem *gicr_base;
#endif
};

#ifndef __ASSEMBLY__
void irq_desc_register(struct irq *, void (*irq_handle)(int, int, void *), void *);
void gic_send_sgi(u32 hw_irq, int core_mask);
#endif
#endif /* _FSL_LAYERSCAPE_INT_GIC_H */
