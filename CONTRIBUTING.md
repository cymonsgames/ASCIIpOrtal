# Contribute to ASCIIpOrtal

ASCIIpOrtal is free software (as in freedom), feel free to look at the
code and adapt it to suit your needs.
As your release might be out-of-date, you might want to have a look at
our Github repository first:
https://github.com/cymonsgames/ASCIIpOrtal

---
## Bug report

If you find a bug, please report it either on the issue tracker at
[Github](https://github.com/cymonsgames/ASCIIpOrtal/issues) or on the
[ASCIIpOrtal forum](http://cymonsgames.com/forum/index.php?board=14.0).

---
## Contributing code

If you have some improvements or any change to the code in mind,
please act according to this list:
 + check the git repo. This may be already implemented but not
   released;
 + check the issue tracker at Github;
 + check the forum;
 + if your idea hasn't been implemented yet, well, create a post on
   the forum first;
 + code it for real.

If possible, fork the Github repo and fill a merge request when you're
done. Another way to contribute code is to send out patches on the forum.

---
## Porting ASCIIpOrtal to new systems

So far, ASCIIpOrtal has been known to work on the following systems:
 + Windows (cross-compiled on a Linux box, but native compilation
   should work using Mingw);
 + Linux 32 bits;
 + Linux 64 bits;
 + Mac OS X;
 + GP2X;
 + DINGOO.

If you want to add a new system to this list, you should start a new
thread on the forum, as it often requires a lot work (getting SDL to
run on the target system, handling user input, ...).

---
## Official releases

We officially release ASCIIpOrtal packages for the following systems:
 + Windows: binary zip file;
 + Linux: binary tarball for 32 and 64-bits, debian package (not yet
   available), Archlinux package (AUR);
 + OS X Intel dmg (courtesy of cynddl, it might be sometimes outdated)

See the website for downloads: http://cymonsgames.com/zorun/test-website
for the moment (it should become http://asciiportal.cymonsgames.com soon)

We would be very happy to add other systems to that list -- especially
GP2X and DINGOO, since the code is already running on these.

To build the releases, we are relying on a custom Makefile set: see
next section. The actual build is done on a 64 bits GNU/Linux system.

---
## Adding a Makefile

If you are willing to help, take a look at our Makefile set; it should
be as easy as copying 'Makefile.example' and start editing it.
Make sure to also read 'Makefile.common', and to add your port to the
list in the main 'Makefile'.

If you get it to work well, please contribute with your Makefile as
well as informations on how to compile ASCIIpOrtal for your target
system. For your information, the build is expected to run on a
64 bits GNU/Linux machine.
