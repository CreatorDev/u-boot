/*
 * Clock set up code for MIPS32 CPU-core SPL
 *
 * Copyright (c) 2015 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "clocks.h"

#define PISTACHIO_CLOCK_SWITCH		0x18144200

#define SYS_EXTERN_PLL_BYPASS_MASK	0x00002000
#define SYS_PLL_CTRL4_ADDR		0x18144048
#define SYS_INTERNAL_PLL_BYPASS_MASK	0x10000000
#define SYS_PLL_PD_CTRL_ADDR		0x18144044
#define SYS_PLL_PD_CTRL_PD_MASK		0x00000039
#define SYS_PLL_DACPD_ADDR		0x18144044
#define SYS_PLL_DACPD_MASK		0x00000002
#define SYS_PLL_DSMPD_ADDR		0x18144044
#define SYS_PLL_DSMPD_MASK		0x00000004

#define MIPS_EXTERN_PLL_BYPASS_MASK	0x00000002
#define MIPS_PLL_CTRL2_ADDR		0x18144008
#define MIPS_INTERNAL_PLL_BYPASS_MASK	0x10000000
#define MIPS_PLL_PD_CTRL_ADDR		0x18144004
#define MIPS_PLL_PD_CTRL_PD_MASK	0x0D000000
#define MIPS_PLL_DSMPD_ADDR		0x18144004
#define MIPS_PLL_DSMPD_MASK		0x02000000

/* Definitions for PLL dividers */
#define SYS_PLL_POSTDIV_ADDR		0x18144040
#define SYS_PLL_POSTDIV1_MASK		0x07000000
#define SYS_PLL_POSTDIV1_SHIFT		24
#define SYS_PLL_POSTDIV2_MASK		0x38000000
#define SYS_PLL_POSTDIV2_SHIFT		27
#define SYS_PLL_STATUS_ADDR		0x18144038
#define SYS_PLL_STATUS_LOCK_MASK	0x00000001

#define SYS_PLL_REFDIV_ADDR		0x1814403C
#define SYS_PLL_REFDIV_MASK		0x0000003F
#define SYS_PLL_REFDIV_SHIFT		0
#define SYS_PLL_FEEDBACK_ADDR		0x1814403C
#define SYS_PLL_FEEDBACK_MASK		0x0003FFC0
#define SYS_PLL_FEEDBACK_SHIFT		6

#define MIPS_PLL_POSTDIV_ADDR		0x18144004
#define MIPS_PLL_POSTDIV1_MASK		0x001C0000
#define MIPS_PLL_POSTDIV1_SHIFT		18
#define MIPS_PLL_POSTDIV2_MASK		0x00E00000
#define MIPS_PLL_POSTDIV2_SHIFT		21
#define MIPS_PLL_STATUS_ADDR		0x18144000
#define MIPS_PLL_STATUS_LOCK_MASK	0x00000001

#define MIPS_REFDIV_ADDR		0x18144004
#define MIPS_REFDIV_MASK		0x0000003F
#define MIPS_REFDIV_SHIFT		0
#define MIPS_FEEDBACK_ADDR		0x18144004
#define MIPS_FEEDBACK_MASK		0x0003FFC0
#define MIPS_FEEDBACK_SHIFT		6

/* Definitions for system clock setup */
#define SYSCLKINTERNAL_CTRL_ADDR	0x18144244
#define SYSCLKINTERNAL_MASK		0X00000007

/* Definitions for MIPS clock setup */
#define MIPSCLKINTERNAL_CTRL_ADDR	0x18144204
#define MIPSCLKINTERNAL_MASK		0x00000003
#define MIPSCLKOUT_CTRL_ADDR		0x18144208
#define MIPSCLKOUT_MASK			0x000000FF

/* Peripheral Clock gate reg */
#define MIPS_CLOCK_GATE			0x18144900
#define RPU_CLOCK_GATE			0x18144904
#define MIPS_CLOCK_GATE_ALL_ON		0x3fff
#define RPU_CLOCK_GATE_ALL_OFF		0x0

/* Definitions for AUXADC clock setup */
#define MIPSCLKGATE_ADDR		0x18144104
#define AUXADC_CLK_INTERNAL_BYPASS_MASK	0x00400000
#define AUXADC_CLK_OUT_BYPASS_MASK	0x00800000
#define AUXADCCLKINTERNAL_CTRL_ADDR	0x18144260
#define AUXADCCLKINTERNAL_CTRL_MASK	0x00000007
#define AUXADCCLKOUT_CTRL_ADDR		0x18144264
#define AUXADCCLKOUT_CTRL_MASK		0x000003FF

/* Definitions for USB clock setup */
#define USBPHYCLKOUT_CTRL_ADDR		0x1814422C
#define USBPHYCLKOUT_MASK		0X0000003F
#define USBPHYCONTROL1_ADDR		0x18149004
#define USBPHYCONTROL1_FSEL_SHIFT	2
#define USBPHYCONTROL1_FSEL_MASK	0x1C
#define USBPHYSTRAPCTRL_ADDR		0x18149010
#define USBPHYSTRAPCTRL_REFCLKSEL_SHIFT	4
#define USBPHYSTRAPCTRL_REFCLKSEL_MASK	0x30
#define USBPHYSTATUS_ADDR		0x18149014
#define USBPHYSTATUS_RX_PHY_CLK_MASK	0x200
#define USBPHYSTATUS_RX_UTMI_CLK_MASK	0x100
#define USBPHYSTATUS_VBUS_FAULT_MASK	0x80

/* Definitions for UART0/1 setup */
#define UART0CLKINTERNAL_CTRL_ADDR	0x18144234
#define UART0CLKINTERNAL_MASK		0x00000007
#define UART0CLKOUT_CTRL_ADDR		0x18144238
#define UART0CLKOUT_MASK		0x000003FF
#define UART1CLKINTERNAL_CTRL_ADDR	0x1814423C
#define UART1CLKINTERNAL_MASK		0x00000007
#define UART1CLKOUT_CTRL_ADDR		0x18144240
#define UART1CLKOUT_MASK		0x000003FF

/* Definitions for I2C setup */
#define I2CCLKDIV1_CTRL_ADDR(i)		(0x18144800 + 0x013C + (2*(i)*4))
#define I2CCLKDIV1_MASK			0x0000007F
#define I2CCLKOUT_CTRL_ADDR(i)		(0x18144800 + 0x0140 + (2*(i)*4))
#define I2CCLKOUT_MASK			0x0000007F

/* Definitions for ROM clock setup */
#define ROMCLKOUT_CTRL_ADDR		0x1814490C
#define ROMCLKOUT_MASK			0x0000007F

/* Definitions for ETH clock setup */
#define ENETCLKMUX_MASK			0x00004000
#define ENETCLKDIV_CTRL_ADDR		0x18144230
#define ENETCLKDIV_MASK			0x0000003F

/* Definitions for timeout values */
#define PLL_TIMEOUT_VALUE_US		20000
#define USB_TIMEOUT_VALUE_US		200000
#define SYS_CLK_LOCK_DELAY		3

/* Definitions for Watchdog setup */
#define WDCLKDIV1_CTRL_ADDR		(0x18144800 + 0x0124)
#define WDCLKDIV1_MASK			0x0000007F
#define WDCLKOUT_CTRL_ADDR		(0x18144800 + 0x0128)
#define WDCLKOUT_MASK			0x0000007F

#ifdef CONFIG_SPL_BUILD
struct pll_parameters {
	u32 external_bypass_mask;
	u32 ctrl_addr;
	u32 internal_bypass_mask;
	u32 power_down_ctrl_addr;
	u32 power_down_ctrl_mask;
	u32 dacpd_addr;
	u32 dacpd_mask;
	u32 dsmpd_addr;
	u32 dsmpd_mask;
	u32 postdiv_addr;
	u32 postdiv1_shift;
	u32 postdiv1_mask;
	u32 postdiv2_shift;
	u32 postdiv2_mask;
	u32 status_addr;
	u32 status_lock_mask;
	u32 refdivider;
	u32 refdiv_addr;
	u32 refdiv_shift;
	u32 refdiv_mask;
	u32 feedback;
	u32 feedback_addr;
	u32 feedback_shift;
	u32 feedback_mask;
};

enum plls {
	SYS_PLL = 0,
	MIPS_PLL = 1
};

static struct pll_parameters pll_params[] = {
	[SYS_PLL] = {
		.external_bypass_mask = SYS_EXTERN_PLL_BYPASS_MASK,
		.ctrl_addr = SYS_PLL_CTRL4_ADDR,
		.internal_bypass_mask = SYS_INTERNAL_PLL_BYPASS_MASK,
		.power_down_ctrl_addr = SYS_PLL_PD_CTRL_ADDR,
		.power_down_ctrl_mask = SYS_PLL_PD_CTRL_PD_MASK,
		/* Noise cancellation */
		.dacpd_addr = SYS_PLL_DACPD_ADDR,
		.dacpd_mask = SYS_PLL_DACPD_MASK,
		.dsmpd_addr = SYS_PLL_DSMPD_ADDR,
		/*
		 * 0 - Integer mode
		 * SYS_PLL_DSMPD_MASK - Fractional mode
		 */
		.dsmpd_mask = 0,
		.postdiv_addr = SYS_PLL_POSTDIV_ADDR,
		.postdiv1_shift = SYS_PLL_POSTDIV1_SHIFT,
		.postdiv1_mask = SYS_PLL_POSTDIV1_MASK,
		.postdiv2_shift = SYS_PLL_POSTDIV2_SHIFT,
		.postdiv2_mask = SYS_PLL_POSTDIV2_MASK,
		.status_addr = SYS_PLL_STATUS_ADDR,
		.status_lock_mask = SYS_PLL_STATUS_LOCK_MASK,
		.refdivider = 0, /* Not defined yet */
		.refdiv_addr = SYS_PLL_REFDIV_ADDR,
		.refdiv_shift = SYS_PLL_REFDIV_SHIFT,
		.refdiv_mask = SYS_PLL_REFDIV_MASK,
		.feedback = 0, /* Not defined yet */
		.feedback_addr = SYS_PLL_FEEDBACK_ADDR,
		.feedback_shift = SYS_PLL_FEEDBACK_SHIFT,
		.feedback_mask = SYS_PLL_FEEDBACK_MASK
	},

	[MIPS_PLL] = {
		.external_bypass_mask = MIPS_EXTERN_PLL_BYPASS_MASK,
		.ctrl_addr = MIPS_PLL_CTRL2_ADDR,
		.internal_bypass_mask = MIPS_INTERNAL_PLL_BYPASS_MASK,
		.power_down_ctrl_addr = MIPS_PLL_PD_CTRL_ADDR,
		.power_down_ctrl_mask = MIPS_PLL_PD_CTRL_PD_MASK,
		.dacpd_addr = 0,
		.dacpd_mask = 0,
		.dsmpd_addr = MIPS_PLL_DSMPD_ADDR,
		.dsmpd_mask = MIPS_PLL_DSMPD_MASK,
		.postdiv_addr = MIPS_PLL_POSTDIV_ADDR,
		.postdiv1_shift = MIPS_PLL_POSTDIV1_SHIFT,
		.postdiv1_mask = MIPS_PLL_POSTDIV1_MASK,
		.postdiv2_shift = MIPS_PLL_POSTDIV2_SHIFT,
		.postdiv2_mask = MIPS_PLL_POSTDIV2_MASK,
		.status_addr = MIPS_PLL_STATUS_ADDR,
		.status_lock_mask = MIPS_PLL_STATUS_LOCK_MASK,
		.refdivider = 0, /* Not defined yet */
		.refdiv_addr = MIPS_REFDIV_ADDR,
		.refdiv_shift = MIPS_REFDIV_SHIFT,
		.refdiv_mask = MIPS_REFDIV_MASK,
		.feedback = 0, /* Not defined yet */
		.feedback_addr = MIPS_FEEDBACK_ADDR,
		.feedback_shift = MIPS_FEEDBACK_SHIFT,
		.feedback_mask = MIPS_FEEDBACK_MASK
	}
};

void system_clk_setup(u8 divider)
{
	u32 reg;
	/* Check input parameters */
	assert(!(divider & ~(SYSCLKINTERNAL_MASK)));

	/* Set system clock divider */
	reg = readl(SYSCLKINTERNAL_CTRL_ADDR);
	reg &= ~SYSCLKINTERNAL_MASK;
	reg |= divider & SYSCLKINTERNAL_MASK;
	writel(reg, SYSCLKINTERNAL_CTRL_ADDR);
	/* Small delay to cover a maximum lock time of 1500 cycles */
	udelay(SYS_CLK_LOCK_DELAY);
}


/*
 * MIPS CPU dividers: division by 1 -> 546 MHz
 * This is set up as we cannot make any assumption about
 * the values set or not by the boot ROM code
 */
void mips_clk_setup(u8 divider1, u8 divider2)
{
	u32 reg;

	/* Check input parameters */
	assert(!(divider1 & ~(MIPSCLKINTERNAL_MASK)));
	assert(!(divider2 & ~(MIPSCLKOUT_MASK)));

	/* Set divider 1 */
	reg = readl(MIPSCLKINTERNAL_CTRL_ADDR);
	reg &= ~MIPSCLKINTERNAL_MASK;
	reg |= divider1 & MIPSCLKINTERNAL_MASK;
	writel(reg, MIPSCLKINTERNAL_CTRL_ADDR);

	/* Set divider 2 */
	reg = readl(MIPSCLKOUT_CTRL_ADDR);
	reg &= ~MIPSCLKOUT_MASK;
	reg |= divider2 & MIPSCLKOUT_MASK;
	writel(reg, MIPSCLKOUT_CTRL_ADDR);
}

static int pll_setup(struct pll_parameters *param,
		    u8 divider1, u8 divider2)
{
	u32 reg;

	/* Check input parameters */
	assert(!((divider1 << param->postdiv1_shift) &
		~(param->postdiv1_mask)));
	assert(!((divider2 << param->postdiv2_shift) &
		~(param->postdiv2_mask)));

	/* Temporary bypass PLL (select XTAL as clock input) */
	reg = readl(PISTACHIO_CLOCK_SWITCH);
	reg &= ~(param->external_bypass_mask);
	writel(reg, PISTACHIO_CLOCK_SWITCH);

	/* Un-bypass PLL's internal bypass */
	reg = readl(param->ctrl_addr);
	reg &= ~(param->internal_bypass_mask);
	writel(reg, param->ctrl_addr);

	/* Disable power down */
	reg = readl(param->power_down_ctrl_addr);
	reg &= ~(param->power_down_ctrl_mask);
	writel(reg, param->power_down_ctrl_addr);

	/* Noise cancellation */
	if (param->dacpd_addr) {
		reg = readl(param->dacpd_addr);
		reg &= ~(param->dacpd_mask);
		writel(reg, param->dacpd_addr);
	}

	/* Functional mode */
	if (param->dsmpd_addr) {
		reg = readl(param->dsmpd_addr);
		reg &= ~(param->dsmpd_mask);
		writel(reg, param->dsmpd_addr);
	}

	if (param->feedback_addr) {
		assert(!((param->feedback << param->feedback_shift) &
			~(param->feedback_mask)));
		reg = readl(param->feedback_addr);
		reg &= ~(param->feedback_mask);
		reg |= (param->feedback << param->feedback_shift) &
			param->feedback_mask;
		writel(reg, param->feedback_addr);
	}

	if (param->refdiv_addr) {
		assert(!((param->refdivider << param->refdiv_shift) &
			~(param->refdiv_mask)));
		reg = readl(param->refdiv_addr);
		reg &= ~(param->refdiv_mask);
		reg |= (param->refdivider << param->refdiv_shift) &
			param->refdiv_mask;
		writel(reg, param->refdiv_addr);
	}

	/* Read postdivider register value */
	reg = readl(param->postdiv_addr);
	/* Set divider 1 */
	reg &= ~(param->postdiv1_mask);
	reg |= (divider1 << param->postdiv1_shift) &
			param->postdiv1_mask;
	/* Set divider 2 */
	reg &= ~(param->postdiv2_mask);
	reg |= (divider2 << param->postdiv2_shift) &
			param->postdiv2_mask;
	/* Write back to register */
	writel(reg, param->postdiv_addr);

	/* Waiting for PLL to lock*/
	int count = 0;
	while(!(readl(param->status_addr) & param->status_lock_mask)){
		count = count + 1000;
		udelay(1000);
		if(count >= PLL_TIMEOUT_VALUE_US)
			return PLL_TIMEOUT;
	}

	/* Start using PLL */
	reg = readl(PISTACHIO_CLOCK_SWITCH);
	reg |= param->external_bypass_mask;
	writel(reg, PISTACHIO_CLOCK_SWITCH);

	return CLOCKS_OK;
}

/* Setup system PLL at 700 MHz */
int sys_pll_setup(u8 divider1, u8 divider2, u8 refdivider, u32 feedback)
{
	pll_params[SYS_PLL].refdivider = refdivider;
	pll_params[SYS_PLL].feedback = feedback;
	return pll_setup(&(pll_params[SYS_PLL]), divider1, divider2);
}


/* Setup MIPS PLL at 546 MHz */
int mips_pll_setup(u8 divider1, u8 divider2, u8 refdivider, u32 feedback)
{
	pll_params[MIPS_PLL].refdivider = refdivider;
	pll_params[MIPS_PLL].feedback = feedback;
	return pll_setup(&(pll_params[MIPS_PLL]), divider1, divider2);
}

/*
 * Setup UART1 clock and MFIOs
 * System PLL divided by 5 divided by 76 -> 1.8421 Mhz
 */
void uart1_clk_setup(u8 divider1, u16 divider2)
{
	u32 reg;

	/* Check input parameters */
	assert(!(divider1 & ~(UART1CLKINTERNAL_MASK)));
	assert(!(divider2 & ~(UART1CLKOUT_MASK)));

	/* Set divider 1 */
	reg = readl(UART1CLKINTERNAL_CTRL_ADDR);
	reg &= ~UART1CLKINTERNAL_MASK;
	reg |= divider1 & UART1CLKINTERNAL_MASK;
	writel(reg, UART1CLKINTERNAL_CTRL_ADDR);

	/* Set divider 2 */
	reg = readl(UART1CLKOUT_CTRL_ADDR);
	reg &= ~UART1CLKOUT_MASK;
	reg |= divider2 & UART1CLKOUT_MASK;
	writel(reg, UART1CLKOUT_CTRL_ADDR);
}

/* usb_clk_setup: sets up USB clock */
int usb_clk_setup(u8 divider, u8 refclksel, u8 fsel)
{
	u32 reg;

	/* Check input parameters */
	assert(!(divider & ~(USBPHYCLKOUT_MASK)));
	assert(!((refclksel << USBPHYSTRAPCTRL_REFCLKSEL_SHIFT) &
		~(USBPHYSTRAPCTRL_REFCLKSEL_MASK)));
	assert(!((fsel << USBPHYCONTROL1_FSEL_SHIFT) &
		~(USBPHYCONTROL1_FSEL_MASK)));

	/* Set USB divider */
	reg = readl(USBPHYCLKOUT_CTRL_ADDR);
	reg &= ~USBPHYCLKOUT_MASK;
	reg |= divider & USBPHYCLKOUT_MASK;
	writel(reg, USBPHYCLKOUT_CTRL_ADDR);

	/* Set REFCLKSEL */
	reg = readl(USBPHYSTRAPCTRL_ADDR);
	reg &= ~USBPHYSTRAPCTRL_REFCLKSEL_MASK;
	reg |= (refclksel << USBPHYSTRAPCTRL_REFCLKSEL_SHIFT) &
		USBPHYSTRAPCTRL_REFCLKSEL_MASK;
	writel(reg, USBPHYSTRAPCTRL_ADDR);

	/* Set FSEL */
	reg = readl(USBPHYCONTROL1_ADDR);
	reg &= ~USBPHYCONTROL1_FSEL_MASK;
	reg |= (fsel << USBPHYCONTROL1_FSEL_SHIFT) &
		USBPHYCONTROL1_FSEL_MASK;
	writel(reg, USBPHYCONTROL1_ADDR);

	/* Waiting for USB clock status */
	int count = 0;
	while(1){
		reg = readl(USBPHYSTATUS_ADDR);
		count = count + 1000;
		udelay(1000);
		if (reg & USBPHYSTATUS_VBUS_FAULT_MASK)
			return USB_VBUS_FAULT;
		if(count >= USB_TIMEOUT_VALUE_US)
			return PLL_TIMEOUT;
		if ((reg & USBPHYSTATUS_RX_PHY_CLK_MASK) &&
			(reg & USBPHYSTATUS_RX_UTMI_CLK_MASK))
			break;
	}

	return CLOCKS_OK;
}

void setup_clk_gate_defaults(void)
{
	writel(MIPS_CLOCK_GATE_ALL_ON, MIPS_CLOCK_GATE);
	writel(RPU_CLOCK_GATE_ALL_OFF, RPU_CLOCK_GATE);
}

void rom_clk_setup(u8 divider)
{
	u32 reg;

	/* Check input parameter */
	assert(!(divider & ~(ROMCLKOUT_MASK)));

	/* Set ROM divider */
	reg = readl(ROMCLKOUT_CTRL_ADDR);
	reg &= ~ROMCLKOUT_MASK;
	reg |= divider & ROMCLKOUT_MASK;
	writel(reg, ROMCLKOUT_CTRL_ADDR);
}

void eth_clk_setup(u8 mux, u8 divider)
{

	u32 reg;

	/* Check input parameters */
	assert(!(divider & ~(ENETCLKDIV_MASK)));
	/* This can be either 0 or 1, selecting between
	 * ENET and system clock as clocksource */
	assert(!(mux & ~(0x1)));

	/* Set ETH divider */
	reg = readl(ENETCLKDIV_CTRL_ADDR);
	reg &= ~ENETCLKDIV_MASK;
	reg |= divider & ENETCLKDIV_MASK;
	writel(reg, ENETCLKDIV_CTRL_ADDR);

	/* Select source */
	if (mux) {
		reg = readl(PISTACHIO_CLOCK_SWITCH);
		reg |= ENETCLKMUX_MASK;
		writel(reg, PISTACHIO_CLOCK_SWITCH);
	}
}

/*
 * i2c_clk_setup: sets up clocks for I2C
 * divider1: 7-bit divider value
 * divider2: 7-bit divider value
 */
void i2c_clk_setup(u8 divider1, u16 divider2, u8 interface)
{
	u32 reg;

	/* Check input parameters */
	assert(!(divider1 & ~(I2CCLKDIV1_MASK)));
	assert(!(divider2 & ~(I2CCLKOUT_MASK)));
	assert(interface < 4);
	/* Set divider 1 */
	reg = readl(I2CCLKDIV1_CTRL_ADDR(interface));
	reg &= ~I2CCLKDIV1_MASK;
	reg |= divider1 & I2CCLKDIV1_MASK;
	writel(reg, I2CCLKDIV1_CTRL_ADDR(interface));

	/* Set divider 2 */
	reg = readl(I2CCLKOUT_CTRL_ADDR(interface));
	reg &= ~I2CCLKOUT_MASK;
	reg |= divider2 & I2CCLKOUT_MASK;
	writel(reg, I2CCLKOUT_CTRL_ADDR(interface));
}

/*
 * wd_clk_setup: sets up clocks for pistachio watchdog
 * divider1: 7-bit divider value
 * divider2: 7-bit divider value
 */
#ifdef CONFIG_PISTACHIO_WATCHDOG
void wd_clk_setup(u8 divider1, u16 divider2)
{
	u32 reg;

	/* Check input parameters */
	assert(!(divider1 & ~(WDCLKDIV1_MASK)));
	assert(!(divider2 & ~(WDCLKOUT_MASK)));

	/* Set divider 1 */
	reg = readl(WDCLKDIV1_CTRL_ADDR);
	reg &= ~WDCLKDIV1_MASK;
	reg |= divider1 & WDCLKDIV1_MASK;
	writel(reg, WDCLKDIV1_CTRL_ADDR);

	/* Set divider 2 */
	reg = readl(WDCLKOUT_CTRL_ADDR);
	reg &= ~WDCLKOUT_MASK;
	reg |= divider2 & WDCLKOUT_MASK;
	writel(reg, WDCLKOUT_CTRL_ADDR);
}
#endif /*CONFIG_PISTACHIO_WATCHDOG*/
#endif
