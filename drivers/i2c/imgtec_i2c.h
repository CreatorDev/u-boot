/*
 * Imgtec I2C driver internal structures
 *
 * Copyright (C) 2015 Imagination Technologies
 * Ionela Voinescu <ionela.voinescu@imgtec.com>
 * Tianci Ma <tianci.ma@imgtec.com>

 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __IMGTEC_I2C_H__
#define __IMGTEC_I2C_H__

/*
 * Information about i2c controller and internal state
 */
struct imgtec_i2c_bus {
	uint32_t clkrate;
	uint32_t int_enable;
	uint8_t local_status;
	uint32_t current_len;
	uint8_t *current_buf;
	uint32_t base;
};

struct imgtec_i2c_bus_platdata {
	uint32_t clkrate;
};

#endif
