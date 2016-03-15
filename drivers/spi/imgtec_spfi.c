/*
 * Imgtec SPI driver
 *
 * Copyright (C) 2015 Imagination Technologies
 *
 * TODO:
 *      (1)Use pin control driver functionality when available and
 *      remove existing temporary GPIO helper functions
 *
 *      (2)Move GPIO values in board specific file
 *
 *	(3)Define valid CS lines in board specific file
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <common.h>
#include <malloc.h>
#include <spi.h>

#include "imgtec_spfi.h"

/*
 * TODO1: Use pin control driver functionality when available and
 *       remove existing temporary GPIO helper functions
 */
#define GPIO_BASE			0x18101C00
#define GPIO_CONTROL(bank)		((GPIO_BASE) + 0x200 + ((bank) * 0x24))
#define GPIO_CTRL_BIT_EN(pin)		(GPIO_CONTROL((pin) / 16) + 0x00)
#define GPIO_CTRL_OUTPUT_EN(pin)	(GPIO_CONTROL((pin) / 16) + 0x04)
#define GPIO_CTRL_OUTPUT(pin)		(GPIO_CONTROL((pin) / 16) + 0x08)
#define GPIO_CTRL_INPUT(pin)		(GPIO_CONTROL((pin) / 16) + 0x0C)

/* TODO2: Move GPIO values in board specific file */
#define SPI1_CS0_GPIO			CONFIG_SYS_SPI1_CS0_GPIO
#define SPI1_CS1_GPIO			CONFIG_SYS_SPI1_CS1_GPIO

/* Functions for manipulating GPIO regs. */
static inline void gpio_write(uint32_t addr, unsigned int pin, uint32_t value)
{
	writel((0x10000 | !!(value)) << (pin % 16), addr);
}

static inline void gpio_set(unsigned int pin, unsigned int value)
{
	gpio_write(GPIO_CTRL_OUTPUT(pin), pin, value);
}

static inline void gpio_direction_output(unsigned int pin)
{
	gpio_write(GPIO_CTRL_OUTPUT_EN(pin), pin, 1);
	gpio_write(GPIO_CTRL_BIT_EN(pin), pin, 1);
}

#define spi_read_reg_field(regval, field)		\
(							\
	((field##_MASK) == 0xFFFFFFFF) ?		\
	(regval) :					\
	(((regval) & (field##_MASK)) >> (field##_SHIFT))\
)

#define spi_write_reg_field(regval, field, val)		\
(							\
	((field##_MASK) == 0xFFFFFFFF) ?		\
	(val) :						\
	(((regval) & ~(field##_MASK)) |			\
	(((val) << (field##_SHIFT)) & (field##_MASK)))	\
)

#define IMG_SPIM_BASE_ADDRESS(i)	(0x18100F00 + (0x100 * (i)))
#define SPI0_BUS			0
#define SPI1_BUS			1

/*
 * Parameter register
 * Each of these corresponds to a single port (ie CS line) in the interface
 * Fields	Name		Description
 * ======	====		===========
 * b31:24	CLK_RATE	Bit Clock rate = (24.576 * value / 512) MHz
 * b23:16	CS_SETUP	Chip Select setup = (40 * value) ns
 * b15:8	CS_HOLD		Chip Select hold = (40 * value) ns
 * b7:0		CS_DELAY	Chip Select delay = (40 * value) ns
 */

#define SPIM_CLK_DIVIDE_MASK	0xFF000000
#define SPIM_CS_SETUP_MASK	0x00FF0000
#define SPIM_CS_HOLD_MASK	0x0000FF00
#define SPIM_CS_DELAY_MASK	0x000000FF
#define SPIM_CS_PARAM_MASK	(SPIM_CS_SETUP_MASK \
				| SPIM_CS_HOLD_MASK \
				| SPIM_CS_DELAY_MASK)

#define SPIM_CLK_DIVIDE_SHIFT		24
#define SPIM_CS_SETUP_SHIFT		16
#define SPIM_CS_HOLD_SHIFT		8
#define SPIM_CS_DELAY_SHIFT		0
#define SPIM_CS_PARAM_SHIFT		0

/* Control register */

#define SPFI_DRIBBLE_COUNT_MASK	0x000e0000
#define SPFI_MEMORY_IF_MASK	0x00008000
#define SPIM_BYTE_DELAY_MASK	0x00004000
#define SPIM_CS_DEASSERT_MASK	0x00002000
#define SPIM_CONTINUE_MASK	0x00001000
#define SPIM_SOFT_RESET_MASK	0x00000800
#define SPIM_SEND_DMA_MASK	0x00000400
#define SPIM_GET_DMA_MASK	0x00000200
#define SPIM_EDGE_TX_RX_MASK	0x00000100
#define SPFI_TMODE_MASK		0x000000e0
#define SPFI_TMODE_DQ_MASK	0x0000001c
#define SPFI_TX_RX_MASK		0x00000002
#define SPFI_EN_MASK		0x00000001

#define SPFI_DRIBBLE_COUNT_SHIFT	17
#define SPFI_MEMORY_IF_SHIFT		15
#define SPIM_BYTE_DELAY_SHIFT		14
#define SPIM_CS_DEASSERT_SHIFT		13
#define SPIM_CONTINUE_SHIFT		12
#define SPIM_SOFT_RESET_SHIFT		11
#define SPIM_SEND_DMA_SHIFT		10
#define SPIM_GET_DMA_SHIFT		9
#define SPIM_EDGE_TX_RX_SHIFT		8
#define SPFI_TMODE_SHIFT		5
#define SPFI_TMODE_DQ_SHIFT		2
#define SPFI_TX_RX_SHIFT		1
#define SPFI_EN_SHIFT			0

/* Transaction register*/

#define SPFI_TSIZE_MASK			0xffff0000
#define SPFI_CMD_LENGTH_MASK		0x0000e000
#define SPFI_ADDR_LENGTH_MASK		0x00001c00
#define SPFI_DUMMY_LENGTH_MASK		0x000003e0
#define SPFI_PI_LENGTH_MASK		0x0000001c

#define SPFI_TSIZE_SHIFT		16
#define SPFI_CMD_LENGTH_SHIFT		13
#define SPFI_ADDR_LENGTH_SHIFT		10
#define SPFI_DUMMY_LENGTH_SHIFT		5
#define SPFI_PI_LENGTH_SHIFT		2

/* Port state register */

#define SPFI_PORT_SELECT_MASK		0x00700000
/* WARNING the following bits are reversed */
#define SPFI_CLOCK0_IDLE_MASK		0x000f8000
#define SPFI_CLOCK0_PHASE_MASK		0x00007c00
#define SPFI_CS0_IDLE_MASK		0x000003e0
#define SPFI_DATA0_IDLE_MASK		0x0000001f

#define SPIM_CLOCK0_IDLE_MASK		0x000f8000
#define SPIM_CLOCK0_PHASE_MASK		0x00007c00
#define SPIM_CS0_IDLE_MASK		0x000003e0
#define SPIM_DATA0_IDLE_MASK		0x0000001f

#define SPIM_PORT0_MASK			0x00084210

#define SPFI_PORT_SELECT_SHIFT		20
/* WARNING the following bits are reversed, bit 0 is highest */
#define SPFI_CLOCK0_IDLE_SHIFT		19
#define SPFI_CLOCK0_PHASE_SHIFT		14
#define SPFI_CS0_IDLE_SHIFT		9
#define SPFI_DATA0_IDLE_SHIFT		4

#define SPIM_CLOCK0_IDLE_SHIFT		19
#define SPIM_CLOCK0_PHASE_SHIFT		14
#define SPIM_CS0_IDLE_SHIFT		9
#define SPIM_DATA0_IDLE_SHIFT		4

/*
 * Interrupt registers
 * SPFI_GDOF_MASK means Rx buffer full, not an overflow, because clock stalls
 * SPFI_SDUF_MASK means Tx buffer empty, not an underflow, because clock stalls
 */
#define SPFI_IACCESS_MASK	0x00001000
#define SPFI_GDEX8BIT_MASK	0x00000800
#define SPFI_ALLDONE_MASK	0x00000200
#define SPFI_GDFUL_MASK		0x00000100
#define SPFI_GDHF_MASK		0x00000080
#define SPFI_GDEX32BIT_MASK	0x00000040
#define SPFI_GDTRIG_MASK	0x00000020
#define SPFI_SDFUL_MASK		0x00000008
#define SPFI_SDHF_MASK		0x00000004
#define SPFI_SDE_MASK		0x00000002
#define SPFI_SDTRIG_MASK	0x00000001

#define SPFI_IACCESS_SHIFT		12
#define SPFI_GDEX8BIT_SHIFT		11
#define SPFI_ALLDONE_SHIFT		9
#define SPFI_GDFUL_SHIFT		8
#define SPFI_GDHF_SHIFT			7
#define SPFI_GDEX32BIT_SHIFT		6
#define SPFI_GDTRIG_SHIFT		5
#define SPFI_SDFUL_SHIFT		3
#define SPFI_SDHF_SHIFT			2
#define SPFI_SDE_SHIFT			1
#define SPFI_SDTRIG_SHIFT		0

/* SPFI register block */
#define SPFI_PORT_0_PARAM_REG_OFFSET		0x00
#define SPFI_PORT_1_PARAM_REG_OFFSET		0x04
#define SPFI_PORT_2_PARAM_REG_OFFSET		0x08
#define SPFI_PORT_3_PARAM_REG_OFFSET		0x0C
#define SPFI_PORT_4_PARAM_REG_OFFSET		0x10
#define SPFI_CONTROL_REG_OFFSET			0x14
#define SPFI_TRANSACTION_REG_OFFSET		0x18
#define SPFI_PORT_STATE_REG_OFFSET		0x1C

#define SPFI_SEND_LONG_REG_OFFSET		0x20
#define SPFI_SEND_BYTE_REG_OFFSET		0x24
#define SPFI_GET_LONG_REG_OFFSET		0x28
#define SPFI_GET_BYTE_REG_OFFSET		0x2C

#define SPFI_INT_STATUS_REG_OFFSET		0x30
#define SPFI_INT_ENABLE_REG_OFFSET		0x34
#define SPFI_INT_CLEAR_REG_OFFSET		0x38

#define SPFI_IMMEDIATE_STATUS_REG_OFFSET	0x3c

#define SPFI_FLASH_BASE_ADDRESS_REG_OFFSET	0x48
#define SPFI_FLASH_STATUS_REG_OFFSET		0x4C

#define IMG_FALSE				0
#define IMG_TRUE				1

/* Number of SPIM interfaces*/
#define SPIM_NUM_BLOCKS				2
/* Number of chip select lines supported by the SPI master port. */
#define SPIM_NUM_PORTS_PER_BLOCK		(SPIM_DUMMY_CS)
/* Maximum transfer size (in bytes) for the SPI master port. */
#define SPIM_MAX_TRANSFER_BYTES			0xFFFF
/* Maximum size of a flash command: command bytes+address_bytes. */
#define SPIM_MAX_FLASH_COMMAND_BYTES		0x8
/* Write operation to fifo done in blocks of 16 words (64 bytes) */
#define SPIM_MAX_BLOCK_BYTES			0x40
/* Microseconds until timeout is returned */
#define SPI_TIMEOUT_VALUE_MS			500
/* Maximum bytes pending */
#define SPFI_DATA_REQUEST_MAX_SIZE		8

/* This type defines the SPIM device numbers (chip select lines). */
enum spim_device {
	/* Device 0 (CS0). */
	SPIM_DEVICE0 = 0,
	/* Device 1 (CS1). */
	SPIM_DEVICE1,
	/* Device 2 (CS2). */
	SPIM_DEVICE2,
	/* Device 3 (CS3). */
	SPIM_DEVICE3,
	/* Device 4 (CS4). */
	SPIM_DEVICE4,
	/* Dummy chip select. */
	SPIM_DUMMY_CS

};

/* Command transfer mode */
enum command_mode {
	/* Command, address, dummy and PI cycles are transferred on sio0 */
	SPIM_CMD_MODE_0 = 0,
	/*
	 * Command and Address is transferred on sio0 port only but dummy
	 * cycles and PI is transferred on all the interface ports.
	 */
	SPIM_CMD_MODE_1,
	/*
	 * Command is transferred on sio0 port only but address, dummy
	 * and PI is transferred on all the interface portS
	 */
	SPIM_CMD_MODE_2,
	/*
	 * Command, address, dummy and PI bytes are transferred on all
	 * the interfaces
	 */
	SPIM_CMD_MODE_3
};

/* Data transfer mode */
enum transfer_mode {
	/* Transfer data in single mode */
	SPIM_DMODE_SINGLE = 0,
	/* Transfer data in dual mode */
	SPIM_DMODE_DUAL,
	/* Transfer data in quad mode */
	SPIM_DMODE_QUAD
};

/* SPIM initialisation function return value.*/
enum spim_return {
	/* Initialisation parameters are valid. */
	SPIM_OK = 0,
	/* Mode parameter is invalid. */
	SPIM_INVALID_SPI_MODE,
	/* Chip select idle level is invalid. */
	SPIM_INVALID_CS_IDLE_LEVEL,
	/* Data idle level is invalid. */
	SPIM_INVALID_DATA_IDLE_LEVEL,
	/* Chip select line parameter is invalid. */
	SPIM_INVALID_CS_LINE,
	/* Transfer size parameter is invalid. */
	SPIM_INVALID_SIZE,
	/* Read/write parameter is invalid. */
	SPIM_INVALID_READ_WRITE,
	/* Continue parameter is invalid. */
	SPIM_INVALID_CONTINUE,
	/* Invalid block index */
	SPIM_INVALID_BLOCK_INDEX,
	/* Extended error values */
	/* Invalid bit rate */
	SPIM_INVALID_BIT_RATE,
	/* Invalid CS hold value */
	SPIM_INVALID_CS_HOLD_VALUE,
	/* API function called before API is initialised */
	SPIM_API_NOT_INITIALISED,
	/* SPI driver initialisation failed */
	SPIM_DRIVER_INIT_ERROR,
	/* Invalid transfer description */
	SPIM_INVALID_TRANSFER_DESC,
	/* Timeout indicating an error on the bus*/
	SPIM_TIMEOUT_ERROR,
	/* Non aligned error */
	SPIM_NON_BYTE_ALIGNED
};

static inline struct imgtec_spi_slave *to_imgtec_spi_slave(
			struct spi_slave *slave)
{
	return container_of(slave, struct imgtec_spi_slave, slave);
}

/* Sets port parameters in port state register. */
static void setparams(struct spi_slave *slave, uint32_t port,
		      struct spim_device_parameters *params)
{
	uint32_t spim_parameters = 0, port_state;
	uint32_t base = to_imgtec_spi_slave(slave)->base;

	port_state = readl(base + SPFI_PORT_STATE_REG_OFFSET);
	port_state &= ~((SPIM_PORT0_MASK >> port) | SPFI_PORT_SELECT_MASK);
	port_state |= params->cs_idle_level << (SPIM_CS0_IDLE_SHIFT - port);
	port_state |=
		params->data_idle_level << (SPIM_DATA0_IDLE_SHIFT - port);

	/* Clock idle level and phase */
	switch (params->spi_mode) {
	case SPIM_MODE_0:
		break;
	case SPIM_MODE_1:
		port_state |= (1 << (SPIM_CLOCK0_PHASE_SHIFT - port));
		break;
	case SPIM_MODE_2:
		port_state |= (1 << (SPIM_CLOCK0_IDLE_SHIFT - port));
		break;
	case SPIM_MODE_3:
		port_state |= (1 << (SPIM_CLOCK0_IDLE_SHIFT - port)) |
				 (1 << (SPIM_CLOCK0_PHASE_SHIFT - port));
		break;
	}
	/* Set port state register */
	writel(port_state, base + SPFI_PORT_STATE_REG_OFFSET);

	/* Set up values to be written to device parameter register */
	spim_parameters |= params->bitrate << SPIM_CLK_DIVIDE_SHIFT;
	spim_parameters |= params->cs_setup << SPIM_CS_SETUP_SHIFT;
	spim_parameters |= params->cs_hold << SPIM_CS_HOLD_SHIFT;
	spim_parameters |= params->cs_delay << SPIM_CS_DELAY_SHIFT;

	writel(spim_parameters,
	       base + SPFI_PORT_0_PARAM_REG_OFFSET + 4 * port);
}

/* Checks the set bitrate */
static int check_bitrate(uint32_t rate)
{
	/* Bitrate must be 1, 2, 4, 8, 16, 32, 64, or 128 */
	switch (rate) {
	case 1:
	case 2:
	case 4:
	case 8:
	case 16:
	case 32:
	case 64:
	case 128:
		return SPIM_OK;
	default:
		return -SPIM_INVALID_BIT_RATE;
	}
	return -SPIM_INVALID_BIT_RATE;
}

/* Checks device parameters for errors */
static int check_device_params(struct spim_device_parameters *pdev_param)
{
	if (pdev_param->spi_mode > SPIM_MODE_3)
		return -SPIM_INVALID_SPI_MODE;
	if (check_bitrate(pdev_param->bitrate) != SPIM_OK)
		return -SPIM_INVALID_BIT_RATE;
	if (pdev_param->cs_idle_level > 1)
		return -SPIM_INVALID_CS_IDLE_LEVEL;
	if (pdev_param->data_idle_level > 1)
		return -SPIM_INVALID_DATA_IDLE_LEVEL;
	return SPIM_OK;
}

static void spfi_switch(struct spi_slave *slave, int on)
{
	struct imgtec_spi_slave *bus = to_imgtec_spi_slave(slave);
	uint32_t reg;

	if (on && !(bus->complete))
		return;
	reg = readl(bus->base + SPFI_CONTROL_REG_OFFSET);
	reg = spi_write_reg_field(reg, SPFI_EN, !!on);
	writel(reg, bus->base + SPFI_CONTROL_REG_OFFSET);
}

static int spfi_wait_all_done(struct spi_slave *slave)
{
	struct imgtec_spi_slave *bus = to_imgtec_spi_slave(slave);
	unsigned long deadline = get_timer(0) + SPI_TIMEOUT_VALUE_MS;

	do {
		if (get_timer(0) > deadline)
			return -SPIM_TIMEOUT_ERROR;
	} while (!(readl(bus->base + SPFI_INT_STATUS_REG_OFFSET) &
			SPFI_ALLDONE_MASK));
	writel(SPFI_ALLDONE_MASK, bus->base + SPFI_INT_CLEAR_REG_OFFSET);

	return SPIM_OK;
}

/* tx/rx will be 0 if "bytes" bytes must be transmitted/received */
static int spim_config(struct spi_slave *slave, unsigned int tx,
			unsigned int rx, unsigned int bytes)
{
	struct imgtec_spi_slave *bus = to_imgtec_spi_slave(slave);
	uint32_t base, reg;
	int is_pending, ret;

	base = bus->base;

	/*
	 * For read or write transfers of less than 8 bytes (cmd = 1 byte,
	 * addr up to 7 bytes), SPFI will be configured, but not enabled
	 * (unless it is the last transfer in the queue).The transfer will
	 * be enabled by the subsequent transfer.
	 * A pending transfer is determined by the content of the
	 * transaction register: if command part is set and tsize
	 * is not.
	 */
	reg = readl(base + SPFI_TRANSACTION_REG_OFFSET);
	is_pending = (reg & SPFI_CMD_LENGTH_MASK) && (!(reg & SPFI_TSIZE_MASK));

	if (!(bus->flags & SPI_XFER_END) && !tx &&
		(bytes <= SPFI_DATA_REQUEST_MAX_SIZE) && !is_pending) {
		reg = spi_write_reg_field(0, SPFI_CMD_LENGTH, 1);
		reg = spi_write_reg_field(reg, SPFI_ADDR_LENGTH, bytes - 1);
		bus->complete = 0;
	} else {
		bus->complete = 1;
		if (is_pending) {
			if (!tx) {
				/* Finish pending transfer first for transmit */
				spfi_switch(slave, 1);
				ret = spfi_wait_all_done(slave);
				if (ret < 0)
					return ret;
				spfi_switch(slave, 0);
				reg = 0;
			}
			/* Keep setup from peding transfer */
			reg = spi_write_reg_field(reg, SPFI_TSIZE, bytes);
		} else {
			reg = spi_write_reg_field(0, SPFI_TSIZE, bytes);
		}
	}
	/* Set transaction register */
	writel(reg, base + SPFI_TRANSACTION_REG_OFFSET);

	/* Clear status */
	writel(0xffffffff, base + SPFI_INT_CLEAR_REG_OFFSET);

	/* Set control register */
	reg = readl(base + SPFI_CONTROL_REG_OFFSET);
	/* Set send DMA if write transaction exists */
	reg = spi_write_reg_field(reg, SPIM_SEND_DMA, (!tx || is_pending) ?
							1 : 0);
	/* Set get DMA if read transaction exists */
	reg = spi_write_reg_field(reg, SPIM_GET_DMA, (!rx) ? 1 : 0);
	/* Same edge used for higher operational frequency */
	reg = spi_write_reg_field(reg, SPIM_EDGE_TX_RX, 1);

	/* Set transfer mode: single, dual or quad */
	if ((slave->op_mode_rx & SPI_OPM_RX_QOF) ||
			(slave->op_mode_tx & SPI_OPM_TX_QPP))
		reg = spi_write_reg_field(reg, SPFI_TMODE, SPIM_DMODE_QUAD);
	else if (slave->op_mode_rx & SPI_OPM_RX_DOUT)
		reg = spi_write_reg_field(reg, SPFI_TMODE, SPIM_DMODE_DUAL);
	else
		reg = spi_write_reg_field(reg, SPFI_TMODE, SPIM_DMODE_SINGLE);

	writel(reg, base + SPFI_CONTROL_REG_OFFSET);

	return SPIM_OK;
}

/* Function that carries out read/write operations */
static int spim_io(struct spi_slave *slave, void *din,
		   const void *dout, unsigned int bytes)
{
	struct imgtec_spi_slave *bus;
	unsigned int tx, rx;
	int both_hf, either_hf, incoming, transferred = 0;
	uint32_t status = 0, data, base;
	unsigned long deadline = get_timer(0) + SPI_TIMEOUT_VALUE_MS;
	int ret = SPIM_OK;

	if (!slave) {
		printf("%s: Error: slave not initialized.\n", __func__);
		return -SPIM_API_NOT_INITIALISED;
	}
	bus = to_imgtec_spi_slave(slave);
	base = bus->base;

	tx = (dout) ? 0 : bytes;
	rx = (din) ? 0 : bytes;
	ret = spim_config(slave, tx, rx, bytes);
	if (ret < 0) {
		printf("%s: SPIM config failed.\n", __func__);
		return ret;
	}
	spfi_switch(slave, 1);

	/* We send/receive as long as we still have data available */
	while ((tx < bytes) || (rx < bytes)) {
		/*
		 * If we exceed the allocated time we return with timeout
		 * The get_timer function is time consuming so call it only if
		 * no bytes were transferred in the previous iteration
		 */
		if (!transferred && (get_timer(0) > deadline))
			return -SPIM_TIMEOUT_ERROR;

		transferred = 0;
		/* Store the current state of the INT status register */
		status = readl(base + SPFI_INT_STATUS_REG_OFFSET);

		/*
		 * Clear all statuses indicating the state of the FIFOs
		 * The stored status will be used in this iteration
		 */
		writel(SPFI_SDFUL_MASK | SPFI_SDHF_MASK | SPFI_GDHF_MASK,
		       base + SPFI_INT_CLEAR_REG_OFFSET);

		/*
		 * Top up TX unless both RX and TX are half full,
		 * in which case RX needs draining more urgently
		 */
		both_hf = ((status & (SPFI_GDHF_MASK | SPFI_SDHF_MASK))
			== (SPFI_GDHF_MASK | SPFI_SDHF_MASK));
		if ((tx < bytes) && !(status & SPFI_SDFUL_MASK) &&
		    (!(rx < bytes) || !both_hf)) {
			if ((bytes - tx) >= sizeof(uint32_t)) {
				memcpy(&data, (uint8_t *)dout + tx,
				       sizeof(uint32_t));
				writel(data, base + SPFI_SEND_LONG_REG_OFFSET);
				tx += sizeof(uint32_t);
			} else {
				data = *((uint8_t *)dout + tx);
				writel(data, base + SPFI_SEND_BYTE_REG_OFFSET);
				tx++;
			}
			transferred = 1;
		}
		/*
		 * Drain RX unless neither RX or TX are half full,
		 * in which case TX needs filling more urgently.
		 */
		either_hf = (status & (SPFI_SDHF_MASK |
						SPFI_GDHF_MASK));
		incoming = (status & (SPFI_GDEX8BIT_MASK |
					SPFI_GDEX32BIT_MASK));
		if ((rx < bytes) && incoming && (!(tx < bytes) || either_hf)) {
			if (((bytes - rx) >= sizeof(uint32_t)) &&
			    (status & SPFI_GDEX32BIT_MASK)) {
				data = readl(base +
						SPFI_GET_LONG_REG_OFFSET);
				memcpy((uint8_t *)din + rx, &data,
				       sizeof(uint32_t));
				rx += sizeof(uint32_t);
				writel(SPFI_GDEX32BIT_MASK,
				       base + SPFI_INT_CLEAR_REG_OFFSET);
			} else if (status & SPFI_GDEX8BIT_MASK) {
				data  = (uint8_t)readl(base +
					SPFI_GET_BYTE_REG_OFFSET);
				*((uint8_t *)din + rx) = data;
				rx++;
				writel(SPFI_GDEX8BIT_MASK, base +
					SPFI_INT_CLEAR_REG_OFFSET);
			}
			transferred = 1;
		}
	}
	/* If we're not supposed to finalize the transaction, just return */
	if (!(bus->complete))
		return SPIM_OK;
	/*
	 * We wait for ALLDONE trigger to be set, signaling that it's ready
	 * to accept another transaction
	 */
	ret = spfi_wait_all_done(slave);
	/* Disable SPFI for it to not interfere with pending transactions */
	spfi_switch(slave, 0);

	return ret;
}

void spi_cs_activate(struct spi_slave *slave)
{
	if (!slave) {
		printf("%s: Error: slave not initialized.\n", __func__);
		return;
	}
	/* Pull the line down */
	gpio_set(to_imgtec_spi_slave(slave)->gpio, 0);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	if (!slave) {
		printf("%s: Error: slave not initialized.\n", __func__);
		return;
	}
	/* Pull the line up */
	gpio_set(to_imgtec_spi_slave(slave)->gpio, 1);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	switch (bus) {
	case SPI1_BUS:
		/*
		 * TODO3: We're only allowing access to CS 0 and 1 for
		 * the moment; later this will be reglemented someplace else
		 */
		if (cs <= SPIM_DEVICE1)
			return 1;
#ifdef CONFIG_SYS_SPI0
	/*
	 * Only access to interface 1 is allowed by default
	 * Access to interface 0 must be enabled through configuration
	 */
	case SPI0_BUS:
		if (cs <= SPIM_DEVICE4)
			return 1;
#endif
	default:
		/* Invalid bus number. Do nothing */
		return 0;
	}
}

/* Claim the bus and prepare it for communication */
int spi_claim_bus(struct spi_slave *slave)
{
	int ret;
	struct imgtec_spi_slave *bus;
	uint32_t reg;

	if (!slave) {
		printf("%s: Error: slave not initialized.\n", __func__);
		return -SPIM_API_NOT_INITIALISED;
	}
	bus = to_imgtec_spi_slave(slave);
	if (bus->initialised)
		return SPIM_OK;

	/* Soft reset peripheral internals */
	writel(SPIM_SOFT_RESET_MASK, bus->base + SPFI_CONTROL_REG_OFFSET);
	/* Clear control register */
	writel(0, bus->base + SPFI_CONTROL_REG_OFFSET);

	/* Check device parameters */
	ret = check_device_params(&bus->device_parameters);
	if (ret) {
		printf("%s: Error: incorrect device parameters.\n", __func__);
		return ret;
	}
	/* Set device parameters */
	setparams(slave, slave->cs, &bus->device_parameters);

	/* Port state register setup */
	reg = readl(bus->base +  SPFI_PORT_STATE_REG_OFFSET);
	reg = spi_write_reg_field(reg, SPFI_PORT_SELECT, slave->cs);
	writel(reg, bus->base + SPFI_PORT_STATE_REG_OFFSET);

	/* Set the control register */
	reg = (bus->device_parameters).inter_byte_delay ?
			SPIM_BYTE_DELAY_MASK : 0;
	/*
	 * Set up the command/addr/dummy mode to be 0
	 * The upper layers have no knowledge of this mode but they
	 * will always use X2/X4 operations for dual and quad which are
	 * consistent with this command mode: cmd/addr/dummy transferred
	 * on one line, data transferred on all lines
	 */
	reg = spi_write_reg_field(reg, SPFI_TMODE_DQ, SPIM_CMD_MODE_0);
	writel(reg, bus->base + SPFI_CONTROL_REG_OFFSET);
	bus->initialised = IMG_TRUE;

	/* Assert CS */
	spi_cs_activate(slave);

	return SPIM_OK;
}

/* Release the SPI bus */
void spi_release_bus(struct spi_slave *slave)
{
	struct imgtec_spi_slave *bus;

	if (!slave) {
		printf("%s: Error: slave not initialized.\n", __func__);
		return;
	}
	bus = to_imgtec_spi_slave(slave);
	bus->initialised = IMG_FALSE;

	/* De-assert CS */
	spi_cs_deactivate(slave);
	/* Soft reset peripheral internals */
	writel(SPIM_SOFT_RESET_MASK, bus->base +
		SPFI_CONTROL_REG_OFFSET);
	writel(0, bus->base + SPFI_CONTROL_REG_OFFSET);
}

/* SPI transfer */
int  spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
	      void *din, unsigned long flags)
{
	unsigned int to_transfer, bytes_trans = 0;
	int bytes, ret = 0;
	struct imgtec_spi_slave *bus;

	if (!slave) {
		printf("%s: Error: slave not initialized.\n", __func__);
		return -SPIM_API_NOT_INITIALISED;
	}
	bus = to_imgtec_spi_slave(slave);
	/* Set flags for current transaction */
	bus->flags = flags;

	/* SPI core configured to do 8 bit transfers */
	if (bitlen % 8) {
		printf("%s: Non byte aligned SPI transfer.\n", __func__);
		return -SPIM_NON_BYTE_ALIGNED;
	} else {
		bytes = bitlen / 8;
	}

	if (!dout && !din && (bitlen != 0)) {
		printf("%s: Error: both buffers are NULL.\n", __func__);
		return -SPIM_INVALID_TRANSFER_DESC;
	}

	/* Start the transaction, if necessary. */
	if ((flags & SPI_XFER_BEGIN))
		spi_cs_activate(slave);

	/*
	 * Transfers with size bigger than
	 * SPIM_MAX_TANSFER_BYTES = 64KB - 1 (0xFFFF) are divided.
	 */
	while (bytes - bytes_trans) {
		if ((bytes - bytes_trans) > SPIM_MAX_TRANSFER_BYTES)
			to_transfer = SPIM_MAX_TRANSFER_BYTES;
		else
			to_transfer = bytes - bytes_trans;
		ret = spim_io(slave, (din) ? (din + bytes_trans) : NULL,
			      (dout) ? (dout + bytes_trans) : NULL,
					to_transfer);
		if (ret) {
			printf("%s: Error: Transfer failed with error %d!\n",
				__func__, ret);
			return ret;
		}
		bytes_trans += to_transfer;
	}

	/* Stop the transaction, if requested. */
	if ((flags & SPI_XFER_END))
		spi_cs_deactivate(slave);

	return SPIM_OK;
}

/* Initialize SPI slave */
void spi_init(void)
{
	/* Nothing to do */
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct imgtec_spi_slave *img_slave = NULL;
	struct spim_device_parameters *device_parameters;

	if (!spi_cs_is_valid(bus, cs)) {
		printf("%s: Error: Invalid chip select!\n", __func__);
		return NULL;
	}
	img_slave = spi_alloc_slave(struct imgtec_spi_slave, bus, cs);
	if (!img_slave) {
		printf("%s: Error: Unable to allocate slave!\n", __func__);
		return NULL;
	}
	img_slave->base = IMG_SPIM_BASE_ADDRESS(bus);

	/* TODO1: TODO3: this wil be done properly, later on, through DT */
	switch (bus) {
	case SPI1_BUS:
		if (cs == SPIM_DEVICE0)
			img_slave->gpio = SPI1_CS0_GPIO;
		if (cs == SPIM_DEVICE1)
			img_slave->gpio = SPI1_CS1_GPIO;
		break;
#ifdef CONFIG_SYS_SPI0
	case SPI0_BUS:
		printf("%s: Error: No GPIOs configured for this interface!\n",
		       __func__);
		return NULL;
#endif
	default:
		printf("%s: Error: Invalid interface!\n", __func__);
		return NULL;
	}
	gpio_direction_output(img_slave->gpio);

	device_parameters = &img_slave->device_parameters;
	device_parameters->bitrate = 64;
	device_parameters->cs_setup = 0;
	device_parameters->cs_hold = 0;
	device_parameters->cs_delay = 0;
	device_parameters->spi_mode = SPIM_MODE_0;
	device_parameters->cs_idle_level = 1;
	device_parameters->data_idle_level = 0;
	img_slave->initialised = IMG_FALSE;

	return &img_slave->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct imgtec_spi_slave *bus = to_imgtec_spi_slave(slave);

	free(bus);
}
