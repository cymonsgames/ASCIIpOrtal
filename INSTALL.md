# About ASCIIpOrtal

See README.
This is documenting the way to build ASCIIpOrtal; if you only want to
play, you should check ASCIIpOrtal main website, we have pre-compiled
packages for Windows, Linux 32-bits and Linux 64-bits (we also have an
experimental package for OS X on intel).

http://cymonsgames.com/asciiportal/

---
## Compiling ASCIIpOrtal

Many Makefiles are provided for convenience, though the default one
should be ok for most usages.
Note that this has only be tested on a GNU/Linux system (but might
work as well on others unices).
On Windows, it might work using Mingw, but you're on your own and you
are likely to have to tweak the build system a bit.
If you are able to build it, feel free to contribute! Best would be to
use 'Makefile.example' as a starting point though.

---
## Dependencies

To build ASCIIpOrtal the standard way (that is, using SDL to emulate a
curses environment), you need the following:
 + a C++ compiler (GNU g++ is preferred) along with the standard library;
 + SDL headers and library;
 + SDL_mixer headers and library;
 + yaml-cpp headers and library.
 + PDCurses compiled with SDL support;

The last one is the trickier to get, it might not be available on your
system.
However, for convenience, the standard Makefile target downloads and
builds it for you, see below.

Note that the 'nosdl' port is only using standard ncurses, so you don't
need SDL nor PDCurses. However, it's text-only and it's a bit buggy at
the moment (most notably, the keyboard input really sucks)

---
## Using the Makefiles

If you have neither PDCurses with SDL support, nor yaml-cpp available on
your system, then run this target:
```
$ make everything
```
It should download and build PDCurses and yaml-cpp, then build ASCIIpOrtal.

If you already have PDCurses and yaml-cpp on your system, you might want to
use the 'linux' target (note that it could work fine on OS X too):
```
$ make linux
```
It basically does the same, without building PDCurses or yaml-cpp first.
