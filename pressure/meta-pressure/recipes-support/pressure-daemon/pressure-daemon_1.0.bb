SUMMARY = "Pressure daemon (reads IIO sysfs and acts on thresholds)"
LICENSE = "MIT"
SRC_URI = "file://pressure_daemon.py                    file://pressure.service"
S = "${WORKDIR}"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/pressure_daemon.py ${D}${bindir}/pressure_daemon.py
    install -d ${D}${systemd_unitdir}
    install -m 0644 ${WORKDIR}/pressure.service ${D}${systemd_unitdir}/pressure.service
}

SYSTEMD_SERVICE_${PN} = "pressure.service"
