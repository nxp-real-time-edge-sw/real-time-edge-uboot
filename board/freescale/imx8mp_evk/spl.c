// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2019, 2021 NXP
 *
 */

#include <common.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <power/pmic.h>

#include <power/pca9450.h>
#include <asm/arch/clock.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <dm/uclass-internal.h>
#include <dm/device-internal.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <asm/arch/ddr.h>

#include "imx8mp_factory_test.h"

DECLARE_GLOBAL_DATA_PTR;

#define GPIO3_BASE_ADDR    0x30220000

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
#ifdef CONFIG_SPL_BOOTROM_SUPPORT
	return BOOT_DEVICE_BOOTROM;
#else
	switch (boot_dev_spl) {
	case SD1_BOOT:
	case MMC1_BOOT:
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD3_BOOT:
	case MMC3_BOOT:
		return BOOT_DEVICE_MMC2;
	case QSPI_BOOT:
		return BOOT_DEVICE_NOR;
	case NAND_BOOT:
		return BOOT_DEVICE_NAND;
	case USB_BOOT:
		return BOOT_DEVICE_BOARD;
	default:
		return BOOT_DEVICE_NONE;
	}
#endif
}

void spl_dram_init(void)
{
	ddr_init(&dram_timing);
}

void spl_board_init(void)
{
	arch_misc_init();

	/*
	 * Set GIC clock to 500Mhz for OD VDD_SOC. Kernel driver does
	 * not allow to change it. Should set the clock after PMIC
	 * setting done. Default is 400Mhz (system_pll1_800m with div = 2)
	 * set by ROM for ND VDD_SOC
	 */
#if defined(CONFIG_IMX8M_LPDDR4) && !defined(CONFIG_IMX8M_VDD_SOC_850MV)
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);

	puts("Normal Boot\n");
#endif
}

#if CONFIG_IS_ENABLED(DM_PMIC_PCA9450)
int power_init_board(void)
{
	struct udevice *dev;
	int ret;

	ret = pmic_get("pmic@25", &dev);
	if (ret == -ENODEV) {
		puts("No pca9450@25\n");
		return 0;
	}
	if (ret != 0)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

#ifdef CONFIG_IMX8M_LPDDR4
	/*
	 * increase VDD_SOC to typical value 0.95V before first
	 * DRAM access, set DVS1 to 0.85v for suspend.
	 * Enable DVS control through PMIC_STBY_REQ and
	 * set B1_ENMODE=1 (ON by PMIC_ON_REQ=H)
	 */
#ifdef CONFIG_IMX8M_VDD_SOC_850MV
	/* set DVS0 to 0.85v for special case*/
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x14);
#else
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x1C);
#endif
	pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* Kernel uses OD/OD freq for SOC */
	/* To avoid timing risk from SOC to ARM,increase VDD_ARM to OD voltage 0.95v */
	pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x1C);
#elif defined(CONFIG_IMX8M_DDR4)
	/* DDR4 runs at 3200MTS, uses default ND 0.85v for VDD_SOC and VDD_ARM */
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	/* Set NVCC_DRAM to 1.2v for DDR4 */
	pmic_reg_write(dev, PCA9450_BUCK6OUT, 0x18);
#endif

	return 0;
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

#ifdef CONFIG_FACTORY_TEST

#define TEST_SIZE		0x100000
// Generate test pattern based on address
uint32_t generate_test_pattern(uint64_t base_addr, uint32_t offset)
{
	return (uint32_t)(base_addr >> 8) ^ offset ^ 0xA5A5A5A5;
}

void write_test_data(uint64_t* test_addresses, int num_locations)
{

	for (int loc = 0; loc < num_locations; loc++) {
		volatile uint32_t *test_addr = (uint32_t*)test_addresses[loc];

		uint32_t addr_high = (uint32_t)(test_addresses[loc] >> 32);
		uint32_t addr_low = (uint32_t)(test_addresses[loc] & 0xFFFFFFFF);

		if(addr_high != 0){
			printf("DDRINFO: Writing to location %d: 0x%08x%08x\n", loc + 1, addr_high, addr_low);
		} else {
			printf("DDRINFO: Writing to location %d: 0x%08x\n", loc + 1, addr_low);
		}


		// Write 1MB of test data (256K words)
		for (uint32_t i = 0; i < (TEST_SIZE / 4); i++) {
			uint32_t test_pattern = generate_test_pattern(test_addresses[loc], i);
			test_addr[i] = test_pattern;
		}
		printf(" Location %d write complete\n", loc + 1);
	}
}
void read_and_verify_test_data(uint64_t* test_addresses, int num_locations)
{
	int test_passed = 1;
	uint32_t total_errors = 0;

	for (int loc = 0; loc < num_locations; loc++) {
		volatile uint32_t *test_addr = (uint32_t*)test_addresses[loc];
		uint32_t location_errors = 0;

		uint32_t addr_high = (uint32_t)(test_addresses[loc] >> 32);
		uint32_t addr_low = (uint32_t)(test_addresses[loc] & 0xFFFFFFFF);

		if(addr_high != 0){
			printf("DDRINFO: Verifying location %d: 0x%08x%08x\n", loc + 1, addr_high, addr_low);
		} else {
			printf("DDRINFO: Verifying to location %d: 0x%08x\n", loc + 1, addr_low);
		}
		// Verify 1MB of test data
		for (uint32_t i = 0; i < (TEST_SIZE / 4); i++) {
			uint32_t expected_pattern = generate_test_pattern(test_addresses[loc], i);
			uint32_t read_value = test_addr[i];

			if (read_value != expected_pattern) {
				if (location_errors < 10) {  // Limit error prints
					printf("ERROR at 0x%08x%08x+0x%x: expected 0x%08x, got 0x%08x\n", 
									 addr_high, addr_low, i * 4, expected_pattern, read_value);
				}
				location_errors++;
				test_passed = 0;
			}
		}

		if (location_errors == 0) {
			printf(" Location %d verification PASSED\n", loc + 1);
		} else {
			printf(" Location %d verification FAILED (%d errors)\n", loc + 1, location_errors);
			total_errors += location_errors;
		}
	}
	
	if(test_passed){
		printf("DDRINFO: 1MB DDR test PASSED - All 6 locations verified successfully\n");
	} else {
		printf("DDRINFO: 1MB DDR test FAILED - Total errors: %d\n", total_errors);
	}

}
#endif

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0) {
		printf("Failed to find clock node. Check device tree\n");
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	power_init_board();

	/* DDR initialization */
	spl_dram_init();

#ifdef CONFIG_LED_STATUS
	*(volatile uint32_t *)(GPIO3_BASE_ADDR + 0x04) |= (1 << 16);  // GDIR
	*(volatile uint32_t *)(GPIO3_BASE_ADDR + 0x00) |= (1 << 16);  // DR
#endif

#ifdef CONFIG_FACTORY_TEST
	run_factory_test();
#else
	board_init_r(NULL, 0);
#endif

}
