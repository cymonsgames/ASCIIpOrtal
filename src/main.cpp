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
#include <sys/stat.h>
#include <sys/types.h>
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

// gets the content of an environment variable
string get_env_var( string const & key ) {
  char * val;
  val = getenv( key.c_str() );
  string retval = "";
  if (val != NULL) {
    retval = val;
  }
  return retval;
}

int loadmaps (string mappack) {    // Looks for the mappack directory and loads
          // the files mappack001.txt, mappack002.txt etc and stores it in maps.
  string line, name, basename, localdir;
  int level = 0;

  for (int c = 0; c < rawmaps.size(); c++) {
    for (int d = 0; d < rawmaps[c].size(); d++) {
      rawmaps[c][d].clear();
    }
    rawmaps[c].clear();
  }
  rawmaps.clear();

  // try to find a user-specific place to load the data from
#ifdef WIN32
  // not used atm, since Windows users seem to be happy with the crappy way
  localdir = get_env_var("APPDATA");
  if (localdir != "")
    localdir = localdir + "\\asciiportal\\";
#else
  localdir = get_env_var("HOME");
  if (localdir != "") {
    localdir = localdir + "/.asciiportal/";
    struct stat buffer;
    if (stat(localdir.c_str(), &buffer) != 0) {
      cout << "Home directory " << localdir << " not existing, creating it." << endl;
      mkdir(localdir.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
      ofstream readme;
      readme.open( (localdir + "readme").c_str() );
      if (readme.is_open()) {
        readme << "This is the user-specific data directory for asciiportal.\n\n"
               << "It is used to store the current level for each mappack,\n"
               << "under <mappack_name>/save.dat\n\n"
               << "You might want to provide custom mappacks; just put your maps under\n"
               << "a <mappack_name> subdirectory, and make sure your maps are named properly\n"
               << "('001.txt', '002.txt', ...)\n\n"
               << "Keep in mind that this directory is searched first; if you provide maps\n"
               << "under a mappack name that already exists (e.g. 'maps/'), they'll get used\n"
               << "instead of the official ones.\n"
               << "Note that this can be used to extend the official mappacks: just start the\n"
               << "number of your maps where the official maps number ends.\n";
        readme.close();
      }
    }
  }
#endif


  maxlevel = 0;
  // load maxlevel
  string maxlevelfilename;
#ifdef WIN32
  maxlevelfilename = mappack + "\\save.dat";
#else
  maxlevelfilename = localdir + mappack + "/save.dat";
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
    basename = mappack + "/" + num.str() + ".txt";
    // On Linux/MacOS, the user-specific directory is searched first for maps
    name = localdir + basename;
#endif
    ifstream mapfile(name.c_str());
#ifndef WIN32
    if (! mapfile.is_open())
      mapfile.open(basename.c_str());
#endif
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
  string resolution;
  istringstream res_buffer;
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
          graphicsdefault = 0;
          // we need to parse the resolution, eg "1024x768"
          resolution = argv[c];
          res_buffer.str(resolution.substr(0, resolution.find('x')));
          res_buffer >> width;
	  res_buffer.clear();
          res_buffer.str(resolution.substr(resolution.find('x')+1));
          res_buffer >> height;
          break;
        case 'd' : graphicsdefault = 1; break;
        case 'v' : font = argv[++c]; break;
        default :
          cout << "ASCIIpOrtal Command Line Parameters:\n\n"
          << " -?                  This help menu\n"
          << " -m [mappackname]    Load a map pack (default: 'maps')\n"
#ifndef __NOSDL__
          << " -d                  80x24 character text graphics window (overrides all other options)\n"
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
  _exit(0);
}
