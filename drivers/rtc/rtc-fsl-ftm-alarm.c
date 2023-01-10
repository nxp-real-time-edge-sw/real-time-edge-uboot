// SPDX-License-Identifier: GPL-2.0+
/*
 *  Freescale FlexTimer Module (FTM) alarm device driver.
 *
 * Copyright 2023 NXP
 *
 */

#include <dm.h>
#include <rtc.h>
#include <irq_func.h>
#include <clk.h>
#if defined(CONFIG_MP) && defined(CONFIG_PPC)
#include <asm/mp.h>
#endif
#include <dm/device_compat.h>
#include <cpu_func.h>
#include <ftm_alarm.h>
#include <asm/io.h>

/* FLEXTIMER module Status And Control register */
#define FLEXTIMER_SC_TOF			BIT(7)
#define FLEXTIMER_SC_TOIE			BIT(6)
#define FLEXTIMER_SC_CLKS(x)			((x) << 3)
#define FLEXTIMER_SC_CLKS_MASK			FLEXTIMER_SC_CLKS(0x3)
#define FLEXTIMER_SC_PS(x)			(x)
#define FLEXTIMER_SC_PS_MASK			(0x7)

/* FLEXTIMER module Features Mode Selection register */
#define FLEXTIMER_MODE_WPDIS			BIT(2)
#define FLEXTIMER_MODE_FTMEN			BIT(1)

/* FLEXTIMER module Fault Mode Status register */
#define FLEXTIMER_FMS_WPEN			BIT(6)

struct ftm_module {
	u32 SC;			/* 0x00 */
	u32 CNT;		/* 0x04 */
	u32 MOD;		/* 0x08 */
	u32 C0SC;		/* 0x0C */
	u32 C0v;		/* 0x10 */
	u32 C1SC;		/* 0x14 */
	u32 C1v;		/* 0x18 */
	u32 C2SC;		/* 0x1C */
	u32 C2v;		/* 0x20 */
	u32 C3SC;		/* 0x24 */
	u32 C3v;		/* 0x28 */
	u32 C4SC;		/* 0x2C */
	u32 C4v;		/* 0x30 */
	u32 C5SC;		/* 0x34 */
	u32 C5v;		/* 0x38 */
	u32 C6SC;		/* 0x3C */
	u32 C6v;		/* 0x40 */
	u32 C7SC;		/* 0x44 */
	u32 C7v;		/* 0x48 */
	u32 CNTIN;		/* 0x4C */
	u32 STATUS;		/* 0x50 */
	u32 MODE;		/* 0x54 */
	u32 SYNC;		/* 0x58 */
	u32 OUTINIT;		/* 0x5C */
	u32 OUTMASK;		/* 0x60 */
	u32 COMBINE;		/* 0x64 */
	u32 DEADTIME;		/* 0x68 */
	u32 EXTTRIG;		/* 0x6C */
	u32 POL;		/* 0x70 */
	u32 FMS;		/* 0x74 */
	u32 FILTER;		/* 0x78 */
	u32 FLTCTRL;		/* 0x7C */
	u32 QDCTRL;		/* 0x80 */
	u32 CONF;		/* 0x84 */
	u32 FLTPOL;		/* 0x88 */
	u32 SYNCONF;		/* 0x8C */
	u32 INVCTRL;		/* 0x90 */
	u32 SWOCTRL;		/* 0x94 */
	u32 PWMLOAD;		/* 0x98 */
};

#define FTM1_BASE           ((u32)0x029D0000)
#define FTM2_BASE           ((u32)0x029E0000)
#define FTM3_BASE           ((u32)0x029F0000)
#define FTM4_BASE           ((u32)0x02A00000)
#define FTM5_BASE           ((u32)0x02A10000)
#define FTM6_BASE           ((u32)0x02A20000)
#define FTM7_BASE           ((u32)0x02A30000)
#define FTM8_BASE           ((u32)0x02A40000)

#define FTM1                ((struct ftm_module *)FTM1_BASE)
#define FTM2                ((struct ftm_module *)FTM2_BASE)
#define FTM3                ((struct ftm_module *)FTM3_BASE)
#define FTM4                ((struct ftm_module *)FTM4_BASE)
#define FTM5                ((struct ftm_module *)FTM5_BASE)
#define FTM6                ((struct ftm_module *)FTM6_BASE)
#define FTM7                ((struct ftm_module *)FTM7_BASE)
#define FTM8                ((struct ftm_module *)FTM8_BASE)

enum ftm_clksrc_e {
	clock_no = 0,
	clock_sys,
	clock_fixed,
	clock_external
};

enum presdiv_e {
	div_1 = 0,
	div_2,
	div_4,
	div_8,
	div_16,
	div_32,
	div_64,
	div_128,
};

struct ftm_init_t {
	bool overflow_is_enable;
	enum ftm_clksrc_e clk;
	enum presdiv_e prescale;
	u8 overflow_is_skip;
	u16 cnt_init;
	u16 mod;
};

struct rtc_ftm_alarm_priv {
    struct ftm_module *base;
	ftm_irq_func overflow_handle;
	u32 irq_num;
};

static u32 ftm_read(void *addr)
{
	return in_be32(addr);
}

static void ftm_write(u32 val, void *addr)
{
	out_be32(addr, val);
	return;
}

void ftm_write_protection(struct ftm_module *ftm_base, bool is_enable)
{
	u32 reg = ftm_read(&(ftm_base->FMS));

	if (is_enable) {
		reg |= FLEXTIMER_FMS_WPEN;
		ftm_write(reg, &(ftm_base->FMS));
	} else{
		reg = ftm_read(&(ftm_base->MODE));
		reg |= FLEXTIMER_MODE_WPDIS;
		ftm_write(reg, &(ftm_base->MODE));
	}
}

void ftm_counter_enable(struct udevice *dev)
{
	struct rtc_ftm_alarm_priv *priv = dev_get_priv(dev);
	struct ftm_module *ftm_base = priv->base;
	u32 val = 0;

	ftm_write_protection(ftm_base, false);

	/* Select and enable counter clock source */
	val = ftm_read(&(ftm_base->SC));
	val &= ~(FLEXTIMER_SC_PS_MASK | FLEXTIMER_SC_CLKS_MASK);
	val |= FLEXTIMER_SC_CLKS(clock_sys);
	ftm_write(val, &(ftm_base->SC));

	ftm_write_protection(ftm_base, true);
}

void ftm_counter_disable(struct udevice *dev)
{
	struct rtc_ftm_alarm_priv *priv = dev_get_priv(dev);
	struct ftm_module *ftm_base = priv->base;
	u32 val = 0;

	ftm_write_protection(ftm_base, false);

	val = ftm_read(&(ftm_base->SC));
	val &= ~(FLEXTIMER_SC_PS_MASK | FLEXTIMER_SC_CLKS_MASK);
	ftm_write(val, &(ftm_base->SC));

	ftm_write_protection(ftm_base, true);
}

static inline void ftm_counter_reset(struct ftm_module *base)
{
	/*
     * The CNT register contains the FTM counter value.
     * Reset clears the CNT register. Writing any value to COUNT
     * updates the counter with its initial value, CNTIN.
     */
	ftm_write(0x00, &(base->CNT));
}

u64 ftm_get_count(struct udevice *dev)
{
	struct rtc_ftm_alarm_priv *priv = dev_get_priv(dev);
	struct ftm_module *reg = priv->base;

	return ftm_read(&(reg->CNT));
}

void ftm_reset_count(struct udevice *dev)
{
	struct rtc_ftm_alarm_priv *priv = dev_get_priv(dev);

	ftm_counter_reset(priv->base);

	return;
}

static inline void ftm_irq_acknowledge(struct ftm_module *ftm_base)
{
	unsigned int timeout = 100;

	while ((FLEXTIMER_SC_TOF & ftm_read(&(ftm_base->SC))) && timeout--)
		ftm_write(ftm_read(&(ftm_base->SC)) &
		(~FLEXTIMER_SC_TOF), &(ftm_base->SC));
}

static void ftm_timer_irq(int hw_irq, int src_coreid, void *data)
{
	struct udevice *dev = (struct udevice *)data;
	struct rtc_ftm_alarm_priv *priv = dev_get_priv(dev);

	ftm_irq_acknowledge(priv->base);
	if (priv->overflow_handle != NULL)
		priv->overflow_handle(NULL);
}

static void ftm_timer_register_irq(struct udevice *dev)
{
	struct rtc_ftm_alarm_priv *priv = dev_get_priv(dev);
	u32 hw_irq = priv->irq_num;
	u32 core_id = get_core_id();

	gic_irq_register(hw_irq, ftm_timer_irq, (void *)dev);
	gic_set_type(hw_irq);
	gic_set_target(1 << core_id, hw_irq);
	enable_interrupts();
}

void ftm_irq_enable(struct rtc_ftm_alarm_priv *priv)
{
    struct ftm_module *base = priv->base;
	u32 reg = 0;
	ftm_write_protection(base, false);

	reg = ftm_read(&(base->SC));
	reg |= FLEXTIMER_SC_TOIE;
	ftm_write(reg, &(base->SC));

	ftm_write_protection(base, true);

	return;
}

void ftm_irq_disable(struct rtc_ftm_alarm_priv *priv)
{
	struct ftm_module *base = priv->base;
	u32 reg = 0;

	ftm_write_protection(base, false);

	reg = ftm_read(&(base->SC));
	reg &= ~FLEXTIMER_SC_TOIE;
	ftm_write(reg, &(base->SC));

	ftm_write_protection(base, true);
	return;
}

static void ftm_rtc_clean_alarm(struct udevice *dev)
{
	struct rtc_ftm_alarm_priv *priv = dev_get_priv(dev);
	struct ftm_module *ftm_base = priv->base;

	ftm_counter_disable(dev);

	ftm_write(0x00, &(ftm_base->CNTIN));
	ftm_write(~0, &(ftm_base->MOD));

	ftm_counter_reset(ftm_base);
}

static void ftm_set_alarm_ticks(struct ftm_module *ftm_base, u16 ticks)
{
	ftm_write(ticks, &(ftm_base->MOD));
}

void ftm_set_irq_handler(struct udevice *dev, void (* func)(void *))
{
	struct rtc_ftm_alarm_priv *priv = dev_get_priv(dev);

	priv->overflow_handle = func;

	return;
}

void ftm_rtc_set_alarm(struct udevice *dev, u16 ticks, void (* func)(void *))
{
	struct rtc_ftm_alarm_priv *priv = dev_get_priv(dev);

	ftm_rtc_clean_alarm(dev);
	ftm_irq_disable(priv);

	ftm_set_irq_handler(dev, func);

	ftm_set_alarm_ticks(priv->base, ticks);

	ftm_irq_enable(priv);
	ftm_counter_enable(dev);

	return;
}

void ftm_rtc_alarm_stop(struct udevice *dev)
{
	struct rtc_ftm_alarm_priv *priv = dev_get_priv(dev);

	ftm_irq_disable(priv);
	ftm_counter_disable(dev);
}

static void inline ftm_set_ftm_enable(struct ftm_module *ftm_base)
{
	u32 val = 0;

	ftm_write_protection(ftm_base, false);

	val = ftm_read(&(ftm_base->MODE));
	val |= FLEXTIMER_MODE_FTMEN;
	ftm_write(val, &(ftm_base->MODE));

	ftm_write_protection(ftm_base, true);
}


static void ftm_rtc_init(struct udevice *dev)
{
	struct rtc_ftm_alarm_priv *priv = dev_get_priv(dev);

	ftm_counter_disable(dev);

	ftm_rtc_clean_alarm(dev);

	ftm_set_ftm_enable(priv->base);

	ftm_timer_register_irq(dev);

	ftm_irq_disable(priv);
}

static int ftm_rtc_probe(struct udevice *dev)
{
	struct rtc_ftm_alarm_priv *priv;
	fdt_addr_t addr;
	int err;
	u32 irq, core_id = get_core_id();

	dev_info(dev, "Probe start\n");
	dev_dbg(dev, "Clock source setting: %u\n", clock_sys);

	if (core_id != CONFIG_FTM_ALARM_COREID) {
		dev_err(dev,
				"Not target core - %d, will not probe.\n", core_id);
		return -EINVAL;
	}

    priv = dev_get_priv(dev);

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	err = dev_read_u32_index(dev, "interrupts", 1, &irq);
	dev_dbg(dev, "irq_num : %u\n", irq);

	priv->base = (struct ftm_module *)addr;
	priv->overflow_handle = NULL;
	priv->irq_num = irq;

	ftm_rtc_init(dev);

	dev_info(dev, "Probe finished.\n");

	return 0;
}

/*
 * Note:
 * Flextimer is not a RTC device, so ftm_rtc_get/ftm_rtc_set/
 * ftm_rtc_reset are dummy APIs.
 */
static int ftm_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	return 0;
}

static int ftm_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	return 0;
}

static int ftm_rtc_reset(struct udevice *dev)
{
	return 0;
}

static const struct rtc_ops ftm_rtc_ops = {
    .get = ftm_rtc_get,
	.set = ftm_rtc_set,
	.reset = ftm_rtc_reset,
};

static const struct udevice_id rtc_ftm_alarm_ids[] = {
	{ .compatible = "fsl,ls1046a-ftm-alarm" },
	{ .compatible = "fsl,ls1021a-ftm-alarm" },
	{}
};

U_BOOT_DRIVER(rtc_ftm_alarm) = {
	.name = "ftm_alarm",
	.id = UCLASS_RTC,
	.of_match = rtc_ftm_alarm_ids,
    .priv_auto  = sizeof(struct rtc_ftm_alarm_priv),
    .probe = ftm_rtc_probe,
    .ops = &ftm_rtc_ops,
};
