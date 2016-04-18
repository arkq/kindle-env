# Provides access to files located in mounted Kindle root image

HOMEPAGE = "http://www.amazon.com/help/kindlesoftwareupdates"

LICENSE = "ASL"
LIC_FILES_CHKSUM = "file://${LAYER_LICENSE_DIR}/ASL;md5=a74d8cc3ec1d6f912703ad96fccd3d07"

FILESEXTRAPATHS_append := ":${@os.environ['KINDLE_ROOTDIR']}"
