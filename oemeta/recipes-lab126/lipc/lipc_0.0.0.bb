SUMMARY = "Kindle LIPC library"

inherit kindle-image

SRC_URI = "\
  file://usr/bin/lipc-hash-prop \
  file://usr/bin/lipc-send-event \
  file://usr/bin/lipc-wait-event \
  file://usr/bin/lipc-daemon \
  file://usr/bin/lipc-get-prop \
  file://usr/bin/lipc-probe \
  file://usr/bin/lipc-set-prop \
  file://usr/lib/liblipc.so \
  file://lipc-daemon-events.conf \
  file://lipc-daemon-props.conf \
"

RDEPENDS_${PN} = "dbus glib-2.0"

do_install() {

    install -d ${D}${sysconfdir}
    install -m 0644 ${WORKDIR}/lipc-daemon-events.conf ${D}${sysconfdir}
    install -m 0644 ${WORKDIR}/lipc-daemon-props.conf ${D}${sysconfdir}

    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/usr/bin/* ${D}${bindir}

    install -d ${D}${libdir}
    install -m 0755 ${WORKDIR}/usr/lib/liblipc.so ${D}${libdir}/liblipc.so.0.0.0
    ln -sf liblipc.so.0.0.0 ${D}${libdir}/liblipc.so.0
    ln -sf liblipc.so.0.0.0 ${D}${libdir}/liblipc.so

}
