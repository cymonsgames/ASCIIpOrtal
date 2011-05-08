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
// for stat
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

// base directory for media, maps, ...
// if available, use "/usr/share/asciiportal/", else "" (current dir)
string basepath;

// base directory for save data and custom maps
// if available, "~/.asciiportal/", else "" (current dir)
string userpath;

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

int loadmaps (string mappack) { // Looks for the mappack directory and loads
  // the files 001.txt, 002.txt etc and stores it in maps.
  string line, name;
  int level = 0;

  // cleanup
  for (int c = 0; c < rawmaps.size(); c++) {
    for (int d = 0; d < rawmaps[c].size(); d++) {
      rawmaps[c][d].clear();
    }
    rawmaps[c].clear();
  }
  rawmaps.clear();
  
  // load maxlevel for the current mappack
  maxlevel = 0;
  string maxlevelfilename;
#ifdef WIN32
  maxlevelfilename = mappack + "\\save.dat";
#else
  maxlevelfilename = userpath + mappack + "/save.dat";
#endif
  ifstream maxlevelfile;
  maxlevelfile.open (maxlevelfilename.c_str());
  if (maxlevelfile.is_open()) {
    maxlevelfile >> maxlevel;
    maxlevelfile.close();
  }
    
  // load level files
  while (1) {
    level++;
    unsigned int maxwidth = 0;
    stringstream num;
    num << setw(3) << setfill( '0' ) << level;
#ifdef WIN32
    name = mappack + "\\" + num.str() + ".txt";
#else
    // On Linux/MacOS, the user-specific directory is searched first for custom maps
    name = userpath + mappack + "/" + num.str() + ".txt";
#endif

    ifstream mapfile(name.c_str());

#ifndef WIN32
    // on unix, if read fails in user directory, fall back to basepath
    if (! mapfile.is_open()) {
      name = basepath + mappack + "/" + num.str() + ".txt";
      mapfile.open(name.c_str());
    }
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
    } else return --level; // to return 0 if no map was found
  }
}

int main(int args, char* argv[]) {
  string mappack="maps";
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
          << " -m [mappackname]    Load a map pack (default: 'maps')\n"
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

#ifndef WIN32
  struct stat buffer;
  // Search for decent basepath and userpath (on unix)
  if (stat("/usr/share/asciiportal/", &buffer) == 0)
    basepath = "/usr/share/asciiportal/";

  userpath = get_env_var("HOME");
  if (userpath != "") {
    userpath = userpath + "/.asciiportal/";
    if (stat(userpath.c_str(), &buffer) != 0) {
      cout << "Home directory " << userpath << " not existing, creating it." << endl;
      mkdir(userpath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
      ofstream readme;
      readme.open( (userpath + "readme").c_str() );
      if (readme.is_open()) {
        readme << "This is the user-specific data directory for ASCIIpOrtal.\n\n"
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


  if (!loadmaps (mappack)) {
    cerr << "Error - The mappack '" << mappack << "' does not exist.\n\n"
	 << "You must place all level files in a directory named '" << mappack << "/'.\n"
#ifndef WIN32
	 << "On unix systems, you may provide custom map packs in '" << userpath << "'.\n"
#endif
	 << "The default mappack is in the '" << basepath << "maps/' directory.\n\n";

    if (mappack != "maps") {
      mappack = "maps";
      if (!loadmaps (mappack)) {
        cerr << "Error - No map files found in default location.\n"
	     << "Double check your installation!\n\n"
	     << "Press ENTER to continue.\n";
        exit(1);
      }
    }

    cin.get();
  }

  graphics_init (fullscreen, height, width);
#ifndef __NOSOUND__
  sound_init ();
#endif

  while (play (mappack) >= 0) ;

  graphics_deinit();
  cout << "Thank you for playing ASCIIpOrtal\n";
  exit(0);
}
