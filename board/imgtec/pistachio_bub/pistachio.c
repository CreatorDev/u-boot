/*
 * Copyright (C) 2015 Imagination Technologies
 * Author: Govindraj Raja <govindraj.raja@imgtec.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dwmmc.h>
#include <fdtdec.h>
#include <miiphy.h>
#include <malloc.h>
#include <netdev.h>
#include <serial.h>

#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/mmu.h>
#include <asm/mipsregs.h>
#include <asm/pistachio.h>
#include <asm-generic/sections.h>
#include <watchdog.h>

#include "mfio.h"

DECLARE_GLOBAL_DATA_PTR;
const char *enet_dtb_macaddr = 0;

int reloc_tlb_fixup(void)
{
	u32 text_start, text_size, data_start, data_size;
	u32 early_mem_start, remaining_unmapped;

	text_start = (u32)gd->relocaddr;
	text_size = (u32)__text_end - (u32)__text_start;
	data_start = text_start + text_size;
	data_size = gd->ram_top - data_start;
	early_mem_start = CONFIG_SYS_TEXT_BASE + CONFIG_UBOOT_EARLY_MEM;
	remaining_unmapped = text_start - early_mem_start;

	/* Text segment is maped cached */
	assert(!identity_map((u32)text_start, text_size, C0_ENTRYLO_WB));
	/* All data sections are mapped uncached to have DMA coherency */
	assert(!identity_map((u32)data_start, data_size, C0_ENTRYLO_UC));
	assert(!identity_map((u32)early_mem_start, remaining_unmapped,
		C0_ENTRYLO_UC));

	return 0;
}

phys_size_t initdram(int board_type)
{
#ifdef CONFIG_OF_CONTROL
	int node;
	fdt_addr_t addr;
        fdt_size_t size;

	node  = fdt_path_offset(gd->fdt_blob, "/memory");
	addr = fdtdec_get_addr_size(gd->fdt_blob, node, "reg", &size);
	if (addr == FDT_ADDR_T_NONE || size == 0) {
		printf("\n DRAM: Can't get mem size\n");
		return -1;
	}

	return (phys_size_t)size;
#else
	return CONFIG_SYS_MEM_SIZE;
#endif

}

int checkboard(void)
{
	return 0;
}

/*
 * Print CPU information
 */
int print_cpuinfo(void)
{
	printf("MIPS(interAptiv): IMG Pistachio %dMHz.\n", CONFIG_SYS_MHZ);
	return 0;
}

#ifdef CONFIG_OF_CONTROL
static const char *get_dtb_macaddr(u32 ifno)
{
	int node, len;
	char enet[16];
	const char *mac, *path;

	node = fdt_path_offset(gd->fdt_blob, "/aliases");
	if (node < 0) {
		printf("\n enet0: no /aliases found in dtb \n");
		return NULL;
	}

	sprintf(enet, "ethernet%d", ifno);
	path = fdt_getprop(gd->fdt_blob, node, enet, NULL);
	if (!path) {
		printf("enet0: enet0 alias not found \n");
		return NULL;
	}

	node = fdt_path_offset(gd->fdt_blob, path);
	mac = fdt_getprop(gd->fdt_blob, node, "mac-address", &len);
	if (mac && is_valid_ethaddr((u8 *)mac))
		return mac;

        return NULL;
}
#endif

int board_eth_init(bd_t *bs)
{
	mfio_setup_ethernet();

	/* try to get a valid macaddr from dtb */
#ifdef CONFIG_OF_CONTROL
	enet_dtb_macaddr = get_dtb_macaddr(0);

	if (enet_dtb_macaddr)
		eth_setenv_enetaddr("ethaddr", (u8 *)enet_dtb_macaddr);
	else
		printf("No valid Mac-addr found from dtb\n");
#endif

#ifndef CONFIG_DM_ETH
	if (designware_initialize(PISTACHIO_ETHERNET,
			PHY_INTERFACE_MODE_RMII) >= 0)
		return 1;
#endif

	return 0;
}

int board_mmc_init(bd_t *bis)
{
	struct dwmci_host *host = NULL;

	mfio_setup_mmc();

	host = malloc(sizeof(struct dwmci_host));
	if (!host) {
		printf("dwmci_host malloc fail!\n");
		return 1;
	}

	memset(host, 0, sizeof(struct dwmci_host));
	host->name = "Synopsys Mobile storage";
	host->ioaddr = (void *)0x18142000;
	host->buswidth = 4;
	host->dev_index = 0;
	host->bus_hz = 200000000;

	add_dwmci(host, host->bus_hz, 25000000);

	return 0;
}

void _machine_restart(void)
{
	// Generate system reset
	writel(0x1, PISTACHIO_WD + 0x0000);
}

int board_early_init_f(void)
{
	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_PISTACHIO_WATCHDOG
	hw_watchdog_init();
#endif
	return 0;
}

int misc_init_r(void)
{
	return 0;
}

#ifndef DM_SERIAL
struct serial_device *default_serial_console(void)
{
	return &eserial2_device;
}
#endif
