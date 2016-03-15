/*
 * Copyright (C) 2015 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _PISTACHIO_CONFIG_MARDUK_H
#define _PISTACHIO_CONFIG_MARDUK_H

#include <configs/pistachio_bub.h>

#ifdef NAND_BOOT
#ifdef CONFIG_SYS_SPI1_CS1_GPIO
#undef CONFIG_SYS_SPI1_CS1_GPIO
#endif /* CONFIG_SYS_SPI1_CS1_GPIO */
#define CONFIG_SYS_SPI1_CS1_GPIO	1
#endif /* NAND_BOOT */

#ifdef CONFIG_TPM
#ifdef CONFIG_TPM_TIS_I2C_BUS_NUMBER
#undef CONFIG_TPM_TIS_I2C_BUS_NUMBER
#endif /* CONFIG_TPM_TIS_I2C_BUS_NUMBER */
#define CONFIG_TPM_TIS_I2C_BUS_NUMBER	2
#endif /* CONFIG_TPM */

#ifdef CONFIG_DRAM_DDR2
#undef CONFIG_DRAM_DDR2
#endif
#define CONFIG_DRAM_DDR3

#endif /* _PISTACHIO_CONFIG_MARDUK_H */
