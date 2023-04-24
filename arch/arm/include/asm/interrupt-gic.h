// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2023 NXP
 *
 */
#ifndef _FSL_LAYERSCAPE_INT_GIC_H
#define _FSL_LAYERSCAPE_INT_GIC_H

struct gic_chip_data {
	void __iomem *gicd_base;
#if defined(CONFIG_GICV2)
	void __iomem *gicc_base;
#elif defined(CONFIG_GICV3)
	void __iomem *gicr_base;
#endif
};

#endif /* _FSL_LAYERSCAPE_INT_GIC_H */
