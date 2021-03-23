# Reset this
SRC_URI = "${EMBEDDEDSW_SRCURI}"

inherit esw

ESW_COMPONENT_SRC = "/lib/sw_apps/versal_plm/src/"

ESW_COMPONENT = "versal_plm.elf"

do_compile_append() {
    ${MB_OBJCOPY} -O binary ${B}/${ESW_COMPONENT} ${B}/plm.bin
}

do_install() {
    :
}

DEPENDS += "xilstandalone xiltimer xilffs xilpdi xilplmi xilloader xilpm xilsecure xilsem"