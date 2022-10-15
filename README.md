# Using u-boot on Creator (Ci40) Marduk platform

## Toolchain setup
If you are using the toolchain produced by OpenWrt you must setup the environment as explained
[here](https://wiki.openwrt.org/doc/devel/crosscompile).

For the MIPS official toolchain see
[here](https://community.imgtec.com/developers/mips/tools/codescape-mips-sdk/download-codescape-mips-sdk-essentials/).

If building your own toolchain then note binutils-2.33.1 or earlier is required.

## Configure and build it
	$ make CROSS_COMPILE=<toolchain-prefix> pistachio_marduk_defconfig
	$ make CROSS_COMPILE=<toolchain-prefix>

This will generate `u-boot-pistachio_<board_name>-<version>.img`

    eg. u-boot-pistachio_marduk-2015.10-v1.0.4.img

OR `u-boot-pistachio_<board_name>-<version>-<githash>.img`, if there are extra commits above the tag.

    e.g. u-boot-pistachio_marduk-2015.10-v1.0.4-g75cbe38.img

## Flash it onto your Ci40
See our [docs](https://docs.creatordev.io/ci40/guides/openwrt-platform/#flashing-u-boot-binary).
