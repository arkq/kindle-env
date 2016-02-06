Kindle Touch Terminal
=====================

KTTerm is a terminal emulator for Kindle Touch device. It is based on the GTK+ terminal emulator
widget - [VTE](https://github.com/GNOME/vte).

![Screenshot](/tools/ktterm/screenshot.png?raw=true)


Keyboard shortcuts
------------------

* `Fn` + `F` - toggle full-screen mode
* `Fn` + `plus (+)` - increase font size
* `Fn` + `minus (-)` - decrease font size
* `Fn` + `0` - restore font size

These functions are available only with the build-in keyboard.


Keyboard toggle
---------------

It is possible to hide/show keyboard by the tap-and-hold action on the upper-right corner of the
KTTerm window. This action is available for build-in and Kindle on-board keyboard.


Compilation
-----------

Prior to compilation, one has to initialize Kindle development environment. How to achieve this,
see the [README](/README.md) file in the root directory of this repository. Then, do as follows:

	$ autoreconf --install
	$ mkdir build && cd build
	$ ../configure --host=armv7a-softfp-linux-gnueabi
	$ make kindle-relink

**Note:** Using the kindle-relink build target requires static libraries for json-c and VTE. It
is possible to compile KTTerm using standard build target, however such an action will produce
executable which may be incompatible with libraries present on the Kindle device.


Similar projects
----------------

1. [Kterm](http://www.fabiszewski.net/kindle-terminal/) - terminal emulator for Kindle Touch and
	 Paperwhite
