#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "yaml-cpp/yaml.h"
#include "ap_maps.h"

extern const int CharData [MAXColors][6];

// Overloading extraction operators to ease manipulation
void operator>>(ifstream& instream, mp_properties& p) {
  YAML::Node node = YAML::Load(instream);

  p.protocol = node["protocol"].as<int>();

  if (p.protocol > MAPS_PROTOCOL) { // new format we don't know about
    //TODO: use the window manager when it's done
    string _name;
    _name = node["name"].as<std::string>();
    cerr << "Warning!! This map pack (" << _name << ") makes use of a newer description format! Please upgrade your ASCIIpOrtal, or weird things will happen..." << endl;
    return;
  }

  p.name = node["name"].as<std::string>();
  p.description = node["description"].as<std::string>();
  p.author = node["author"].as<std::string>();
  p.version = node["version"].as<std::string>();
  p.difficulty = node["difficulty"].as<int>();
  p.priority = node["priority"].as<int>();
  //node["rating"].as<int>() >> p.rating;
}

void operator<<(ostream& outstream, const mp_properties& p) {
  YAML::Emitter out;
  out << YAML::Comment("This is a YAML-formatted file designed to store informations about the given map pack.") << YAML::Newline;
  out << YAML::BeginMap;
  out << YAML::Key << "protocol" << YAML::Value << p.protocol;
  out << YAML::Comment("The protocol version used. Leave this to the default value.");
  out << YAML::Key << "name" << YAML::Value << p.name;
  out << YAML::Comment("The name of the map pack. Should not exceed 30 characters.");
  out << YAML::Key << "description" << YAML::Value << YAML::Literal << p.description;
  out << YAML::Key << "author" << YAML::Value << p.author;
  out << YAML::Comment("The author of the map pack.");
  out << YAML::Key << "version" << YAML::Value << p.version;
  out << YAML::Comment("Version. You might want to provide a changelog file.");
  out << YAML::Key << "difficulty" << YAML::Value << p.difficulty;
  out << YAML::Comment("Difficulty range from 0 (easiest) to 10 (hardest)");
  out << YAML::Key << "priority" << YAML::Value << p.priority;
  out << YAML::Comment("This is used for official map packs only. The default should be fine.");
  out << YAML::EndMap << YAML::Newline;

  outstream << out.c_str();
}


void operator>>(ifstream& instream, mp_save& s)
{
  YAML::Node node = YAML::Load(instream);

  s.version = node["version"].as<int>();
  s.numportals = node["numportals"].as<int>();
  s.numdeaths = node["numdeaths"].as<int>();
  s.numticks = node["numticks"].as<int>();
  s.numsteps = node["numsteps"].as<int>();
  s.numlevels = node["numlevels"].as<int>();
  s.maxlevel = node["maxlevel"].as<int>();
  s.lastlevel = node["lastlevel"].as<int>();
}

void operator<<(ostream& outstream, const mp_save& s) {
  YAML::Emitter out;
  out << YAML::Comment("This is a Yaml-formatted save file for ASCIIpOrtal.") << YAML::Newline;
  out << YAML::BeginMap;
  out << YAML::Key << "version" << YAML::Value << s.version;
  out << YAML::Key << "numportals" << YAML::Value << s.numportals;
  out << YAML::Key << "numdeaths" << YAML::Value << s.numdeaths;
  out << YAML::Key << "numticks" << YAML::Value << s.numticks;
  out << YAML::Key << "numsteps" << YAML::Value << s.numsteps;
  out << YAML::Key << "numlevels" << YAML::Value << s.numlevels;
  out << YAML::Key << "maxlevel" << YAML::Value << s.maxlevel;
  out << YAML::Key << "lastlevel" << YAML::Value << s.lastlevel;
  out << YAML::EndMap << YAML::Newline;

  outstream << out.c_str();
}

void MapPack::dump() {
  cerr << "Dumping mappack:" << endl;
  cerr << save;
  cerr << properties;
  cerr << "Name: " << name << endl;

  cerr << "lvl.id: " << lvl.id << endl;
  cerr << "lvl.name: " << lvl.name << endl;
  cerr << "lvl.mappack: " << lvl.mappack << endl;
  cerr << "lvl.ticks: " << lvl.ticks << endl;
}


// Initialize 'properties' and the bundled file manager. Map loading
// will be done on-demand.
MapPack::MapPack(string const & _name) : filemgr(_name) {
  // Not found
  if (filemgr.get_fullpath() == "") {
    name = "";
    return;
  }

  name = _name;

  // load properties from a file
  load_properties();

  // load saved data
  load_save();

  if (save.lastlevel >= get_number_maps())
    // final level
    set_currentlevel(get_number_maps());
  else
    set_currentlevel(save.lastlevel);
}

void MapPack::load_properties() {
  if (file_exists(filemgr.get_infos())) {
    ifstream infos(filemgr.get_infos().c_str());
    infos >> properties;
    infos.close();
  }
  else {
    properties.protocol = 1;
    properties.name = name + "'s funny levels";
    properties.description = "This is a dumb default description intended to serve as an example.\nYou should really edit it to reflect the description of your own map pack!";
    properties.author = "ASCIIpOrtal";
    properties.version = "0.1";
    properties.difficulty = rand() % 11;
    properties.priority = 5;
    //properties.rating = 0;

    cerr << "File " << filemgr.get_infos() << " not existing, creating it..." << endl;

    ofstream infos(filemgr.get_infos().c_str());
    infos << properties;
    infos.close();
    //ofstream tmp("/tmp/infos.yaml");
    //tmp << out.c_str();
  }
  // calculate how many levels we have
  properties.number_maps = filemgr.get_number_maps();
}

void MapPack::load_save() {
  if (file_exists(filemgr.get_save())) {
    ifstream savefile(filemgr.get_save().c_str());
    savefile >> save;
    savefile.close();
  }
  else { // save data was not found
    save.version = 1;
    save.numportals = 0;
    save.numdeaths = 0;
    save.numticks = 0;
    save.numsteps = 0;
    save.numlevels = 0;
    save.maxlevel = 0;
    save.lastlevel = 0;

    // we still look at the old save.dat stuff
    if (file_exists(filemgr.get_old_save())) {
      ifstream oldsavefile(filemgr.get_old_save().c_str());
      oldsavefile >> save.maxlevel;
      oldsavefile.close();
    }

  }

#ifdef GODMODE
  save.maxlevel = get_number_maps();
  cout << "Godmode activated! maxlevel set to " << save.maxlevel << endl;
#endif

  if (save.lastlevel == 0) save.lastlevel = save.maxlevel + 1;

}

void MapPack::write_save() {
  ofstream savefile(filemgr.get_save().c_str());
  savefile << save;
  savefile.close();
}

void MapPack::set_maxlevel(int new_maxlvl) {
  if (new_maxlvl > save.maxlevel) {
    save.maxlevel = new_maxlvl;
    write_save();
  }

}

void MapPack::set_lastlevel(int last) {
  save.lastlevel = last;
  write_save();
}

int MapPack::get_lastlevel() const {
  return save.lastlevel;
}

int MapPack::get_number_maps() const {
  return properties.number_maps;
}

int MapPack::set_currentlevel(int newlvl) {
  //cerr << "Setting current level to " << newlvl << endl;
  if (newlvl <= 0 || newlvl > get_number_maps() + 1)
    return -1;

  lvl.clear();
  lvl.id = newlvl;
  set_lastlevel(newlvl);

  if (lvl.id >= get_number_maps() + 1)
    return -1; // the map pack is over, we don't load any map

  load_map();
}

void MapPack::reload_level() {
  // keep stats
  levelstats oldstats = lvl.stats;
  set_currentlevel(get_currentlevel());
  lvl.stats = oldstats;
}

// incrementation of the current level
MapPack &MapPack::operator++(void) {
  set_currentlevel(get_currentlevel() + 1);
  return *this;
}

// decrementation of the current level
MapPack &MapPack::operator--(void) {
  set_currentlevel(get_currentlevel() - 1);
  return *this;
}

bool MapPack::operator<(const MapPack & mp) const {
  // Higher priority is better (show up first)
  return (this->properties.priority > mp.properties.priority)
    || (this->properties.priority == mp.properties.priority
        && this->properties.difficulty < mp.properties.difficulty);
}

int MapPack::load_map() {
  // Clear the previous state
  lvl.clear();

  // Loads the new map
  ifstream mapfile( filemgr.get_map(lvl.id).c_str() );

  vector<string> rawmap;
  unsigned int maxwidth = 0;
  string line;

  if (mapfile.is_open()) {
    while (! mapfile.eof() ) {
      getline (mapfile, line);
      if (line.length() > maxwidth) maxwidth = line.length();
      rawmap.push_back(line);
    }
    mapfile.close();
  }
  else
    return -1;

  // Parsing: setup map, and objects from raw data.
  int yy, xx, syy;
  int hasplayer = 0;

  lvl.aimobject.coord.x = 1; // used to remember x
  lvl.aimobject.type = SHOT1;
  lvl.aimobject.tick = 0;
  lvl.aimobject.d.x = 1;
  lvl.aimobject.d.y = 0;

  vector<int> blankline(maxwidth + 2, NONSTICK); // one extra line to start
  lvl.map.push_back(blankline);
  syy = 1; // screen yy
  for (yy = 0; yy < (signed)rawmap.size(); ++yy) {
#ifndef __NOSOUND__
    if (rawmap[yy].find("music") == 0) {
      if (rawmap[yy].find("default") == 6) {
        if ((rawmap[yy][13] >= '1') && (rawmap[yy][13] <= '9'))
          lvl.musicid = rawmap[yy][13] - '1' + 1;
        else lvl.musicid = 0;
      }
      else
        lvl.musicfile = rawmap[yy].substr (6, (signed)rawmap[yy].size() - 6);
    } else
#endif
    if (rawmap[yy].find("message") == 0) {
      if ((rawmap[yy][7] >= '1') && (rawmap[yy][7] <= '9')) {
        lvl.texttrigger[rawmap[yy][7] - '1'] = rawmap[yy].substr (9, (signed)rawmap[yy].size() - 9);
      } else {
        lvl.pager.add_scrolling(rawmap[yy].substr (8, (signed)rawmap[yy].size() - 8));
#ifndef __NOSOUND__
        lvl.has_message = true;
#endif
      }
    } else if (rawmap[yy].find("name") == 0) {
      lvl.name = rawmap[yy].substr (5, (signed)rawmap[yy].size() - 5);
      debug("parsing: level name found: " + lvl.name);
      lvl.pager.set_levelname(lvl.name);
    } else {
      vector<int> mapline(maxwidth + 2, NONSTICK);

      for (xx = 0; xx < (signed)rawmap[yy].length(); xx++) { // process the line
        int check;
        if ((toupper(rawmap[yy][xx]) >= 'A') && (toupper(rawmap[yy][xx]) <= 'M')) {
          if (toupper(rawmap[yy][xx])== rawmap[yy][xx])
            check = SWITCH;
          else check = DOOR;
        } else if ((rawmap[yy][xx] >= '1') && (rawmap[yy][xx] <= '9')) {
          check = TEXTTRIGGER;
        } else if ((rawmap[yy][xx] == '<') || (rawmap[yy][xx] == '>')) {
          check = BOULDER;
        } else for (check = 0; (check < MAXObjects) && (rawmap[yy][xx] != CharData[check][0]); check++) ;

        if (check < MAXObjects) { // then we have a match
          if (check < MAXWall) {  // and it's a map object.
            mapline[xx + 1] = check;
          } else { // No, it's a dynamic object, not a map object.
            mapline[xx + 1] = NONE;
            object newobject;
            newobject.coord.x = xx + 1;
            newobject.coord.y = syy;
            newobject.type = check;
            newobject.tick = 0;
            newobject.d.x = 0;
            newobject.d.y = 0;

            if (check == SWITCH) {
              newobject.d.y = rawmap[yy][xx] - 'A';
              mapline[xx + 1] = NONSTICK;
            }
            if (check == DOOR) {
              newobject.d.y = rawmap[yy][xx] - 'a';
              newobject.d.x = 4;
            }
            if (check == TEXTTRIGGER) {
              newobject.d.y = rawmap[yy][xx] - '1';
            }
            if (check == BOULDER) {
              if (rawmap[yy][xx] == '<') newobject.d.x = -1;
              if (rawmap[yy][xx] == '>') newobject.d.x = 1;
            }
            if (newobject.type == PLAYER) hasplayer++;

            lvl.objm.objs.push_back(newobject);
          }
        }
      }
      lvl.map.push_back(mapline); // add the line to the current map.
      syy++;
    }
  }
  lvl.map.push_back(blankline);

  rawmap.clear();
  lvl.objm.resetmap(lvl.map[0].size(), lvl.map.size());

  // TODO: error handling

    if (lvl.objm.objs.size() == 0)
      debug("objm.objs.size() == 0 : this is supposed to be bad");
#if WAITING_FOR_REFACTORING
    mvprintw (LINES / 2 - 1, (COLS - 20) / 2, "Error in level %03d", lvl + 1);
    mvprintw (LINES / 2 , (COLS - 24) / 2,"Level contains no objects", lvl + 1);
    mvprintw (LINES / 2 + 1, (COLS - 11) / 2, "Press a key");
    refresh ();
    nodelay(stdscr,0);
    int input = getch();
    do {
#ifdef PDCURSES
      if (input == KEY_RESIZE) {
        resize_term(0,0);
        input = ERR;
      }
#endif
      input = getch ();
    } while (input == ERR);
    return 0;
    }

  if (hasplayer != 1) {
    mvprintw (LINES / 2 - 1, (COLS - 20) / 2, "Error in level %03d", lvl + 1);
    mvprintw(LINES / 2, (COLS - 50) / 2,"Level contains more or less than one player object");
    mvprintw (LINES / 2 + 1, (COLS - 11) / 2, "Press a key");
    refresh ();
    nodelay(stdscr,0);
    int input = getch();
    do {
#ifdef PDCURSES
      if (input == KEY_RESIZE) {
        resize_term(0,0);
        input = ERR;
      }
#endif
      input = getch ();
    } while (input == ERR);
    return 0;
    }
#endif
    return 0;
} // MapPack::load_map

void MapPack::update_stats() {
  //TODO
  return;
}

