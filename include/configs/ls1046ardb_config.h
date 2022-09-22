// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define CONFIG_ICC

/* set Master core size to 512M, slave core size to 256M, share mem to 256M */
#define CONFIG_SYS_DDR_SDRAM_SLAVE_SIZE	(256 * 1024 * 1024)
#define CONFIG_SYS_DDR_SDRAM_MASTER_SIZE	(512 * 1024 * 1024)
#define CONFIG_SYS_DDR_SDRAM_SHARE_RESERVE_SIZE (16 * 1024 * 1024)
#define CONFIG_SYS_DDR_SDRAM_SHARE_SIZE \
	((256 * 1024 * 1024) - CONFIG_SYS_DDR_SDRAM_SHARE_RESERVE_SIZE)

#define CONFIG_SYS_DDR_SDRAM_SHARE_BASE \
	(CONFIG_SYS_DDR_SDRAM_BASE + CONFIG_SYS_DDR_SDRAM_MASTER_SIZE \
	+ CONFIG_SYS_DDR_SDRAM_SLAVE_SIZE * (CONFIG_MAX_CPUS - 1))

#define CONFIG_SYS_DDR_SDRAM_SHARE_RESERVE_BASE \
	(CONFIG_SYS_DDR_SDRAM_SHARE_BASE + CONFIG_SYS_DDR_SDRAM_SHARE_SIZE)

/* Number of cores in each cluster */
#define CORE_NUM_PER_CLUSTER    4

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS		\
			"ipaddr=192.168.1.1\0" \
			"eth1addr=00:04:9F:04:F0:F1\0" \
			"eth2addr=00:1F:7B:63:35:E9\0" \
			"eth3addr=00:04:9F:04:F0:F3\0" \
			"eth4addr=00:04:9F:04:F0:F4\0" \
			"eth5addr=00:04:9F:04:F0:F5\0" \
			"eth6addr=00:04:9F:04:F0:F6\0" \
			"eth7addr=68:05:ca:35:cc:61\0" \
			"ethact=FM1@DTSEC5\0"			\

