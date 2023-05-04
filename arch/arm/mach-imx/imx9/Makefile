# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2022,2026 NXP

obj-y += lowlevel_init.o

ifeq ($(CONFIG_SCMI_FIRMWARE),y)
obj-y += scmi/
else
obj-y += soc.o clock.o clock_root.o trdc.o
endif

ifneq ($(CONFIG_SPL_BUILD),y)
obj-y += imx_bootaux.o misc.o
else
obj-$(CONFIG_SPL_IMX_BBSM) += bbsm.o
endif

ifeq ($(CONFIG_IMX_SNPS_DDR_PHY_QB_GEN),y)
ifeq ($(CONFIG_XPL_BUILD),y)
obj-$(CONFIG_SPL_IMX_QB) += qb.o
else
obj-$(CONFIG_CMD_IMX_QB) += qb.o
endif
endif

obj-$(CONFIG_MP) += mp.o
