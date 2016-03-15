/*
 * Copyright (C) 2015 Imagination Technologies
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <bootcount.h>
void bootcount_store(ulong a)
{
	raw_bootcount_store((volatile u32 *)CONFIG_SYS_BOOTCOUNT_ADDR,
			    (BOOTCOUNT_MAGIC & 0xffff0000) | (a & 0x0000ffff));
}

ulong bootcount_load(void)
{
	unsigned long val;
	val = raw_bootcount_load((volatile u32 *)CONFIG_SYS_BOOTCOUNT_ADDR);
	if ((val & 0xffff0000) != (BOOTCOUNT_MAGIC & 0xffff0000))
		return 0;
	else
		return val & 0x0000ffff;
}
