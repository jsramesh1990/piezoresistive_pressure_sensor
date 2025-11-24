DESCRIPTION = "Install device tree overlays for ADS1115"
LICENSE = "MIT"
SRC_URI = "file://myboard-ads1115.dts"

do_install() {
    install -d ${D}/boot
    dtc -I dts -O dtb ${WORKDIR}/myboard-ads1115.dts -o ${D}/boot/myboard-ads1115.dtbo
}

FILES_${PN} += "/boot/myboard-ads1115.dtbo"
