SUMMARY = "Open-source LIPC header file"
HOMEPAGE = "https://github.com/Arkq/openlipc"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.txt;beginline=3;md5=49302caf63949bad4b75fb86fd99b26b"

DEPENDS = "lipc"

SRC_URI = "git://github.com/Arkq/openlipc.git"
SRCREV = "master"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

EXTRA_OECONF += "--without-lipc-prop"
EXTRA_OECONF += "--without-lipc-probe"
