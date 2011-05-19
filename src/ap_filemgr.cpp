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
using namespace std;

// for readdir
#include <dirent.h>

// for stat
#include <sys/stat.h>
#include <sys/types.h>

#include "asciiportal.h"
#include "ap_filemgr.h"


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

void makedir( string const & path) {
#ifndef WIN32
  mkdir(path.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#else
  mkdir(path.c_str());
#endif
}

// returns a list of all subdirectories of the given directory
vector<string> FileManager::get_subdirectories ( string const & _dir ) {
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

string FileManager::get_lvl_filename(int level) {
  stringstream num;
  num << setw(3) << setfill( '0' ) << level;
  return (num.str() + ".txt");
}

string FileManager::try_locations(string locations[]) {
  int count = sizeof(locations);
  for (int i = 0; i < count; ++i) {
    if (file_exists(locations[i]))
      return locations[i];
  }
  // Not found
  return "";
}


// Initialize everything based on the environment
FileManager::FileManager() {
  userpath = "";
  basepath = ".";
  s = "\\";

#ifndef WIN32
  s = "/";

  // Search for a decent basepath for media and maps (on unix)
  if (file_exists("/usr/share/asciiportal"))
    basepath = "/usr/share/asciiportal";

  // Look for a user-specific directory to store save data and custom maps
  userpath = get_env_var("HOME");
  if (userpath != "") {
    userpath += "/.asciiportal";
    if (! file_exists(userpath)) {
      cout << "Home directory " << userpath << " not existing, creating it." << endl;
      makedir(userpath);
      ofstream readme;
      readme.open( (userpath + s + "readme").c_str() );
      if (readme.is_open()) {
        readme << "This is the user-specific data directory for ASCIIpOrtal.\n\n"
               << "It is used to store the current level for each map pack.\n"
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
#endif //!WIN32

  // now find available map packs
  mappacks.clear();
  custommappacks.clear();
  mappacks = get_subdirectories(basepath + s + "maps");
  if (userpath != "")
    custommappacks = get_subdirectories(userpath);
  
}

string FileManager::get_userpath()
{
  return userpath;
}

string FileManager::get_basepath()
{
  return basepath;
}

string FileManager::get_media(string const & media) {
  return(basepath + s + "media" + s + media);
}

string FileManager::get_media(string const & mappack, string const & media) {
  string locs[3] = { userpath + s + mappack + s + media,
                     basepath + s + "maps" + s + mappack + s + media,
                     basepath + s + "media" + s + media };
  return try_locations(locs);
}

string FileManager::get_map(string const & mappack, int level) {
  string levelfilename = get_lvl_filename(level);
  string localpath = userpath + s + mappack + s + levelfilename;
  if (file_exists(localpath))
    return localpath;
  else
    return basepath + s + "maps" + s + mappack + s + levelfilename;
}

// Get the path of the inscreen file, falling back to the default one
// if not found.
string FileManager::get_inscreen(string const & mappack)
{
  string inscreen = "inscreen.txt";
  string locs[3] = { userpath + s + mappack + s + inscreen,
                     basepath + s + "maps" + s + mappack + s + inscreen,
                     basepath + s + "maps" + s + "default" + s + inscreen };

  return try_locations(locs);
}

// Get the path of the credits file.
string FileManager::get_credits(string const & mappack)
{
  string credits = "credits.txt";
  string locs[2] = { userpath + s + mappack + s + credits,
                     basepath + s + "maps" + s + mappack + s + credits };

  return try_locations(locs);
}

// Get the path of the infos file.
string FileManager::get_infos(string const & mappack)
{
  string infos = "infos.txt";
  string locs[2] = { userpath + s + mappack + s + infos,
                     basepath + s + "maps" + s + mappack + s + infos };

  return try_locations(locs);
}

int FileManager::get_maxlevel(string const & mappack) {
  int maxlevel;
  string mapdir;
  if (userpath != "")
    mapdir = userpath + s + mappack;
  else
    mapdir = basepath + s + "maps" + s + mappack;
  
  string maxlevelfilename = mapdir + s + "save.dat";

  ifstream maxlevelfile;
  maxlevelfile.open ( maxlevelfilename.c_str() );
  if (maxlevelfile.is_open()) {
    maxlevelfile >> maxlevel;
    maxlevelfile.close();
    return maxlevel;
  }
  else return 0;
}

void FileManager::save_maxlevel(string const & mappack, int level) {
  string mapdir;
  if (userpath != "")
    mapdir = userpath + s + mappack;
  else
    mapdir = basepath + s + "maps" + s + mappack;
  
  string maxlevelfilename = mapdir + s + "save.dat";

  if (! file_exists(mapdir) )
    // create the mappack directory under the user-specific directory
    makedir(mapdir);

  ofstream maxlevelfile;
  maxlevelfile.open (maxlevelfilename.c_str());
  if (maxlevelfile.is_open()) {
    maxlevelfile << level;
    maxlevelfile.close();
  }
}

vector<MapPack> FileManager::list_mappacks() {
  return mappacks;
}
