// ASCIIpOrtal ver 1.2 by Joseph Larson
// Copyright (c) 2009 Joseph Larson
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
#include "menu.h"

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

// from draw.cpp
extern int CharData[MAXObjects][5];
extern int cheatview;

#include <sys/time.h>

vector<vector<int> > map;
vector <object> objs;
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
  objs.clear();
  lvlname.clear();
  stopmessages ();

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

  object aimobject; // objs[0] will be reserved for aim data
  aimobject.coord.x = 1; // used to remember x
  aimobject.type = SHOT1;
  aimobject.tick = 0;
  aimobject.d.x = 1;
  aimobject.d.y = 0;
  objs.push_back(aimobject);

  vector<int> blankline(rawmaps_maxwidth[lvl] + 2, NONSTICK); // one extra line to start
  map.push_back(blankline);
  syy = 1; // screen yy
  for (yy = 0; yy < (signed)rawmaps[lvl].size(); yy++) {
#ifndef __NOSOUND__
    if (rawmaps[lvl][yy].find("music") == 0) {
      if (rawmaps[lvl][yy].find("default") == 6) {
        if ((rawmaps[lvl][yy][13] >= '1') && (rawmaps[lvl][yy][13] <= '9'))
          default_ambience(rawmaps[lvl][yy][13] - '1' + 1);
        else default_ambience(0);
      }
      else load_ambience(mappack, rawmaps[lvl][yy].substr (6, (signed)rawmaps[lvl][yy].size() - 6));
      start_ambience();
    } else
#endif
    if (rawmaps[lvl][yy].find("message") == 0) {
      if ((rawmaps[lvl][yy][7] >= '1') && (rawmaps[lvl][yy][7] <= '9')) {
        texttrigger[rawmaps[lvl][yy][7] - '1'] = rawmaps[lvl][yy].substr (9, (signed)rawmaps[lvl][yy].size() - 9);
      } else {
        startscrollmessage(rawmaps[lvl][yy].substr (8, (signed)rawmaps[lvl][yy].size() - 8));
#ifndef __NOSOUND__
        play_sound(VOICE + rand() % 10);
#endif
      }
    } else if (rawmaps[lvl][yy].find("name") == 0) {
      lvlname = rawmaps[lvl][yy].substr (5, (signed)rawmaps[lvl][yy].size() - 5);
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

            objs.push_back(newobject);
          }
        }
      }
      map.push_back(mapline); // add the line to the current map.
      syy++;
    }
  }
  map.push_back(blankline);

  if ((signed)objs.size() == 0) {
    mvprintw (LINES / 2 - 1, (COLS - 20) / 2, "Error in level %03d", lvl + 1);
    mvprintw (LINES / 2 , (COLS - 24) / 2,"Level contains no objects", lvl + 1);
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
  if (hasplayer != 1) {
    mvprintw (LINES / 2 - 1, (COLS - 20) / 2, "Error in level %03d", lvl + 1);
    mvprintw(LINES / 2, (COLS - 50) / 2,"Level contains more or less than one player object");
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
  return 1;
}

int hitswall(int yy, int xx) {
  if ((yy < 0) || (yy > (signed)map.size()) || (xx < 0) || (xx > (signed)map[0].size()))
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

int hitsobj(int num, int yy, int xx) { // returns object number of hit object or num
  int val = num;

  for (int c = 1; c < (signed)objs.size(); c++) {
    if ((objs[c].coord.x == xx) && (objs[c].coord.y == yy) && (c != num)) {
      if ((objs[c].type == SHOT1) || (objs[c].type == SHOT2) || (objs[c].type == FLASH) || (objs[c].type == NONE)) {
      } else {
        if (objs[c].type == TEXTTRIGGER) {
          if (val == num) val = c; // Texttriggers get lowest priority.
          else continue;
        }
        if (objs[c].type == DOOR) {
          if (objs[c].d.x > 0) val = c;
        } else val = c;
        if ((objs[c].type == PORTAL1) && (objs[c].type == PORTAL2)) // Portals get more priority
          return val;
      }
    }
  }
  return val;
}

int still_alive (int pl) {
  if (pl < 0) return 0;
  if ((objs[pl].type != PLAYER)) return 0;
  if (objs[pl].coord.y > (signed)map.size()) return 0;

  int underplayer = map[(int)objs[pl].coord.y][(int)objs[pl].coord.x];
  if (underplayer == FFIELD) return 0;
  if (underplayer == SPIKE) return 0;
  int hitobject = objs[hitsobj(pl, objs[pl].coord.y, objs[pl].coord.x)].type;
  if (hitobject == BOULDER) return 0;
  return 1;
}

void fireportal (int por, int pl) {
#ifndef __NOSOUND__
  play_sound(GUNSHOT);
#endif
  for (int c = 1; c < (signed)objs.size(); c++) {
    if (objs[c].type == por) objs.erase(objs.begin() + c--);
  }
  object newobject;
  newobject.coord.x = objs[pl].coord.x + objs[0].d.x;
  newobject.coord.y = objs[pl].coord.y + objs[0].d.y;
  newobject.type = por;
  newobject.d.x = objs[0].d.x;
  newobject.d.y = objs[0].d.y;
  objs.push_back(newobject);
}



int in_portal () { // Technically both portals are the same space.
  int por1, por2;
  por1 = por2 = 0;

  for (int c = 1; c < (signed)objs.size(); c++) {
    if (objs[c].type == PORTAL1) por1 = c;
    if (objs[c].type == PORTAL2) por2 = c;
  }
  if (por1 && por2) {
    int smacked = hitsobj(por1, objs[por1].coord.y, objs[por1].coord.x);
    if (por1 != smacked)
      return smacked;
    smacked = hitsobj(por2, objs[por2].coord.y, objs[por2].coord.x);
    if (por2 != smacked)
      return smacked;
  }
  return 0;
}

int switch_in_portal() { // find what's in the portal and switch it to the other portal
  int c_obj = in_portal();
  if (c_obj) {
    int por1, por2;
    por1 = por2 = 0;

    for (int c = 1; c < (signed)objs.size(); c++) {
      if (objs[c].type == PORTAL1) por1 = c;
      if (objs[c].type == PORTAL2) por2 = c;
    }
    if ((objs[c_obj].coord.x == objs[por2].coord.x) && (objs[c_obj].coord.y == objs[por2].coord.y)) {
      int t = por2;
      por2 = por1;
      por1 = t;
    } // por1 is from, por 2 is to.

    if (objs[por1].d.x == 0) objs[c_obj].d.x = 0;
    if (objs[por1].d.y == 0) objs[c_obj].d.y = 0; // only allow one direction vector through.

    objs[c_obj].coord.x = objs[por2].coord.x;
    objs[c_obj].coord.y = objs[por2].coord.y; // move

    XY temp, control; // rotate d vector
    int rotation = 0;
    control.x = -objs[por1].d.x; control.y = -objs[por1].d.y;
    while (!((control.x == objs[por2].d.x) && (control.y == objs[por2].d.y))) {
      temp.x = control.x; temp.y = control.y;
      control.x = temp.y; control.y = -temp.x;
      temp.x = objs[c_obj].d.x; temp.y = objs[c_obj].d.y;
      objs[c_obj].d.x = temp.y; objs[c_obj].d.y = -temp.x;
      rotation++;
    }

    if (objs[c_obj].type == PLAYER) {
      if (objs[0].d.x == -objs[por2].d.x) { // try to avoid flipflop scenarios due to aiming.
        if (objs[0].d.y != 0) objs[0].d.x = 0;
        else objs[0].d.x = -objs[0].d.x;
      }
      if (objs[0].d.y == -objs[por2].d.y) {
        if (objs[0].d.x != 0) objs[0].d.y = 0;
        else objs[0].d.y = -objs[0].d.y;
      }
    }

    if (objs[c_obj].d.x == 2 * sc(objs[c_obj].d.x))
	    objs[c_obj].d.x = sc(objs[c_obj].d.x); // slow the slide for small drops
	  if (objs[c_obj].d.y == -1) objs[c_obj].d.y = -2; // help out of holes.
	  if ((objs[c_obj].type == PLAYER) && (objs[por2].d.y == -1) && (objs[c_obj].d.y == 0) && (objs[c_obj].d.x == 0)) objs[c_obj].d.y = -2;
	  if (objs[c_obj].d.y < -2) objs[c_obj].d.y++;

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
  int colobj = hitsobj(0, yy, xx);
  switch (objs[colobj].type) {
    case DOOR : if (objs[colobj].d.x > 0) col = NONSTICK; break;
    case DUPLICATOR :
    case PORTAL1:
    case PORTAL2:
      col = NONSTICK;
  }
  return col;
}

int will_hit (int c) {
  int c_obj; // = hitsobj(c, objs[c].coord.y + d.y, objs[c].coord.x + d.x);
  int c_wall; // = hitswall(objs[c].coord.y + d.y, objs[c].coord.x + d.x);
  XY d;

  if ((objs[c].type == BOULDER) && ((objs[c].d.y > 0) || (ticks % boulderbeats))) {
    d.x = 0;
  } else d.x = sc(objs[c].d.x);
  d.y = sc(objs[c].d.y);

  c_obj = hitsobj(c, objs[c].coord.y + d.y, objs[c].coord.x + d.x);
  c_wall = hitswall(objs[c].coord.y + d.y, objs[c].coord.x + d.x);

  if ((c_obj != c) && (objs[c_obj].type != NONE)) { // if hit object
    int objtype = objs[c_obj].type;

    if (objtype == TEXTTRIGGER) {
      if (objs[c].type == PLAYER) {
        if (texttrigger[(signed)objs[c_obj].d.y].size()) {
          startscrollmessage(texttrigger[objs[c_obj].d.y]);
#ifndef __NOSOUND__
          play_sound(VOICE + rand() % 10);
#endif
        }
        for (int z = 0; z < (signed)objs.size(); z++)
          if ((objs[z].type == TEXTTRIGGER) && (objs[z].d.y == objs[c_obj].d.y))
            objs[z].type = NONE;
      }
      if ((objs[c].type == SWITCH) || (objs[c].type == SWITCHON))
        return objtype;
      return NONE;
    }
    if ((objtype == PORTAL1) || (objtype == PORTAL2)) { // if hit portal
      if (((objs[c_obj].d.x != 0) && (objs[c_obj].d.x != d.x))
        || ((objs[c_obj].d.y != 0) && (objs[c_obj].d.y != d.y))) { // only allow in if portal is right direction
        int other = (objtype == PORTAL1) ? PORTAL2 : PORTAL1; // find the other portal
        int other_por;
        for (other_por = 1; (other_por < (signed)objs.size()) && (objs[other_por].type != other); other_por++) ;
        if (other_por < (signed)objs.size()) { // Yes 2 portals present
          if ((in_portal()) && (c != in_portal())) { // if something's in the other portal
            c_obj = in_portal();
            objtype = objs[c_obj].type;
          } else objtype = NONE;
        }
      } else objtype = c_wall;
    }

    if (((objs[c].type == PLAYER) || (objs[c].type == BOULDER)) && (objtype == BOX)) {
      if (!objs[c].d.y) {
        objs[c_obj].d.x += d.x;
        if ((move_object (c_obj)) && (objs[c].type != BOULDER)) {
          objtype = NONE;
        }
      }
    }
    if ((objs[c].type == PLAYER) && (objtype == BOULDER)) { // if player hit boulder
      if (d.y <= 0) {
#ifndef __NOSOUND__
        play_sound (CRUSH);
#endif
#ifndef __NOSOUND__
        play_sound (CRUSH);
#endif
        objs[c].type = NONE; //   kill player
        return NONE;
      } else d.y = 0;
    } else if ((objs[c].type == BOULDER) && (objtype == PLAYER)) { // if boulder hit player
#ifndef __NOSOUND__
	    play_sound (CRUSH);
#endif
      objs[c_obj].type = NONE;
      return NONE;
    }
    return objtype;
  }
  if ((objs[c].type == PLAYER) && (c_wall == SPIKE)) {
    objs[c].type = NONE; //   kill player
#ifndef __NOSOUND__
    play_sound (CRUSH);
#endif
    return NONE;
  }
  return c_wall;
}

int applyd (int c) {
  int dx, dy;

  if ((objs[c].type == BOULDER) && ((objs[c].d.y != 0) || (ticks % boulderbeats))) {
    dx = 0;
  } else dx = sc(objs[c].d.x);
  dy = sc(objs[c].d.y);
  objs[c].coord.x += dx; objs[c].coord.y += dy;
  if (objs[c].type != BOULDER) {
    if (objs[c].d.x != 0) objs[c].d.x -= dx;
  } else if ((objs[c].d.x < -1) || (objs[c].d.x > 1)) objs[c].d.x -= dx;
  if (objs[c].d.y < 0) objs[c].d.y++;

  if (objs[c].coord.y < 0) objs[c].coord.y = 0; //  bounds checking
  if (objs[c].coord.x < 0) objs[c].coord.x = 0;
  if (objs[c].coord.y >= (signed)map.size()) objs[c].coord.y = (signed)map.size() - 1;
  if (objs[c].coord.x >= (signed)map[0].size()) objs[c].coord.x = (signed)map[0].size() - 1;

  if (dx || dy) return 1;
  return 0;
}

int collapse_portals () {
  int playsound = 0;

  int in_por = in_portal();
  if (in_por) {
    int por1, por2;
    for (int d = 1; d < (signed)objs.size(); d++) {
      if (((objs[d].type == PORTAL1) || (objs[d].type == PORTAL2))
        && (in_por == hitsobj(d, objs[d].coord.y, objs[d].coord.x)))
        por1 = d;
      else por2 = d;
    }
    objs[in_por].d.x += objs[por1].d.x; // push anything stuck in portal out.
    objs[in_por].d.y += objs[por1].d.y; // before collapsing it.
    objs[in_por].tick--; // make sure a move is possible.
    if (!move_object(in_por)) {
      switch_in_portal();
      objs[in_por].d.x += objs[por2].d.x;
      objs[in_por].d.y += objs[por2].d.y;
      objs[in_por].tick--;
      if (!move_object(in_por)) objs[in_por].type = FLASH;
    }
  }

  for (int e = 1; e < (signed)objs.size(); e++)
  if ((objs[e].type == PORTAL1) || (objs[e].type == PORTAL2)
    || (objs[e].type == SHOT1) || (objs[e].type == SHOT2)) {
    objs[e].d.y = CharData[objs[e].type][2];
    objs[e].type = FLASH; // cancel all portals
    playsound = 1;
  }
#ifndef __NOSOUND__
  if (playsound == 1) play_sound (PORTALCOLLAPSE);
#endif
}

int move_object (int c) { // collision detection and movement.
  int c_obj;

  if (objs[c].tick >= ticks) return 0;
  objs[c].tick = ticks;
  if (objs[c].type == BOX) objs[c].tick--;

  c_obj = hitsobj(c, objs[c].coord.y, objs[c].coord.x);
  if ((objs[c_obj].type == PORTAL1) || (objs[c_obj].type == PORTAL2)) { // if obj[c] is in portal
    int needswitch = 0;
    if ((objs[c].d.x) && (sc(objs[c].d.x) == -objs[c_obj].d.x)) needswitch = 1;
    if ((objs[c].d.y) && (sc(objs[c].d.y) == -objs[c_obj].d.y)) needswitch = 1;
    if (objs[c].type == PLAYER) {
      if ((objs[0].d.x) && (sc(objs[0].d.x) == -objs[c_obj].d.x)) needswitch = 1;
      if ((objs[0].d.y) && (sc(objs[0].d.y) == -objs[c_obj].d.y)) needswitch = 1;
    }

    if (needswitch) { // if facing the right way
      int rotations = switch_in_portal();
      c_obj = hitsobj(c, objs[c].coord.y, objs[c].coord.x);
      if (objs[c].type == PLAYER) {
#ifndef __NOSOUND__
        play_sound(THROUGH);
#endif
        draw_rotate(c, rotations);
      }
    }
  } // And that's how portals should work.

  XY oldd;

  oldd.x = objs[c].d.x; oldd.y = objs[c].d.y;
  if (will_hit(c) == NONE) return applyd(c);
  objs[c].d.y = 0;
  if (will_hit(c) == NONE) return applyd(c); // try without dy
  if ((objs[c].type == PLAYER) && (objs[c].d.x)) {
    objs[c].d.y = -1; // step up;

    int porcheck = hitsobj(c, objs[c].coord.y, objs[c].coord.x);
    if ((objs[porcheck].type == PORTAL1) || (objs[porcheck].type == PORTAL2)) {
      if (objs[porcheck].d.y == 1) {
        int rotations = switch_in_portal();
        c_obj = hitsobj(c, objs[c].coord.y, objs[c].coord.x);
#ifndef __NOSOUND__
        play_sound(THROUGH);
#endif
        draw_rotate(c, rotations);
      }
    }
    if (will_hit(c) == NONE) return applyd(c);
    objs[c].d.y = 0;
  }
  objs[c].d.y = oldd.y;
  if (objs[c].type == BOULDER) {
    objs[c].d.x = -sc(oldd.x);
  } else objs[c].d.x = 0;
  if (will_hit(c) == NONE) return applyd(c); // try without dx
  objs[c].d.y = 0;
  return 0; // didn't move
}

int physics () { // returns the index of the player so we don't have to search for this 3 times.
  int playerindex = 0;
  int por = 0;

  for (int c = 1; c < (signed)objs.size(); c++) {

    por = 0;
    switch (objs[c].type) { // Every tick movements (like gravity)
      case NONE: break;
      case DUPLICATOR:
        if (objs[c].d.y > 0) objs[c].d.y--;
        else {
          int dupe = hitsobj(c, objs[c].coord.y - 1, objs[c].coord.x);
          if ((c != dupe) && (objs[dupe].type != PLAYER)
            && (c == hitsobj(c, objs[c].coord.y + 1, objs[c].coord.x))) {
            object newobject;
            newobject.coord.x = objs[c].coord.x;
            newobject.coord.y = objs[c].coord.y;
            newobject.type = objs[dupe].type;
            newobject.tick = objs[dupe].tick;
            newobject.d.x = objs[dupe].d.x;
            newobject.d.y = 0;
            objs.push_back(newobject);
            objs[c].d.y = beatsperdupe;
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
        int colobj = hitsobj(c, objs[c].coord.y - 1, objs[c].coord.x);
        if (c != colobj) {
          if (objs[c].type == SWITCH) {
            objs[c].type = SWITCHON;
            if ((objs[colobj].type == PLAYER) || (objs[colobj].type == BOX)) playsound = 2;
          }
          int otheropen = 0;
          for (int s = 1; s < (signed)objs.size(); s++)
            if ((objs[s].type == SWITCH) && (objs[s].d.y == objs[c].d.y))
              otheropen = 1;
          if (!otheropen) // open the doors
            for (int d = 1; d < (signed)objs.size(); d++) {
              if ((objs[d].type == DOOR) && (objs[d].d.y == objs[c].d.y)) {
                if (((objs[colobj].type == PLAYER) || (objs[colobj].type == BOX)) && (objs[d].d.x == 4)) playsound = 1;
                if (--objs[d].d.x < 0) objs[d].d.x = 0;
              }
            }
#ifndef __NOSOUND__
          if (playsound == 1) play_sound(DOOROPEN);
          if (playsound == 2) play_sound(SWITCHHIT);
#endif
        } else {
          objs[c].type = SWITCH;
          for (int d = 1; d < (signed)objs.size(); d++) {
            if ((objs[d].type == DOOR) && (objs[d].d.y == objs[c].d.y))
              if (objs[c].d.y >= 6) { // if it's a momentary switch
                if (d == hitsobj(d, objs[d].coord.y, objs[d].coord.x)) { // and there's nothing in the door
                  if (objs[d].d.x == 0) playsound = 1;
                  if (++objs[d].d.x > 4) objs[d].d.x = 4; // close the door
                }
              } else if (objs[d].d.x < 4) // else finish opening the door.
                if (--objs[d].d.x < 0) objs[d].d.x = 0;
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
		      int col = por_col(objs[c].coord.y, objs[c].coord.x);
          if (col) {
            if (col == NONSTICK) {
              objs[c].d.y = CharData[objs[c].type][2];
              objs[c].type = FLASH;
#ifndef __NOSOUND__
              play_sound (PORTALFAIL);
#endif
              z = portalspeed; continue;
            }
            // Calculate portal normal.
            int east = por_col(objs[c].coord.y, objs[c].coord.x + 1);
            if (objs[hitsobj(c,objs[c].coord.y, objs[c].coord.x + 1)].type == DOOR) east = NONE;

            int west = por_col(objs[c].coord.y, objs[c].coord.x - 1);
            if (objs[hitsobj(c,objs[c].coord.y, objs[c].coord.x - 1)].type == DOOR) west = NONE;

            int north= por_col(objs[c].coord.y - 1, objs[c].coord.x);
            if (objs[hitsobj(c,objs[c].coord.y - 1, objs[c].coord.x)].type == DOOR) north = NONE;

            int south= por_col(objs[c].coord.y + 1, objs[c].coord.x);
            if (objs[hitsobj(c,objs[c].coord.y + 1, objs[c].coord.x)].type == DOOR) south = NONE;

            int yback= por_col(objs[c].coord.y - objs[c].d.y, objs[c].coord.x);
            if (map[objs[c].coord.y - objs[c].d.y][objs[c].coord.x] == FFIELD) yback = NONSTICK;

            int xback= por_col(objs[c].coord.y, objs[c].coord.x - objs[c].d.x);
            if (map[objs[c].coord.y][objs[c].coord.x - objs[c].d.x] == FFIELD) xback = NONSTICK;

            if (east && west && yback == NONE) {
              objs[c].d.x = 0; objs[c].d.y = -objs[c].d.y;
            } else if (south && north && xback == NONE) {
              objs[c].d.y = 0; objs[c].d.x = -objs[c].d.x;
            } else {
#ifndef __NOSOUND__
              play_sound (PORTALFAIL);
#endif
              objs[c].d.y = CharData[objs[c].type][2];
              objs[c].type = FLASH;
              z = portalspeed; continue;
            }
            for (int d = 1; d < (signed)objs.size(); d++) {
              if ((c != d) && (objs[d].type == (PORTAL1 + por))) {
                int in_por = hitsobj(d, objs[d].coord.y, objs[d].coord.x);
                if ((in_por != d) && (objs[in_por].type != PORTAL1) && (objs[in_por].type != PORTAL2)){
                  objs[in_por].d.x += objs[d].d.x; // push anything stuck in portal out.
                  objs[in_por].d.y += objs[d].d.y; // before collapsing it.
                  if (!move_object(in_por)) {
                    switch_in_portal();
                  }
                }
                objs[d].d.y = CharData[objs[d].type][2];
                objs[d].type = FLASH;
              }
            }
            levelstats.numportals++;
            animateportal = 2;
#ifndef __NOSOUND__
		        play_sound (PORTALCREATE);
#endif
            objs[c].type = PORTAL1 + por;
            objs[0].type = SHOT1 + ((por + 1) % 2);
            z = portalspeed;
          } else {
            objs[c].coord.y += objs[c].d.y;
            objs[c].coord.x += objs[c].d.x;
          }
        }
        continue;
      case PLAYER: // gravity
        if (move_player(c) < 0) return -1;
        playerindex = c;
        if (map[objs[c].coord.y + 1][objs[c].coord.x + objs[c].d.x] == LADDER) break;
        if (map[objs[c].coord.y + 1][objs[c].coord.x] == LADDER) break;
        if (map[objs[c].coord.y][objs[c].coord.x] == LADDER) break;
        if (map[objs[c].coord.y + sc(objs[c].d.y)][objs[c].coord.x + sc(objs[c].d.x)] == LADDER) break;
      case BOULDER: {
        if ((map[objs[c].coord.y + 1][objs[c].coord.x] == LADDER) && (rand() < RAND_MAX / 2)) break;
      }
      case BOX:
        if ((objs[c].d.x > 2) || (objs[c].d.x < -2) || (objs[c].d.y < -2)) break; // fling!
        if (++objs[c].d.y > maxfling) objs[c].d.y = maxfling;
      default:
        break;
    } // End every tick movements.

    if (!(ticks % beatspermove)) { // X-axis automovements
      switch (objs[c].type) {
        case BOULDER:
          if (objs[c].d.x == 0) objs[c].d.x = (rand() < RAND_MAX / 2) ? 1 : -1;
          break;
        case PLAYER:
        case BOX:
          switch (map[objs[c].coord.y + 1][objs[c].coord.x]) {
            case LTREAD:
			  if (objs[c].d.x == 0) objs[c].d.x --;
			  break;
            case RTREAD:
			  if (objs[c].d.x == 0) objs[c].d.x ++;
			  break;
          }
          break;
      }
    }
    move_object(c);

    switch (objs[c].type) { // kill objects / clean up
      case PLAYER:
        if (map[objs[c].coord.y][objs[c].coord.x] == LADDER) {
          objs[c].d.y = 0;
          objs[c].d.x = 0;
        }
      case BOULDER:
      case BOX: {
        switch (map[objs[c].coord.y][objs[c].coord.x]) {
          case PFIELD: {
            int playsound = 0;
            if (objs[c].type == PLAYER) {
              collapse_portals();
            } else {
#ifndef __NOSOUND__
			        play_sound (SIZZLE);
#endif
              objs[c].d.y = COLOR_WHITE;
              objs[c].type = FLASH;
            }
            break;
          }
          case XFIELD:
#ifndef __NOSOUND__
		        play_sound (SIZZLE);
#endif
            objs[c].d.y = COLOR_WHITE;
            objs[c].type = FLASH;
        }
      }
    }
  }
  return playerindex;
}

int move_player (int pl) {
  int input;
  XY d;

  d.x = d.y = 0;
  input = getinput();
  if (input == ERR) {
    return 1;
  }
  switch (input) {
    case '1' : objs[0].d.y =  1; objs[0].d.x = -1; break;
    case '2' : objs[0].d.y =  1; objs[0].d.x =  0; break;
    case '3' : objs[0].d.y =  1; objs[0].d.x =  1; break;
    case '4' : objs[0].d.y =  0; objs[0].d.x = -1; break;
    case '6' : objs[0].d.y =  0; objs[0].d.x =  1; break;
    case '7' : objs[0].d.y = -1; objs[0].d.x = -1; break;
    case '8' : objs[0].d.y = -1; objs[0].d.x =  0; break;
    case '9' : objs[0].d.y = -1; objs[0].d.x =  1; break;
    case '0' :
    case 'Z' :
    case 'z' : objs[0].type = SHOT1; fireportal(objs[0].type, pl); break;
    case '.' :
    case 'X' :
    case 'x' : objs[0].type = SHOT2; fireportal(objs[0].type, pl); break;
    case 'C' :
    case 'c' : collapse_portals (); break;
    case '5' :
    case '\n':
    case ' ' :
      fireportal(objs[0].type, pl);
      break;
    case '?' : help_menu (); break;
    case 27 : // ASCII for escape
    case KEY_F(1) :
    case 'P' :
    case 'p' : {
      int pause=pause_menu(pl);
      switch (pause) {
        case 0 : break; // Resume
        case 1 : objs[pl].type = NONE; break; // Restart
        case 2 : {
          int newlvl = select_level (maxlevel, level); // Select Level
          if (newlvl != level) {
            level = newlvl;
            objs[pl].type = NONE;
            ticks = 0;
            levelstats.clear();
            levelstats.numdeaths --; // Correct since death is the tool for this one.
          }
          break;
        }
        case 3 : help_menu (); break;
        default : return -1; break; // Quit
      }
    }
    break;
    case KEY_F(2) :
      cheatview = (cheatview + 1) % 3;
      switch (cheatview) {
        case 0: statusmessage ("No Portal Edges view mode."); break;
        case 1: statusmessage ("Portal Fill view mode"); break;
        case 2: statusmessage ("Portal Edges view mode."); break;
      }
      break;
    case KEY_F(3) :
      statusmessage ("Default speed restored");
      gamespeed = defaultspeed;
      break;
    case '+' :
    case ']' :
      gamespeed += 2; // a little cheating with the case statements;
    case '-' :
    case '[' :
      gamespeed --;
      if (gamespeed > maxspeed) gamespeed = maxspeed - 1;
      else if (gamespeed < 1) gamespeed = 1;
      else switch (gamespeed) {
        case 1 : statusmessage ("Very slow speed"); break;
        case 2 : statusmessage ("Slow speed"); break;
        case 3 : statusmessage ("Normal speed"); break;
        case 4 : statusmessage ("Fast speed"); break;
        case 5 : statusmessage ("Very fast speed"); break;
      }
      break;
    case KEY_F(10) :
      statusmessage ("Roguelike-ish speed mode");
      gamespeed = 0;
      break;
#ifndef __NOSOUND__
    case 'M' :
    case 'm' : toggle_ambience (); break;
#endif
#ifdef PDCURSES
    case KEY_RESIZE: resize_term(0,0); break;
#endif
    case 'd' :
    case 'D' :
    case KEY_RIGHT :
      if (objs[0].d.x == 1) {
        if (objs[pl].d.x <= 0) objs[pl].d.x ++;
        levelstats.numsteps++;
      } else {
        objs[0].coord.x = objs[0].d.x = 1;
      }
      objs[0].d.y = 0;
      break;
    case 'a' :
    case 'A' :
    case KEY_LEFT :
      if (objs[0].d.x == -1) {
        if (objs[pl].d.x >= 0) objs[pl].d.x --;
        levelstats.numsteps++;
      } else {
        objs[0].coord.x = objs[0].d.x = -1;
      }
      objs[0].d.y = 0;
      break;
    case 'w' :
    case 'W' :
    case KEY_UP :
      if ((map[objs[pl].coord.y][objs[pl].coord.x] == LADDER)
      && ((!hitswall(objs[pl].coord.y - 1, objs[pl].coord.x) || (objs[hitsobj(pl, objs[pl].coord.y - 1, objs[pl].coord.x)].type == PORTAL1) || (objs[hitsobj(pl, objs[pl].coord.y - 1, objs[pl].coord.x)].type == PORTAL2))))
      {
        if (objs[pl].d.y >= 0) objs[pl].d.y --;
        levelstats.numsteps++;
      } else if ((objs[0].d.x == 0) && (objs[0].d.y > 0)) {
        objs[0].d.x = objs[0].coord.x;
      } else {
        objs[0].d.y--;
        if (objs[0].d.y < -1) {
          objs[0].d.y = -1;
          objs[0].d.x = 0;
        }
      }
      break;
    case 's' :
    case 'S' :
    case KEY_DOWN :
      if (((map[(int)objs[pl].coord.y][(int)objs[pl].coord.x] == LADDER)
      || (map[(int)objs[pl].coord.y + 1][(int)objs[pl].coord.x] == LADDER))
      && ((!hitswall(objs[pl].coord.y + 1, objs[pl].coord.x) || (objs[hitsobj(pl, objs[pl].coord.y + 1, objs[pl].coord.x)].type == PORTAL1) || (objs[hitsobj(pl, objs[pl].coord.y + 1, objs[pl].coord.x)].type == PORTAL2))))
      {
        objs[pl].d.y ++;
        levelstats.numsteps++;
      } else if ((objs[0].d.x == 0) && (objs[0].d.y < 0)) {
        objs[0].d.x = objs[0].coord.x;
      } else {
        objs[0].d.y++;
        if (objs[0].d.y > 1) {
          objs[0].d.y = 1;
          objs[0].d.x = 0;
        }
      }
      break;
  }
//  flushinp ();
  return 1;
}

int play (string mappack) {
  int player = -1;
  unsigned long long int start, stop;
  double seconds;

  level = 0;

  int play = 0;
  do {
    switch (main_menu (mappack)) {
      case 0: // Play
        play = 1; break;
      case 1: // Select Level
        level = select_level (maxlevel, maxlevel); play = 1;
        break;
      case 2: { // Change Map Set
        string newmap = select_mapset ();
        if (!loadmaps (newmap)) {
          mvprintw (LINES / 2 + 3, (COLS - 16) / 2, "Invalid Map Pack"); refresh ();
          restms (250);
          loadmaps (mappack);
        } else {
          mvprintw (LINES / 2 + 3, (COLS - 15) / 2, "Map Pack Loaded"); refresh ();
          restms (250);

          mappack = newmap;
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
  } while (!play);
#ifndef __NOSOUND__
  default_ambience (0);
  start_ambience ();
#endif
  while ((level < (signed)rawmaps.size()) && !setup_level(level, mappack)) level ++;
  ticks = 0;
  levelstats.clear();
  while (level < (signed)rawmaps.size()) {
    for (int c = 0; c < (signed)objs.size(); c++) // clean up NONEs.
      if (objs[c].type == NONE)
        objs.erase(objs.begin() + c--);
      else if (objs[c].type == FLASH)
        objs[c].type = NONE;

    start = get_microseconds();
    player = physics ();
    if (player == -1) return 0;
    if (still_alive(player)) {
      draw_screen (player);
      if (map[objs[player].coord.y][objs[player].coord.x] == GOAL) {
#ifndef __NOSOUND__
        play_sound(WIN);
#endif
        stop = get_microseconds();
        levelstats.numticks = ticks;
        level += displaystats (levelstats, level);
        if ((level > maxlevel) && (level < (signed)rawmaps.size())) {
          maxlevel = level;
          // save maxlevel
          string maxlevelfilename;
#ifdef WIN32
          maxlevelfilename = mappack + "\\save.dat";
#else
          maxlevelfilename = mappack + "/save.dat";
#endif
          ofstream maxlevelfile;
          maxlevelfile.open (maxlevelfilename.c_str());
          maxlevelfile << maxlevel;
          maxlevelfile.close();
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
		        draw_screen(player);
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
      fillscreen(CharData[XFIELD][1] | color_pair(XFIELD) | A_ALTCHARSET); refresh();
      restms(150);
      while ((level < (signed)rawmaps.size()) && !setup_level(level, mappack)) level ++;
      flushinput();
      draw_screen (player);
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

