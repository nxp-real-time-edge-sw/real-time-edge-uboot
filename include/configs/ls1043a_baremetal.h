// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2023 NXP
 */

#ifndef _LS1043A_BAREMETAL_H_
#define _LS1043A_BAREMETAL_H_

/* Number of cores in each cluster */
#define CORE_NUM_PER_CLUSTER 4

#if defined(CONFIG_BAREMETAL)

#define CFG_BAREMETAL_FIRST_CORE 0

/* set Master core size to 512M, slave core to 256M, share mem to 256M */
#define CFG_BAREMETAL_SYS_SDRAM_MASTER_SIZE	(512 * 1024 * 1024UL)
#define CFG_BAREMETAL_SYS_SDRAM_SLAVE_SIZE	(256 * 1024 * 1024UL)

#define CFG_BAREMETAL_SYS_SDRAM_RESERVE_SIZE	(16 * 1024 * 1024UL)
#define CFG_BAREMETAL_SYS_SDRAM_SHARE_SIZE \
	((256 * 1024 * 1024UL) - CFG_BAREMETAL_SYS_SDRAM_RESERVE_SIZE)
#define CFG_BAREMETAL_SYS_SDRAM_SHARE_BASE \
	(CFG_SYS_SDRAM_BASE + CFG_BAREMETAL_SYS_SDRAM_MASTER_SIZE \
	+ CFG_BAREMETAL_SYS_SDRAM_SLAVE_SIZE * (CONFIG_MAX_CPUS - 1))
#define CFG_BAREMETAL_SYS_SDRAM_RESERVE_BASE \
	(CFG_BAREMETAL_SYS_SDRAM_SHARE_BASE + CFG_BAREMETAL_SYS_SDRAM_SHARE_SIZE)

#define SHARED_GPIO_REQUEST_INFO

#if defined(CONFIG_BAREMETAL_SLAVE_MODE)

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS       \
		"ipaddr=192.168.1.1\0" \
		"eth1addr=00:04:9F:04:F0:F1\0" \
		"eth2addr=00:1F:7B:63:35:E9\0" \
		"eth3addr=00:04:9F:04:F0:F3\0" \
		"eth4addr=00:04:9F:04:F0:F4\0" \
		"eth5addr=00:04:9F:04:F0:F5\0" \
		"eth6addr=00:04:9F:04:F0:F6\0" \
		"eth7addr=68:05:ca:35:cc:61\0" \
		"ethact=FM1@DTSEC3\0"

#endif /* CONFIG_BAREMETAL_SLAVE_MODE */

#endif /* CONFIG_BAREMETAL */

#endif /* _LS1043A_BAREMETAL_H_ */