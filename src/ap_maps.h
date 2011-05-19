/*
 * This holds everything related to map packs organization, map
 * loading and parsing, etc.
 */

#ifndef MAPS_H_INCLUDED
#define MAPS_H_INCLUDED

#include <string>
using namespace std;

#include "ap_object.h"

enum state {
  MAPPACK_ERROR,
  MAP_ERROR,
  OK
};

// General properties of a map pack.
// Once loaded from a file, this shouldn't be changing.
struct properties {
  int number_maps;

  // Short name (eg. "Default levels")
  // This is what shows up in the map pack selection menu.
  // It should not be greater than xx characters.
  string name;

  // Long description
  string description;

  string author;
  string version;

  // From 0 (easiest) to 10 (impossible)
  // This is what gets map packs sorted in the menu.
  int difficulty;

  // Additional sorting criteria that gets used before difficulty.
  // Useful for official map packs to show up first.
  // Higher is better.
  // (default: 0 ; official map packs: around 10)
  int priority;

  // Not used at the moment: reserved for future use.
  int rating;
};

// This stores the necessary informations on a given level
struct level {
  // level identification
  int level_id;
  string levelname;
  // the mappack name we're in
  string mappack;
  
  // Stores the current map
  vector< vector<int> > map;

  // Stores the message associated with triggers
  string texttrigger[9];

  ObjectManager objm;
  object aimobject;
  Pager pager;

  // loads map, texttrigger, objm, ...
  void load_map();

  void clear();
};

class MapPack {
private:

  state state;

  // Holds everything related to the current map
  level currentlevel;

  // The maximum level reached for this map pack.
  // Upon starting, its value is 0; when a map pack is done, its value
  // is 'properties.number_maps'.
  int maxlevel;

public:
  MapPack(string const & path);

  // Location of the map pack (eg. "default")
  string path;
  
  // eg. "/usr/share/asciiportal/maps/default"
  string fullpath;
  
  properties properties;

  int get_maxlevel();
  void set_maxlevel(int);
  int get_currentlevel();

  // This loads the required map as appropriate
  void set_currentlevel(int);

  // incrementation of the current level (postfix notation)
  MapPack &operator++(void);
  MapPack &operator--(void);

  void clear();
};

#endif // MAPS_H_INCLUDED
