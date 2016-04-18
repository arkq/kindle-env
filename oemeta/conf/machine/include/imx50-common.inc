# Provides the i.MX50 common settings

require conf/machine/include/imx-base.inc

# Tune for soft float-point with neon co-processor
DEFAULTTUNE_mx50 ?= "cortexa8t-neon"
require conf/machine/include/tune-cortexa8.inc

UBOOT_ENTRYPOINT_mx50 = "0x70008000"
