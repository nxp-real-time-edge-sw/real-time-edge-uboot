// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019-2023 NXP
 *
 */

#ifndef _LS1028A_BAREMETAL_H_
#define _LS1028A_BAREMETAL_H_

/* Number of cores in each cluster */
#define CORE_NUM_PER_CLUSTER 2

#if defined(CONFIG_BAREMETAL)

#define CONFIG_BAREMETAL_FIRST_CORE 0

#define CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ  (4*1024)
#define CONFIG_STACKSIZE_FIQ  (4*1024)

/* set Master core size to 512M, slave core to 256M, share mem to 256M */
#define CONFIG_BAREMETAL_SYS_SDRAM_MASTER_SIZE	(512 * 1024 * 1024UL)
#define CONFIG_BAREMETAL_SYS_SDRAM_SLAVE_SIZE		(256 * 1024 * 1024UL)
#define CONFIG_BAREMETAL_SYS_SDRAM_RESERVE_SIZE	(16 * 1024 * 1024UL)
#define CONFIG_BAREMETAL_SYS_SDRAM_SHARE_SIZE \
	((256 * 1024 * 1024UL) - CONFIG_BAREMETAL_SYS_SDRAM_RESERVE_SIZE)
#define CONFIG_BAREMETAL_SYS_SDRAM_SHARE_BASE \
	(CONFIG_SYS_SDRAM_BASE + CONFIG_BAREMETAL_SYS_SDRAM_MASTER_SIZE \
	+ CONFIG_BAREMETAL_SYS_SDRAM_SLAVE_SIZE * (CONFIG_MAX_CPUS - 1))
#define CONFIG_BAREMETAL_SYS_SDRAM_RESERVE_BASE \
	(CONFIG_BAREMETAL_SYS_SDRAM_SHARE_BASE + \
	CONFIG_BAREMETAL_SYS_SDRAM_SHARE_SIZE)

#if defined(CONFIG_BAREMETAL_SLAVE_MODE)

#undef CONFIG_EXTRA_ENV_SETTINGS

#endif /* BAREMETAL_SLAVE_MODE */

#endif /* CONFIG_BAREMETAL */

#endif /* _LS1028A_BAREMETAL_H_ */