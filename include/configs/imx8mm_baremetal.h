// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2023 NXP
 *
 */

#ifndef _IMX8MM_BAREMETAL_H_
#define _IMX8MM_BAREMETAL_H_

/* Number of cores in each cluster */
#define CORE_NUM_PER_CLUSTER	4
#define CONFIG_MAX_CPUS		4

/* GICv3 base address */
#define GICD_BASE	0x38800000
#define GICR_BASE	0x38880000

#if defined(CONFIG_BAREMETAL)

#define CFG_BAREMETAL_FIRST_CORE 0

/* set Master core size to 512M, slave core to 256M, share mem to 256M */
#define CFG_BAREMETAL_SYS_SDRAM_MASTER_SIZE	(512 * 1024 * 1024UL)
#define CFG_BAREMETAL_SYS_SDRAM_SLAVE_SIZE	(32 * 1024 * 1024UL)
#define CFG_BAREMETAL_SYS_SDRAM_RESERVE_SIZE	(4 * 1024 * 1024UL)
#define CFG_BAREMETAL_SYS_SDRAM_SHARE_SIZE \
	((32 * 1024 * 1024UL) - CFG_BAREMETAL_SYS_SDRAM_RESERVE_SIZE)
#define CFG_BAREMETAL_SYS_SDRAM_SHARE_BASE \
	(CFG_SYS_SDRAM_BASE + CFG_BAREMETAL_SYS_SDRAM_MASTER_SIZE \
	+ CFG_BAREMETAL_SYS_SDRAM_SLAVE_SIZE * (CONFIG_MAX_CPUS - 1))
#define CFG_BAREMETAL_SYS_SDRAM_RESERVE_BASE \
	(CFG_BAREMETAL_SYS_SDRAM_SHARE_BASE + \
	CFG_BAREMETAL_SYS_SDRAM_SHARE_SIZE)

#if defined(CONFIG_BAREMETAL_SLAVE_MODE)

#undef CONFIG_EXTRA_ENV_SETTINGS

#endif /* CONFIG_BAREMETAL_SLAVE_MODE */

#endif /* CONFIG_BAREMETAL */

#endif /* _IMX8MM_BAREMETAL_H_ */