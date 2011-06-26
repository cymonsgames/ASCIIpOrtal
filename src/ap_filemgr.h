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

/*
 * A file manager for ASCIIpOrtal
 *
 * The idea is that the main code should not know anything about the
 * file system layout.
 * Indeed, this can prove very complicated: media and maps may be
 * stored in the current dir, or in /usr/share/asciiportal; custom
 * maps and save data may be stored in a user-specific directory; on
 * windows, the path separator is '\\', while it's '/' on unix...
 * 
 * This is supposed to provide a high-level interface for this.
 *
 * For instance, calling 'get_media("mymusic.ogg")' would return a
 * correct path for the file;
 *               calling 'get_map("mymappack", 42)' would return the
 * path of the desired map;
 *               calling 'save_maxlevel("mappack", 12)' would update
 * the save file accordingly;
 *               calling 'list_official_mappacks()' would return a
 * proper list of mappacks;
 *               and so on...
 *
 * The point is that the code does not even need to know how and where
 * files are stored, it only needs to know if he wants a map, or a
 * media, or something else...
 */
#ifndef AP_FILEMGR_H_INCLUDED
#define AP_FILEMGR_H_INCLUDED

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

#include "asciiportal.h"

// Base class for a file manager
class FileManager {
protected:
  // eg. "/usr/share/asciiportal" or ""
  string basepath;

  // eg. "~/.asciiportal" if available
  string userpath;

  // path separator
  string s;

  // returns a list of all subdirectories of the given directory
  vector<string> get_subdirectories(string const &) const;

  // get the name of the given map file (eg 21 -> '0021.txt')
  string get_lvl_filename(int) const;

  // try to find the given file in multiple locations; the first
  // location where the file does exist gets chosen, "" if no such
  // location exists.
  string try_locations(vector<string> const& locations, string const & filename) const;

  // same thing, but operates directly on files.
  string try_locations(vector<string> const& locations) const;

public:
  FileManager();

  // various constants paths and file names
  // (WTF? C++ can't initialize constant members in a class
  // declaration? The actual syntax for this is awful...)
  static string inscreen;
  static string credits;
  static string infos;
  // This makes sure we only have to modify it in one place if we want
  // to change it.
  static string default_mappack;

  string get_userpath() const { return userpath; };
  string get_basepath() const { return basepath; };

  // Get the path to the given media file.
  string get_media(string const & media) const;

  // lists the potential mappacks
  vector<string> list_mappacks() const;
};


// Specialized class to be used embedded into a map pack object.
class MapPack_FileManager : public FileManager {
private:
  /*
  // Used to access some properties of the map pack we're in.
  Mappack *host;
  */

  // Name of the host map pack (eg. "default" or "n_a_portal")
  // This also is the name of the directory where the level files are stored.
  string name;

  // eg. "/usr/share/asciiportal/maps/default" or "~/.asciiportal/default"
  // Empty string if not found.
  string fullpath;

public:
  /*
  // The FileManager constructor should be called first by the compiler.
  // A pointer to the host MapPack object.
  MapPack_FileManager(Mappack *mappack);
  */
  // The FileManager constructor should be called first by the compiler.
  // Name of the host map pack, see above.
  MapPack_FileManager(string const & name);

  string get_fullpath() const { return fullpath; };

  // This looks into the host map pack directory, but falls back to the
  // media directory if not found.
  // Overloads the FileManager method.
  string get_media(string const & media) const;

  // Get the path to the given level's map file.
  string get_map(int level) const { return fullpath + s + get_lvl_filename(level); };

  // Get the path to the 'inscreen' file for the host mappack.
  // Falls back to the 'default' one if not found.
  string get_inscreen() const;

  // Get the path to the 'credits' file for the host mappack.
  string get_credits() const { return fullpath + s + credits; };

  // Get the path to the 'infos' file for the host mappack.
  string get_infos() const { return fullpath + s + infos; };

  // Get the path to the save file for the host mappack.
  string get_save() const;

  // Get the path to the old save file (save.dat) for the host
  // mappack.
  // Only useful for people upgrading.
  string get_old_save() const;

  // Looks into the mappack directory to calculate how many levels we get
  int get_number_maps() const;
};

bool file_exists(string const &);

#endif // AP_DRAW_H_INCLUDED
