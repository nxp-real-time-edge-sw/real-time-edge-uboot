// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2022 NXP
 *
 */

#define CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ  (4*1024)
#define CONFIG_STACKSIZE_FIQ  (4*1024)

/* PCIe */
#define CONFIG_PCIE1			/* PCIE controller 1 */
#define CONFIG_PCIE2			/* PCIE controller 2 */
#define FSL_PCIE_COMPAT			"fsl,ls1021a-pcie"
#ifndef CONFIG_PCI
#define CONFIG_PCI
#endif
#ifdef CONFIG_PCI
#define CONFIG_PCI_SCAN_SHOW
#endif

/* Number of cores in each cluster */
#define CORE_NUM_PER_CLUSTER	2

#define CONFIG_SLAVE_FIRST_CORE                 1

/*set Master core size to 512M, slave core size to 256M, share mem to 256M */
#define CONFIG_SYS_DDR_SDRAM_SHARE_RESERVE_SIZE (16 * 1024 * 1024)
#define CONFIG_SYS_DDR_SDRAM_SHARE_SIZE \
	((256 * 1024 * 1024) - CONFIG_SYS_DDR_SDRAM_SHARE_RESERVE_SIZE)

#define CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ  (4*1024)
#define CONFIG_STACKSIZE_FIQ  (4*1024)