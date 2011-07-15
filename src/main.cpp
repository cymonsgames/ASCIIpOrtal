// ASCIIpOrtal ver 1.3 by Joseph Larson
// Copyright (c) 2009, 2011 Joseph Larson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// The source links with SDL, PDCurses compiled for SDL rendering (PDCSDL)
// and SDL mixer.

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <locale.h>
#include <unistd.h>

// for chdir() on windows
#include <unistd.h>

// for dirname
#include <libgen.h>

// Necessary to compile on OS X
#include <SDL/SDL.h>

using namespace std;
#include "asciiportal.h"
#include "ap_filemgr.h"
#ifndef __NOSOUND__
#include "ap_sound.h"
#endif
#include "ap_draw.h"
#include "ap_play.h"
#include "menu.h"

// File manager for filesystem operations
FileManager filemgr;

// easy debug
void debug(string message) {
#ifdef DEBUG_ME
  cerr << message << endl;
#endif
}

int main(int args, char* argv[]) {
  // to be able to find maps/media in the same directory as the binary
  chdir(dirname(argv[0]));

  string mappack_name = filemgr.default_mappack;
  bool fullscreen = false;
  bool pureAscii = false;
  string resolution;
  istringstream res_buffer;
  int height=480;
  int width=600;

  for (int c = 1; c < args; c++) {
    if ((argv[c][0] == '-') || (argv[c][0] == '/') || (argv[c][0] == '\\')) {
      switch (argv[c][1]) {
        case 'm' : mappack_name = argv[++c]; break;
#ifndef __NOSDL__
        case 'f' : fullscreen = true; break;
        case 'w' : fullscreen = false; break;
        case 'r' :
          c++;
          // we need to parse the resolution, eg "1024x768"
          resolution = argv[c];
          res_buffer.str(resolution.substr(0, resolution.find('x')));
          res_buffer >> width;
	  res_buffer.clear();
          res_buffer.str(resolution.substr(resolution.find('x')+1));
          res_buffer >> height;
          break;
#endif
        case 'a' : pureAscii = true; break;
        default :
          cout << "ASCIIpOrtal Command Line Parameters:\n\n"
               << " -?                  This help menu\n"
               << " -m [mappackname]    Load a map pack (default: '" << filemgr.default_mappack << "')\n"
#ifndef __NOSDL__
               << " -r <width>x<height> Choose the screen resolution (eg. 600x480, the default)\n"
               << " -f                  Fullscreen mode\n"
               << " -w                  Windowed mode (default)\n"
#endif
               << " -a                  Use only standard ASCII characters\n"
            ;
          exit (0); break;
      }
    }
  }

  MapPack mappack(mappack_name);

  if (mappack.name == "") {
    cerr << "Error - The mappack '" << mappack_name << "' does not exist.\n\n"
	 << "You must place all level files in a sub-directory of 'maps/' named '" << mappack_name << "/'.\n"
#ifndef WIN32
	 << "On unix systems, you may provide custom map packs in '" << filemgr.get_userpath() << "/'.\n"
#endif
	 << "Bundled official map packs are in the '" << filemgr.get_basepath() << "/maps/' directory.\n\n";

    if (mappack_name != filemgr.default_mappack) {
      MapPack mappack_(filemgr.default_mappack);
      if (mappack_.name == "") {
        cerr << "Error - No map files found in default location.\n"
	     << "Double check your installation!\n\n"
	     << "Press ENTER to continue.\n";
        exit(1);
      }
      mappack = mappack_;
    }

    cin.get();
  }

  setlocale (LC_ALL, "");

  graphics_init (fullscreen, height, width, pureAscii);
#ifndef __NOSOUND__
  sound_init ();
#endif

  while (main_menu (mappack) >= 0) ;

  graphics_deinit();
  cout << "Thank you for playing ASCIIpOrtal\n";
  _exit(0);
}
