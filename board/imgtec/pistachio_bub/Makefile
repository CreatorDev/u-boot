#
# Copyright (C) 2015 Imagination Technologies
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y				= mfio.o
obj-y				+= pistachio.o
ifdef CONFIG_SPL_BUILD
obj-y				+= clocks.o
ifdef CONFIG_DRAM_DDR2
obj-y				+= ddr2_init.o
endif
ifdef CONFIG_DRAM_DDR3
obj-y				+= ddr3_init.o
endif
obj-y				+= spl_pistachio.o
endif
