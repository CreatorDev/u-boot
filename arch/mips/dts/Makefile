dtb-$(CONFIG_MACH_PISTACHIO) += pistachio_bub.dtb pistachio_marduk.dtb pistachio_beetle_mbub.dtb pistachio_concerto_mbub.dtb

targets += $(dtb-y)

# Add any required device tree compiler flags here
DTC_FLAGS +=

PHONY += dtbs
dtbs: $(addprefix $(obj)/, $(dtb-y))
	@:

clean-files := *.dtb
