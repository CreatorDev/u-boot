/*
 * Copyright (C) 2016 Imagination Technologies
 * Author: Avinash Tahakik <avinash.tahakik@imgtec.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Notes
 * -----
 * The timeout value is rounded to the next power of two clock cycles.
 * This is configured using the PDC_WDT_CONFIG register, according to this
 * formula:
 *
 *     timeout = 2^(delay + 1) clock cycles
 *
 * Where 'delay' is the value written in PDC_WDT_CONFIG register.
 *
 * Therefore, the hardware only allows to program watchdog timeouts, expressed
 * as a power of two number of watchdog clock cycles. The current implementation
 * guarantees that the actual watchdog timeout will be _at least_ the value
 * programmed in the imgpdg_wdt driver.
 *
 * The following table shows how the user-configured timeout relates
 * to the actual hardware timeout (watchdog clock @ 40000 Hz):
 *
 * input timeout | WD_DELAY | actual timeout
 * -----------------------------------
 *      10       |   18     |  13 seconds
 *      20       |   19     |  26 seconds
 *      30       |   20     |  52 seconds
 *      60       |   21     |  104 seconds
 *
 * Albeit coarse, this granularity would suffice most watchdog uses.
 * If the platform allows it, the user should be able to change the watchdog
 * clock rate and achieve a finer timeout granularity.
 */

#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/log2.h>

/* registers */

#define PDC_WDT_BASE				0x18102100
#define PDC_WDT_SOFT_RESET			0x00
#define PDC_WDT_SOFT_RESET_VALUE	0X1
#define PDC_WDT_CONFIG				0x04
#define PDC_WDT_CONFIG_ENABLE		(1 << 31)
#define PDC_WDT_CONFIG_DELAY_MASK	0x1f

#define PDC_WDT_TICKLE1				0x08
#define PDC_WDT_TICKLE1_MAGIC		0xabcd1234
#define PDC_WDT_TICKLE2				0x0c
#define PDC_WDT_TICKLE2_MAGIC		0x4321dcba

#define CLOCK_RATE					32871

#ifndef CONFIG_PISTACHIO_WATCHDOG_TIMEOUT_SEC
#define CONFIG_PISTACHIO_WATCHDOG_TIMEOUT_SEC 60
#endif

static void wdt_keepalive(void)
{
	writel(PDC_WDT_TICKLE1_MAGIC, PDC_WDT_BASE + PDC_WDT_TICKLE1);
	writel(PDC_WDT_TICKLE2_MAGIC, PDC_WDT_BASE + PDC_WDT_TICKLE2);
}

static void wdt_set_timeout(unsigned int timeout)
{
	unsigned int val;

	val = readl(PDC_WDT_BASE + PDC_WDT_CONFIG) & ~PDC_WDT_CONFIG_DELAY_MASK;
	val |= order_base_2(timeout * CLOCK_RATE) - 1;
	writel(val, PDC_WDT_BASE + PDC_WDT_CONFIG);
}

void hw_watchdog_reset(void)
{
	wdt_keepalive();
}

void hw_watchdog_init(void)
{
	unsigned int val;

	wdt_set_timeout(CONFIG_PISTACHIO_WATCHDOG_TIMEOUT_SEC);

	val = readl(PDC_WDT_BASE + PDC_WDT_CONFIG);
	val |= PDC_WDT_CONFIG_ENABLE;
	writel(val, PDC_WDT_BASE + PDC_WDT_CONFIG);
}
