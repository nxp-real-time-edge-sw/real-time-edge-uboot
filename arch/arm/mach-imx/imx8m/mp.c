// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Gabriel Huau <contact@huau-gabriel.fr>
 *
 * Copyright 2023 NXP
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <asm/system.h>
#include <linux/errno.h>
#include <linux/psci.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/imx-regs.h>

#ifndef CONFIG_MAX_CPUS
#define CONFIG_MAX_CPUS 4
#endif

/* A53 Reset Control Register (SRC_A53RCR0) */
#define SRC_A53RCR0_CORE_1_RESET_OFFSET		5
#define SRC_A53RCR0_CORE_1_RESET_MASK		(1<<SRC_A53RCR0_CORE_1_RESET_OFFSET)
#define SRC_A53RCR0_CORE_2_RESET_OFFSET		6
#define SRC_A53RCR0_CORE_2_RESET_MASK		(1<<SRC_A53RCR0_CORE_2_RESET_OFFSET)
#define SRC_A53RCR0_CORE_3_RESET_OFFSET		7
#define SRC_A53RCR0_CORE_3_RESET_MASK		(1<<SRC_A53RCR0_CORE_3_RESET_OFFSET)

/* A53 Reset Control Register (SRC_A53RCR1) */
#define SRC_A53RCR1_CORE_1_ENABLE_OFFSET	1
#define SRC_A53RCR1_CORE_1_ENABLE_MASK		(1<<SRC_A53RCR1_CORE_1_ENABLE_OFFSET)
#define SRC_A53RCR1_CORE_2_ENABLE_OFFSET	2
#define SRC_A53RCR1_CORE_2_ENABLE_MASK		(1<<SRC_A53RCR1_CORE_2_ENABLE_OFFSET)
#define SRC_A53RCR1_CORE_3_ENABLE_OFFSET	3
#define SRC_A53RCR1_CORE_3_ENABLE_MASK		(1<<SRC_A53RCR1_CORE_3_ENABLE_OFFSET)

#define IMX_SRC_BASE			(0x30390000U)
#define IMX_GPC_BASE			(0x303a0000U)
#define SRC_GPR_CX_START_ADDRL_MASK	0x3fffff
#define SRC_GPR_CX_START_ADDRH_MASK	0xffff
#define SRC_GPR_CX_START_ADDRH_SHIFT	22
#define SRC_A53RCR1_ENABLE_CORES_MASK	0xf

#define IMX8M_SRC_A53RCR0		(0x4)
#define IMX8M_SRC_A53RCR1		(0x8)
#define IMX8M_SRC_M4RCR			(0xc)
#define IMX8M_SRC_OTG1PHY_SCR		(0x20)
#define IMX8M_SRC_OTG2PHY_SCR		(0x24)
#define IMX8M_SRC_GPR1_OFFSET		(0x74)

#define IMX8M_LPCR_A53_AD		0x4

#define IMX8M_SRC_SCR_M4_ENABLE_MASK	BIT(3)
#define IMX8M_SRC_SCR_M4C_NON_SCLR_RST_MASK	BIT(0)
#define COREx_WFI_PDN(core_id) (1 << ((core_id) < 2 ? (core_id) * 2 : ((core_id) - 2) * 2 + 16))
#define COREx_PGC_PCR(core_id) (0x800 + (core_id) * 0x40)
#define CPU_PGC_UP_TRG 0xD0

static struct src *src = (struct src *)SRC_BASE_ADDR;

static uint32_t cpu_reset_mask[CONFIG_MAX_CPUS] = {
	0, /* We don't really want to modify the cpu0 */
	SRC_A53RCR0_CORE_1_RESET_MASK,
	SRC_A53RCR0_CORE_2_RESET_MASK,
	SRC_A53RCR0_CORE_3_RESET_MASK
};

static uint32_t cpu_ctrl_mask[CONFIG_MAX_CPUS] = {
	0, /* We don't really want to modify the cpu0 */
	SRC_A53RCR1_CORE_1_ENABLE_MASK,
	SRC_A53RCR1_CORE_2_ENABLE_MASK,
	SRC_A53RCR1_CORE_3_ENABLE_MASK
};

int cpu_reset(u32 nr)
{
	/* Software reset of the CPU N */
	src->a53rcr |= cpu_reset_mask[nr];
	return 0;
}

int cpu_status(u32 nr)
{
	printf("core %d => %d\n", nr, !!(src->a53rcr1 & cpu_ctrl_mask[nr]));
	return 0;
}


static inline void mmio_write_32(uintptr_t addr, uint32_t value)
{
	*(volatile uint32_t*)addr = value;
}

static inline uint32_t mmio_read_32(uintptr_t addr)
{
	return *(volatile uint32_t*)addr;
}

static inline void mmio_clrbits_32(uintptr_t addr, uint32_t clear)
{
	mmio_write_32(addr, mmio_read_32(addr) & ~clear);
}

static inline void mmio_setbits_32(uintptr_t addr, uint32_t set)
{
	mmio_write_32(addr, mmio_read_32(addr) | set);
}

void imx_set_cpu_entry(unsigned int core_id, uintptr_t sec_entrypoint)
{
	uint64_t temp_base;

	temp_base = (uint64_t) sec_entrypoint;
	temp_base >>= 2;

	mmio_write_32(IMX_SRC_BASE + IMX8M_SRC_GPR1_OFFSET + (core_id << 3),
			((uint32_t)(temp_base >> 22) & 0xffff));
	mmio_write_32(IMX_SRC_BASE + IMX8M_SRC_GPR1_OFFSET + (core_id << 3) + 4,
			((uint32_t)temp_base & 0x003fffff));
}

void imx_set_cpu_pwr_on(unsigned int core_id)
{
	/* clear the wfi power down bit of the core */
	mmio_clrbits_32(IMX_GPC_BASE + IMX8M_LPCR_A53_AD,
			COREx_WFI_PDN(core_id));

	/* assert the ncpuporeset */
	mmio_clrbits_32(IMX_SRC_BASE + IMX8M_SRC_A53RCR1, (1 << core_id));
	/* assert the pcg pcr bit of the core */
	mmio_setbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	/* sw power up the core */
	mmio_setbits_32(IMX_GPC_BASE + CPU_PGC_UP_TRG, (1 << core_id));

	/* wait for the power up finished */
	while ((mmio_read_32(IMX_GPC_BASE + CPU_PGC_UP_TRG)
				& (1 << core_id)) != 0)
		;

	/* deassert the pcg pcr bit of the core */
	mmio_clrbits_32(IMX_GPC_BASE + COREx_PGC_PCR(core_id), 0x1);
	/* deassert the ncpuporeset */
	mmio_setbits_32(IMX_SRC_BASE + IMX8M_SRC_A53RCR1, (1 << core_id));
}

#ifdef CONFIG_BAREMETAL
/* Currently baremetal is using spintable for cpu release */
int cpu_release(u32 cpuid, int argc, char *const argv[])
{
	uint64_t boot_addr;

	boot_addr = simple_strtoul(argv[0], NULL, 16);
	flush_dcache_all();
	imx_set_cpu_entry(cpuid, boot_addr);
	imx_set_cpu_pwr_on(cpuid);

	return 0;
}
#else
/* PSCI method to kick secondary cores */
int cpu_release(u32 nr, int argc, char *const argv[])
{
	u64 boot_addr;

	boot_addr = simple_strtoull(argv[0], NULL, 16);

	/* Use PSCI to kick the core */
	struct pt_regs regs;

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
#endif

int is_core_valid(unsigned int core)
{
	if(core < CONFIG_MAX_CPUS)
		return 1;
	return 0;
}

int cpu_disable(u32 nr)
{
	/* Disable the CPU N */
	src->a53rcr1 &= ~cpu_ctrl_mask[nr];
	return 0;
}
