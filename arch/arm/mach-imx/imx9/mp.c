// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 NXP
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <linux/psci.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/imx-regs.h>

static u64 logical_id_to_hwid[CONFIG_MAX_CPUS] = {0, 0x100};

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
	printf("Feature is not implemented.\n");
        return 0;
}

int cpu_status(u32 nr)
{
	printf("Feature is not implemented.\n");
	return 0;
}

int cpu_release(u32 nr, int argc, char *const argv[])
{
	u64 boot_addr;
	struct pt_regs regs;

	if (nr >= CONFIG_MAX_CPUS) {
		printf("Invalid CPU ID %d\n", nr);
		return -1;
	}

	boot_addr = simple_strtoull(argv[0], NULL, 16);

	printf("begin to kick cpu core #%d to address %llx\n",
			nr, boot_addr);
	regs.regs[0] = PSCI_0_2_FN64_CPU_ON;
	regs.regs[1] = logical_id_to_hwid[nr];
	regs.regs[2] = boot_addr;
	regs.regs[3] = 0;
	smc_call(&regs);
	if (regs.regs[0])
		return -1;

	return 0;
}
