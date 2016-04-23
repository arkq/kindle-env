SUMMARY = "Kindle Touch Terminal"
HOMEPAGE = "https://github.com/Arkq/ktterm"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.txt;beginline=5;md5=838c366f69b72c5df05c96dff79b35f2"

DEPENDS = "json-c gtk+ openlipc vte"

SRC_URI = "git://github.com/Arkq/ktterm.git"
SRCREV = "master"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

EXTRA_OECONF += "--with-lipc"
