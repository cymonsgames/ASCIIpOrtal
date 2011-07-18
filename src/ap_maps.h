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


// Statistics of a level.
// Gets discarded at the end of the level (but its content might be
// added to the map pack stats).
struct levelstats {
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

// Informations about a map pack.
// Gets written to the disk.
struct mp_save {
  // for future use
  int version;
  /* stats */
  int numportals;
  int numdeaths;
  int numticks;
  int numsteps;
  // Number of levels accomplished (not necessarily distincts)
  int numlevels;

  // The maximum level reached for this map pack.
  // Upon starting, its value is 0; when level i is done, its value is i;
  // when a map pack is done, its value is 'properties.number_maps'.
  int maxlevel;
  // Last played level
  int lastlevel;
};

// Hold global persistent statistics, to be able to provide nice
// things like an achievements system, etc.
struct globalstats {
  int numportals;
  int numdeaths;
  int numsteps;
  // Number of levels accomplished (not necessarily distincts)
  int numlevels;
  // Number of map packs accomplished
  int nummappacks;
};


// General properties of a map pack.
// Once loaded from a file, this shouldn't be changing.
struct mp_properties {
  // Used for future improvements. '1' for now.
  int protocol;

  int number_maps;

  // Short name (eg. "Default levels")
  // This is what shows up in the map pack selection menu.
  // It should not be greater than 30 characters.
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
  // (default: 5 ; official map packs: around 10)
  int priority;

  // Not used at the moment: reserved for future use.
  int rating;
};

//void operator>>(const YAML::Node&, mp_properties&);

// This stores the necessary informations on a given level
struct level {
  // Level identification
  int id;
  string name;
  // the mappack name we're in
  string mappack;

  int ticks;
  
#ifndef __NOSOUND__
  // either a custom music file, or if empty, a default ambience (between 0 and 9)
  string musicfile;
  int musicid;

  // whether we need to play GladOS voice at the very beginning of the level
  bool has_message;
#endif
  
  // Stores the current map
  vector< vector<int> > map;

  // Stores the message associated with triggers
  string texttrigger[9];

  ObjectManager objm;
  object aimobject;
  Pager pager;

  // Statistics
  levelstats stats;

  void clear() {
    for (int yy = 0; yy < (signed)map.size(); ++yy)
      map[yy].clear();
    for (int yy = 0; yy < 9; ++yy)
      texttrigger[yy].clear();
    map.clear();
    objm.objs.clear();
    name.clear();
    pager.clear();
    stats.clear();
    ticks = 0;
#ifndef __NOSDL__
    musicid = 0;
    musicfile.clear();
    has_message = false;
#endif
  };
};

class MapPack {
private:
  void load_properties();
  void load_save();
  void write_save();

public:
  // Loads map pack description, initialize the bundled file manager.
  MapPack(string const & name);

  void dump();

  // Copy operator
  //  MapPack &operator=(const MapPack &source);

  // Holds map pack informations that needs to be saved
  mp_save save;

  // Holds everything related to the current level.
  level lvl;

  // Embedded file manager.
  MapPack_FileManager filemgr;

  // Name of the map pack (eg. "default")
  // Empty string if loading has failed.
  string name;
  
  mp_properties properties;

  int get_maxlevel() const { return save.maxlevel; };
  void set_maxlevel(int);
  int get_currentlevel() const { return lvl.id; };

  // this causes save data to be flushed to disk
  void set_lastlevel(int);
  int get_lastlevel() const;

  int get_number_maps() const;

  // This loads the required level as appropriate.
  int set_currentlevel(int);

  void reload_level();

  // Incrementation of the current level (prefix notation)
  MapPack &operator++(void);
  MapPack &operator--(void);

  // Used to sort map packs in the menu (by difficulty, etc)
  bool operator<(const MapPack &) const;


  // Loads map, texttrigger, objm, ...
  int load_map();

  // Hold persistent global statistics
  //globalstats stats;

  // Adds level stats to mappack stats
  void update_stats();

  //void clear();
};

#endif // MAPS_H_INCLUDED
