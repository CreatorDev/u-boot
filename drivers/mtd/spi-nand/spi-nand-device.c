/*
 * Support for SPI NAND devices
 *
 * Copyright (C) 2015 Imagination Technologies
 *
 * TODO:
 * 1. Make the corresponding changes when DT is available
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/compat.h>
#include <linux/err.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/spi-nand.h>
#include <linux/sizes.h>
#include <nand.h>
#include <spi.h>

#ifdef CONFIG_CMD_NAND
/* Necessary for NAND command */
extern nand_info_t nand_info[];
#endif

DECLARE_GLOBAL_DATA_PTR;

/* SPI NAND commands */
#define	SPI_NAND_WRITE_ENABLE		0x06
#define	SPI_NAND_WRITE_DISABLE		0x04
#define	SPI_NAND_GET_FEATURE		0x0f
#define	SPI_NAND_SET_FEATURE		0x1f
#define	SPI_NAND_PAGE_READ		0x13
#define	SPI_NAND_READ_CACHE		0x03
#define	SPI_NAND_FAST_READ_CACHE	0x0b
#define	SPI_NAND_READ_CACHE_X2		0x3b
#define	SPI_NAND_READ_CACHE_X4		0x6b
#define	SPI_NAND_READ_CACHE_DUAL_IO	0xbb
#define	SPI_NAND_READ_CACHE_QUAD_IO	0xeb
#define	SPI_NAND_READ_ID		0x9f
#define	SPI_NAND_PROGRAM_LOAD		0x02
#define	SPI_NAND_PROGRAM_LOAD4		0x32
#define	SPI_NAND_PROGRAM_EXEC		0x10
#define	SPI_NAND_PROGRAM_LOAD_RANDOM	0x84
#define	SPI_NAND_PROGRAM_LOAD_RANDOM4	0xc4
#define	SPI_NAND_BLOCK_ERASE		0xd8
#define	SPI_NAND_RESET			0xff

#define SPI_NAND_GD5F_READID_LEN	2
#define SPI_NAND_MT29F_READID_LEN	2

#define SPI_NAND_GD5F_ECC_MASK		(BIT(0) | BIT(1) | BIT(2))
#define SPI_NAND_GD5F_ECC_UNCORR	(BIT(0) | BIT(1) | BIT(2))
#define SPI_NAND_GD5F_ECC_SHIFT		4

#define SPI_NAND_MT29F_ECC_MASK		(BIT(0) | BIT(1))
#define SPI_NAND_MT29F_ECC_UNCORR	(BIT(1))
#define SPI_NAND_MT29F_ECC_SHIFT		4

static struct nand_ecclayout ecc_layout_gd5f = {
	.eccbytes = 128,
	.eccpos = {
		128, 129, 130, 131, 132, 133, 134, 135,
		136, 137, 138, 139, 140, 141, 142, 143,
		144, 145, 146, 147, 148, 149, 150, 151,
		152, 153, 154, 155, 156, 157, 158, 159,
		160, 161, 162, 163, 164, 165, 166, 167,
		168, 169, 170, 171, 172, 173, 174, 175,
		176, 177, 178, 179, 180, 181, 182, 183,
		184, 185, 186, 187, 188, 189, 190, 191,
		192, 193, 194, 195, 196, 197, 198, 199,
		200, 201, 202, 203, 204, 205, 206, 207,
		208, 209, 210, 211, 212, 213, 214, 215,
		216, 217, 218, 219, 220, 221, 222, 223,
		224, 225, 226, 227, 228, 229, 230, 231,
		232, 233, 234, 235, 236, 237, 238, 239,
		240, 241, 242, 243, 244, 245, 246, 247,
		248, 249, 250, 251, 252, 253, 254, 255
	},
	.oobfree = { {1, 127} }
};

static struct nand_ecclayout ecc_layout_mt29f = {
	.eccbytes = 32,
	.eccpos = {
		8, 9, 10, 11, 12, 13, 14, 15,
		24, 25, 26, 27, 28, 29, 30, 31,
		40, 41, 42, 43, 44, 45, 46, 47,
		56, 57, 58, 59, 60, 61, 62, 63,
	 },
};

static struct nand_flash_dev spi_nand_flash_ids[] = {
	{
		.name = "SPI NAND 512MiB 3,3V",
		.id = { NAND_MFR_GIGADEVICE, 0xb4 },
		.chipsize = 512,
		.pagesize = SZ_4K,
		.erasesize = SZ_256K,
		.id_len = 2,
		.oobsize = 256,
		.ecc.strength_ds = 8,
		.ecc.step_ds = 512,
		.ecc.layout = &ecc_layout_gd5f,
	},
	{
		.name = "SPI NAND 512MiB 1,8V",
		.id = { NAND_MFR_GIGADEVICE, 0xa4 },
		.chipsize = 512,
		.pagesize = SZ_4K,
		.erasesize = SZ_256K,
		.id_len = 2,
		.oobsize = 256,
		.ecc.strength_ds = 8,
		.ecc.step_ds = 512,
		.ecc.layout = &ecc_layout_gd5f,
	},
	{
		.name = "SPI NAND 512MiB 3,3V",
		.id = { NAND_MFR_MICRON, 0x32 },
		.chipsize = 512,
		.pagesize = SZ_2K,
		.erasesize = SZ_128K,
		.id_len = 2,
		.oobsize = 64,
		.ecc.strength_ds = 4,
		.ecc.step_ds = 512,
		.ecc.layout = &ecc_layout_mt29f,
	},
	{
		.name = "SPI NAND 256MiB 3,3V",
		.id = { NAND_MFR_MICRON, 0x22 },
		.chipsize = 256,
		.pagesize = SZ_2K,
		.erasesize = SZ_128K,
		.id_len = 2,
		.oobsize = 64,
		.ecc.strength_ds = 4,
		.ecc.step_ds = 512,
		.ecc.layout = &ecc_layout_mt29f,
	},
};

enum spi_nand_device_variant {
	SPI_NAND_GENERIC,
	SPI_NAND_MT29F,
	SPI_NAND_GD5F,
};

/* Information about an attached NAND chip */
struct fdt_nand {
	u8 op_mode_rx; /* RX bus width */
	u8 op_mode_tx; /* TX bus width */
};

struct spi_nand_device_cmd {
	/*
	 * Command and address. I/O errors have been observed if a
	 * separate spi_transfer is used for command and address,
	 * so keep them together.
	 */
	u32 n_cmd;
	u8 cmd[5];

	/* Tx data */
	u32 n_tx;
	u8 *tx_buf;

	/* Rx data */
	u32 n_rx;
	u8 *rx_buf;
};

struct spi_nand_device {
	struct spi_nand	spi_nand;
	struct spi_slave *spi;
	struct spi_nand_device_cmd cmd;
	struct fdt_nand config;
};

static int spi_nand_send_command(struct spi_slave *spi,
				 struct spi_nand_device_cmd *cmd)
{
	if (!cmd->n_cmd) {
		dev_err(&spi->dev, "cannot send an empty command\n");
		return -EINVAL;
	}

	if (cmd->n_tx && cmd->n_rx) {
		dev_err(&spi->dev, "cannot send and receive data at the same time\n");
		return -EINVAL;
	}

	spi_claim_bus(spi);

	/* Command and address */
	if (spi_xfer(spi, cmd->n_cmd * 8, cmd->cmd, NULL,
		(cmd->n_tx || cmd->n_rx) ? SPI_XFER_BEGIN :
		SPI_XFER_BEGIN | SPI_XFER_END)) {
		dev_err(&spi->dev, "Failed to send command.\n");
		return -1;
	}

	/* Data to be transmitted */
	if (cmd->n_tx) {
		if (spi_xfer(spi, cmd->n_tx * 8, cmd->tx_buf, NULL,
			     SPI_XFER_END)) {
			dev_err(&spi->dev, "Failed to send data.\n");
			return -1;
		}
	}

	/* Data to be received */
	if (cmd->n_rx) {
		if (spi_xfer(spi, cmd->n_rx * 8, NULL, cmd->rx_buf,
			     SPI_XFER_END)) {
			dev_err(&spi->dev, "Failed to receive data.\n");
			return -1;
		}
	}

	spi_release_bus(spi);

	return 0;
}

static int spi_nand_device_reset(struct spi_nand *snand)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	cmd->n_cmd = 1;
	cmd->cmd[0] = SPI_NAND_RESET;

	dev_dbg(snand->dev, "%s\n", __func__);

	return spi_nand_send_command(snand_dev->spi, cmd);
}

static int spi_nand_device_read_reg(struct spi_nand *snand, u8 opcode, u8 *buf)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	cmd->n_cmd = 2;
	cmd->cmd[0] = SPI_NAND_GET_FEATURE;
	cmd->cmd[1] = opcode;
	cmd->n_rx = 1;
	cmd->rx_buf = buf;

	dev_dbg(snand->dev, "%s: reg 0%x\n", __func__, opcode);

	return spi_nand_send_command(snand_dev->spi, cmd);
}

static int spi_nand_device_write_reg(struct spi_nand *snand, u8 opcode, u8 *buf)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	cmd->n_cmd = 2;
	cmd->cmd[0] = SPI_NAND_SET_FEATURE;
	cmd->cmd[1] = opcode;
	cmd->n_tx = 1;
	cmd->tx_buf = buf;

	dev_dbg(snand->dev, "%s: reg 0%x\n", __func__, opcode);

	return spi_nand_send_command(snand_dev->spi, cmd);
}

static int spi_nand_device_write_enable(struct spi_nand *snand)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	cmd->n_cmd = 1;
	cmd->cmd[0] = SPI_NAND_WRITE_ENABLE;

	dev_dbg(snand->dev, "%s\n", __func__);

	return spi_nand_send_command(snand_dev->spi, cmd);
}

static int spi_nand_device_write_disable(struct spi_nand *snand)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	cmd->n_cmd = 1;
	cmd->cmd[0] = SPI_NAND_WRITE_DISABLE;

	dev_dbg(snand->dev, "%s\n", __func__);

	return spi_nand_send_command(snand_dev->spi, cmd);
}

static int spi_nand_device_write_page(struct spi_nand *snand,
				      unsigned int page_addr)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	cmd->n_cmd = 4;
	cmd->cmd[0] = SPI_NAND_PROGRAM_EXEC;
	cmd->cmd[1] = (u8)((page_addr & 0xff0000) >> 16);
	cmd->cmd[2] = (u8)((page_addr & 0xff00) >> 8);
	cmd->cmd[3] = (u8)(page_addr & 0xff);

	dev_dbg(snand->dev, "%s: page 0x%x\n", __func__, page_addr);

	return spi_nand_send_command(snand_dev->spi, cmd);
}

static int spi_nand_device_store_cache(struct spi_nand *snand,
				       unsigned int page_offset, size_t length,
				       u8 *write_buf)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;
	struct fdt_nand *config = &snand_dev->config;
	int ret;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	cmd->n_cmd = 3;
	if (config->op_mode_tx & SPI_OPM_TX_QPP)
		cmd->cmd[0] = SPI_NAND_PROGRAM_LOAD4;
	else
		cmd->cmd[0] = SPI_NAND_PROGRAM_LOAD;
	cmd->cmd[1] = (u8)((page_offset & 0xff00) >> 8);
	cmd->cmd[2] = (u8)(page_offset & 0xff);
	cmd->n_tx = length;
	cmd->tx_buf = write_buf;

	dev_dbg(snand->dev, "%s: offset 0x%x\n", __func__, page_offset);

	snand_dev->spi->op_mode_tx = config->op_mode_tx;
	ret = spi_nand_send_command(snand_dev->spi, cmd);
	snand_dev->spi->op_mode_tx = 0;

	return ret;
}

static int spi_nand_device_load_page(struct spi_nand *snand,
				     unsigned int page_addr)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	cmd->n_cmd = 4;
	cmd->cmd[0] = SPI_NAND_PAGE_READ;
	cmd->cmd[1] = (u8)((page_addr & 0xff0000) >> 16);
	cmd->cmd[2] = (u8)((page_addr & 0xff00) >> 8);
	cmd->cmd[3] = (u8)(page_addr & 0xff);

	dev_dbg(snand->dev, "%s: page 0x%x\n", __func__, page_addr);

	return spi_nand_send_command(snand_dev->spi, cmd);
}

static int spi_nand_device_read_cache(struct spi_nand *snand,
				      unsigned int page_offset, size_t length,
				      u8 *read_buf)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;
	struct fdt_nand *config = &snand_dev->config;
	int ret;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	if ((config->op_mode_rx & SPI_OPM_RX_DOUT) ||
		(config->op_mode_rx & SPI_OPM_RX_QOF))
		cmd->n_cmd = 5;
	else
		cmd->n_cmd = 4;

	cmd->cmd[0] = (config->op_mode_rx & SPI_OPM_RX_QOF) ?
			SPI_NAND_READ_CACHE_X4 :
			((config->op_mode_rx & SPI_OPM_RX_DOUT) ?
			SPI_NAND_READ_CACHE_X2 :
			SPI_NAND_READ_CACHE);

	cmd->cmd[1] = 0; /* dummy byte */
	cmd->cmd[2] = (u8)((page_offset & 0xff00) >> 8);
	cmd->cmd[3] = (u8)(page_offset & 0xff);
	cmd->cmd[4] = 0; /* dummy byte */
	cmd->n_rx = length;
	cmd->rx_buf = read_buf;

	dev_dbg(snand->dev, "%s: offset 0x%x\n", __func__, page_offset);

	snand_dev->spi->op_mode_rx = config->op_mode_rx;
	ret = spi_nand_send_command(snand_dev->spi, cmd);
	snand_dev->spi->op_mode_rx = 0;

	return ret;
}

static int spi_nand_device_block_erase(struct spi_nand *snand,
				       unsigned int page_addr)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	cmd->n_cmd = 4;
	cmd->cmd[0] = SPI_NAND_BLOCK_ERASE;
	cmd->cmd[1] = (u8)((page_addr & 0xff0000) >> 16);
	cmd->cmd[2] = (u8)((page_addr & 0xff00) >> 8);
	cmd->cmd[3] = (u8)(page_addr & 0xff);

	dev_dbg(snand->dev, "%s: block 0x%x\n", __func__, page_addr);

	return spi_nand_send_command(snand_dev->spi, cmd);
}

#ifdef CONFIG_SPI_NAND_MT29F
static int spi_nand_mt29f_read_id(struct spi_nand *snand, u8 *buf)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	cmd->n_cmd = 1;
	cmd->cmd[0] = SPI_NAND_READ_ID;
	cmd->n_rx = SPI_NAND_MT29F_READID_LEN;
	cmd->rx_buf = buf;

	dev_dbg(snand->dev, "%s\n", __func__);

	return spi_nand_send_command(snand_dev->spi, cmd);
}

static void spi_nand_mt29f_ecc_status(unsigned int status,
				      unsigned int *corrected,
				      unsigned int *ecc_error)
{
	unsigned int ecc_status = (status >> SPI_NAND_MT29F_ECC_SHIFT) &
					     SPI_NAND_MT29F_ECC_MASK;

	*ecc_error = (ecc_status == SPI_NAND_MT29F_ECC_UNCORR) ? 1 : 0;
	if (*ecc_error == 0)
		*corrected = ecc_status;
}
#endif

#ifdef CONFIG_SPI_NAND_GD5F
static int spi_nand_gd5f_read_id(struct spi_nand *snand, u8 *buf)
{
	struct spi_nand_device *snand_dev = snand->priv;
	struct spi_nand_device_cmd *cmd = &snand_dev->cmd;
	int ret = 0;

	memset(cmd, 0, sizeof(struct spi_nand_device_cmd));
	cmd->n_cmd = 1;
	cmd->cmd[0] = SPI_NAND_READ_ID;
	cmd->n_rx = SPI_NAND_GD5F_READID_LEN;
	cmd->rx_buf = buf;

	ret = spi_nand_send_command(snand_dev->spi, cmd);
	dev_dbg(snand->dev, "%s: 0x%08x: errno: %d\n", __func__,
		*((unsigned int *)buf), ret);
	return ret;
}

static void spi_nand_gd5f_ecc_status(unsigned int status,
				     unsigned int *corrected,
				     unsigned int *ecc_error)
{
	unsigned int ecc_status = (status >> SPI_NAND_GD5F_ECC_SHIFT) &
					     SPI_NAND_GD5F_ECC_MASK;

	*ecc_error = (ecc_status == SPI_NAND_GD5F_ECC_UNCORR) ? 1 : 0;
	if (*ecc_error == 0)
		*corrected = (ecc_status > 1) ? (2 + ecc_status) : 0;
}
#endif

static int fdt_decode_nand(const void *blob, int node, struct fdt_nand *config)
{
	int ret;

	ret = fdtdec_get_int(gd->fdt_blob, node, "spi-rx-bus-width", 0);
	config->op_mode_rx = 0;
	if (ret == 4) /* Quad mode */
		config->op_mode_rx = SPI_OPM_RX_QOF;
	else if (ret == 2) /* Dual mode */
		config->op_mode_rx = SPI_OPM_RX_DOUT;

	ret = fdtdec_get_int(gd->fdt_blob, node, "spi-tx-bus-width", 0);
	config->op_mode_tx = 0;
	if (ret == 4) /* Quad mode */
		config->op_mode_tx = SPI_OPM_TX_QPP;
	return 0;
}

void board_nand_init(void)
{
	struct spi_nand_device *priv;
	struct spi_nand *snand;
	struct fdt_nand *config;
	int ret, node;
	char *name = "spi-nand";

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dev_err(snand->dev, "%s: Out of memory\n", __func__);
		return;
	}

	snand = &priv->spi_nand;
	config = &priv->config;

	snand->read_cache = spi_nand_device_read_cache;
	snand->load_page = spi_nand_device_load_page;
	snand->store_cache = spi_nand_device_store_cache;
	snand->write_page = spi_nand_device_write_page;
	snand->write_reg = spi_nand_device_write_reg;
	snand->read_reg = spi_nand_device_read_reg;
	snand->block_erase = spi_nand_device_block_erase;
	snand->reset = spi_nand_device_reset;
	snand->write_enable = spi_nand_device_write_enable;
	snand->write_disable = spi_nand_device_write_disable;
	/*
	 * TODO1: snand->dev only used for error print and actually
	 * not considerent in any way and therefore left for the
	 * moment as NULL. It will have a proper value when using DT.
	 */
	snand->dev = NULL;
	snand->priv = priv;

	if (CONFIG_SYS_MAX_NAND_DEVICE > 1)
		dev_err(snand->dev, "Only one device supported.\n");

#ifdef CONFIG_CMD_NAND
	snand->mtd = &nand_info[0];
#else
	snand->mtd = kzalloc(sizeof(*snand->mtd), GFP_KERNEL);
	if (!snand->mtd) {
		dev_err(snand->dev, "%s: Out of memory\n", __func__);
		return;
	}
#endif
	snand->name = name;

	/*
	 * gd5f reads three ID bytes, and mt29f reads one dummy address byte
	 * and two ID bytes. Therefore, we could detect both in the same
	 * read_id implementation by reading _with_ and _without_ a dummy byte,
	 * until a proper manufacturer is found.
	 *
	 * This'll mean we won't need to specify any specific compatible string
	 * for a given device, and instead just support spi-nand.
	 */
#ifdef CONFIG_SPI_NAND_MT29F
		snand->read_id = spi_nand_mt29f_read_id;
		snand->get_ecc_status = spi_nand_mt29f_ecc_status;
		node = fdtdec_next_compatible(gd->fdt_blob, 0,
				COMPAT_MICRON_NAND_MT29);
#else
#ifdef CONFIG_SPI_NAND_GD5F
		snand->read_id = spi_nand_gd5f_read_id;
		snand->get_ecc_status = spi_nand_gd5f_ecc_status;
		node = fdtdec_next_compatible(gd->fdt_blob, 0,
				COMPAT_GIGADEVICE_NAND_GD5F);
#else
		dev_err(snand->dev, "unknown device\n");
		return;
#endif /* CONFIG_SPI_NAND_GD5F */
#endif  /* CONFIG_SPI_NAND_MT29F */

	if (node < 0) {
		dev_err(snand->dev, "No compatible node found in device tree.\n");
		return;
	} else if (fdt_decode_nand(gd->fdt_blob, node, config)) {
		dev_err(snand->dev, "Error parsing device-tree node.\n");
		return;
	}

	/*
	 * Initialize spi_slave
	 * TODO1: minimalistic init, just as it needs
	 * this will become better populated when DT is available
	 */
	priv->spi = spi_setup_slave(CONFIG_SF_DEFAULT_BUS, CONFIG_SYS_NAND_CS,
				    0, 0);

	ret = spi_nand_register(snand, spi_nand_flash_ids);
	if (ret) {
		dev_err(snand->dev, "spi_nand_register failed\n");
		return;
	}
	if (nand_register(0)) {
		dev_err(snand->dev, "nand_register failed\n");
		return;
	}
}
