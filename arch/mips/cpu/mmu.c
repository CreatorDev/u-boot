/*
 * Copyright (C) 2015 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/mipsregs.h>
#include <asm/mmu.h>

#include <linux/kernel.h>
#include <linux/sizes.h>

#include <common.h>

#define MIN_PAGE_SIZE SZ_4K

static int add_wired_tlb_entry(u32 entrylo0, u32 entrylo1,
			       u32 entryhi, u32 pgsize)
{
	u32 tlbindex;

	tlbindex = read_c0_wired();
	if (tlbindex >= get_tlb_size() || tlbindex >= C0_WIRED_MASK) {
		return -1;
	}
	write_c0_wired(tlbindex + 1);
	write_c0_index(tlbindex);
	write_c0_pagemask(((pgsize / MIN_PAGE_SIZE) - 1) << C0_PAGEMASK_SHIFT);
	write_c0_entryhi(entryhi);
	write_c0_entrylo0(entrylo0);
	write_c0_entrylo1(entrylo1);
	mtc0_tlbw_hazard();
	tlb_write_indexed();
	tlbw_use_hazard();

	return 0;
}

static u32 pick_pagesize(u32 start, u32 len)
{
	u32 pgsize, max_pgsize;

	max_pgsize = get_max_pagesize();
	for (pgsize = max_pgsize;
	     pgsize >= MIN_PAGE_SIZE;
	     pgsize = pgsize / 4) {
		/*
		 * Each TLB entry maps a pair of virtual pages.  To avoid
		 * aliasing, pick the largest page size that is at most
		 * half the size of the region we're trying to map.
		 */
		if (IS_ALIGNED(start, 2 * pgsize) && (2 * pgsize <= len))
			break;
	}

	return pgsize;
}

/*
 * Identity map the memory from [start,start+len] in the TLB using the
 * largest suitable page size so as to conserve TLB entries.
 */
int identity_map(u32 start, size_t len, u32 coherency)
{
	u32 pgsize, pfn, entryhi, entrylo0, entrylo1;

	coherency &= C0_ENTRYLO_COHERENCY_MASK;
	while (len > 0) {
		pgsize = pick_pagesize(start, len);
		entryhi = start;
		pfn = start >> 12;
		entrylo0 = (pfn << C0_ENTRYLO_PFN_SHIFT) | coherency |
			C0_ENTRYLO_D | C0_ENTRYLO_V | C0_ENTRYLO_G;
		start += pgsize;
		len -= min(len, pgsize);
		if (len >= pgsize) {
			pfn = start >> 12;
			entrylo1 = (pfn << C0_ENTRYLO_PFN_SHIFT) |
				coherency | C0_ENTRYLO_D | C0_ENTRYLO_V |
				C0_ENTRYLO_G;
			start += pgsize;
			len -= min(len, pgsize);
		} else {
			entrylo1 = 0;
		}
		if (add_wired_tlb_entry(entrylo0, entrylo1, entryhi, pgsize))
			return -1;
	}

	return 0;
}
