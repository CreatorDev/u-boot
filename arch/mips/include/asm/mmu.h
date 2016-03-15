/*
 * Copyright (C) 2015 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_MMU_H
#define __ASM_ARCH_MMU_H

#include <asm/mipsregs.h>
#include <asm/types.h>

#include <linux/sizes.h>
#include <linux/types.h>

static inline u32 get_max_pagesize(void)
{
	u32 max_pgsize;

	write_c0_pagemask(C0_PAGEMASK_MASK << C0_PAGEMASK_SHIFT);
	back_to_back_c0_hazard();
	max_pgsize = (((read_c0_pagemask() >> C0_PAGEMASK_SHIFT) &
		       C0_PAGEMASK_MASK) + 1) * 4 * SZ_1K;

	return max_pgsize;
}

static inline u32 get_tlb_size(void)
{
	u32 tlbsize;

	tlbsize = ((read_c0_config1() >> C0_CONFIG1_MMUSIZE_SHIFT) &
		   C0_CONFIG1_MMUSIZE_MASK) + 1;

	return tlbsize;
}

int identity_map(u32 start, size_t len, u32 coherency);

#endif /* __ASM_ARCH_MMU_H */
