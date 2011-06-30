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

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
using namespace std;

// for readdir
#include <dirent.h>

// for stat
#include <sys/stat.h>
#include <sys/types.h>

#include "asciiportal.h"
#include "ap_filemgr.h"

string FileManager::inscreen = "inscreen.txt";
string FileManager::credits = "credits.txt";
string FileManager::infos = "infos.yaml";
string FileManager::default_mappack = "default";

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

bool file_exists( string const & filename ) {
  struct stat buffer;
  return (stat(filename.c_str(), &buffer) == 0);
}

string FileManager::try_locations(vector<string> const& locations, string const & filename) const {
  for (int i = 0; i < locations.size(); ++i) {
    if (file_exists(locations[i] + s + filename))
      return locations[i];
  }
  // Not found
  return "";
}

string FileManager::try_locations(vector<string> const& locations) const {
  for (int i = 0; i < locations.size(); ++i) {
    if (file_exists(locations[i]))
      return locations[i];
  }
  // Not found
  return "";
}

void makedir( string const & path) {
#ifndef WIN32
  mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#else
  mkdir(path.c_str());
#endif
}

// returns a list of all subdirectories of the given directory
vector<string> FileManager::get_subdirectories ( string const & _dir ) const {
  // code heavily inspired from 'man 3 stat'
  struct dirent *dp;
  struct stat statbuf;
  string filename, fullpath, dir;
  vector<string> result;
  if (_dir.size() == 0) dir = ".";
  else dir = _dir;
  DIR *dirp = opendir(dir.c_str());

  while ((dp = readdir(dirp)) != NULL) {
    filename = dp->d_name;
    fullpath = dir + s + filename;
    if (stat(fullpath.c_str(), &statbuf) == -1)
      continue;
    
    if (filename != "." && filename != ".." && S_ISDIR(statbuf.st_mode))
      result.push_back(filename);
  }

  return result;
}

string FileManager::get_lvl_filename(int level) const {
  stringstream num;
  num << setw(3) << setfill( '0' ) << level;
  return (num.str() + ".txt");
}

// Initialize everything based on the environment
FileManager::FileManager() {
  userpath = "";
  basepath = ".";

#ifdef WIN32
  s = "\\";
#else
  s = "/";

  // Search for a decent basepath for media and maps (on unix)
  if (file_exists("/usr/share/asciiportal"))
    basepath = "/usr/share/asciiportal";
#endif

  // Look for a user-specific directory to store save data and custom maps
#ifdef WIN32
  userpath = get_env_var("APPDATA");
#else
  userpath = get_env_var("HOME");
#endif
  if (userpath != "") {
    userpath += s + ".asciiportal";
    if (! file_exists(userpath)) {
      cout << "Home directory " << userpath << " not existing, creating it." << endl;
      makedir(userpath);
      ofstream readme;
      readme.open( (userpath + s + "readme").c_str() );
      if (readme.is_open()) {
        readme << "This is the user-specific data directory for ASCIIpOrtal.\n\n"
               << "It is used to store save data for each map pack, in Yaml format.\n"
               << "You might also want to provide custom mappacks; just put your maps under\n"
               << "a <map_pack_name> subdirectory, and make sure your maps are named properly\n"
               << "('001.txt', '002.txt', ...)\n\n"
               << "Keep in mind that this directory is searched first; if you provide maps\n"
               << "under a mappack name that already exists (e.g. 'default/'), they'll get used\n"
               << "instead of the official ones.\n";
        //<< "Note that this can be used to extend the official mappacks: just start the\n"
        //<< "number of your maps where the official maps number ends.\n";
        readme.close();
      }
    }
  }
} //FileManager::FileManager


string FileManager::get_media(string const & media) const {
  return(basepath + s + "media" + s + media);
}

vector<string> FileManager::list_mappacks() const {
  vector<string> official = get_subdirectories(basepath + s + "maps");
  if (userpath != "") {
    vector<string> custom = get_subdirectories(userpath);
    official.insert(official.end(), custom.begin(), custom.end());

    // We need to remove duplicates
    sort(official.begin(), official.end());
    official.erase( unique(official.begin(), official.end()), official.end() );
  }
  return official;
}


MapPack_FileManager::MapPack_FileManager(string const & _name) {
  name = _name;
  
  // Try to find where is the map stored. We look for the 'infos.yaml'
  // file.
  vector<string> paths;
  paths.push_back(userpath + s + name);
  paths.push_back(basepath + s + "maps" + s + name);
  fullpath = try_locations(paths, infos);
}

string MapPack_FileManager::get_media(string const & media) const {
  vector<string> locs;
  locs.push_back(fullpath + s + media);
  locs.push_back(basepath + s + "media" + s + media);
  return try_locations(locs);
}

// Get the path of the inscreen file, falling back to the default one
// if not found.
string MapPack_FileManager::get_inscreen() const {
  vector<string> locs;
  locs.push_back(fullpath + s + inscreen);
  locs.push_back(basepath + s + "maps" + s + default_mappack + s + inscreen);
  return try_locations(locs);
}

string MapPack_FileManager::get_save() const {
  return userpath + s + name + ".yaml";
}

string MapPack_FileManager::get_old_save() const {
  vector<string> locs;
  locs.push_back(userpath + s + name + s + "save.dat");
  locs.push_back(basepath + s + "maps" + s + name + s + "save.dat");
  return try_locations(locs);
}

// returns true if the given file name is likely to contain a map
// (i.e. "xyz.txt" where x, y, z are digits)
bool is_level_file(string const & name) {
  if (name.size() != 7)
    return false;
  if (name.substr(3, 4) != ".txt")
    return false;
  for (int i=0; i < 3; ++i) {
    if (!isdigit(name[i]))
      return false;
  }

  return true;
}

int MapPack_FileManager::get_number_maps() const {

  struct dirent *dp;
  struct stat statbuf;
  string filename, fullname;
  int result = 0;
  DIR *dirp = opendir(fullpath.c_str());

  while ((dp = readdir(dirp)) != NULL) {
    filename = dp->d_name;
    fullname = fullpath + s + filename;
    if (stat(fullname.c_str(), &statbuf) == -1)
      continue;
    if (is_level_file(filename))
      ++result;
  }

  return result;
}
