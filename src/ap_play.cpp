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
#include <sys/time.h>
using namespace std;

#include "ap_play.h"

#ifndef __NOSDL__
#include "SDL/SDL.h"
#endif
#ifndef __NOSOUND__
#include "ap_sound.h"
#endif

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
extern int animateportal;
extern FileManager filemgr;

// from ap_draw.cpp
extern const int CharData[MAXObjects][6];
extern int cheatview;

Game::Game(level &level) : lvl(level),
                           NULLOBJ(level.objm.NULLOBJ)
{
  pause = false;
  light_pause = false;
}

void Game::set_status(string status) {
  lvl.pager.set_status(lvl.ticks, status);
}

int hitswall(level const &lvl, int yy, int xx) {
  if ((yy < 0) || (yy >= lvl.map.size()) || (xx < 0) || (xx >= lvl.map[0].size()))
    return NONSTICK;
  switch (lvl.map[yy][xx]) {
    case NONE:
    case GOAL:
    case LADDER:
    case PFIELD:
    case XFIELD:
      return 0; // Everyone and everything passes through these
  }
  return lvl.map[yy][xx];
}

// returns iterator to hit object, else it0
objiter hitsobj(level &lvl, objiter it0, int yy, int xx) {
  objiter val = it0;

  if (xx < 0 || xx >= lvl.objm.objmap[yy].size() || yy < 0 || yy >= lvl.objm.objmap.size())
    return val;

  objset *s = &(lvl.objm.objmap[yy][xx]);
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

bool Game::still_alive () {
  if (lvl.objm.player == NULLOBJ || lvl.objm.player->coord.y > lvl.map.size()) return false;

  int underplayer = lvl.map[lvl.objm.player->coord.y][lvl.objm.player->coord.x];
  if (underplayer == FFIELD || underplayer == SPIKE) return false;

  int hitobject = hitsobj(lvl, lvl.objm.player, lvl.objm.player->coord.y, lvl.objm.player->coord.x)->type;
  if (hitobject == BOULDER) return false;

  return true;
}

void Game::fireportal (int por) {
#ifndef __NOSOUND__
  play_sound(GUNSHOT);
#endif
  if (lvl.objm.shots[por - SHOT1] != NULLOBJ) {
    lvl.objm.shots[por - SHOT1]->type = NONE;
    lvl.objm.shots[por - SHOT1] = NULLOBJ;
  }
  object newobject;
  newobject.coord.x = lvl.objm.player->coord.x + lvl.aimobject.d.x;
  newobject.coord.y = lvl.objm.player->coord.y + lvl.aimobject.d.y;
  newobject.type = por;
  newobject.tick = 0;
  newobject.d.x = lvl.aimobject.d.x;
  newobject.d.y = lvl.aimobject.d.y;
  lvl.objm.addobj(newobject);
}

objiter Game::in_portal () { // Technically both portals are the same space.
  objiter por1 = lvl.objm.portals[0];
  objiter por2 = lvl.objm.portals[1];
  if (por1 != NULLOBJ && por2 != NULLOBJ) {
    objiter smacked = hitsobj(lvl, por1, por1->coord.y, por1->coord.x);
    if (por1 != smacked)
      return smacked;
    smacked = hitsobj(lvl, por2, por2->coord.y, por2->coord.x);
    if (por2 != smacked)
      return smacked;
  }
  return NULLOBJ;
}

// find what's in the portal and switch it to the other portal
int Game::switch_in_portal() {
  objiter c_obj = in_portal();
  objiter por1 = lvl.objm.portals[0];
  objiter por2 = lvl.objm.portals[1];

  if (c_obj != NULLOBJ) {
    if ((c_obj->coord.x == por2->coord.x) && (c_obj->coord.y == por2->coord.y)) {
      objiter t = por2;
      por2 = por1;
      por1 = t;
    } // por1 is from, por 2 is to.

    if (por1->d.x == 0) c_obj->d.x = 0;
    if (por1->d.y == 0) c_obj->d.y = 0; // only allow one direction vector through.

    lvl.objm.moveobj(c_obj, por2->coord.x, por2->coord.y);

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
      if (lvl.aimobject.d.x == -por2->d.x) { // try to avoid flipflop scenarios due to aiming.
        if (lvl.aimobject.d.y != 0) lvl.aimobject.d.x = 0;
        else lvl.aimobject.d.x = -lvl.aimobject.d.x;
      }
      if (lvl.aimobject.d.y == -por2->d.y) {
        if (lvl.aimobject.d.x != 0) lvl.aimobject.d.y = 0;
        else lvl.aimobject.d.y = -lvl.aimobject.d.y;
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

int Game::por_col (int yy, int xx) { // Collision specific to portal shots
  int col = lvl.map[yy][xx];
  if ((yy < 0) || (yy >= (signed)lvl.map.size())
  || (xx < 0) || (xx >= (signed)lvl.map[0].size()))
  col = NONSTICK;
  else switch (lvl.map[yy][xx]) {
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
  objiter colobj = hitsobj(lvl, NULLOBJ, yy, xx);
  if (colobj != NULLOBJ)
  switch (colobj->type) {
    case DOOR : if (colobj->d.x > 0) col = NONSTICK; break;
    case DUPLICATOR :
    case PORTAL1:
    case PORTAL2:
      col = NONSTICK;
  }
  return col;
}

bool Game::has_won() {
  return (lvl.map[lvl.objm.player->coord.y][lvl.objm.player->coord.x] == GOAL);
}

int Game::will_hit (objiter c) {
  objiter c_obj; // = hitsobj(c, c->coord.y + d.y, c->coord.x + d.x);
  int c_wall; // = hitswall(c->coord.y + d.y, c->coord.x + d.x);
  XY d;

  if ((c->type == BOULDER) && ((c->d.y > 0) || (lvl.ticks % boulderbeats)))
    d.x = 0;
  else
    d.x = sc(c->d.x);
  d.y = sc(c->d.y);

  c_obj = hitsobj(lvl, c, c->coord.y + d.y, c->coord.x + d.x);
  c_wall = hitswall(lvl, c->coord.y + d.y, c->coord.x + d.x);

  if ((c_obj != c) && (c_obj->type != NONE)) { // if hit object
    int objtype = c_obj->type;

    if (objtype == TEXTTRIGGER) {
      if (c->type == PLAYER) {
        if (lvl.texttrigger[c_obj->d.y].size()) {
          lvl.pager.add_scrolling(lvl.texttrigger[c_obj->d.y]);
#ifndef __NOSOUND__
          play_voice();
#endif
        }
        lvl.objm.killtriggers(c_obj->d.y);
      }
      if ((c->type == SWITCH) || (c->type == SWITCHON))
        return objtype;
      return NONE;
    }
    if ((objtype == PORTAL1) || (objtype == PORTAL2)) { // if hit portal
      if (((c_obj->d.x != 0) && (c_obj->d.x != d.x))
          || ((c_obj->d.y != 0) && (c_obj->d.y != d.y))) { // only allow in if portal is right direction
        objiter other_por = lvl.objm.portals[(objtype == PORTAL1) ? 1 : 0]; // get the other portal
        if (other_por != NULLOBJ) { // Yes 2 portals present
          objiter inp = in_portal();
          if (inp != lvl.objm.objs.end() && c != inp) { // if something's in the other portal
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
        lvl.objm.player = NULLOBJ;
        c->type = NONE; //   kill player
        return NONE;
      } else d.y = 0;
    } else if ((c->type == BOULDER) && (objtype == PLAYER)) { // if boulder hit player
#ifndef __NOSOUND__
      play_sound (CRUSH);
#endif
      lvl.objm.player = NULLOBJ;
      c_obj->type = NONE;
      return NONE;
    }
    return objtype;
  }
  if ((c->type == PLAYER) && (c_wall == SPIKE)) {
    lvl.objm.player = NULLOBJ;
    c->type = NONE; //   kill player
#ifndef __NOSOUND__
    play_sound (CRUSH);
#endif
    return NONE;
  }
  return c_wall;
}

int Game::applyd (objiter c) {
  int dx, dy;

  if ((c->type == BOULDER) && ((c->d.y != 0) || (lvl.ticks % boulderbeats))) {
    dx = 0;
  } else dx = sc(c->d.x);
  dy = sc(c->d.y);
  lvl.objm.moveobj(c, c->coord.x + dx, c->coord.y + dy);
  if (c->type != BOULDER) {
    if (c->d.x != 0) c->d.x -= dx;
  } else if ((c->d.x < -1) || (c->d.x > 1)) c->d.x -= dx;
  if (c->d.y < 0) c->d.y++;

  if (dx || dy) return 1;
  return 0;
}

void Game::collapse_portals () {
  int playsound = 0;

  objiter in_por = in_portal();
  if (in_por != NULLOBJ) {
    objiter por1 = lvl.objm.portals[0];
    objiter por2 = lvl.objm.portals[1];
    if (in_por == hitsobj(lvl, por2, por2->coord.y, por2->coord.x)) { // FIXME: redundant, need to rework in_portal()
      por1 = lvl.objm.portals[1];
      por2 = lvl.objm.portals[0];
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
          lvl.objm.player = NULLOBJ; // a unique way to die
        in_por->type = FLASH;
      }
    }
  }

  // cancel all portals
  for (int i = 0; i < 2; ++i) {
    if (lvl.objm.portals[i] != NULLOBJ) {
      lvl.objm.portals[i]->d.y = CharData[PORTAL1 + i][3];
      lvl.objm.portals[i]->type = FLASH;
      lvl.objm.portals[i] = NULLOBJ;
      playsound = 1;
    }
    if (lvl.objm.shots[i] != NULLOBJ) {
      lvl.objm.shots[i]->d.y = CharData[SHOT1 + i][3];
      lvl.objm.shots[i]->type = FLASH;
      lvl.objm.shots[i] = NULLOBJ;
      playsound = 1;
    }
  }

#ifndef __NOSOUND__
  if (playsound) play_sound (PORTALCOLLAPSE);
#endif
}

// collision detection and movement.
int Game::move_object (objiter c) {
  objiter c_obj;

  if (c->tick >= lvl.ticks) return 0;
  c->tick = lvl.ticks;
  if (c->type == BOX) c->tick--;

  c_obj = hitsobj(lvl, c, c->coord.y, c->coord.x);
  if ((c_obj->type == PORTAL1) || (c_obj->type == PORTAL2)) { // if obj[c] is in portal
    bool needswitch = false;
    if ((c->d.x) && (sc(c->d.x) == -c_obj->d.x)) needswitch = true;
    if ((c->d.y) && (sc(c->d.y) == -c_obj->d.y)) needswitch = true;
    if (c->type == PLAYER) {
      if ((lvl.aimobject.d.x) && (sc(lvl.aimobject.d.x) == -c_obj->d.x)) needswitch = true;
      if ((lvl.aimobject.d.y) && (sc(lvl.aimobject.d.y) == -c_obj->d.y)) needswitch = true;
    }

    if (needswitch) { // if facing the right way
      int rotations = switch_in_portal();
      c_obj = hitsobj(lvl, c, c->coord.y, c->coord.x);
      if (c->type == PLAYER) {
#ifndef __NOSOUND__
        play_sound(THROUGH);
#endif
        draw_rotate(lvl, rotations);
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

    objiter porcheck = hitsobj(lvl, c, c->coord.y, c->coord.x);
    if ((porcheck->type == PORTAL1) || (porcheck->type == PORTAL2)) {
      if (porcheck->d.y == 1) {
        int rotations = switch_in_portal();
        c_obj = hitsobj(lvl, c, c->coord.y, c->coord.x);
#ifndef __NOSOUND__
        play_sound(THROUGH);
#endif
        draw_rotate(lvl, rotations);
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

int Game::physics () {
  int por = 0;

  for (objiter c = lvl.objm.objs.begin(); c != lvl.objm.objs.end(); c++) {
    por = 0;
    switch (c->type) { // Every tick movements (like gravity)
      case NONE: break;
      case DUPLICATOR:
        if (c->d.y > 0) c->d.y--;
        else {
          objiter dupe = hitsobj(lvl, c, c->coord.y - 1, c->coord.x);
          if ((c != dupe) && (dupe->type == BOX || dupe->type == BOULDER)
              && (c == hitsobj(lvl, c, c->coord.y + 1, c->coord.x))) {
            object newobject;
            newobject.coord.x = c->coord.x;
            newobject.coord.y = c->coord.y;
            newobject.type = dupe->type;
            newobject.tick = dupe->tick;
            newobject.d.x = dupe->d.x;
            newobject.d.y = 0;
            lvl.objm.addobj(newobject);
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
        objiter colobj = hitsobj(lvl, c, c->coord.y - 1, c->coord.x);
        if (c != colobj) {
          if (c->type == SWITCH) {
            c->type = SWITCHON;
            if ((colobj->type == PLAYER) || (colobj->type == BOX)) playsound = 2;
          }
          int otheropen = 0;
          for (int i = 0; i < lvl.objm.switches[c->d.y].size(); i++)
            if (lvl.objm.switches[c->d.y][i]->type == SWITCH)
              otheropen = 1;
          if (!otheropen) // open the doors
            for (int i = 0; i < lvl.objm.doors[c->d.y].size(); i++) {
              objiter d = lvl.objm.doors[c->d.y][i];
              if (((colobj->type == PLAYER) || (colobj->type == BOX)) && (d->d.x == 4)) playsound = 1;
              if (--d->d.x < 0) d->d.x = 0;
            }
#ifndef __NOSOUND__
          if (playsound == 1) play_sound(DOOROPEN);
          if (playsound == 2) play_sound(SWITCHHIT);
#endif
        } else {
          c->type = SWITCH;
          for (int i = 0; i < lvl.objm.doors[c->d.y].size(); i++) {
            objiter d = lvl.objm.doors[c->d.y][i];
            if (c->d.y >= 6) { // if it's a momentary switch
              if (d == hitsobj(lvl, d, d->coord.y, d->coord.x)) { // and there's nothing in the door
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
              c->d.y = CharData[c->type][3];
              lvl.objm.shots[c->type - SHOT1] = NULLOBJ;
              c->type = FLASH;
#ifndef __NOSOUND__
              play_sound (PORTALFAIL);
#endif
              z = portalspeed; continue;
            }
            // Calculate portal normal.
            int east = por_col(c->coord.y, c->coord.x + 1);
            if (hitsobj(lvl, c, c->coord.y, c->coord.x + 1)->type == DOOR) east = NONE;

            int west = por_col(c->coord.y, c->coord.x - 1);
            if (hitsobj(lvl, c, c->coord.y, c->coord.x - 1)->type == DOOR) west = NONE;

            int north= por_col(c->coord.y - 1, c->coord.x);
            if (hitsobj(lvl, c, c->coord.y - 1, c->coord.x)->type == DOOR) north = NONE;

            int south= por_col(c->coord.y + 1, c->coord.x);
            if (hitsobj(lvl, c,c->coord.y + 1, c->coord.x)->type == DOOR) south = NONE;

            int yback= por_col(c->coord.y - c->d.y, c->coord.x);
            if (lvl.map[c->coord.y - c->d.y][c->coord.x] == FFIELD) yback = NONSTICK;

            int xback= por_col(c->coord.y, c->coord.x - c->d.x);
            if (lvl.map[c->coord.y][c->coord.x - c->d.x] == FFIELD) xback = NONSTICK;

            if (east && west && yback == NONE) {
              c->d.x = 0; c->d.y = -c->d.y;
            } else if (south && north && xback == NONE) {
              c->d.y = 0; c->d.x = -c->d.x;
            } else {
#ifndef __NOSOUND__
              play_sound (PORTALFAIL);
#endif
              c->d.y = CharData[c->type][3];
              lvl.objm.shots[c->type - SHOT1] = NULLOBJ;
              c->type = FLASH;
              z = portalspeed; continue;
            }
            if (lvl.objm.portals[por] != NULLOBJ) {
              objiter d = lvl.objm.portals[por];
              objiter in_por = hitsobj(lvl, d, d->coord.y, d->coord.x);
              if ((in_por != d) && (in_por->type != PORTAL1) && (in_por->type != PORTAL2)){
                in_por->d.x += d->d.x; // push anything stuck in portal out.
                in_por->d.y += d->d.y; // before collapsing it. FIXME: similar code elsewhere-- remove the redundancy
                if (!move_object(in_por)) {
                  switch_in_portal();
                }
              }
              d->d.y = CharData[d->type][3];
              d->type = FLASH;
            }
            lvl.stats.numportals++;
            animateportal = 2;
#ifndef __NOSOUND__
            play_sound (PORTALCREATE);
#endif
            lvl.objm.shots[c->type - SHOT1] = NULLOBJ;
            lvl.objm.portals[por] = c;
            c->type = PORTAL1 + por;
            lvl.aimobject.type = SHOT1 + ((por + 1) % 2);
            z = portalspeed;
          } else {
            lvl.objm.moveobj(c, c->coord.x + c->d.x, c->coord.y + c->d.y);
          }
        }
        continue;
      case PLAYER: // gravity
        if (move_player() < 0) return -1;
        if (lvl.map[c->coord.y + 1][c->coord.x + c->d.x] == LADDER) break;
        if (lvl.map[c->coord.y + 1][c->coord.x] == LADDER) break;
        if (lvl.map[c->coord.y][c->coord.x] == LADDER) break;
        if (lvl.map[c->coord.y + sc(c->d.y)][c->coord.x + sc(c->d.x)] == LADDER) break;
      case BOULDER: {
        if ((lvl.map[c->coord.y + 1][c->coord.x] == LADDER) && (rand() < RAND_MAX / 2)) break;
      }
      case BOX:
        if ((c->d.x > 2) || (c->d.x < -2) || (c->d.y < -2)) break; // fling!
        if (++(c->d.y) > maxfling) c->d.y = maxfling;
      default:
        break;
    } // End every tick movements.

    if (!(lvl.ticks % beatspermove)) { // X-axis automovements
      switch (c->type) {
        case BOULDER:
          if (c->d.x == 0) c->d.x = (rand() < RAND_MAX / 2) ? 1 : -1;
          break;
        case PLAYER:
        case BOX:
          switch (lvl.map[c->coord.y + 1][c->coord.x]) {
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
        if (lvl.map[c->coord.y][c->coord.x] == LADDER) {
          c->d.y = 0;
          c->d.x = 0;
        }
      case BOULDER:
      case BOX: {
        switch (lvl.map[c->coord.y][c->coord.x]) {
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
              lvl.objm.player = NULLOBJ;
            c->type = FLASH;
        }
      }
    }
  }
  return 0;
}

int Game::move_player () {
  int input;
  XY d;

  d.x = d.y = 0;
  input = getinput();
  if (input == ERR) {
    return 1;
  }
  switch (input) {
    case '1' : lvl.aimobject.d.y =  1; lvl.aimobject.d.x = -1; break;
    case '2' : lvl.aimobject.d.y =  1; lvl.aimobject.d.x =  0; break;
    case '3' : lvl.aimobject.d.y =  1; lvl.aimobject.d.x =  1; break;
    case '4' : lvl.aimobject.d.y =  0; lvl.aimobject.d.x = -1; break;
    case '6' : lvl.aimobject.d.y =  0; lvl.aimobject.d.x =  1; break;
    case '7' : lvl.aimobject.d.y = -1; lvl.aimobject.d.x = -1; break;
    case '8' : lvl.aimobject.d.y = -1; lvl.aimobject.d.x =  0; break;
    case '9' : lvl.aimobject.d.y = -1; lvl.aimobject.d.x =  1; break;
    case '0' :
    case 'Z' :
    case 'z' : lvl.aimobject.type = SHOT1; fireportal(lvl.aimobject.type); break;
    case '.' :
    case 'X' :
    case 'x' : lvl.aimobject.type = SHOT2; fireportal(lvl.aimobject.type); break;
    case 'C' :
    case 'c' : collapse_portals (); break;
    case '5' :
    case '\n':
    case ' ' :
      fireportal(lvl.aimobject.type);
      break;
    case '?' : help_menu (); break;
    case 27 : // ASCII for escape
    case KEY_F(1) : pause = true; break;
    case 'P' :
    case 'p' : light_pause = true; break;
    case KEY_F(2) :
      cheatview = (cheatview + 1) % 4;
      switch (cheatview) {
        case 0: set_status("No Portal Edges view mode."); break;
        case 1: set_status("Portal Fill view mode"); break;
        case 2: set_status("Portal Edges view mode."); break;
        case 3: set_status("Portal Mixed Edges view mode."); break;
      }
      break;
    case KEY_F(3) :
      set_status("Default speed restored");
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
        case 1 : set_status("Very slow speed"); break;
        case 2 : set_status("Slow speed"); break;
        case 3 : set_status("Normal speed"); break;
        case 4 : set_status("Fast speed"); break;
        case 5 : set_status("Very fast speed"); break;
      }
      break;
    case KEY_F(10) :
      set_status("Roguelike-ish speed mode");
      gamespeed = 0;
      break;
#ifndef __NOSOUND__
    case 'M' :
    case 'm' : set_status("Toggling music");
    toggle_ambience (); break;
#endif
    case 'R' :
    case 'r' :
      lvl.objm.player->type = NONE;
      lvl.objm.player = NULLOBJ;
      break;
#ifdef PDCURSES
    case KEY_RESIZE : resize_term(0,0); break;
#endif
    case 'd' :
    case 'D' :
    case KEY_RIGHT :
      if (lvl.aimobject.d.x == 1) {
        if (lvl.objm.player->d.x <= 0) lvl.objm.player->d.x ++;
        lvl.stats.numsteps++;
      } else {
        lvl.aimobject.coord.x = lvl.aimobject.d.x = 1;
      }
      lvl.aimobject.d.y = 0;
      break;
    case 'a' :
    case 'A' :
    case KEY_LEFT :
      if (lvl.aimobject.d.x == -1) {
        if (lvl.objm.player->d.x >= 0) lvl.objm.player->d.x --;
        lvl.stats.numsteps++;
      } else {
        lvl.aimobject.coord.x = lvl.aimobject.d.x = -1;
      }
      lvl.aimobject.d.y = 0;
      break;
    case 'w' :
    case 'W' :
    case KEY_UP :
      if ((lvl.map[lvl.objm.player->coord.y][lvl.objm.player->coord.x] == LADDER)
          && ((!hitswall(lvl, lvl.objm.player->coord.y - 1, lvl.objm.player->coord.x)
               || (hitsobj(lvl, lvl.objm.player, lvl.objm.player->coord.y - 1, lvl.objm.player->coord.x)->type == PORTAL1)
               || (hitsobj(lvl, lvl.objm.player, lvl.objm.player->coord.y - 1, lvl.objm.player->coord.x)->type == PORTAL2))))
        {
          if (lvl.objm.player->d.y >= 0) lvl.objm.player->d.y --;
          lvl.stats.numsteps++;
        } else
        if ((lvl.aimobject.d.x == 0) && (lvl.aimobject.d.y > 0)) {
          lvl.aimobject.d.x = lvl.aimobject.coord.x;
        } else {
          lvl.aimobject.d.y--;
          if (lvl.aimobject.d.y < -1) {
            lvl.aimobject.d.y = -1;
            lvl.aimobject.d.x = 0;
          }
        }
      break;
    case 's' :
    case 'S' :
    case KEY_DOWN :
      if (((lvl.map[lvl.objm.player->coord.y][lvl.objm.player->coord.x] == LADDER)
      || (lvl.map[lvl.objm.player->coord.y + 1][lvl.objm.player->coord.x] == LADDER))
          && ((!hitswall(lvl, lvl.objm.player->coord.y + 1, lvl.objm.player->coord.x) || (hitsobj(lvl, lvl.objm.player, lvl.objm.player->coord.y + 1, lvl.objm.player->coord.x)->type == PORTAL1) || (hitsobj(lvl, lvl.objm.player, lvl.objm.player->coord.y + 1, lvl.objm.player->coord.x)->type == PORTAL2))))
      {
        lvl.objm.player->d.y ++;
        lvl.stats.numsteps++;
      } else if ((lvl.aimobject.d.x == 0) && (lvl.aimobject.d.y < 0)) {
        lvl.aimobject.d.x = lvl.aimobject.coord.x;
      } else {
        lvl.aimobject.d.y++;
        if (lvl.aimobject.d.y > 1) {
          lvl.aimobject.d.y = 1;
          lvl.aimobject.d.x = 0;
        }
      }
      break;
  }
//  flushinp ();
  return 1;
}

void light_pause(MapPack &mappack) {
  mappack.lvl.pager.set_status(mappack.lvl.ticks, "Game paused (press 'p' to resume)");
  mappack.lvl.pager.print_status(mappack.lvl.ticks);

  refresh();
  restms(10);
  setblocking(true);
  int input;
  do {
    input = getinput();
  } while (input != 'p' && input != 'P');
  flushinput();
  setblocking(false);

  mappack.lvl.pager.set_status(mappack.lvl.ticks, "Game resumed");
}

// main loop for playing a level
int play_level(MapPack &mappack) {
  unsigned long long int start, stop;
  double seconds;

  level &lvl = mappack.lvl;
  Game game(lvl);

#ifndef __NOSOUND__
  // ambience music
  if (lvl.musicfile.size() == 0)
    default_ambience(lvl.musicid);
  else
    load_ambience(mappack, lvl.musicfile);
  start_ambience();

  if (lvl.has_message)
    play_voice();
#endif

  do { // while still alive and has not yet won

    for (objiter c = lvl.objm.objs.begin(); c != lvl.objm.objs.end(); ) { // clean up NONEs.
      if (c->type == FLASH) {
        c->type = NONE;
        c++;
      }
      else if (c->type == NONE) c = lvl.objm.delobj(c);
      else c++;
    }
 
    start = get_microseconds();

    if (!lvl.objm.verify()) {
      cout << "object manager state is inconsistent!\n";
      lvl.objm.dump();
      return 0;
    }

    if (game.physics() < 0) return 0;
    if (!game.still_alive()) break;
    if (game.light_pause) {
      light_pause(mappack);
      game.light_pause = false;
    }
    if (game.pause) {
      if (pause_menu(mappack) == -1) return 0;
      game.pause = false;
    }
    draw_screen(lvl);
    stop = get_microseconds();

    seconds = ((double)stop - (double)start)/1000000.0;
    if (gamespeed > 0) {
      if (seconds < (1.0 / beatspersecond[gamespeed]))
        restms (((1.0 / beatspersecond[gamespeed]) - seconds) * 1000);
      lvl.ticks++;
    } else { // handles rogue-like time mode (?)
      while (!pollevent()) {
        restms (150);
        draw_screen(lvl);
        refresh();
      }
      lvl.ticks++;
    }

  } while (game.still_alive() && !game.has_won());

  if (!game.still_alive()) return 2;
  if (game.has_won()) return 3;
}


int play(MapPack &mappack) {
  level &lvl = mappack.lvl;

  // Have we finished the map pack yet?
  bool finished = false;
    
#ifndef __NOSOUND__
  default_ambience (0);
  start_ambience ();
#endif
 
  while (!finished) {
    switch (play_level(mappack)) {
    case 0: return 0; break;
    case 2: // We're dead...
#ifndef __NOSOUND__
      beep ();
#endif
      lvl.stats.numdeaths++;
      mappack.save.numticks += lvl.ticks;
      mappack.save.numdeaths++;
      lvl.stats.numportals = 0;
      fillscreen(screenchar(XFIELD)); refresh();
      restms(100);
      fillscreen(CharData[XFIELD][2] | color_pair(XFIELD) | WA_ALTCHARSET); refresh();
      restms(150);
      mappack.reload_level();
      flushinput();
      break;
    case 3: // We've won!
#ifndef __NOSOUND__
      play_sound(WIN);
#endif
      lvl.stats.numticks = lvl.ticks;
      mappack.update_stats();
      mappack.set_maxlevel(mappack.get_currentlevel());
      // if the user wants to quit now, he will restart with the next level
      mappack.set_lastlevel(mappack.get_currentlevel() + 1);
      if (displaystats(lvl)) {
        if (mappack.get_currentlevel() == mappack.get_number_maps())
          finished = true;
        ++mappack;
      } else
        mappack.reload_level();
      break;
    } // switch
  } // while (!finished)

  roll_credits(mappack);
  return 1;
}

unsigned long long int get_microseconds() {
  struct timeval t;
  gettimeofday( &t, 0 );
  return t.tv_sec * 1000000 + t.tv_usec;
}

