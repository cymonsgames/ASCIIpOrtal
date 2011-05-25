/*
 * This holds everything related to map packs organization, map
 * loading and parsing, etc.
 */

#ifndef MAPS_H_INCLUDED
#define MAPS_H_INCLUDED

#include <string>
using namespace std;

#include "asciiportal.h"
#include "ap_object.h"
#include "ap_pager.h"
#include "ap_filemgr.h"
#include "ap_draw.h"

class MapPack_FileManager;

// Statistics of a level
class levelstats {
 public:
  int numportals;
  int numdeaths;
  int numticks;
  int numsteps;
  void clear() {
    numportals = 0;
    numdeaths = 0;
    numsteps = 0;
    numticks = 0;
  };
};

// Hold global persistent statistics, to be able to provide nice
// things like an achievements system, etc.
class globalstats {
 public:
  int numportals;
  int numdeaths;
  int numsteps;
  // Number of levels accomplished
  int numlevels;
  // Number of map packs accomplished
  int nummappacks;
};


// General properties of a map pack.
// Once loaded from a file, this shouldn't be changing.
class properties {
 public:
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
class level {
public:
  // Level identification
  int level_id;
  string levelname;
  // the mappack name we're in
  string mappack;

  int ticks;
  
  // Stores the current map
  vector< vector<int> > map;

  // Stores the message associated with triggers
  string texttrigger[9];

  ObjectManager objm;
  object aimobject;
  Pager pager;

  // Statistics
  levelstats stats;
};

class MapPack {
private:
  // The maximum level reached for this map pack.
  // Upon starting, its value is 0; when a map pack is done, its value
  // is 'properties.number_maps'.
  int maxlevel;

public:
  // Loads map pack description, initialize the bundled file manager.
  MapPack(string const & name);

  // Copy operator
  //  MapPack &operator=(const MapPack &source);

  // Holds everything related to the current level.
  level lvl;

  // Embedded file manager.
  MapPack_FileManager filemgr;

  // Name of the map pack (eg. "default")
  // Empty string if loading has failed.
  string name;
  
  properties properties;

  int get_maxlevel();
  // The current level is marked is maxlevel
  void set_maxlevel();
  int get_currentlevel();

  // This loads the required level as appropriate.
  void set_currentlevel(int);

  // Incrementation of the current level (prefix notation)
  MapPack &operator++(void);
  MapPack &operator--(void);

  // Loads map, texttrigger, objm, ...
  void load_map();

  // Hold persistent global statistics
  globalstats stats;

  // Adds level stats to global stats
  void update_stats();

  void clear_level();

  void clear();
};

#endif // MAPS_H_INCLUDED
