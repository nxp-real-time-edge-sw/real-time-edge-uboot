// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2022 NXP
 *
 */

#define CONFIG_BAREMETAL_FIRST_CORE                1

/* set Master core size to 512M, slave core size to 256M, share mem to 256M */

/* Number of cores in each cluster */
#define CORE_NUM_PER_CLUSTER    4
#define CONFIG_MAX_CPUS  4