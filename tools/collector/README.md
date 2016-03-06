Collector - Collections builder
===============================

Collector is a simple automatic collections manager. It will help you organize books on the Kindle
device by creating collections based on the directory structure of documents. For a quick peek of
this functionality, see the example below.

Given the directory structure as follows

	documents
	|-- George Orwell
	|   |-- A Clergyman's Daughter.mobi
	|   `-- Nineteen Eighty-Four.mobi
	`-- Stephen Hawking
	    `-- A Brief History of Time.mobi

by default this utility will create one collection based on the directory named "George Orwell".
The "Stephen Hawking" directory will not trigger collection creation, because it contains only one
book. Such a behavior can be changed by using the command line argument `-s` - run `collector -h`
for more information.


Compilation
-----------

Prior to compilation, one has to initialize Kindle development environment. How to achieve this,
see the [README](/README.md) file in the root directory of this repository. Then, do as follows:

	$ autoreconf --install
	$ mkdir build && cd build
	$ ../configure --host=armv7a-softfp-linux-gnueabi
	$ make kindle-relink


Similar projects
----------------

1. [KTCollectionsMgr](https://bitbucket.org/NiLuJe/ktcollectionsmgr) - GUI-based collections
	 manager
