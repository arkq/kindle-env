SUMMARY = "Open-source LIPC header file"
HOMEPAGE = "https://github.com/Arkq/openlipc"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.txt;beginline=5;md5=838c366f69b72c5df05c96dff79b35f2"

DEPENDS = "lipc"

SRC_URI = "git://github.com/Arkq/openlipc.git"
SRCREV = "master"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

EXTRA_OECONF += "--without-lipc-prop"
EXTRA_OECONF += "--without-lipc-probe"
