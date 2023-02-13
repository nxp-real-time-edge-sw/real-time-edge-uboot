// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * Copyright 2023 NXP
 *
 */

#include <common.h>
#include <asm/system.h>
#include <asm/global_data.h>
#include <asm/ptrace.h>
#include <irq_func.h>
#include <linux/compiler.h>
#include <efi_loader.h>
#include <asm/io.h>
#if !defined(CONFIG_ARCH_IMX8M) && !defined(CONFIG_ARCH_IMX9)
#include <asm/arch/soc.h>
#endif
#include <cpu_func.h>

DECLARE_GLOBAL_DATA_PTR;

struct giccd_base {
	void __iomem *gicd_base;
	void __iomem *gicc_base;
};

#define GIC_CPU_CTRL			0x00
#define GIC_CPU_PRIMASK			0x04
#define GIC_CPU_BINPOINT		0x08
#define GIC_CPU_INTACK			0x0c
#define GIC_CPU_EOI			0x10
#define GIC_CPU_RUNNINGPRI		0x14
#define GIC_CPU_HIGHPRI			0x18
#define GIC_CPU_ALIAS_BINPOINT		0x1c
#define GIC_CPU_ACTIVEPRIO		0xd0
#define GIC_CPU_IDENT			0xfc

#define GICC_ENABLE			0x1
#define GICC_INT_PRI_THRESHOLD		0xf0
#define GICC_IAR_MASK                  0x1fff
#define GICC_IAR_INT_ID_MASK		0x3ff
#define GICC_INT_SPURIOUS		1023
#define GICC_DIS_BYPASS_MASK		0x1e0

#define GIC_DIST_CTRL			0x000
#define GIC_DIST_CTR			0x004
#define GIC_DIST_IGROUP			0x080
#define GIC_DIST_ENABLE_SET		0x100
#define GIC_DIST_ENABLE_CLEAR		0x180
#define GIC_DIST_PENDING_SET		0x200
#define GIC_DIST_PENDING_CLEAR		0x280
#define GIC_DIST_ACTIVE_SET		0x300
#define GIC_DIST_ACTIVE_CLEAR		0x380
#define GIC_DIST_PRI			0x400
#define GIC_DIST_TARGET			0x800
#define GIC_DIST_CONFIG			0xc00
#define GIC_DIST_SOFTINT		0xf00
#define GIC_DIST_SGI_PENDING_CLEAR	0xf10
#define GIC_DIST_SGI_PENDING_SET	0xf20

#define GICD_ENABLE			0x3
#define GICD_DISABLE			0x0
#define GICD_INT_ACTLOW_LVLTRIG		0x0
#define GICD_INT_EN_CLR_X32		0xffffffff
#define GICD_INT_EN_SET_SGI		0x0000ffff
#define GICD_INT_EN_CLR_PPI		0xffff0000
#define GICD_INT_DEF_PRI		0xa0
#define GICD_INT_DEF_PRI_X4		((GICD_INT_DEF_PRI << 24) |\
					(GICD_INT_DEF_PRI << 16) |\
					(GICD_INT_DEF_PRI << 8) |\
					GICD_INT_DEF_PRI)

#define GICH_HCR		0x0
#define GICH_VTR		0x4
#define GICH_VMCR		0x8
#define GICH_MISR		0x10
#define GICH_EISR0		0x20
#define GICH_EISR1		0x24
#define GICH_ELRSR0		0x30
#define GICH_ELRSR1		0x34
#define GICH_APR		0xf0
#define GICH_LR0		0x100

#define GICH_HCR_EN		(1 << 0)
#define GICH_HCR_UIE		(1 << 1)

#define GICH_LR_VIRTUALID		(0x3ff << 0)
#define GICH_LR_PHYSID_CPUID_SHIFT	(10)
#define GICH_LR_PHYSID_CPUID		(7 << GICH_LR_PHYSID_CPUID_SHIFT)
#define GICH_LR_STATE			(3 << 28)
#define GICH_LR_PENDING_BIT		(1 << 28)
#define GICH_LR_ACTIVE_BIT		(1 << 29)
#define GICH_LR_EOI			(1 << 19)

#define GICH_VMCR_CTRL_SHIFT		0
#define GICH_VMCR_CTRL_MASK		(0x21f << GICH_VMCR_CTRL_SHIFT)
#define GICH_VMCR_PRIMASK_SHIFT		27
#define GICH_VMCR_PRIMASK_MASK		(0x1f << GICH_VMCR_PRIMASK_SHIFT)
#define GICH_VMCR_BINPOINT_SHIFT		21
#define GICH_VMCR_BINPOINT_MASK		(0x7 << GICH_VMCR_BINPOINT_SHIFT)
#define GICH_VMCR_ALIAS_BINPOINT_SHIFT	18
#define GICH_VMCR_ALIAS_BINPOINT_MASK	\
	(0x7 << GICH_VMCR_ALIAS_BINPOINT_SHIFT)

#define GICH_MISR_EOI			(1 << 0)
#define GICH_MISR_U			(1 << 1)

#if defined(CONFIG_GICV3)
#define GICD_CTLR           0x0000
#define GICD_IROUTER            0x6000
#define GICD_CTLR_RWP           (1U << 31)
#define MPIDR_LEVEL_BITS 8
#define MPIDR_LEVEL_MASK ((1 << MPIDR_LEVEL_BITS) - 1)
#define MPIDR_AFFINITY_LEVEL(mpidr, level) \
		((mpidr >> (MPIDR_LEVEL_BITS * level)) & MPIDR_LEVEL_MASK)
#endif


struct giccd_base gic_d;
struct giccd_base get_gic_offset(void);

void *g_gic_irq_cb[1024] __section(".data");
void *g_gic_irq_data[1024] __section(".data");

extern u32 gic_ack_int_v3(void);
extern void gic_end_int_v3(u32 ack);

void gic_set_offset(void)
{
	gic_d = get_gic_offset();
}

u32 gic_ack_int(void)
{
	u32 ack = *((volatile unsigned int *)(gic_d.gicc_base +
		    GIC_CPU_INTACK));
	ack &= GICC_IAR_MASK;

	return ack;
}

static inline void gic_end_int(u32 ack)
{
	*((volatile unsigned int *)(gic_d.gicc_base +
		    GIC_CPU_EOI)) = (ack & GICC_IAR_MASK);
}

void gic_irq_register(int irq_num, void (*irq_handle)(int, int, void *), void *data)
{
	g_gic_irq_cb[irq_num] = (void *)irq_handle;
	g_gic_irq_data[irq_num] = data;
}

void gic_mask_irq(unsigned long hw_irq)
{
	u32 mask = 1 << (hw_irq % 32);
	*((volatile unsigned int *)(gic_d.gicd_base +
		    GIC_DIST_ENABLE_CLEAR + (hw_irq / 32) * 4)) = mask;
}

void gic_unmask_irq(unsigned long hw_irq)
{
	u32 mask = 1 << (hw_irq % 32);
	*((volatile unsigned int *)(gic_d.gicd_base +
		    GIC_DIST_ENABLE_SET + (hw_irq / 32) * 4)) = mask;
}

u32 gic_get_pending(unsigned long hw_irq)
{
	u32 pending = *((volatile unsigned int *)(gic_d.gicd_base +
		    GIC_DIST_PENDING_SET + (hw_irq / 32) * 4));
	return pending;
}

#if defined(CONFIG_GICV3)
static u64 gic_mpidr_to_affinity(unsigned long mpidr)
{
	u64 aff;

	aff = ((u64)MPIDR_AFFINITY_LEVEL(mpidr, 3) << 32 |
			MPIDR_AFFINITY_LEVEL(mpidr, 2) << 16 |
			MPIDR_AFFINITY_LEVEL(mpidr, 1) << 8  |
			MPIDR_AFFINITY_LEVEL(mpidr, 0));

	return aff;
}
#endif

void gic_set_target(u32 core_mask, unsigned long hw_irq)
{
#if defined(CONFIG_GICV2)
	u32 val = core_mask << ((hw_irq % 4) * 8);
	u32 mask = 0xff << ((hw_irq % 4) * 8);
	*((volatile unsigned int *)(gic_d.gicd_base +
		    GIC_DIST_TARGET + (hw_irq / 4) * 4)) &= ~mask;
	*((volatile unsigned int *)(gic_d.gicd_base +
		    GIC_DIST_TARGET + (hw_irq / 4) * 4)) |= val;
#elif defined(CONFIG_GICV3)
	unsigned int phy_core;
	int i;
	void *reg;
	void __iomem *base = gic_d.gicd_base;
	u32 val;

    /* convert core mask to physical core */
	for (i = 0; i < CONFIG_MAX_CPUS; i++) {
		if ((core_mask >> i) & 0x1) {
			phy_core = i;
			break;
		}
	}
	gic_mask_irq(hw_irq);
	reg = base + GICD_IROUTER + (hw_irq * 8);
	val = gic_mpidr_to_affinity(phy_core);
	writeq_relaxed(val, reg);
	gic_unmask_irq(hw_irq);

#endif
}

void gic_set_type(unsigned long hw_irq)
{
	u32 confmask = 0x2 << ((hw_irq % 16) * 2);

	gic_mask_irq(hw_irq);
	*((volatile unsigned int *)(gic_d.gicd_base +
		    GIC_DIST_CONFIG + (hw_irq / 16) * 4)) |= confmask;
	gic_unmask_irq(hw_irq);
}

void gic_set_sgi(int core_mask, u32 hw_irq)
{
	u32 val;
	if (hw_irq > 16) {
		printf("Interrupt id num: %u is not valid, SGI[0 - 15]\n",
		       hw_irq);
		return;
	}

	val = (core_mask << 16) | 0x8000 | hw_irq;
	*((volatile unsigned int *)(gic_d.gicd_base +
		    GIC_DIST_SOFTINT)) |= val;

	return;
}

void gic_set_pri_per_cpu(void)
{
	int i;

	for (i = 0; i < 32; i += 4) {
		*((volatile unsigned int *)(gic_d.gicd_base +
			    GIC_DIST_PRI + (i / 4) * 4)) = 0x80808080;
	}

	*((volatile unsigned int *)(gic_d.gicc_base +
		    GIC_CPU_PRIMASK)) = 0xf0;
}

void gic_set_pri_common(void)
{
	unsigned long value;
	int i;

	/* set the SGI interrupts for this core to group 1 */
	*((volatile unsigned int *)(gic_d.gicd_base +
		GIC_DIST_IGROUP)) = 0xffffffff;
	*((volatile unsigned int *)(gic_d.gicd_base +
		GIC_DIST_CTRL)) = GICD_ENABLE;
	u32 bypass = *((volatile unsigned int *)
		(gic_d.gicc_base + GIC_CPU_CTRL));
	bypass &= GICC_DIS_BYPASS_MASK;
	bypass |= GICC_ENABLE;
	*((volatile unsigned int *)(gic_d.gicc_base +
		GIC_CPU_CTRL)) = bypass;

	for (i = 32; i < 256; i += 4) {
		*((volatile unsigned int *)(gic_d.gicd_base +
			GIC_DIST_PRI + (i / 4) * 4)) = 0x70707070;
	}

	/* route interrupt to EL2 */
	if (current_el() == 2) {
		asm volatile("mrs %0, HCR_EL2" : "=r" (value));
		value |= (1 << 4);
		asm volatile("msr HCR_EL2, %0" : : "r" (value));
	}
}

void gic_set_pri_irq(u32 hw_irq, u8 pri)
{
	u32 val = pri << ((hw_irq % 4) * 8);
	u32 mask = 0xff << ((hw_irq % 4) * 8);
	*((volatile unsigned int *)(gic_d.gicd_base +
		    GIC_DIST_PRI + (hw_irq / 4) * 4)) &= ~mask;
	*((volatile unsigned int *)(gic_d.gicd_base +
		    GIC_DIST_PRI + (hw_irq / 4) * 4)) |= val;
}

void gic_enable_dist(void)
{
	return;
}

int interrupt_init(void)
{
	enable_interrupts();

	return 0;
}

void enable_interrupts(void)
{
	asm volatile(
		/* arch_local_irq_enable */
		"msr	daifclr, #2"
		:
		:
		: "memory");

	return;
}

int disable_interrupts(void)
{
	asm volatile(
		/* arch_local_irq_disable */
		"msr	daifset, #2"
		:
		:
		: "memory");

	return 0;
}

static void show_efi_loaded_images(struct pt_regs *regs)
{
	efi_print_image_infos((void *)regs->elr);
}

static void dump_instr(struct pt_regs *regs)
{
	u32 *addr = (u32 *)(regs->elr & ~3UL);
	int i;

	printf("Code: ");
	for (i = -4; i < 1; i++)
		printf(i == 0 ? "(%08x) " : "%08x ", addr[i]);
	printf("\n");
}

void show_regs(struct pt_regs *regs)
{
	int i;

	if (gd->flags & GD_FLG_RELOC)
		printf("elr: %016lx lr : %016lx (reloc)\n",
		       regs->elr - gd->reloc_off,
		       regs->regs[30] - gd->reloc_off);
	printf("elr: %016lx lr : %016lx\n", regs->elr, regs->regs[30]);

	for (i = 0; i < 29; i += 2)
		printf("x%-2d: %016lx x%-2d: %016lx\n",
		       i, regs->regs[i], i+1, regs->regs[i+1]);
	printf("\n");
	dump_instr(regs);
}

/*
 * do_bad_sync handles the impossible case in the Synchronous Abort vector.
 */
void do_bad_sync(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Synchronous Abort\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_irq handles the impossible case in the Irq vector.
 */
void do_bad_irq(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Irq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_fiq handles the impossible case in the Fiq vector.
 */
void do_bad_fiq(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Fiq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_bad_error handles the impossible case in the Error vector.
 */
void do_bad_error(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("Bad mode in \"Error\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_sync handles the Synchronous Abort exception.
 */
void do_sync(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("\"Synchronous Abort\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_irq handles the Irq exception.
 */
void do_irq(struct pt_regs *pt_regs, unsigned int esr)
{
	u32 ack;
	int hw_irq;
	int src_coreid; /* just for SGI, will be 0 for other */
	void (*irq_handle)(int, int, void *);
	void *data;

#if defined(CONFIG_GICV3)
#define ICC_IAR_INT_ID_MASK	0xffffff
	ack = gic_ack_int_v3();
	hw_irq = ack & ICC_IAR_INT_ID_MASK;
	/* FIX ME: use the fixed source coreID for ls1028a */
	src_coreid = 0;

	if (hw_irq  >= 1024 && hw_irq <= 8191)
		return;

	irq_handle = (void (*)(int, int, void *))g_gic_irq_cb[hw_irq];
	data = g_gic_irq_data[hw_irq];
	if (irq_handle) {
		for(src_coreid = 0; src_coreid < CONFIG_MAX_CPUS; src_coreid++)
			if(src_coreid != get_core_id())
				irq_handle(hw_irq, src_coreid, data);
	}

	gic_end_int_v3(ack);
#elif defined(CONFIG_GICV2)
READ_ACK:
	ack = gic_ack_int();

	hw_irq = ack & GICC_IAR_INT_ID_MASK;
	src_coreid = ack >> 10;

	if (hw_irq  >= 1021)
		return;

	irq_handle = (void (*)(int, int, void *))g_gic_irq_cb[hw_irq];
	data = g_gic_irq_data[hw_irq];
	if (irq_handle)
		irq_handle(hw_irq, src_coreid, data);

	gic_end_int(ack);

	/* core soft reset interrupt id
		core0 : 196, core1 : 197, core2 : 200, core3 : 201
	*/
	if (ack == 196 || ack == 197 || ack == 200 || ack == 201)
		wfi();

	goto READ_ACK;
#endif
}

/*
 * do_fiq handles the Fiq exception.
 */
void do_fiq(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("\"Fiq\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}

/*
 * do_error handles the Error exception.
 * Errors are more likely to be processor specific,
 * it is defined with weak attribute and can be redefined
 * in processor specific code.
 */
void __weak do_error(struct pt_regs *pt_regs, unsigned int esr)
{
	efi_restore_gd();
	printf("\"Error\" handler, esr 0x%08x\n", esr);
	show_regs(pt_regs);
	show_efi_loaded_images(pt_regs);
	panic("Resetting CPU ...\n");
}
