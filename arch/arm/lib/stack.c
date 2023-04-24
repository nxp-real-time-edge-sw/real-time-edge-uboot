// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Andreas Bießmann <andreas@biessmann.org>
 *
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * Copyright 2023 NXP
 */
#include <common.h>
#include <init.h>
#include <lmb.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_reserve_stacks(void)
{
#ifdef CONFIG_SPL_BUILD
	gd->start_addr_sp -= 128;	/* leave 32 words for abort-stack */
	gd->irq_sp = gd->start_addr_sp;
#else
	/* setup stack pointer for exceptions */
	gd->irq_sp = gd->start_addr_sp;

# if !defined(CONFIG_ARM64)
	/* leave 3 words for abort-stack, plus 1 for alignment */
	gd->start_addr_sp -= 16;
# endif
#endif
#  ifdef CONFIG_USE_IRQ
	gd->start_addr_sp -= (CONFIG_STACKSIZE_IRQ + CONFIG_STACKSIZE_FIQ);
	debug("Reserving %zu Bytes for IRQ stack at: %08lx\n",
	      CONFIG_STACKSIZE_IRQ + CONFIG_STACKSIZE_FIQ, gd->start_addr_sp);
	/* 8-byte alignment for ARM ABI compliance */
	gd->start_addr_sp &= ~0x07;
#  endif

	return 0;
}

static ulong get_sp(void)
{
	ulong ret;

	asm("mov %0, sp" : "=r"(ret) : );
	return ret;
}

void arch_lmb_reserve(struct lmb *lmb)
{
	arch_lmb_reserve_generic(lmb, get_sp(), gd->ram_top, 16384);
}
