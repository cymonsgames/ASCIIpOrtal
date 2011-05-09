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

#include <iostream>
#include <vector>
#include <asciiportal.h>

/*
 * TODO: is it possible to define custom binary operators in C++? It
 * would be very convenient to have a '+/' operator that would
 * concatenate string while adding a path separator between each.
 * Eg.: "foo" +/ "bar" +/ "in"  ->  "foo/bar/in" or "foo\\bar\\in"
 */

class FileManager {
 private:
  // TODO: state variables to be initialized at creation (eg. running
  // on windows, having a location for ressources, etc)
 public:
  FileManager();
  // Note that this only tries to find a decent path for the request,
  // it does not even open the file.
  string get_media(string);
  string get_map(string);
  string get_save(string);
  // Store the highest level reached for the given map pack.
  string save_maxlevel(string, int);
  // with proper difficulty ordering.
  vector<string> list_official_mappacks();
  // maybe alphabetically for this one?
  vector<string> list_custom_mappacks();
};
