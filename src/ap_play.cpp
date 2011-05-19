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

#include <curses.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <cmath>
using namespace std;
#include "asciiportal.h"
#ifndef __NOSDL__
#include "SDL/SDL.h"
#endif
#ifndef __NOSOUND__
#include "ap_sound.h"
#endif
#include "ap_play.h"
#include "ap_draw.h"
#include "ap_input.h"
#include "ap_filemgr.h"
#include "menu.h"
#include "ap_object.h"

const int beatspermove = 2;
const int boulderbeats = 2;
const int beatsperdupe = 15;
const int portalspeed = 5;
const int maxfling = 20;

int beatspersecond[6] = {0, 4, 7, 10, 15, 20};
const int defaultspeed = 3;
const int maxspeed = 6;
int gamespeed = defaultspeed;

// from main.cpp
extern vector<vector<string> > rawmaps;
extern vector<int> rawmaps_maxwidth;
extern int animateportal;
extern FileManager filemgr;

// from ap_draw.cpp
extern const int CharData[MAXObjects][5];
extern int cheatview;
extern Pager game_pager;

#include <sys/time.h>

vector<vector<int> > map;
ObjectManager objm;
object aimobject;
string lvlname, texttrigger[9];
int ticks = 0;
int level, maxlevel;
statstype levelstats;

int setup_level (int lvl, string mappack) { // setup map, and objects from raw data.
  int yy, xx, syy;
  int hasplayer = 0;

  for (yy = 0; yy < (signed)map.size(); yy++)  // clear the old.
    map[yy].clear();
  for (yy = 0; yy < 9; yy++)
    texttrigger[yy].clear();
  map.clear();
  objm.objs.clear();
  lvlname.clear();
  game_pager.clear();

  if (level > (signed)rawmaps.size()) {
    mvprintw (LINES / 2 - 1, (COLS - 20) / 2, "Error in level %03d", lvl + 1);
    mvprintw (LINES / 2 , (COLS - 40) / 2,"Mappack does not contain that many maps.");
    mvprintw (LINES / 2 + 1, (COLS - 11) / 2, "Press a key");
    refresh ();
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

  aimobject.coord.x = 1; // used to remember x
  aimobject.type = SHOT1;
  aimobject.tick = 0;
  aimobject.d.x = 1;
  aimobject.d.y = 0;

  vector<int> blankline(rawmaps_maxwidth[lvl] + 2, NONSTICK); // one extra line to start
  map.push_back(blankline);
  syy = 1; // screen yy
  for (yy = 0; yy < (signed)rawmaps[lvl].size(); yy++) {
    if (rawmaps[lvl][yy].find("music") == 0) {
#ifndef __NOSOUND__
      if (rawmaps[lvl][yy].find("default") == 6) {
        if ((rawmaps[lvl][yy][13] >= '1') && (rawmaps[lvl][yy][13] <= '9'))
          default_ambience(rawmaps[lvl][yy][13] - '1' + 1);
        else default_ambience(0);
      }
      else {
        string musicfile = rawmaps[lvl][yy].substr (6, (signed)rawmaps[lvl][yy].size() - 6);
        load_ambience(mappack, musicfile);
      }
      start_ambience();
#endif
    } else
    if (rawmaps[lvl][yy].find("message") == 0) {
      if ((rawmaps[lvl][yy][7] >= '1') && (rawmaps[lvl][yy][7] <= '9')) {
        texttrigger[rawmaps[lvl][yy][7] - '1'] = rawmaps[lvl][yy].substr (9, (signed)rawmaps[lvl][yy].size() - 9);
      } else {
        game_pager.add_scrolling(rawmaps[lvl][yy].substr (8, (signed)rawmaps[lvl][yy].size() - 8));
#ifndef __NOSOUND__
        play_sound(VOICE + rand() % 10);
#endif
      }
    } else if (rawmaps[lvl][yy].find("name") == 0) {
      lvlname = rawmaps[lvl][yy].substr (5, (signed)rawmaps[lvl][yy].size() - 5);
      debug("parsing: level name found: " + lvlname);
      game_pager.set_levelname(lvlname);
    } else {
      vector<int> mapline(rawmaps_maxwidth[lvl] + 2, NONSTICK);

      for (xx = 0; xx < (signed)rawmaps[lvl][yy].length(); xx++) { // process the line
        int check;
        if ((toupper(rawmaps[lvl][yy][xx]) >= 'A') && (toupper(rawmaps[lvl][yy][xx]) <= 'M')) {
          if (toupper(rawmaps[lvl][yy][xx])== rawmaps[lvl][yy][xx])
            check = SWITCH;
          else check = DOOR;
        } else if ((rawmaps[lvl][yy][xx] >= '1') && (rawmaps[lvl][yy][xx] <= '9')) {
          check = TEXTTRIGGER;
        } else if ((rawmaps[lvl][yy][xx] == '<') || (rawmaps[lvl][yy][xx] == '>')) {
          check = BOULDER;
        } else for (check = 0; (check < MAXObjects) && (rawmaps[lvl][yy][xx] != CharData[check][0]); check++) ;

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
              newobject.d.y = rawmaps[lvl][yy][xx] - 'A';
              mapline[xx + 1] = NONSTICK;
            }
            if (check == DOOR) {
              newobject.d.y = rawmaps[lvl][yy][xx] - 'a';
              newobject.d.x = 4;
            }
            if (check == TEXTTRIGGER) {
              newobject.d.y = rawmaps[lvl][yy][xx] - '1';
            }
            if (check == BOULDER) {
              if (rawmaps[lvl][yy][xx] == '<') newobject.d.x = -1;
              if (rawmaps[lvl][yy][xx] == '>') newobject.d.x = 1;
            }
            if (newobject.type == PLAYER) hasplayer++;

            objm.objs.push_back(newobject);
          }
        }
      }
      map.push_back(mapline); // add the line to the current map.
      syy++;
    }
  }
  map.push_back(blankline);

  debug("Parsing map '" + mappack + "' done.");

  objm.resetmap(map[0].size(), map.size());

  if (objm.objs.size() == 0) {
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
  return 1;
}

// handles in-game menu
int pause_menu() {
  int pause = display_pause_menu();
  switch (pause) {
  case 0 : case -1 : break; // Resume
  case 1 : // Restart
    objm.player->type = NONE;
    objm.player = objm.NULLOBJ;
    break;
  case 2 : { // Select level
    int newlvl = select_level (maxlevel, level);
    if (newlvl == -1) // Back to the menu (ESC key or something similar)
      pause_menu();
    else if (newlvl != level) { // Change level
      level = newlvl;
      objm.player->type = NONE;
      objm.player = objm.NULLOBJ;
      ticks = 0;
      levelstats.clear();
      levelstats.numdeaths --; // Correct since death is the tool for this one.
    }
    break;
  }
  case 3 : help_menu (); break; // Help
  default : return -1; break; // Quit
  }
}

int hitswall(int yy, int xx) {
  if ((yy < 0) || (yy >= map.size()) || (xx < 0) || (xx >= map[0].size()))
    return NONSTICK;
  switch (map[yy][xx]) {
    case NONE:
    case GOAL:
    case LADDER:
    case PFIELD:
    case XFIELD:
      return 0; // Everyone and everything passes through these
  }
  return map[yy][xx];
}

objiter hitsobj(objiter it0, int yy, int xx) { // returns iterator to hit object, else it0
  objiter val = it0;

  if (xx < 0 || xx >= objm.objmap[yy].size() || yy < 0 || yy >= objm.objmap.size())
    return val;

  objset *s = &(objm.objmap[yy][xx]);
  for (objset::iterator it = s->begin(); it != s->end(); it++) {
    if (*it != it0) {
      switch ((*it)->type) {
        case SHOT1:
        case SHOT2:
        case FLASH:
        case NONE:
          break;
        case TEXTTRIGGER:
          if (val == it0) val = *it; // Texttriggers get lowest priority.
          break;
        case DOOR:
          if ((*it)->d.x > 0) val = *it;
          break;
        case PORTAL1:
        case PORTAL2: // Portals get highest priority
          return *it;
          break;
        default:
          val = *it;
          break;
      }
    }
  }
  return val;
}

int still_alive () {
  if (objm.player == objm.NULLOBJ || objm.player->coord.y > map.size()) return 0;

  int underplayer = map[objm.player->coord.y][objm.player->coord.x];
  if (underplayer == FFIELD || underplayer == SPIKE) return 0;

  int hitobject = hitsobj(objm.player, objm.player->coord.y, objm.player->coord.x)->type;
  if (hitobject == BOULDER) return 0;

  return 1;
}

void fireportal (int por) {
#ifndef __NOSOUND__
  play_sound(GUNSHOT);
#endif
  if (objm.shots[por - SHOT1] != objm.NULLOBJ) {
    objm.shots[por - SHOT1]->type = NONE;
    objm.shots[por - SHOT1] = objm.NULLOBJ;
  }
  object newobject;
  newobject.coord.x = objm.player->coord.x + aimobject.d.x;
  newobject.coord.y = objm.player->coord.y + aimobject.d.y;
  newobject.type = por;
  newobject.tick = 0;
  newobject.d.x = aimobject.d.x;
  newobject.d.y = aimobject.d.y;
  objm.addobj(newobject);
}

objiter in_portal () { // Technically both portals are the same space.
  objiter por1 = objm.portals[0];
  objiter por2 = objm.portals[1];
  if (por1 != objm.NULLOBJ && por2 != objm.NULLOBJ) {
    objiter smacked = hitsobj(por1, por1->coord.y, por1->coord.x);
    if (por1 != smacked)
      return smacked;
    smacked = hitsobj(por2, por2->coord.y, por2->coord.x);
    if (por2 != smacked)
      return smacked;
  }
  return objm.NULLOBJ;
}

int switch_in_portal() { // find what's in the portal and switch it to the other portal
  objiter c_obj = in_portal();
  objiter por1 = objm.portals[0];
  objiter por2 = objm.portals[1];

  if (c_obj != objm.NULLOBJ) {
    if ((c_obj->coord.x == por2->coord.x) && (c_obj->coord.y == por2->coord.y)) {
      objiter t = por2;
      por2 = por1;
      por1 = t;
    } // por1 is from, por 2 is to.

    if (por1->d.x == 0) c_obj->d.x = 0;
    if (por1->d.y == 0) c_obj->d.y = 0; // only allow one direction vector through.

    objm.moveobj(c_obj, por2->coord.x, por2->coord.y);

    XY temp, control; // rotate d vector
    int rotation = 0;
    control.x = -por1->d.x; control.y = -por1->d.y;
    while (!((control.x == por2->d.x) && (control.y == por2->d.y))) {
      temp.x = control.x; temp.y = control.y;
      control.x = temp.y; control.y = -temp.x;
      temp.x = c_obj->d.x; temp.y = c_obj->d.y;
      c_obj->d.x = temp.y; c_obj->d.y = -temp.x;
      rotation++;
    }

    if (c_obj->type == PLAYER) {
      if (aimobject.d.x == -por2->d.x) { // try to avoid flipflop scenarios due to aiming.
        if (aimobject.d.y != 0) aimobject.d.x = 0;
        else aimobject.d.x = -aimobject.d.x;
      }
      if (aimobject.d.y == -por2->d.y) {
        if (aimobject.d.x != 0) aimobject.d.y = 0;
        else aimobject.d.y = -aimobject.d.y;
      }
    }

    if (c_obj->d.x == 2 * sc(c_obj->d.x))
      c_obj->d.x = sc(c_obj->d.x); // slow the slide for small drops
    if (c_obj->d.y == -1) c_obj->d.y = -2; // help out of holes.
    if ((c_obj->type == PLAYER) && (por2->d.y == -1) && (c_obj->d.y == 0) && (c_obj->d.x == 0)) c_obj->d.y = -2;
    if (c_obj->d.y < -2) c_obj->d.y++;

    return rotation;
  }
  return -1;
}

int sc (int x) {
  if (x < 0) return -1;
  if (x > 0) return 1;
  return 0;
}

int por_col (int yy, int xx) { // Collision speciffic to portal shots
  int col = map[yy][xx];
  if ((yy < 0) || (yy >= (signed)map.size())
  || (xx < 0) || (xx >= (signed)map[0].size()))
  col = NONSTICK;
  else switch (map[yy][xx]) {
    case NONE:
    case GOAL:
    case LADDER:
    case FFIELD:
    case XFIELD:
    case SPIKE:
      col = 0;
      break;
    case PFIELD:
      col = NONSTICK;
  }
  objiter colobj = hitsobj(objm.NULLOBJ, yy, xx);
  if (colobj != objm.NULLOBJ)
  switch (colobj->type) {
    case DOOR : if (colobj->d.x > 0) col = NONSTICK; break;
    case DUPLICATOR :
    case PORTAL1:
    case PORTAL2:
      col = NONSTICK;
  }
  return col;
}

int will_hit (objiter c) {
  objiter c_obj; // = hitsobj(c, c->coord.y + d.y, c->coord.x + d.x);
  int c_wall; // = hitswall(c->coord.y + d.y, c->coord.x + d.x);
  XY d;

  if ((c->type == BOULDER) && ((c->d.y > 0) || (ticks % boulderbeats)))
    d.x = 0;
  else
    d.x = sc(c->d.x);
  d.y = sc(c->d.y);

  c_obj = hitsobj(c, c->coord.y + d.y, c->coord.x + d.x);
  c_wall = hitswall(c->coord.y + d.y, c->coord.x + d.x);

  if ((c_obj != c) && (c_obj->type != NONE)) { // if hit object
    int objtype = c_obj->type;

    if (objtype == TEXTTRIGGER) {
      if (c->type == PLAYER) {
        if (texttrigger[c_obj->d.y].size()) {
          game_pager.add_scrolling(texttrigger[c_obj->d.y]);
#ifndef __NOSOUND__
          play_sound(VOICE + rand() % 10);
#endif
        }
        objm.killtriggers(c_obj->d.y);
      }
      if ((c->type == SWITCH) || (c->type == SWITCHON))
        return objtype;
      return NONE;
    }
    if ((objtype == PORTAL1) || (objtype == PORTAL2)) { // if hit portal
      if (((c_obj->d.x != 0) && (c_obj->d.x != d.x))
          || ((c_obj->d.y != 0) && (c_obj->d.y != d.y))) { // only allow in if portal is right direction
        objiter other_por = objm.portals[(objtype == PORTAL1) ? 1 : 0]; // get the other portal
        if (other_por != objm.NULLOBJ) { // Yes 2 portals present
          objiter inp = in_portal();
          if (inp != objm.objs.end() && c != inp) { // if something's in the other portal
            c_obj = inp;
            objtype = c_obj->type;
          }
          else objtype = NONE;
        }
      }
      else objtype = c_wall;
    }

    if (((c->type == PLAYER) || (c->type == BOULDER)) && (objtype == BOX)) {
      if (!c->d.y) {
        c_obj->d.x += d.x;
        if ((move_object (c_obj)) && (c->type != BOULDER)) {
          objtype = NONE;
        }
      }
    }
    if ((c->type == PLAYER) && (objtype == BOULDER)) { // if player hit boulder
      if (d.y <= 0) {
#ifndef __NOSOUND__
        play_sound (CRUSH);
#endif
#ifndef __NOSOUND__
        play_sound (CRUSH);
#endif
        objm.player = objm.NULLOBJ;
        c->type = NONE; //   kill player
        return NONE;
      } else d.y = 0;
    } else if ((c->type == BOULDER) && (objtype == PLAYER)) { // if boulder hit player
#ifndef __NOSOUND__
      play_sound (CRUSH);
#endif
      objm.player = objm.NULLOBJ;
      c_obj->type = NONE;
      return NONE;
    }
    return objtype;
  }
  if ((c->type == PLAYER) && (c_wall == SPIKE)) {
    objm.player = objm.NULLOBJ;
    c->type = NONE; //   kill player
#ifndef __NOSOUND__
    play_sound (CRUSH);
#endif
    return NONE;
  }
  return c_wall;
}

int applyd (objiter c) {
  int dx, dy;

  if ((c->type == BOULDER) && ((c->d.y != 0) || (ticks % boulderbeats))) {
    dx = 0;
  } else dx = sc(c->d.x);
  dy = sc(c->d.y);
  objm.moveobj(c, c->coord.x + dx, c->coord.y + dy);
  if (c->type != BOULDER) {
    if (c->d.x != 0) c->d.x -= dx;
  } else if ((c->d.x < -1) || (c->d.x > 1)) c->d.x -= dx;
  if (c->d.y < 0) c->d.y++;

  if (dx || dy) return 1;
  return 0;
}

void collapse_portals () {
  int playsound = 0;

  objiter in_por = in_portal();
  if (in_por != objm.NULLOBJ) {
    objiter por1 = objm.portals[0];
    objiter por2 = objm.portals[1];
    if (in_por == hitsobj(por2, por2->coord.y, por2->coord.x)) { // FIXME: redundant, need to rework in_portal()
      por1 = objm.portals[1];
      por2 = objm.portals[0];
    }
    in_por->d.x += por1->d.x; // push anything stuck in portal out.
    in_por->d.y += por1->d.y; // before collapsing it.
    in_por->tick--; // make sure a move is possible.
    if (!move_object(in_por)) {
      switch_in_portal();
      in_por->d.x += por2->d.x;
      in_por->d.y += por2->d.y;
      in_por->tick--;
      if (!move_object(in_por)) {
        if (in_por->type == PLAYER)
          objm.player = objm.NULLOBJ; // a unique way to die
        in_por->type = FLASH;
      }
    }
  }

  // cancel all portals
  for (int i = 0; i < 2; ++i) {
    if (objm.portals[i] != objm.NULLOBJ) {
      objm.portals[i]->d.y = CharData[PORTAL1 + i][2];
      objm.portals[i]->type = FLASH;
      objm.portals[i] = objm.NULLOBJ;
      playsound = 1;
    }
    if (objm.shots[i] != objm.NULLOBJ) {
      objm.shots[i]->d.y = CharData[SHOT1 + i][2];
      objm.shots[i]->type = FLASH;
      objm.shots[i] = objm.NULLOBJ;
      playsound = 1;
    }
  }

#ifndef __NOSOUND__
  if (playsound) play_sound (PORTALCOLLAPSE);
#endif
}

int move_object (objiter c) { // collision detection and movement.
  objiter c_obj;

  if (c->tick >= ticks) return 0;
  c->tick = ticks;
  if (c->type == BOX) c->tick--;

  c_obj = hitsobj(c, c->coord.y, c->coord.x);
  if ((c_obj->type == PORTAL1) || (c_obj->type == PORTAL2)) { // if obj[c] is in portal
    int needswitch = 0;
    if ((c->d.x) && (sc(c->d.x) == -c_obj->d.x)) needswitch = 1;
    if ((c->d.y) && (sc(c->d.y) == -c_obj->d.y)) needswitch = 1;
    if (c->type == PLAYER) {
      if ((aimobject.d.x) && (sc(aimobject.d.x) == -c_obj->d.x)) needswitch = 1;
      if ((aimobject.d.y) && (sc(aimobject.d.y) == -c_obj->d.y)) needswitch = 1;
    }

    if (needswitch) { // if facing the right way
      int rotations = switch_in_portal();
      c_obj = hitsobj(c, c->coord.y, c->coord.x);
      if (c->type == PLAYER) {
#ifndef __NOSOUND__
        play_sound(THROUGH);
#endif
        draw_rotate(rotations);
      }
    }
  } // And that's how portals should work.

  XY oldd;

  oldd.x = c->d.x; oldd.y = c->d.y;
  if (will_hit(c) == NONE) return applyd(c);
  c->d.y = 0;
  if (will_hit(c) == NONE) return applyd(c); // try without dy
  if ((c->type == PLAYER) && (c->d.x)) {
    c->d.y = -1; // step up;

    objiter porcheck = hitsobj(c, c->coord.y, c->coord.x);
    if ((porcheck->type == PORTAL1) || (porcheck->type == PORTAL2)) {
      if (porcheck->d.y == 1) {
        int rotations = switch_in_portal();
        c_obj = hitsobj(c, c->coord.y, c->coord.x);
#ifndef __NOSOUND__
        play_sound(THROUGH);
#endif
        draw_rotate(rotations);
      }
    }
    if (will_hit(c) == NONE) return applyd(c);
    c->d.y = 0;
  }
  c->d.y = oldd.y;
  if (c->type == BOULDER) {
    c->d.x = -sc(oldd.x);
  } else c->d.x = 0;
  if (will_hit(c) == NONE) return applyd(c); // try without dx
  c->d.y = 0;
  return 0; // didn't move
}

int physics () {
  int por = 0;

  for (objiter c = objm.objs.begin(); c != objm.objs.end(); c++) {
    por = 0;
    switch (c->type) { // Every tick movements (like gravity)
      case NONE: break;
      case DUPLICATOR:
        if (c->d.y > 0) c->d.y--;
        else {
          objiter dupe = hitsobj(c, c->coord.y - 1, c->coord.x);
          if ((c != dupe) && (dupe->type == BOX || dupe->type == BOULDER)
            && (c == hitsobj(c, c->coord.y + 1, c->coord.x))) {
            object newobject;
            newobject.coord.x = c->coord.x;
            newobject.coord.y = c->coord.y;
            newobject.type = dupe->type;
            newobject.tick = dupe->tick;
            newobject.d.x = dupe->d.x;
            newobject.d.y = 0;
            objm.addobj(newobject);
            c->d.y = beatsperdupe;
#ifndef __NOSOUND__
            if (newobject.type == BOX) play_sound (DUPLICATE);
#endif
          }
        }
        continue;
        break;
      case SWITCH:
      case SWITCHON: {
        int playsound = 0;
        objiter colobj = hitsobj(c, c->coord.y - 1, c->coord.x);
        if (c != colobj) {
          if (c->type == SWITCH) {
            c->type = SWITCHON;
            if ((colobj->type == PLAYER) || (colobj->type == BOX)) playsound = 2;
          }
          int otheropen = 0;
          for (int i = 0; i < objm.switches[c->d.y].size(); i++)
            if (objm.switches[c->d.y][i]->type == SWITCH)
              otheropen = 1;
          if (!otheropen) // open the doors
            for (int i = 0; i < objm.doors[c->d.y].size(); i++) {
              objiter d = objm.doors[c->d.y][i];
              if (((colobj->type == PLAYER) || (colobj->type == BOX)) && (d->d.x == 4)) playsound = 1;
              if (--d->d.x < 0) d->d.x = 0;
            }
#ifndef __NOSOUND__
          if (playsound == 1) play_sound(DOOROPEN);
          if (playsound == 2) play_sound(SWITCHHIT);
#endif
        } else {
          c->type = SWITCH;
          for (int i = 0; i < objm.doors[c->d.y].size(); i++) {
            objiter d = objm.doors[c->d.y][i];
            if (c->d.y >= 6) { // if it's a momentary switch
              if (d == hitsobj(d, d->coord.y, d->coord.x)) { // and there's nothing in the door
                if (d->d.x == 0) playsound = 1;
                if (++d->d.x > 4) d->d.x = 4; // close the door
              }
            } else if (d->d.x < 4) // else finish opening the door.
              if (--d->d.x < 0) d->d.x = 0;
          }
#ifndef __NOSOUND__
          if (playsound == 1) play_sound(DOORCLOSE);
#endif
        }
      }
      case TEXTTRIGGER:
      case PORTAL1:
      case PORTAL2:
      case FLASH:
      case DOOR:
      case DOOR1:
      case DOOR2:
      case DOOR3:
        continue;
        break; // Unnecessary because of the continue, but it looks nice.
      case SHOT2: // gravityless
        por = 1;
      case SHOT1: // gravityless
        for (int z = 0; z < portalspeed; z++) {
          int col = por_col(c->coord.y, c->coord.x);
          if (col) {
            if (col == NONSTICK) {
              c->d.y = CharData[c->type][2];
              objm.shots[c->type - SHOT1] = objm.NULLOBJ;
              c->type = FLASH;
#ifndef __NOSOUND__
              play_sound (PORTALFAIL);
#endif
              z = portalspeed; continue;
            }
            // Calculate portal normal.
            int east = por_col(c->coord.y, c->coord.x + 1);
            if (hitsobj(c,c->coord.y, c->coord.x + 1)->type == DOOR) east = NONE;

            int west = por_col(c->coord.y, c->coord.x - 1);
            if (hitsobj(c,c->coord.y, c->coord.x - 1)->type == DOOR) west = NONE;

            int north= por_col(c->coord.y - 1, c->coord.x);
            if (hitsobj(c,c->coord.y - 1, c->coord.x)->type == DOOR) north = NONE;

            int south= por_col(c->coord.y + 1, c->coord.x);
            if (hitsobj(c,c->coord.y + 1, c->coord.x)->type == DOOR) south = NONE;

            int yback= por_col(c->coord.y - c->d.y, c->coord.x);
            if (map[c->coord.y - c->d.y][c->coord.x] == FFIELD) yback = NONSTICK;

            int xback= por_col(c->coord.y, c->coord.x - c->d.x);
            if (map[c->coord.y][c->coord.x - c->d.x] == FFIELD) xback = NONSTICK;

            if (east && west && yback == NONE) {
              c->d.x = 0; c->d.y = -c->d.y;
            } else if (south && north && xback == NONE) {
              c->d.y = 0; c->d.x = -c->d.x;
            } else {
#ifndef __NOSOUND__
              play_sound (PORTALFAIL);
#endif
              c->d.y = CharData[c->type][2];
              objm.shots[c->type - SHOT1] = objm.NULLOBJ;
              c->type = FLASH;
              z = portalspeed; continue;
            }
            if (objm.portals[por] != objm.NULLOBJ) {
              objiter d = objm.portals[por];
              objiter in_por = hitsobj(d, d->coord.y, d->coord.x);
              if ((in_por != d) && (in_por->type != PORTAL1) && (in_por->type != PORTAL2)){
                in_por->d.x += d->d.x; // push anything stuck in portal out.
                in_por->d.y += d->d.y; // before collapsing it. FIXME: similar code elsewhere-- remove the redundancy
                if (!move_object(in_por)) {
                  switch_in_portal();
                }
              }
              d->d.y = CharData[d->type][2];
              d->type = FLASH;
            }
            levelstats.numportals++;
            animateportal = 2;
#ifndef __NOSOUND__
            play_sound (PORTALCREATE);
#endif
            objm.shots[c->type - SHOT1] = objm.NULLOBJ;
            objm.portals[por] = c;
            c->type = PORTAL1 + por;
            aimobject.type = SHOT1 + ((por + 1) % 2);
            z = portalspeed;
          } else {
            objm.moveobj(c, c->coord.x + c->d.x, c->coord.y + c->d.y);
          }
        }
        continue;
      case PLAYER: // gravity
        if (move_player() < 0) return -1;
        if (map[c->coord.y + 1][c->coord.x + c->d.x] == LADDER) break;
        if (map[c->coord.y + 1][c->coord.x] == LADDER) break;
        if (map[c->coord.y][c->coord.x] == LADDER) break;
        if (map[c->coord.y + sc(c->d.y)][c->coord.x + sc(c->d.x)] == LADDER) break;
      case BOULDER: {
        if ((map[c->coord.y + 1][c->coord.x] == LADDER) && (rand() < RAND_MAX / 2)) break;
      }
      case BOX:
        if ((c->d.x > 2) || (c->d.x < -2) || (c->d.y < -2)) break; // fling!
        if (++(c->d.y) > maxfling) c->d.y = maxfling;
      default:
        break;
    } // End every tick movements.

    if (!(ticks % beatspermove)) { // X-axis automovements
      switch (c->type) {
        case BOULDER:
          if (c->d.x == 0) c->d.x = (rand() < RAND_MAX / 2) ? 1 : -1;
          break;
        case PLAYER:
        case BOX:
          switch (map[c->coord.y + 1][c->coord.x]) {
            case LTREAD:
        if (c->d.x == 0) c->d.x --;
        break;
            case RTREAD:
        if (c->d.x == 0) c->d.x ++;
        break;
          }
          break;
      }
    }
    move_object(c);

    switch (c->type) { // kill objects / clean up
      case PLAYER:
        if (map[c->coord.y][c->coord.x] == LADDER) {
          c->d.y = 0;
          c->d.x = 0;
        }
      case BOULDER:
      case BOX: {
        switch (map[c->coord.y][c->coord.x]) {
          case PFIELD: {
            if (c->type == PLAYER) {
              collapse_portals();
            } else {
#ifndef __NOSOUND__
              play_sound (SIZZLE);
#endif
              c->d.y = COLOR_WHITE;
              c->type = FLASH;
            }
            break;
          }
          case XFIELD:
#ifndef __NOSOUND__
            play_sound (SIZZLE);
#endif
            c->d.y = COLOR_WHITE;
            if (c->type == PLAYER)
              objm.player = objm.NULLOBJ;
            c->type = FLASH;
        }
      }
    }
  }
  return 0;
}

int move_player () {
  int input;
  XY d;

  d.x = d.y = 0;
  input = getinput();
  if (input == ERR) {
    return 1;
  }
  switch (input) {
    case '1' : aimobject.d.y =  1; aimobject.d.x = -1; break;
    case '2' : aimobject.d.y =  1; aimobject.d.x =  0; break;
    case '3' : aimobject.d.y =  1; aimobject.d.x =  1; break;
    case '4' : aimobject.d.y =  0; aimobject.d.x = -1; break;
    case '6' : aimobject.d.y =  0; aimobject.d.x =  1; break;
    case '7' : aimobject.d.y = -1; aimobject.d.x = -1; break;
    case '8' : aimobject.d.y = -1; aimobject.d.x =  0; break;
    case '9' : aimobject.d.y = -1; aimobject.d.x =  1; break;
    case '0' :
    case 'Z' :
    case 'z' : aimobject.type = SHOT1; fireportal(aimobject.type); break;
    case '.' :
    case 'X' :
    case 'x' : aimobject.type = SHOT2; fireportal(aimobject.type); break;
    case 'C' :
    case 'c' : collapse_portals (); break;
    case '5' :
    case '\n':
    case ' ' :
      fireportal(aimobject.type);
      break;
    case '?' : help_menu (); break;
    case 27 : // ASCII for escape
    case KEY_F(1) :
    case 'P' :
    case 'p' : if (pause_menu () == -1) return -1; break;
    case KEY_F(2) :
      cheatview = (cheatview + 1) % 3;
      switch (cheatview) {
        case 0: game_pager.set_status("No Portal Edges view mode."); break;
        case 1: game_pager.set_status("Portal Fill view mode"); break;
        case 2: game_pager.set_status("Portal Edges view mode."); break;
      }
      break;
    case KEY_F(3) :
      game_pager.set_status("Default speed restored");
      gamespeed = defaultspeed;
      break;
    case '+' :
    case ']' :
      gamespeed += 2; // a little cheating with the case statements;
    case '-' :
    case '[' :
      gamespeed --;
      if (gamespeed >= maxspeed) gamespeed = maxspeed - 1;
      else if (gamespeed < 1) gamespeed = 1;
      else switch (gamespeed) {
        case 1 : game_pager.set_status("Very slow speed"); break;
        case 2 : game_pager.set_status("Slow speed"); break;
        case 3 : game_pager.set_status("Normal speed"); break;
        case 4 : game_pager.set_status("Fast speed"); break;
        case 5 : game_pager.set_status("Very fast speed"); break;
      }
      break;
    case KEY_F(10) :
      game_pager.set_status("Roguelike-ish speed mode");
      gamespeed = 0;
      break;
#ifndef __NOSOUND__
    case 'M' :
    case 'm' : game_pager.set_status("Toggling music");
    toggle_ambience (); break;
#endif
    case 'R' :
    case 'r' :
      objm.player->type = NONE;
      objm.player = objm.NULLOBJ;
      break;
#ifdef PDCURSES
    case KEY_RESIZE : resize_term(0,0); break;
#endif
    case 'd' :
    case 'D' :
    case KEY_RIGHT :
      if (aimobject.d.x == 1) {
        if (objm.player->d.x <= 0) objm.player->d.x ++;
        levelstats.numsteps++;
      } else {
        aimobject.coord.x = aimobject.d.x = 1;
      }
      aimobject.d.y = 0;
      break;
    case 'a' :
    case 'A' :
    case KEY_LEFT :
      if (aimobject.d.x == -1) {
        if (objm.player->d.x >= 0) objm.player->d.x --;
        levelstats.numsteps++;
      } else {
        aimobject.coord.x = aimobject.d.x = -1;
      }
      aimobject.d.y = 0;
      break;
    case 'w' :
    case 'W' :
    case KEY_UP :
      if ((map[objm.player->coord.y][objm.player->coord.x] == LADDER)
      && ((!hitswall(objm.player->coord.y - 1, objm.player->coord.x) || (hitsobj(objm.player, objm.player->coord.y - 1, objm.player->coord.x)->type == PORTAL1) || (hitsobj(objm.player, objm.player->coord.y - 1, objm.player->coord.x)->type == PORTAL2))))
      {
        if (objm.player->d.y >= 0) objm.player->d.y --;
        levelstats.numsteps++;
      } else if ((aimobject.d.x == 0) && (aimobject.d.y > 0)) {
        aimobject.d.x = aimobject.coord.x;
      } else {
        aimobject.d.y--;
        if (aimobject.d.y < -1) {
          aimobject.d.y = -1;
          aimobject.d.x = 0;
        }
      }
      break;
    case 's' :
    case 'S' :
    case KEY_DOWN :
      if (((map[objm.player->coord.y][objm.player->coord.x] == LADDER)
      || (map[objm.player->coord.y + 1][objm.player->coord.x] == LADDER))
      && ((!hitswall(objm.player->coord.y + 1, objm.player->coord.x) || (hitsobj(objm.player, objm.player->coord.y + 1, objm.player->coord.x)->type == PORTAL1) || (hitsobj(objm.player, objm.player->coord.y + 1, objm.player->coord.x)->type == PORTAL2))))
      {
        objm.player->d.y ++;
        levelstats.numsteps++;
      } else if ((aimobject.d.x == 0) && (aimobject.d.y < 0)) {
        aimobject.d.x = aimobject.coord.x;
      } else {
        aimobject.d.y++;
        if (aimobject.d.y > 1) {
          aimobject.d.y = 1;
          aimobject.d.x = 0;
        }
      }
      break;
  }
//  flushinp ();
  return 1;
}

int play (string mappack) {
  unsigned long long int start, stop;
  double seconds;

  level = 0;

  int playing = 0;
  do {
    switch (main_menu (mappack)) {
      case 0: // Play
        playing = 1; break;
      case 1: // Select Level
        level = select_level (maxlevel, maxlevel);
        if (level != -1)
          playing = 1;
        else
          level = 0;
        break;
      case 2: { // Change Map Set
        string newmappack = select_mappack ();
        if (newmappack.size() != 0) {
          attrset(color_pair(HELPMENU));
          fillsquare(LINES / 2 - 3, (COLS - 26) / 2, 7, 26);
          if (!loadmaps (newmappack)) {
            mvprintw (LINES / 2, (COLS - 16) / 2, "Invalid Map Pack"); refresh ();
            restms (150);
            getch();
            loadmaps (mappack);
          } else {
            mvprintw (LINES / 2, (COLS - 15) / 2, "Map Pack Loaded!"); refresh ();
            restms (150);
            getch();
            mappack = newmappack;
          }
        }
        break;
      }
      case 3: // Instructions
        help_menu (); break;
      case 4: // Credits
        roll_credits (mappack); break;
      default: // QUIT
        return -1;
    }
  } while (!playing);
#ifndef __NOSOUND__
  default_ambience (0);
  start_ambience ();
#endif
  while ((level < (signed)rawmaps.size()) && !setup_level(level, mappack)) level ++;
  ticks = 0;
  levelstats.clear();
  while (level < (signed)rawmaps.size()) {
    for (objiter c = objm.objs.begin(); c != objm.objs.end(); ) { // clean up NONEs.
      if (c->type == FLASH) {
        c->type = NONE;
        c++;
      }
      else if (c->type == NONE) c = objm.delobj(c);
      else c++;
    }

    start = get_microseconds();
/*
    if (!objm.verify()) {
      cout << "object manager state is inconsistent!\n";
      return 0;
    }
*/
    if (physics () < 0) return 0;
    if (still_alive()) {
      draw_screen ();
      if (map[objm.player->coord.y][objm.player->coord.x] == GOAL) {
#ifndef __NOSOUND__
        play_sound(WIN);
#endif
        stop = get_microseconds();
        levelstats.numticks = ticks;
        level += displaystats (levelstats, level);
        if ((level > maxlevel) && (level < (signed)rawmaps.size())) {
          maxlevel = level;
          filemgr.save_maxlevel(mappack, maxlevel);
        }
        while ((level < (signed)rawmaps.size()) && !setup_level(level, mappack)) level ++;
        ticks = 0;
        levelstats.clear();
      } else {
        stop = get_microseconds();
        seconds = ((double)stop - (double)start)/1000000.0;
        if (gamespeed > 0) {
          if (seconds < (1.0 / beatspersecond[gamespeed]))
            restms (((1.0 / beatspersecond[gamespeed]) - seconds) * 1000);
          ticks++;
        } else {
          while (!pollevent()) {
            restms (150);
            draw_screen();
            refresh();
          }
          ticks++;
        }
      }
    } else {
#ifndef __NOSOUND__
      beep ();
#endif
      levelstats.numdeaths++;
      levelstats.numportals = 0;
      fillscreen(screenchar(XFIELD)); refresh();
      restms(100);
      fillscreen(CharData[XFIELD][1] | color_pair(XFIELD) | WA_ALTCHARSET); refresh();
      restms(150);
      while ((level < (signed)rawmaps.size()) && !setup_level(level, mappack)) level ++;
      flushinput();
//      draw_screen (player);
    }
  }
  roll_credits(mappack);
  return 1;
}

unsigned long long int get_microseconds() {
  struct timeval t;
  gettimeofday( &t, 0 );
  return t.tv_sec * 1000000 + t.tv_usec;
}

