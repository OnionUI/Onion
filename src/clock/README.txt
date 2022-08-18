This is a simple app that sets the clock.
This will be needed for platforms that only have a framebuffer available
and has SDL 1.2 available. (although, this could be ported to other libraries or one that uses fbdev directly).

This doesn't work on Beta OpenDingux yet as setting the clock requires sudo rights.

How to compile
===============

For now i assume that you have access to a linux computer or a linux VM.
This may work for WSL2 as well but it wasn't tested.

Stock firmware (2014 based)
___________________________

Download this toolchain :
http://www.gcw-zero.com/files/opendingux-gcw0-toolchain.2014-08-20.tar.bz2

And extract it to /opt.
In /opt/gcw0-toolchain, you should have the following folder called "usr". If that's the case, you're good.

Then open up a terminal in this folder and compile it with :
make -f Makefile.gcw0

Once compilation is sucessful, package everything up with :
./package gcw0

You can now push your OPK to your console either via FTP (to /media/apps) with an FTP client like Filezilla
or alternatively, use the external sd card slot and put the OPK in a folder called "apps".


Beta/Nightly firmware
___________________________

Download this toolchain :
http://od.abstraction.se/opendingux/opendingux-gcw0-toolchain.2020-10-01.tar.gz

And extract it to /opt.
In /opt/gcw0-toolchain, you should have the following files and folders:
bin
etc
include
lib
lib64
libexec
man
mipsel-buildroot-linux-musl
mipsel-gcw0-linux-uclibc
relocate-sdk.sh
sbin
share
usr

Then open up a terminal in this folder and compile it with :
make -f Makefile.gcw0

Once compilation is sucessful, package everything up with :
./package gcw0

You can now push your OPK to your console either via MTP (connect your console to your PC, make sure to use USB2 on the RG-350),
and put it in "Apps".
or alternatively, use the external sd card slot and put the OPK in a folder called "apps".
