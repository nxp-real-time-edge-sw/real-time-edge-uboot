// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2023 NXP
 */

#ifndef _LS1043A_BAREMETAL_H_
#define _LS1043A_BAREMETAL_H_

/* Number of cores in each cluster */
#define CORE_NUM_PER_CLUSTER 4

#if defined(CONFIG_BAREMETAL)

#define CONFIG_BAREMETAL_FIRST_CORE 0

/* set Master core size to 512M, slave core to 256M, share mem to 256M */
#define CONFIG_BAREMETAL_SYS_SDRAM_MASTER_SIZE	(512 * 1024 * 1024UL)
#define CONFIG_BAREMETAL_SYS_SDRAM_SLAVE_SIZE	(256 * 1024 * 1024UL)

#define SHARED_GPIO_REQUEST_INFO

#endif /* CONFIG_BAREMETAL */

#endif /* _LS1043A_BAREMETAL_H_ */
