// ASCIIpOrtal ver 1.2 by Joseph Larson
// Copyright (c) 2009 Joseph Larson
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
#include <iomanip>
#include <string>
#include <vector>
#include <ctime>
using namespace std;
#include "asciiportal.h"
#ifndef __NOSOUND__
#include "ap_sound.h"
#endif
#include "ap_draw.h"
#include "ap_play.h"
#include "menu.h"

// begin game
vector<vector<string> > rawmaps;
vector<unsigned int> rawmaps_maxwidth;
extern int maxlevel;

int loadmaps (string mappack) {    // Looks for the mappack directory and loads
          // the files mappack001.txt, mappack002.txt etc and stores it in maps.
  string line, name;
  int level = 0;

  for (int c = 0; c < rawmaps.size(); c++) {
    for (int d = 0; d < rawmaps[c].size(); d++) {
      rawmaps[c][d].clear();
    }
    rawmaps[c].clear();
  }
  rawmaps.clear();

  maxlevel = 0;
  // load maxlevel
  string maxlevelfilename;
#ifdef WIN32
  maxlevelfilename = mappack + "\\save.dat"; // have to add conditional for linux
#else
  maxlevelfilename = mappack + "/save.dat"; // have to add conditional for linux
#endif
  ifstream maxlevelfile;
  maxlevelfile.open (maxlevelfilename.c_str());
  if (maxlevelfile.is_open()) {
    maxlevelfile >> maxlevel;
    maxlevelfile.close();
  }

  while (1) {
    level++;
    unsigned int maxwidth = 0;
    stringstream num;
    num << setw(3) << setfill( '0' ) << level;
#ifdef WIN32
    name = mappack + "\\" + num.str() + ".txt";
#else
    name = mappack + "/" + num.str() + ".txt";
#endif
    ifstream mapfile(name.c_str());
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
    } else return --level;
  }
}

int main(int args, char* argv[]) {
  string mappack="maps";
  int graphicsdefault = 1;
  int fullscreen = 0;
  int height=480;
  int width=600;
  string font="";

  for (int c = 1; c < args; c++) {
    if ((argv[c][0] == '-') || (argv[c][0] == '/') || (argv[c][0] == '\\')) {
      switch (argv[c][1]) {
        case 'm' : mappack = argv[++c]; break;
        case 'f' : graphicsdefault = 0; fullscreen = 1; break;
        case 'w' : graphicsdefault = 0; fullscreen = 0; break;
        case 'r' :
          c++;
          switch (argv[c][0]) {
            case '6' : graphicsdefault = 0; width = 640; height = 480; break;
            case '8' : graphicsdefault = 0; width = 800; height = 600; break;
            case '1' : graphicsdefault = 0; width = 1024; height = 768; break;
          }
          break;
        case 'd' : graphicsdefault = 1;
        case 'v' : font = argv[++c]; break;
        default :
          cout << "ASCIIpOrtal Command Line Parameters:\n\n"
          << " -?               This help menu\n"
          << " -m [mappackname] load a map pack\n"
#ifndef __NOSDL__
          << " -d               80x24 character text graphics window (overrides all other options)\n"
          << " -r 640x480       Choose 640x480 screen resolution\n"
          << " -r 800x600       Choose 800x600 screen resolution\n"
          << " -r 1024x768      Choose 1024x768 screen resolution\n"
          << " -f               Fullscreen\n"
          << " -w               Windowed mode\n"
#endif
          ;
          exit (0); break;
      }
    }
  }


  if (!loadmaps (mappack)) {
    cout << "Error - No map files found.\n\n"
    <<"You must place all level files in a directory with the same base name.\n"
    <<"The default should then be in \\maps\\maps001.txt\n\n";

    if (mappack != "maps") {
      mappack = "maps";
      if (!loadmaps (mappack)) {
        cout << "Error - No map files found in default location.\n\n"
        <<"Press ENTER to continue.\n";
        return 1;
      }
    }

    cin.get();
  }

  graphics_init (graphicsdefault, fullscreen, height, width, font);
#ifndef __NOSOUND__
  sound_init ();
#endif

  while (play (mappack) >= 0) ;

  graphics_deinit();
  cout << "Thank you for playing ASCIIpOrtal\n";
  return 0;
}
