/*
 * Clock set up code header file for MIPS32 CPU-core SPL
 *
 * Copyright (c) 2015 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __IMGTEC_PISTACHIO_CLOCKS_H__
#define __IMGTEC_PISTACHIO_CLOCKS_H__

#include <common.h>
#include <asm/io.h>

/* Functions for PLL setting */
int sys_pll_setup(u8 divider1, u8 divider2, u8 predivider, u32 feedback);
int mips_pll_setup(u8 divider1, u8 divider2, u8 predivider, u32 feedback);

/* Peripheral divider setting */
void system_clk_setup(u8 divider);
void mips_clk_setup(u8 divider1, u8 divider2);
void uart1_clk_setup(u8 divider1, u16 divider2);
void i2c_clk_setup(u8 divider1, u16 divider2, u8 interface);
int usb_clk_setup(u8 divider, u8 refclksel, u8 fsel);
void rom_clk_setup(u8 divider);
void eth_clk_setup(u8 mux, u8 divider);
void setup_clk_gate_defaults(void);
#ifdef CONFIG_PISTACHIO_WATCHDOG
void wd_clk_setup(u8 divider1, u16 divider2);
#endif /*CONFIG_PISTACHIO_WATCHDOG*/

enum {
	CLOCKS_OK = 0,
	PLL_TIMEOUT = -1,
	USB_TIMEOUT = -2,
	USB_VBUS_FAULT = -3
};

#endif
