inherit xilinx-platform-init

FORCE_PLATFORM_INIT[doc] = "This variable is used to force the overriding of all platform init files in u-boot source."

PLATFORM_BOARD_DIR ?= ""
PLATFORM_BOARD_DIR_zynq = "board/xilinx/zynq"
PLATFORM_BOARD_DIR_zynqmp = "board/xilinx/zynqmp"

do_zynq_platform_init() {
	for f in ${PLATFORM_INIT_FILES}; do
		if [ -d "${S}/${PLATFORM_BOARD_DIR}/custom_hw_platform" ]; then
			cp ${PLATFORM_INIT_STAGE_DIR}/$f ${S}/${PLATFORM_BOARD_DIR}/custom_hw_platform/
		else
			cp ${PLATFORM_INIT_STAGE_DIR}/$f ${S}/${PLATFORM_BOARD_DIR}/
		fi
		# Newer u-boot sources use the init files in a sub directory named
		# based on the name of the device tree. This is not straight forward to
		# detect. Instead of detecting just overwrite all the platform init
		# files so that the correct one is always used. This shotgun approach
		# only works due to this recipe being machine arch specific. Do this
		# overwrite un-conditionally as there is no guarantees that the chosen
		# board config does not have the device tree config set.
		for i in ${S}/${PLATFORM_BOARD_DIR}/*/; do
			[ -d $i ] && cp ${PLATFORM_INIT_STAGE_DIR}/$f $i
		done
	done
}

python () {
    # strip the tail _config/_defconfig for better comparison
    def strip_config_name(c):
        for i in ["_config", "_defconfig"]:
            if c.endswith(i):
                return c[0:len(c) - len(i)]
        return c

    # Determine if target machine needs to provide a custom platform init files
    if d.getVar("SPL_BINARY") and d.getVar("SOC_FAMILY") in ["zynq", "zynqmp"]:
        hasconfigs = [strip_config_name(c) for c in (d.getVar("HAS_PLATFORM_INIT") or "").split()]
        currentconfig = strip_config_name(d.getVar("UBOOT_MACHINE"))

        # only add the dependency if u-boot doesn't already provide the platform init files
        if (currentconfig not in hasconfigs) or (d.getVar("FORCE_PLATFORM_INIT") == "1"):
            # force the dependency on a recipe that provides the platform init files
            d.appendVar("DEPENDS", " virtual/xilinx-platform-init")
            # setup task to modify platform init after unpack and before configure
            bb.build.addtask("do_zynq_platform_init", "do_configure", "do_prepare_recipe_sysroot", d)

    if "boot.bin" in d.getVar("SPL_BINARY") and d.getVar("SOC_FAMILY") in ["zynqmp"]:
        # depend on the pmu-firmware build
        d.appendVar("DEPENDS", " virtual/pmu-firmware")
        # determine the path relative to the source tree
        relpath = os.path.relpath(d.expand("${DEPLOY_DIR_IMAGE}/pmu-${MACHINE}.bin"), d.getVar("S"))
        # setup PMU Firmware path via MAKEFLAGS
        d.appendVar("EXTRA_OEMAKE", " CONFIG_PMUFW_INIT_FILE=\"{0}\"".format(relpath))
}
