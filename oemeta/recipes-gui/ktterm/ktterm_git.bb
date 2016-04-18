SUMMARY = "Kindle Touch Terminal"
HOMEPAGE = "https://github.com/Arkq/ktterm"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.txt;beginline=3;md5=49302caf63949bad4b75fb86fd99b26b"

DEPENDS = "json-c gtk+ openlipc vte"

SRC_URI = "git://github.com/Arkq/ktterm.git"
SRCREV = "master"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

EXTRA_OECONF += "--with-lipc"
