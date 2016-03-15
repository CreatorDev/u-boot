/*
 * Copyright (C) 2015 Imagination Technologies
 * Author: Govindraj Raja <govindraj.raja@imgtec.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <asm/pistachio.h>
#include "mfio.h"

static struct pistachio_mfio_reg pistachio_mfio_regs[] =
{
	{0, 0},
	{MFIO_1_FUNC_SEL_START, MFIO_1_FUNC_SEL_END},
	{MFIO_2_FUNC_SEL_START, MFIO_2_FUNC_SEL_END},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{MFIO_15_FUNC_SEL_START, MFIO_15_FUNC_SEL_END},
	{MFIO_16_FUNC_SEL_START, MFIO_16_FUNC_SEL_END},
	{MFIO_17_FUNC_SEL_START, MFIO_17_FUNC_SEL_END},
	{MFIO_18_FUNC_SEL_START, MFIO_18_FUNC_SEL_END},
	{MFIO_19_FUNC_SEL_START, MFIO_19_FUNC_SEL_END},
	{MFIO_20_FUNC_SEL_START, MFIO_20_FUNC_SEL_END},
	{MFIO_21_FUNC_SEL_START, MFIO_21_FUNC_SEL_END},
	{MFIO_22_FUNC_SEL_START, MFIO_22_FUNC_SEL_END},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{MFIO_28_FUNC_SEL_START, MFIO_28_FUNC_SEL_END},
	{MFIO_29_FUNC_SEL_START, MFIO_29_FUNC_SEL_END},
	{MFIO_30_FUNC_SEL_START, MFIO_30_FUNC_SEL_END},
	{MFIO_31_FUNC_SEL_START, MFIO_31_FUNC_SEL_END},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{MFIO_36_FUNC_SEL_START, MFIO_36_FUNC_SEL_END},
	{MFIO_37_FUNC_SEL_START, MFIO_37_FUNC_SEL_END},
	{MFIO_38_FUNC_SEL_START, MFIO_38_FUNC_SEL_END},
	{MFIO_39_FUNC_SEL_START, MFIO_39_FUNC_SEL_END},
	{MFIO_40_FUNC_SEL_START, MFIO_40_FUNC_SEL_END},
	{MFIO_41_FUNC_SEL_START, MFIO_41_FUNC_SEL_END},
	{MFIO_42_FUNC_SEL_START, MFIO_42_FUNC_SEL_END},
	{MFIO_43_FUNC_SEL_START, MFIO_43_FUNC_SEL_END},
	{0, 0},
	{MFIO_45_FUNC_SEL_START, MFIO_45_FUNC_SEL_END},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{MFIO_54_FUNC_SEL_START, MFIO_54_FUNC_SEL_END},
	{MFIO_55_FUNC_SEL_START, MFIO_55_FUNC_SEL_END},
	{MFIO_56_FUNC_SEL_START, MFIO_56_FUNC_SEL_END},
	{MFIO_57_FUNC_SEL_START, MFIO_57_FUNC_SEL_END},
	{MFIO_58_FUNC_SEL_START, MFIO_58_FUNC_SEL_END},
	{0, 0},
	{0, 0},
	{0, 0},
	{0, 0},
	{MFIO_63_FUNC_SEL_START, MFIO_63_FUNC_SEL_END},
	{MFIO_64_FUNC_SEL_START, MFIO_64_FUNC_SEL_END},
	{MFIO_65_FUNC_SEL_START, MFIO_65_FUNC_SEL_END},
	{MFIO_66_FUNC_SEL_START, MFIO_66_FUNC_SEL_END},
	{MFIO_67_FUNC_SEL_START, MFIO_67_FUNC_SEL_END},
	{MFIO_68_FUNC_SEL_START, MFIO_68_FUNC_SEL_END},
	{MFIO_69_FUNC_SEL_START, MFIO_69_FUNC_SEL_END},
	{MFIO_70_FUNC_SEL_START, MFIO_70_FUNC_SEL_END},
	{0, 0},
	{0, 0},
	{MFIO_73_FUNC_SEL_START, MFIO_73_FUNC_SEL_END},
	{MFIO_74_FUNC_SEL_START, MFIO_74_FUNC_SEL_END},
	{MFIO_75_FUNC_SEL_START, MFIO_75_FUNC_SEL_END},
	{MFIO_76_FUNC_SEL_START, MFIO_76_FUNC_SEL_END},
	{MFIO_77_FUNC_SEL_START, MFIO_77_FUNC_SEL_END},
	{MFIO_78_FUNC_SEL_START, MFIO_78_FUNC_SEL_END},
	{MFIO_79_FUNC_SEL_START, MFIO_79_FUNC_SEL_END},
	{MFIO_80_FUNC_SEL_START, MFIO_80_FUNC_SEL_END},
	{MFIO_81_FUNC_SEL_START, MFIO_81_FUNC_SEL_END},
	{MFIO_82_FUNC_SEL_START, MFIO_82_FUNC_SEL_END},
	{MFIO_83_FUNC_SEL_START, MFIO_83_FUNC_SEL_END},
	{MFIO_84_FUNC_SEL_START, MFIO_84_FUNC_SEL_END},
	{MFIO_85_FUNC_SEL_START, MFIO_85_FUNC_SEL_END},
	{MFIO_86_FUNC_SEL_START, MFIO_86_FUNC_SEL_END},
	{MFIO_87_FUNC_SEL_START, MFIO_87_FUNC_SEL_END},
	{MFIO_88_FUNC_SEL_START, MFIO_88_FUNC_SEL_END},
	{MFIO_89_FUNC_SEL_START, MFIO_89_FUNC_SEL_END},
};

static void pistachio_select_gpio(u32 mfio, bool gpio)
{
	u32 reg, val;

	if(gpio)
	{
	/* make GPIO input */
		reg = PISTACHIO_GPIO + 0x204 + (((mfio) / 16) * 0x24);
		val = 0x10000 << (mfio % 16);
		__raw_writel(val, reg);
	}

	reg = PISTACHIO_GPIO + 0x200 + (((mfio) / 16) * 0x24);
	val = 0x10000 | ((gpio) ? (1) : (0));
	val <<= (mfio % 16);

	__raw_writel(val, reg);
}

static void pistachio_select_mfio(u32 mfio, u32 func)
{
	u32 reg, val, mask, shift;

	reg = PISTACHIO_GPIO + 0xC0 +
			(((pistachio_mfio_regs[mfio].startbit) / 32) * 4);
	mask = (1UL << ((pistachio_mfio_regs[mfio].endbit -
			pistachio_mfio_regs[mfio].startbit) + 1)) - 1;
	shift = pistachio_mfio_regs[mfio].startbit % 32;
	mask <<= shift;

	val = __raw_readl(reg);
	val &= ~mask;
	val |= (func << shift) & mask;
	__raw_writel(val, reg);
}

static void pistachio_deselectgpio_selectmfio(u32 mfio, u32 func)
{
        pistachio_select_mfio(mfio, func);
        pistachio_select_gpio(mfio, 0);
}

static void set_slew_rate(u32 mfio)
{
	u32 reg, val, mask, shift;

	reg = PISTACHIO_GPIO + 0x100 + ((mfio / 32 ) * 4);
	shift = mfio % 32;
	mask = 1UL << shift;

	val = __raw_readl(reg);
	val |= mask;
	__raw_writel(val, reg);
}

static void set_drive_strength(u32 mfio, u8 drive_strength)
{
	u32 reg, val, mask, shift;

	reg = PISTACHIO_GPIO + 0x120 + ((mfio / 16 ) * 4);

	shift = (mfio % 16) * 2;
	mask = 3UL << shift;

	val = __raw_readl(reg);
	val &= ~mask;
	val |= ((drive_strength / 4) << shift);

	__raw_writel(val, reg);
}

void mfio_setup_ethernet(void)
{
	pistachio_deselectgpio_selectmfio(63, 0);
	pistachio_deselectgpio_selectmfio(64, 0);
	pistachio_deselectgpio_selectmfio(65, 0);
	pistachio_deselectgpio_selectmfio(66, 0);
	pistachio_deselectgpio_selectmfio(67, 0);
	pistachio_deselectgpio_selectmfio(68, 0);
	pistachio_deselectgpio_selectmfio(69, 0);
	pistachio_deselectgpio_selectmfio(70, 0);
	pistachio_select_gpio(71, 0);

	set_slew_rate(63);
	set_slew_rate(64);
	set_slew_rate(65);
	set_slew_rate(66);
	set_slew_rate(67);
	set_slew_rate(68);
	set_slew_rate(69);
	set_slew_rate(70);
	set_slew_rate(71);

	set_drive_strength(63, 4);
	set_drive_strength(64, 4);
	set_drive_strength(65, 4);
	set_drive_strength(66, 4);
	set_drive_strength(67, 4);
	set_drive_strength(68, 4);
	set_drive_strength(69, 4);
	set_drive_strength(70, 4);

	set_drive_strength(71, 8);
}

void mfio_setup_mmc(void)
{
	pistachio_deselectgpio_selectmfio(15, 0);
	pistachio_deselectgpio_selectmfio(16, 0);
	pistachio_deselectgpio_selectmfio(17, 0);
	pistachio_deselectgpio_selectmfio(18, 0);
	pistachio_deselectgpio_selectmfio(19, 0);
	pistachio_deselectgpio_selectmfio(20, 0);
	pistachio_deselectgpio_selectmfio(21, 0);
	pistachio_deselectgpio_selectmfio(22, 0);

	pistachio_select_gpio(23, 0);
	pistachio_select_gpio(24, 0);
	pistachio_select_gpio(25, 0);
	pistachio_select_gpio(26, 0);
	pistachio_select_gpio(27, 0);

	set_slew_rate(15);
	set_slew_rate(16);
	set_slew_rate(17);
	set_slew_rate(18);
	set_slew_rate(19);
	set_slew_rate(20);
	set_slew_rate(21);
	set_slew_rate(22);
	set_slew_rate(23);
	set_slew_rate(24);
	set_slew_rate(25);

	set_drive_strength(15, 2);
	set_drive_strength(16, 4);
	set_drive_strength(17, 4);
	set_drive_strength(18, 4);
	set_drive_strength(19, 4);
	set_drive_strength(20, 4);
	set_drive_strength(21, 4);
	set_drive_strength(22, 4);
	set_drive_strength(23, 4);
	set_drive_strength(24, 4);
	set_drive_strength(25, 4);
}

/* Setup SPIM1 MFIOs */
void mfio_setup_spim1(void)
{
	/* Use CS lines as GPIO */
	pistachio_select_gpio(0, 0);
	/* TODO: separate this according to board (DTS */
	pistachio_select_gpio(1, 0);
	pistachio_select_gpio(58, 0);

	/* Use all other lines as MFIO - controlled by the SPFI block */
	pistachio_deselectgpio_selectmfio(5, 0);
	pistachio_deselectgpio_selectmfio(4, 0);
	pistachio_deselectgpio_selectmfio(3, 0);
	pistachio_deselectgpio_selectmfio(6, 0);
	pistachio_deselectgpio_selectmfio(7, 0);

	/* Set drive strength to maximum for these MFIOs */
	set_drive_strength(5, 12);
	set_drive_strength(4, 12);
	set_drive_strength(3, 12);
	set_drive_strength(6, 12);
	set_drive_strength(7, 12);
}


void mfio_setup_uart1(void)
{
	pistachio_deselectgpio_selectmfio(59, 0);
	pistachio_deselectgpio_selectmfio(60, 0);
}

#define I2C_DATA_MFIO(i)                (28 + (2*(i)))
#define I2C_CLK_MFIO(i)                 (29 + (2*(i)))
void mfio_setup_i2c(u8 interface)
{
	pistachio_deselectgpio_selectmfio(I2C_DATA_MFIO(interface), 0);
	pistachio_deselectgpio_selectmfio(I2C_CLK_MFIO(interface), 0);
}

#if defined(CONFIG_TARGET_PISTACHIO_MARDUK) || \
	defined(CONFIG_TARGET_PISTACHIO_BEETLE)
#define USBPHYCONTROL0_ADDR		0x18149000
#define USBPHYCONTROL0_USB_OTG_DRVVBUS	11
void mfio_setup_usb_pwr(void)
{
	u32 val;

	pistachio_deselectgpio_selectmfio(86, 0);
	/*
	 * Allow USB block to control the VBUS on USB_PWR_ON(MFIO 86);
	 * this will override rpu_l_pll.
	 *
	 */
	val = __raw_readl(USBPHYCONTROL0_ADDR);
	val |= 1 << USBPHYCONTROL0_USB_OTG_DRVVBUS;
	__raw_writel(val, USBPHYCONTROL0_ADDR);
}
#endif
