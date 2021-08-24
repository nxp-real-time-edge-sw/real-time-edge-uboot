/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2016 Freescale Semiconductor, Inc.
 * Copyright 2019 NXP
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_ARMV7_SECURE_BASE OCRAM_BASE_S_ADDR

#define CONFIG_SYS_INIT_RAM_ADDR	OCRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	OCRAM_SIZE

/*
 * DDR: 800 MHz ( 1600 MT/s data rate )
 */

#define DDR_SDRAM_CFG			0x470c0008
#define DDR_CS0_BNDS			0x008000bf
#define DDR_CS0_CONFIG			0x80014302
#define DDR_TIMING_CFG_0		0x50550004
#define DDR_TIMING_CFG_1		0xbcb38c56
#define DDR_TIMING_CFG_2		0x0040d120
#define DDR_TIMING_CFG_3		0x010e1000
#define DDR_TIMING_CFG_4		0x00000001
#define DDR_TIMING_CFG_5		0x03401400
#define DDR_SDRAM_CFG_2			0x00401010
#define DDR_SDRAM_MODE			0x00061c60
#define DDR_SDRAM_MODE_2		0x00180000
#define DDR_SDRAM_INTERVAL		0x18600618
#define DDR_DDR_WRLVL_CNTL		0x8655f605
#define DDR_DDR_WRLVL_CNTL_2	0x05060607
#define DDR_DDR_WRLVL_CNTL_3	0x05050505
#define DDR_DDR_CDR1			0x80040000
#define DDR_DDR_CDR2			0x00000001
#define DDR_SDRAM_CLK_CNTL		0x02000000
#define DDR_DDR_ZQ_CNTL			0x89080600
#define DDR_CS0_CONFIG_2		0
#define DDR_SDRAM_CFG_MEM_EN	0x80000000
#define SDRAM_CFG2_D_INIT		0x00000010
#define DDR_CDR2_VREF_TRAIN_EN	0x00000080
#define SDRAM_CFG2_FRC_SR		0x80000000
#define SDRAM_CFG_BI			0x00000001

#ifdef CONFIG_SD_BOOT
#define CONFIG_SPL_MAX_SIZE		0x1a000
#define CONFIG_SPL_STACK		0x1001d000
#define CONFIG_SPL_PAD_TO		0x1c000

#define CONFIG_SYS_SPL_MALLOC_START	(CONFIG_SYS_TEXT_BASE + \
		CONFIG_SYS_MONITOR_LEN)
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000
#define CONFIG_SPL_BSS_START_ADDR	0x80100000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_MONITOR_LEN		0x80000
#endif

#define CONFIG_SYS_DDR_SDRAM_BASE	0x80000000UL
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_DDR_SDRAM_BASE

#define CONFIG_CHIP_SELECTS_PER_CTRL	4

/*
 * Serial Port
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_REG_SIZE	1
#define CONFIG_SYS_NS16550_CLK		get_serial_clock()

/*
 * I2C
 */

/* EEPROM */
#define CONFIG_SYS_I2C_EEPROM_NXID
#define CONFIG_SYS_EEPROM_BUS_NUM		0

/*
 * MMC
 */

/* SATA */
#ifndef PCI_DEVICE_ID_FREESCALE_AHCI
#define PCI_DEVICE_ID_FREESCALE_AHCI	0x0440
#endif
#define CONFIG_SCSI_DEV_LIST		{PCI_VENDOR_ID_FREESCALE, \
	PCI_DEVICE_ID_FREESCALE_AHCI}

/* SPI */

/*
 * eTSEC
 */

#ifdef CONFIG_TSEC_ENET
#define CONFIG_MII_DEFAULT_TSEC		1
#define CONFIG_TSEC1			1
#define CONFIG_TSEC1_NAME		"eTSEC1"
#define CONFIG_TSEC2			1
#define CONFIG_TSEC2_NAME		"eTSEC2"

#define TSEC1_PHY_ADDR			1
#define TSEC2_PHY_ADDR			3

#define TSEC1_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED)
#define TSEC2_FLAGS			(TSEC_GIGABIT | TSEC_REDUCED)

#define TSEC1_PHYIDX			0
#define TSEC2_PHYIDX			0

#define CONFIG_ETHPRIME			"eTSEC2"

#define CONFIG_HAS_ETH0
#define CONFIG_HAS_ETH1
#define CONFIG_HAS_ETH2
#endif

/* PCIe */
#define CONFIG_PCIE1		/* PCIE controler 1 */
#define CONFIG_PCIE2		/* PCIE controler 2 */

#define FSL_PCIE_COMPAT		"fsl,ls1021a-pcie"

#ifdef CONFIG_PCI
#define CONFIG_PCI_SCAN_SHOW
#endif

#define CONFIG_PEN_ADDR_BIG_ENDIAN
#define CONFIG_LAYERSCAPE_NS_ACCESS
#define CONFIG_SMP_PEN_ADDR		0x01ee0200
#define COUNTER_FREQUENCY		12500000

#define CONFIG_HWCONFIG
#define HWCONFIG_BUFFER_SIZE		256

#define CONFIG_FSL_DEVICE_DISABLE

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_BOOTMAPSZ		(256 << 20)

#define CONFIG_LS102XA_STREAM_ID

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MONITOR_BASE CONFIG_SPL_TEXT_BASE
#else
/* start of monitor */
#define CONFIG_SYS_MONITOR_BASE CONFIG_SYS_TEXT_BASE
#endif

#include <asm/fsl_secure_boot.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	"bootargs=root=/dev/ram0 rw console=ttyS0,115200 "	\
		"cma=64M@0x0-0xb0000000\0" \
	"initrd_high=0xffffffff\0"      \
	"fdt_addr=0x64f00000\0"		\
	"kernel_addr=0x61000000\0"	\
	"kernelheader_addr=0x60800000\0"	\
	"scriptaddr=0x80000000\0"	\
	"scripthdraddr=0x80080000\0"	\
	"fdtheader_addr_r=0x80100000\0"	\
	"kernelheader_addr_r=0x80200000\0"	\
	"kernel_addr_r=0x81000000\0"	\
	"kernelheader_size=0x40000\0"	\
	"fdt_addr_r=0x90000000\0"	\
	"ramdisk_addr_r=0xa0000000\0"	\
	"load_addr=0x81000000\0"	\
	"kernel_size=0x2800000\0"	\
	"kernel_addr_sd=0x8000\0"	\
	"kernel_size_sd=0x14000\0"	\
	"kernelhdr_addr_sd=0x4000\0"		\
	"kernelhdr_size_sd=0x10\0"		\
	"othbootargs=cma=64M@0x0-0xb0000000\0"	\
	"boot_scripts=ls1021aiot_boot.scr\0"	\
	"boot_script_hdr=hdr_ls1021aiot_bs.out\0"	\
		"scan_dev_for_boot_part="	\
			"part list ${devtype} ${devnum} devplist; "	\
			"env exists devplist || setenv devplist 1; "	\
			"for distro_bootpart in ${devplist}; do "	\
			"if fstype ${devtype} "				\
				"${devnum}:${distro_bootpart} "		\
				"bootfstype; then "			\
				"run scan_dev_for_boot; "		\
			"fi; "			\
		"done\0"			\
	"scan_dev_for_boot="				  \
		"echo Scanning ${devtype} "		  \
				"${devnum}:${distro_bootpart}...; "  \
		"for prefix in ${boot_prefixes}; do "	  \
			"run scan_dev_for_scripts; "	  \
		"done;"					  \
		"\0"					  \
	"boot_a_script="				  \
		"load ${devtype} ${devnum}:${distro_bootpart} "  \
			"${scriptaddr} ${prefix}${script}; "    \
		"env exists secureboot && load ${devtype} "     \
			"${devnum}:${distro_bootpart} "		\
			"${scripthdraddr} ${prefix}${boot_script_hdr} " \
			"&& esbc_validate ${scripthdraddr};"    \
		"source ${scriptaddr}\0"	  \
	"qspi_bootcmd=echo Trying load from qspi..;"	\
		"sf probe && sf read $load_addr "	\
		"$kernel_addr $kernel_size; env exists secureboot "	\
		"&& sf read $kernelheader_addr_r $kernelheader_addr "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0" \
	"nor_bootcmd=echo Trying load from nor..;"	\
		"cp.b $kernel_addr $load_addr "		\
		"$kernel_size; env exists secureboot "	\
		"&& cp.b $kernelheader_addr $kernelheader_addr_r "	\
		"$kernelheader_size && esbc_validate ${kernelheader_addr_r}; " \
		"bootm $load_addr#$board\0"	\
	"sd_bootcmd=echo Trying load from SD ..;"       \
		"mmcinfo && mmc read $load_addr "	\
		"$kernel_addr_sd $kernel_size_sd && "	\
		"env exists secureboot && mmc read $kernelheader_addr_r "		\
		"$kernelhdr_addr_sd $kernelhdr_size_sd "		\
		" && esbc_validate ${kernelheader_addr_r};"	\
		"bootm $load_addr#$board\0"

#define CONFIG_SYS_BOOTM_LEN	(64 << 20) /* Increase max gunzip size */

#endif
