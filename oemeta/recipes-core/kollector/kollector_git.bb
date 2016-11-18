SUMMARY = "Collections builder for Kindle"
HOMEPAGE = "https://github.com/Arkq/kollector"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE.txt;beginline=5;md5=838c366f69b72c5df05c96dff79b35f2"

DEPENDS = "curl glib-2.0 openlipc sqlite3 util-linux"

SRC_URI = "git://github.com/Arkq/kollector.git"
SRCREV = "master"

S = "${WORKDIR}/git"

inherit autotools pkgconfig

EXTRA_OECONF += "--with-watcher"
