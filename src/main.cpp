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
using namespace std;
#include "asciiportal.h"
#include "ap_filemgr.h"
#ifndef __NOSOUND__
#include "ap_sound.h"
#endif
#include "ap_draw.h"
#include "ap_play.h"
#include "menu.h"

// begin game (TODO: get rid of these)
vector<vector<string> > rawmaps;
vector<unsigned int> rawmaps_maxwidth;
extern int maxlevel;

// File manager for filesystem operations
FileManager filemgr;

// easy debug
void debug(string message) {
#ifdef DEBUG_ME
  cerr << message << endl;
#endif
}

// TODO: move this to an appropriate location and fuck this bunch of
// global variables (use a proper structure instead)
int loadmaps (string mappack) { // Looks for the mappack directory and loads
  // the files 001.txt, 002.txt etc and stores it in maps.
  string line;
  int level = 0;

  // cleanup
  for (int c = 0; c < rawmaps.size(); c++) {
    for (int d = 0; d < rawmaps[c].size(); d++) {
      rawmaps[c][d].clear();
    }
    rawmaps[c].clear();
  }
  rawmaps.clear();
  rawmaps_maxwidth.clear();
  
  // load maxlevel for the current mappack
#ifndef GODMODE
  maxlevel = filemgr.get_maxlevel(mappack);
#endif
    
  // load level files
  while (1) {
    ++level;
    unsigned int maxwidth = 0;

    ifstream mapfile( (filemgr.get_map(mappack, level)).c_str() );
    
    if (mapfile.is_open()) {
      vector<string> map;
      while (! mapfile.eof() ) {
        getline (mapfile, line);
        if (line.length() > maxwidth) maxwidth = line.length();
        map.push_back(line);
      }
      rawmaps.push_back(map);
      rawmaps_maxwidth.push_back(maxwidth);
      mapfile.close();
      map.clear();
    }
    else { // no more maps
      --level; // to return 0 if no map was found (I guess)
#ifdef GODMODE
      maxlevel = level;
      cout << "Godmode activated! maxlevel set to " << maxlevel << endl;
#endif
      return level; 
    }
  }
}

int main(int args, char* argv[]) {
  string mappack="default";
  bool fullscreen = false;
  string resolution;
  istringstream res_buffer;
  int height=480;
  int width=600;

  for (int c = 1; c < args; c++) {
    if ((argv[c][0] == '-') || (argv[c][0] == '/') || (argv[c][0] == '\\')) {
      switch (argv[c][1]) {
        case 'm' : mappack = argv[++c]; break;
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
        default :
          cout << "ASCIIpOrtal Command Line Parameters:\n\n"
          << " -?                  This help menu\n"
          << " -m [mappackname]    Load a map pack (default: 'default')\n"
#ifndef __NOSDL__
          << " -r <width>x<height> Choose the screen resolution (eg. 600x480, the default)\n"
          << " -f                  Fullscreen mode\n"
          << " -w                  Windowed mode (default)\n"
#endif
          ;
          exit (0); break;
      }
    }
  }

  if (!loadmaps (mappack)) {
    cerr << "Error - The mappack '" << mappack << "' does not exist.\n\n"
	 << "You must place all level files in a sub-directory of 'maps/' named '" << mappack << "/'.\n"
#ifndef WIN32
	 << "On unix systems, you may provide custom map packs in '" << filemgr.get_userpath() << "/'.\n"
#endif
	 << "Bundled official map packs are in the '" << filemgr.get_basepath() << "/maps/' directory.\n\n";

    if (mappack != "default") {
      mappack = "default";
      if (!loadmaps (mappack)) {
        cerr << "Error - No map files found in default location.\n"
	     << "Double check your installation!\n\n"
	     << "Press ENTER to continue.\n";
        exit(1);
      }
    }

    cin.get();
  }
  
  setlocale (LC_ALL, "");

  graphics_init (fullscreen, height, width);
#ifndef __NOSOUND__
  sound_init ();
#endif

  while (play (mappack) >= 0) ;

  graphics_deinit();
  cout << "Thank you for playing ASCIIpOrtal\n";
  _exit(0);
}
