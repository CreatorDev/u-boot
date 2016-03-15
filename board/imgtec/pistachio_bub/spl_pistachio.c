/*
 * Startup Code for MIPS32 CPU-core SPL
 *
 * Copyright (c) 2015 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/mipsregs.h>
#include <asm/mmu.h>
#include <common.h>
#include <linux/sizes.h>
#include <spl.h>
#include "clocks.h"
#include "ddr_init.h"
#include "mfio.h"

#ifdef CONFIG_SPL_BUILD
u32 spl_boot_device(void)
{
	return BOOT_DEVICE_SPI;
}

void spl_board_init(void)
{
	preloader_console_init();
	if (init_ddr()) {
		printf("%s: Error initializing DDR.\n", __func__);
		hang();
	}
	spl_image.flags = 0;
}

void spl_lowlevel_init(void) {
	int ret;

	/*
	 * System PLL divided by 2 -> 350 MHz
	 * The same frequency will be the input frequency for the SPFI block
	 */
	system_clk_setup(1);

	/*
	 * MIPS CPU dividers: division by 1 -> 546 MHz
	 * This is set up as we cannot make any assumption about
	 * the values set or not by the boot ROM code
	 */
	mips_clk_setup(0, 0);

	/* Setup system PLL at 700 MHz */
	ret = sys_pll_setup(2, 1, 13, 350);
	if (ret != CLOCKS_OK)
		return;

	/* Setup MIPS PLL at 546 MHz */
	ret = mips_pll_setup(2, 1, 5, 105);
	if (ret != CLOCKS_OK)
		return;

	/* Move peripheral clock control
	 * from RPU to MIPS, RPU gate register is not managed
	 * in Linux so disable its default values
	 * and assign MIPS gate reg the default values
	 * *Note*: All unused clocks will be gated by Linux
	 */
	setup_clk_gate_defaults();

	/* Setup SPIM1 MFIOs */
	mfio_setup_spim1();

	/*
	 * Setup UART1 clock and MFIOs
	 * System PLL divided by 5 divided by 76 -> 1.8421 Mhz
	 */
	uart1_clk_setup(4, 75);
	mfio_setup_uart1();
	eth_clk_setup(0, 6);
	rom_clk_setup(1);
	usb_clk_setup(6, 2, 7);
#ifdef CONFIG_PISTACHIO_WATCHDOG
	wd_clk_setup(87, 120);
#endif
#if defined(CONFIG_TARGET_PISTACHIO_MARDUK) || \
	defined(CONFIG_TARGET_PISTACHIO_BEETLE)
	mfio_setup_usb_pwr();
#endif
#if defined(CONFIG_TPM) && defined(CONFIG_TPM_TIS_I2C_BUS_NUMBER)
	i2c_clk_setup(3, 2, CONFIG_TPM_TIS_I2C_BUS_NUMBER);
	mfio_setup_i2c(CONFIG_TPM_TIS_I2C_BUS_NUMBER);
#endif
}

void board_init_f(ulong bootflag)
{
	/*
	 * Reserve 1MB before the location where U-Boot will be executed
	 * for the header. U-Boot will be loaded together with header
	 * in a way that places the beginning of the text segment at
	 * CONFIG_SYS_TEXT_BASE, having the header at lower addresses.
	 */
	u32 header_mem_reserved = 1 * 1024 * 1024;

	write_c0_wired(0);
	assert(!identity_map((u32)CONFIG_SYS_SOC_REG_BASE,
			     CONFIG_SYS_SOC_REG_SIZE, C0_ENTRYLO_UC));
	/*
	 * Map the u-boot code and data used before relocation
	 * The memory before CONFIG_SYS_TEXT_BASE -header_mem_reserved
         * is kept unmapped as NULL guard
	 */
	assert(!identity_map((u32)CONFIG_SYS_TEXT_BASE - header_mem_reserved,
			CONFIG_UBOOT_EARLY_MEM + header_mem_reserved,
			C0_ENTRYLO_WB));
	spl_lowlevel_init();
	board_init_r(NULL, 0);
	hang();
}
#endif
