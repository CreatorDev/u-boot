#
# SPDX-License-Identifier:	GPL-2.0+
#

ifneq ($(CONFIG_SPL_START_S_PATH),)
head-y				:= $(CONFIG_SPL_START_S_PATH:"%"=%)/start.o
else
head-y				:= arch/mips/cpu/start.o
endif

libs-y				+= arch/mips/cpu/
libs-y				+= arch/mips/lib/

libs-$(CONFIG_SOC_AU1X00)	+= arch/mips/mach-au1x00/
