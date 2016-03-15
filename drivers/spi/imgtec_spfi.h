/*
 * Imgtec SPI driver
 *
 * Copyright (C) 2015 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DRIVERS_SPI_IMGTEC_SPFI_H__
#define __DRIVERS_SPI_IMGTEC_SPFI_H__

/* This type defines the SPI Mode.*/
enum spim_mode {
	/* Mode 0 (clock idle low, data valid on first clock transition). */
	SPIM_MODE_0 = 0,
	/* Mode 1 (clock idle low, data valid on second clock transition). */
	SPIM_MODE_1,
	/* Mode 2 (clock idle high, data valid on first clock transition). */
	SPIM_MODE_2,
	/* Mode 3 (clock idle high, data valid on second clock transition). */
	SPIM_MODE_3

};

/* This structure defines communication parameters for a slave device */
struct spim_device_parameters {
	/* Bit rate value.*/
	unsigned char bitrate;
	/*
	 * Chip select set up time.
	 * Time taken between chip select going active and activity occurring
	 * on the clock, calculated by dividing the desired set up time in ns
	 * by the Input clock period. (setup time / Input clock freq)
	 */
	unsigned char cs_setup;
	/*
	 * Chip select hold time.
	 * Time after the last clock pulse before chip select goes inactive,
	 * calculated by dividing the desired hold time in ns by the
	 * Input clock period (hold time / Input clock freq).
	 */
	unsigned char cs_hold;
	/*
	 * Chip select delay time (CS minimum inactive time).
	 * Minimum time after chip select goes inactive before chip select
	 * can go active again, calculated by dividing the desired delay time
	 * in ns by the Input clock period (delay time / Input clock freq).
	 */
	unsigned char cs_delay;
	/*
	 * Byte delay select:
	 * Selects whether or not a delay is inserted between bytes.
	 * 0 - Minimum inter-byte delay
	 * 1 - Inter-byte delay of
	 * (cs_hold/master_clk half period) * master_clk
	 */
	int inter_byte_delay;
	/* SPI Mode. */
	enum spim_mode spi_mode;
	/* Chip select idle level (0=low, 1=high, Others=invalid). */
	unsigned int cs_idle_level;
	/* Data idle level (0=low, 1=high, Others=invalid). */
	unsigned int data_idle_level;

};

struct imgtec_spi_slave {
	/* Generic ineterface slave structure */
	struct spi_slave slave;
	/* SPIM instance device parameters */
	struct spim_device_parameters device_parameters;
	/* SPIM instance base address */
	u32 base;
	/* Boolean property that is TRUE if API has been initialised */
	int initialised;
	/* Value that is set if the transaction should be finalized */
	int complete;
	/* Flags passed with transfer info */
	u32 flags;
	/* GPIO correspondence for CS line */
	unsigned int gpio;
};

#endif /* __DRIVERS_SPI_IMGTEC_SPFI_H__ */
