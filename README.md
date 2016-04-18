Development Environment for Kindle
==================================

**Disclaimer:** This development environment is based on a cross-compiler toolchain, however it
might be used on a native [ARM](https://en.wikipedia.org/wiki/ARM_architecture)-based host (e.g.
Kindle device) as well.


Cross-compiler setup
--------------------

[Kindle devices](https://en.wikipedia.org/wiki/Amazon_Kindle) are based on the [Freescale
i.MX](https://en.wikipedia.org/wiki/I.MX) CPU family. If performance is not an issue, one might
use generic ARM CPU architecture during compilation, however this will produce sub-optimal
executables. In order to create optimal setup environment, please check which CPU particular
device has, and use appropriate settings.


### OpenEmbedded build framework

Using the [OpenEmbedded](http://www.openembedded.org/) build framework seems to be the most
straightforward approach for the cross-complier setup. This framework provides cross-compilation
toolchain (built from scratches), plus optimization patches required for ARM architecture.

Firstly, you have to initialize the build environment. To do so, run the initialization script
provided by this repository as follows:

	$ ./bin/oecore-init

This script will fetch required OpenEmbedded components and will pre-setup build environment for
the ARM architecture. Next step is to tune auto-generated local configuration file (local.conf),
which will be placed in the build/conf/ directory. Set the desired machine as a build target,
e.g.:

> MACHINE = "kindle-touch"

For more information see the [oemeta](/oemeta) directory, which contains Kindle-specific
OpenEmbedded layer.

Afterwards, source the initialization script (`. ./bin/oecore-init`) and you are ready to go.


### Gentoo crossdev

The second easiest way of creating optimal cross-compilation setup is to use [Gentoo
Linux](https://www.gentoo.org/) as a build environment. This instruction is dedicated for the 1st
generation Kindle Touch device.

Install GCC cross-compilation toolchain as follows (it might be required to specify exact libc or
gcc version for build to succeed):

	# emerge sys-devel/crossdev
	# crossdev -t armv7a-softfp-linux-gnueabi

Tune the pre-generated portage make.conf file (located in the
/usr/armv7a-softfp-linux-gnueabi/etc/portage/ directory) to match CPU capabilities.

> -march=armv7-a
> -mtune=cortex-a8
> -mfloat-abi=softfp
> -mfpu=neon
> -mthumb

See the exemplary [portage-make.conf](/portage-make.conf) file, it might give you a hint.


Mounting root image
-------------------

Download an appropriate firmware from the Amazon Kindle Software Updates
[page](http://www.amazon.com/help/kindlesoftwareupdates) and unpack it using the
[kindletool](https://github.com/NiLuJe/KindleTool) extraction tool (which source is linked as a
submodule in the [tools](/tools) directory), as follows:

	$ kindletool extract update_kindle_x.x.x.bin /tmp
	$ gunzip /tmp/rootfs.img.gz
	$ mv /tmp/rootfs.img kindle-rootfs.img

Mount extracted image using provided mount wrapper (root privileges might be required):

	# ./bin/mount.kindle -w kindle-rootfs.img

The original Kindle firmware image contains linker-script libraries which are not suitable for
cross-compilation. In order to link executables with libraries present on the Kindle root image,
it is required to fix these linker-scripts. To do so, use provided `kindle-ldfix` tool as follows:

	# ./bin/kindle-ldfix

For more information see
[this thread](https://stackoverflow.com/questions/7476625/set-global-gcc-default-search-paths) on
Stack Overflow.


Further reading
---------------

1. [Kindle Touch Hacking](http://wiki.mobileread.com/wiki/Kindle_Touch_Hacking)
2. [Kindle Touch hackers group](https://bitbucket.org/katey_hack/)
3. [OpenEmbedded build framework](http://www.openembedded.org/)
4. [Toolchains](http://elinux.org/Toolchains)
