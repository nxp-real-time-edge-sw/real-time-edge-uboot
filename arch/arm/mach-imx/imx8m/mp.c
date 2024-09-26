// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023-2024 NXP
 */

#include <common.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <linux/psci.h>

#define MPIDR_MT_BIT		BIT(24)

#define IMX_SIP_CPU_OFF		0xC200000D

#ifndef CONFIG_MAX_CPUS
#define CONFIG_MAX_CPUS 4
#endif

int is_core_valid(unsigned int core)
{
        if(core < CONFIG_MAX_CPUS)
                return 1;
        return 0;
}

int cpu_reset(u32 nr)
{
	printf("Feature is not implemented.\n");
	return 0;
}

int cpu_disable(u32 nr)
{
	struct pt_regs regs;

	if (nr >= CONFIG_MAX_CPUS) {
		printf("Invalid CPU ID %d\n", nr);
		return -1;
	}

	regs.regs[0] = PSCI_0_2_FN_AFFINITY_INFO;
	regs.regs[1] = nr;
	regs.regs[2] = 0;
	regs.regs[3] = 0;
	smc_call(&regs);
	if (regs.regs[0] == PSCI_0_2_AFFINITY_LEVEL_OFF) {
		printf("CPU Core %d is already off\n", nr);
		return 0;
	}

	regs.regs[0] = IMX_SIP_CPU_OFF;
	regs.regs[1] = nr;
	regs.regs[2] = 0;
	regs.regs[3] = 0;
	smc_call(&regs);
	if (regs.regs[0]) {
		printf("Failed to disable CPU Core %d, please check whether TF-A supports SIP CPU off service: %lx\n",
				nr, regs.regs[0]);
		return -1;
	}
	printf("CPU core #%d is disabled\n", nr);

        return 0;
}

int cpu_status(u32 nr)
{
	struct pt_regs regs;

	if (nr >= CONFIG_MAX_CPUS) {
		printf("Invalid CPU ID %d\n", nr);
		return -1;
	}

	regs.regs[0] = PSCI_0_2_FN_AFFINITY_INFO;
	regs.regs[1] = nr;
	regs.regs[2] = 0;
	regs.regs[3] = 0;
	smc_call(&regs);
	switch (regs.regs[0]) {
	case PSCI_0_2_AFFINITY_LEVEL_ON:
		printf("CPU Core %d is running\n", nr);
		return 0;
	case PSCI_0_2_AFFINITY_LEVEL_OFF:
		printf("CPU Core %d is off\n", nr);
		return 0;
	case PSCI_0_2_AFFINITY_LEVEL_ON_PENDING:
		printf("CPU Core %d is pending\n", nr);
		return 0;
	}

	printf("CPU Core %d status is unknown\n", nr);

	return -1;
}

int cpu_release(u32 nr, int argc, char *const argv[])
{
	u64 boot_addr;
	struct pt_regs regs;

	if (nr >= CONFIG_MAX_CPUS) {
		printf("Invalid CPU ID %d\n", nr);
		return -1;
	}

	regs.regs[0] = PSCI_0_2_FN_AFFINITY_INFO;
	regs.regs[1] = nr;
	regs.regs[2] = 0;
	regs.regs[3] = 0;
	smc_call(&regs);
	if (regs.regs[0] == PSCI_0_2_AFFINITY_LEVEL_ON) {
		printf("CPU Core %d is already running, disalbe it firstly before release it\n", nr);
		return 0;
	}

	boot_addr = simple_strtoull(argv[0], NULL, 16);

	printf("begin to kick cpu core #%d to address %llx\n",
			nr, boot_addr);
	regs.regs[0] = PSCI_0_2_FN64_CPU_ON;
	regs.regs[1] = nr;
	regs.regs[2] = boot_addr;
	regs.regs[3] = 0;
	smc_call(&regs);
	if (regs.regs[0])
		return -1;

	return 0;
}
