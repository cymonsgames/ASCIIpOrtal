#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include "ap_maps.h"

extern const int CharData [MAXColors][5];

// Initialize 'properties' and the bundled file manager. Map loading
// will be done on-demand.
MapPack::MapPack(string const & _name) : filemgr(_name) {
  cerr << "Initializing MapPack with name " << _name << endl;
  // Not found
  if (filemgr.get_fullpath() == "") {
    name = "";
    return;
  }

  name = _name;

  // load properties from a file (infos.txt)
  /* TODO: use JSON or YAML */
  /*
  string filename = filemgr.get_infos(path);
  string line;
  bool ;

  ifstream infos(filename.c_str());
  if (infos.is_open()) {
    while (! infos.eof() ) {
      getline (infos, line);
      if (line[0] == '#') continue; // comment
      if (line.find("maps") == 0) {

      }
      if (line.find("name") == 0) {

      }
      if (line.find("version") == 0) {

      }
      if (line.find("author") == 0) {

      }
      if (line.find("difficulty") == 0) {

      }
      if (line.find("priority") == 0) {

      }
      if (line.find("rating") == 0) {

      }
      if (line.find("description") == 0) {

      }
    }
    infos.close();
  }
  */

  // temporary
  properties.number_maps = 666;
  stringstream stuff;
  stuff << "Example map pack " << rand() % 100;
  properties.name = stuff.str();
  properties.description = "This is a dumb map pack to show the new properties attached to a map pack.\nIt does nothing useful!";
  properties.author = "zorun";
  properties.version = "0.1";
  properties.difficulty = rand() % 11;
  properties.priority = 0;
  properties.rating = 0;
  if (name == "default") {
    properties.priority = 10;
    properties.name = "Default levels! These are quite easy.";
  }


#ifdef GODMODE
  maxlevel = properties.number_maps;
  cout << "Godmode activated! maxlevel set to " << maxlevel << endl;
#else
  // load maxlevel from a file, using filemgr
  // (0 if not found)
  maxlevel = filemgr.fetch_maxlevel();
#endif

  if (maxlevel == properties.number_maps)
    // last level
    set_currentlevel(maxlevel);
  else
    // set the current level to the next level
    set_currentlevel(maxlevel + 1);
}

void MapPack::set_maxlevel(int new_maxlvl) {
  if (new_maxlvl > maxlevel)
    // write it to a file, using filemgr
    filemgr.save_maxlevel(new_maxlvl);
  maxlevel = new_maxlvl;
}

int MapPack::set_currentlevel(int newlvl) {
  if (newlvl <= 0 || newlvl > properties.number_maps + 1)
    return -1;

  lvl.id = newlvl;

  if (lvl.id == properties.number_maps + 1)
    return -1; // the map pack is over, we don't load any map

  load_map();
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
        //if ((rawmap[yy][13] >= '1') && (rawmap[yy][13] <= '9'))
        // default_ambience(rawmap[yy][13] - '1' + 1);
        //else default_ambience(0);
      }
      else {
        string musicfile = rawmap[yy].substr (6, (signed)rawmap[yy].size() - 6);
        //TODO: move this somewhere else
        //load_ambience(mappack, musicfile);
        musicfile.clear();
      }
      //TODO: 
      //start_ambience();
    } else
#endif
    if (rawmap[yy].find("message") == 0) {
      if ((rawmap[yy][7] >= '1') && (rawmap[yy][7] <= '9')) {
        lvl.texttrigger[rawmap[yy][7] - '1'] = rawmap[yy].substr (9, (signed)rawmap[yy].size() - 9);
      } else {
        lvl.pager.add_scrolling(rawmap[yy].substr (8, (signed)rawmap[yy].size() - 8));
#ifndef __NOSOUND__
        //TODO:        play_sound(VOICE + rand() % 10);
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

  /*  if (objm.objs.size() == 0) {
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
    }*/

  /*  if (hasplayer != 1) {
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
    }*/

} // MapPack::load_map

void MapPack::update_stats() {
  //TODO
  return;
}

