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

/*
 * TODO: is it possible to define custom binary operators in C++? It
 * would be very convenient to have a '+/' operator that would
 * concatenate string while adding a path separator between each.
 * Eg.: "foo" +/ "bar" +/ "in"  ->  "foo/bar/in" or "foo\\bar\\in"
 */

class FileManager {
 private:
  // eg. "/usr/share/asciiportal" or ""
  string basepath;
  // eg. "~/.asciiportal" if available
  string userpath;
  // path separator
  string s;
  // available map packs
  vector<string> mappacks;
  // custom ones (only on unix atm)
  vector<string> custommappacks;
  // returns a list of all subdirectories of the given directory
  vector<string> get_subdirectories(string const &);
  // get the name of the given map file (eg 21 -> '0021.txt')
  string get_lvl_filename(int);
  // try to find a file in multiple locations; the first location that
  // exists gets chosen, "" if no location exists.
  string try_locations(string[]);

 public:
  FileManager();
  string get_userpath();
  string get_basepath();
  // Note that the following functions only try to find a decent path
  // for the request, they do not open the file.
  // Get the path to the given media file.
  string get_media(string const & media);
  // This looks into the map pack directory, but falls back to the
  // media directory if not found.
  string get_media(string const & mappack, string const & media);
  string get_map(string const & mappack, int level);
  // Get the path to the 'inscreen' file for the given mappack.
  // Falls back to the 'default' one if not found.
  string get_inscreen(string const & mappack);
  // Get the path to the 'credits' file for the given mappack.
  string get_credits(string const & mappack);
  // Get the highest level reached for the given map pack.
  int get_maxlevel(string const & mappack);
  // Store the highest level reached for the given map pack.
  void save_maxlevel(string const & mappack, int maxlevel);
  // with proper difficulty ordering.
  vector<string> list_official_mappacks();
  // maybe alphabetically for this one?
  vector<string> list_custom_mappacks();
};

#endif // AP_DRAW_H_INCLUDED
