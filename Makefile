# MinGW32 cross-compilation of ASCIIpOrtal for windows, on a Linux or Unix host.
# Written by zorun <zerstorer at free dot fr>
# This works on Archlinux, feel free to adapt for other systems.
# Requires mingw32-gcc, mingw32-pdcurses compiled with sdl support, pdcurses (for xcurses-config command), mingw32-sdl, mingw32-sdl_mixer, zip, wget, find

ASCIIPORTAL_VER = 1.2c-mod-win32

# Used to download the dll
SDL_VER = 1.2.14
SDL_MIXER_VER = 1.2.11

MAPPACKS = maps challengeme evil masterlevels raisonbran648
OTHERFILES = media/ readme.txt pdcicon.bmp map_making_tips.txt

CXX = i486-mingw32-g++

# the -DSDL_main=main part is a *big* hack! I spent some hours stuck with a nasty bug ("undefined reference to `_SDL_main'" in libSDLmain.a) and came up with this; dunno why it works though.
CXXFLAGS = -O2 `xcurses-config --cflags` `/usr/i486-mingw32/bin/sdl-config --cflags` -DSDL_main=main

LINKFLAGS = -L/usr/i486-mingw32/lib -lpdcurses -lSDLmain -lSDL -mwindows -lSDL_mixer -lmingw32 -static-libgcc -static-libstdc++


# You should not need to edit anything past this line, excepted maybe to handle patch-related stuff.

OBJECTS = ap_object.o ap_input.o ap_draw.o ap_play.o ap_sound.o main.o menu.o

ZIPFILE = asciiportal-$(ASCIIPORTAL_VER).zip

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

asciiportal: patched.txt ASCIIpOrtal.exe
	@echo
	@echo "[*] ASCIIpOrtal.exe built."
	@echo "[*] Now run 'make dist' to package everything."

dist: $(ZIPFILE)
	@echo
	@echo "[*] $(ZIPFILE) successfully created."

all: dist

patched.txt:
	# This is needed to cross-compile
	patch -Np0 < asciiportal-mingw.patch
	# This may be commented out if you don't like them
	patch -Np1 < asciiportal-mod.patch
	patch -Np1 < asciiportal-res.patch
	patch -Np1 < asciiportal-save.patch
	@echo "Don't delete that file: it tells the Makefile that patching is done, so he won't do it again (this would be a bad idea)" > patched.txt
	@echo
	@echo "[*] Patching done."
	# We need to call make again, because the -mod patch created a new file
	$(MAKE)


ASCIIpOrtal.exe: $(OBJECTS)
	$(CXX) $^ $(LINKFLAGS) -o $@

SDL.dll:
	wget http://www.libsdl.org/release/SDL-$(SDL_VER)-win32.zip
	unzip SDL-$(SDL_VER)-win32.zip

SDL_mixer.dll:
	wget http://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-$(SDL_MIXER_VER)-win32.zip
	unzip SDL_mixer-$(SDL_MIXER_VER)-win32.zip
	mv README.txt README-SDL_mixer.txt

$(ZIPFILE): asciiportal SDL.dll SDL_mixer.dll clean-saves clean-dist
	mkdir asciiportal
	cp -r *.dll ASCIIpOrtal.exe $(MAPPACKS) $(OTHERFILES) asciiportal/
	zip -r $@ asciiportal/

softclean:
	rm -f *.o ASCIIpOrtal.exe

clean: softclean clean-dist
	rm -f SDL-$(SDL_VER)-win32.zip SDL_mixer-$(SDL_MIXER_VER)-win32.zip

hardclean: clean clean-saves
	rm -f *.dll README-SDL.txt README-SDL_mixer.txt

clean-saves:
	find $(MAPPACKS) -name save.dat -delete

clean-dist:
	rm -rf asciiportal/ $(ZIPFILE)
