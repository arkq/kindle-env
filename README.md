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


### Gentoo crossdev

The easiest way of creating optimal cross-compilation setup is to build all required libraries and
tools from scratches. Using [Gentoo Linux](https://www.gentoo.org/) seems to be the best option
for achieving this goal. This manual is dedicated for the 1st generation Kindle Touch device.

Install GCC cross-compilation toolchain as follows (it might be require to specify exact libc or
gcc version for build to succeed):

	# emerge sys-devel/crossdev
	# crossdev -t armv7a-softfp-linux-gnueabi

Tune the pre-generated portage make.conf file
(/usr/armv7a-softfp-linux-gnueabi/etc/portage/make.conf) to match CPU capabilities.

> -march=armv7-a
> -mtune=cortex-a8
> -mfloat-abi=softfp
> -mfpu=neon
> -mthumb

See the exemplary [portage-make.conf](/examples/portage-make.conf) file in the examples directory,
it might give you a hint.


Mounting root image
-------------------

Download an appropriate firmware from the Amazon Kindle Software Updates
[page](http://www.amazon.com/help/kindlesoftwareupdates) and unpack it using the
[kindletool](https://github.com/NiLuJe/KindleTool) extraction tool (which source is linked in the
[tools](/tools) directory), as follows:

	$ kindletool extract update_kindle_x.x.x.bin /tmp
	$ gunzip /tmp/rootfs.img.gz
	$ mv /tmp/rootfs.img kindle-rootfs.img

Mount extracted image using provided mount wrapper (root privileges might be required):

	# ./bin/mount.kindle kindle-rootfs.img


Further reading
---------------

1. [Kindle Touch Hacking](http://wiki.mobileread.com/wiki/Kindle_Touch_Hacking)
2. [Kindle Touch hackers group](https://bitbucket.org/katey_hack/)
3. [Toolchains](http://elinux.org/Toolchains)
