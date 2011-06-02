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

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#ifdef __NOSDL__
// Used for 'transform' to translate cp437 characters to unicode
#include <algorithm>
#endif

#include <cmath>
#include <ctime>
#include <climits>

using namespace std;

#include "ap_draw.h"

#ifndef __NOSDL__
#include "SDL/SDL.h"
#include "pdcsdl.h"
#endif

#include <curses.h>

#ifndef __NOSOUND__
#include "ap_sound.h"
#endif

#include "menu.h"
#include "ap_play.h"
#include "ap_object.h"
#include "ap_pager.h"


extern const int CharData [MAXColors][5] = // File, Screen, Forground, background, WA_BOLD
{{' ', ' ', COLOR_WHITE, COLOR_BLACK, 0}, // NONE
 {'Q', 'E', COLOR_WHITE, COLOR_GREEN, 1}, // GOAL
 {'+', 254, COLOR_BLACK, COLOR_YELLOW, 0}, // LADDER
 {'#', 219, COLOR_WHITE, COLOR_BLACK, 0}, // NORMAL
#ifndef __NOSDL__
 {'N', 219, COLOR_BLACK, COLOR_WHITE, 1}, // NONSTICK
#else
 {'N', 177, COLOR_WHITE, COLOR_BLACK, 0}, // NONSTICK
#endif
 {'(', '(', COLOR_BLACK, COLOR_WHITE, 0}, // LTREAD
 {')', ')', COLOR_BLACK, COLOR_WHITE, 0}, // RTREAD
 {'$', 177, COLOR_BLACK, COLOR_WHITE, 0}, // FFIELD,
 {'%', 177, COLOR_CYAN, COLOR_BLACK, 0}, // PFIELD
 {'"', 177, COLOR_RED, COLOR_BLACK, 1}, // XFIELD
 {'X', 15, COLOR_RED, COLOR_BLACK, 0}, // SPIKE
 {0, 0, COLOR_WHITE, COLOR_WHITE, 0}, // MAXWall,
 {'=', 205, COLOR_BLUE, COLOR_BLACK, 1}, // DUPLICATOR,
 {0, 7, COLOR_BLUE, COLOR_BLACK, 1}, // SHOT1
 {0, 7, COLOR_YELLOW, COLOR_BLACK, 1}, // SHOT2
 {'y', 9, COLOR_BLUE, COLOR_BLACK, 1}, // PORTAL1
 {'z', 9, COLOR_YELLOW, COLOR_BLACK, 1}, // PORTAL2
 {'A', 220, COLOR_BLACK, COLOR_RED, 1}, // SWITCH
 {0, 220 | WA_PROTECT, COLOR_BLACK, COLOR_GREEN, 1}, // SWITCHON
 {'a', 30, COLOR_BLACK, COLOR_BLUE, 0}, // DOOR
 {0, 254, COLOR_BLUE, COLOR_BLACK, 0}, // DOOR3,
 {0, 249, COLOR_BLUE, COLOR_BLACK, 0}, // DOOR2
 {0, 250, COLOR_BLUE, COLOR_BLACK, 0}, // DOOR1,
 {'&', 3, COLOR_RED, COLOR_CYAN, 0}, // BOX
 {'O', 9, COLOR_MAGENTA, COLOR_BLACK, 1}, // BOULDER
 {'@', 143, COLOR_GREEN, COLOR_BLACK, 1}, // PLAYER
 {'1', ' ', COLOR_WHITE, COLOR_BLACK, 0}, // TEXTTRIGGER
 {0, 15, COLOR_YELLOW, COLOR_BLACK, 0}, // FLASH
 {0, 0, COLOR_WHITE, COLOR_BLACK, 0}, // MAXObj
 {0, 0, COLOR_GREEN, COLOR_BLACK, 0}, // PAUSE
 {0, 0, COLOR_WHITE, COLOR_BLACK, 0}, // MENUDIM
 {0, 0, COLOR_YELLOW, COLOR_BLACK, 1}, // MENUSELECT
 {0, 0, COLOR_YELLOW, COLOR_BLUE, 1}, // HELPMENU
 {0, 0, COLOR_BLACK, COLOR_WHITE, 0} // TEXTFIELD
};

int cheatview = 2;
int animateportal = 0;

#ifdef __NOSDL__
// Used for standard ncurses build on a unicode terminal
const wchar_t cp437_to_unicode[256] = {
    0x0000, 0x263a, 0x263b, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022, 
    0x25d8, 0x25cb, 0x25d9, 0x2642, 0x2640, 0x266a, 0x266b, 0x263c, 
    0x25ba, 0x25c4, 0x2195, 0x203c, 0x00b6, 0x00a7, 0x25ac, 0x21a8, 
    0x2191, 0x2193, 0x2192, 0x2190, 0x221f, 0x2194, 0x25b2, 0x25bc, 
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x2302, 
    0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7, 
    0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5, 
    0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9, 
    0x00ff, 0x00d6, 0x00dc, 0x00a2, 0x00a3, 0x00a5, 0x20a7, 0x0192, 
    0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba, 
    0x00bf, 0x2310, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb, 
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556, 
    0x2555, 0x2563, 0x2551, 0x2557, 0x255d, 0x255c, 0x255b, 0x2510, 
    0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x255e, 0x255f, 
    0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x2567, 
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b, 
    0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580, 
    0x03b1, 0x00df, 0x0393, 0x03c0, 0x03a3, 0x03c3, 0x00b5, 0x03c4, 
    0x03a6, 0x0398, 0x03a9, 0x03b4, 0x221e, 0x03c6, 0x03b5, 0x2229, 
    0x2261, 0x00b1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00f7, 0x2248, 
    0x00b0, 0x2219, 0x00b7, 0x221a, 0x207f, 0x00b2, 0x25a0, 0x00a0
};

// Converts a cp437 character to its unicode equivalent
cchar_t convert_char(chtype character) {
  cchar_t wch;
  wch.chars[1] = 0;
  wch.attr = character & A_ATTRIBUTES & ~WA_ALTCHARSET;
  wch.chars[0] = cp437_to_unicode[character & 255];

  return wch;
}
#endif //NOSDL

int color_pair(int objtype) {
  int fore = CharData[objtype][2];
  int back = CharData[objtype][3];
  return (COLOR_PAIR ((8 * fore) + back) | (WA_BOLD * CharData[objtype][4]));
}

int obj_color_pair(level const & lvl, objiter obj) {
  int fore = CharData[obj->type][2];
  int back = CharData[obj->type][3];
  int b = CharData[obj->type][4];

  if (obj->type == DOOR) {
    if (obj->d.x == 4) {
      fore = obj->d.y % 6 + 1;
      if (fore >= COLOR_GREEN) fore++;
      if (obj->d.y == 12) fore = COLOR_GREEN;
      if (fore == COLOR_BLUE) b = 1;
    } else {
      fore = CharData[DOOR1][2];
      back = CharData[DOOR1][3];
    }
  }
  if (obj->type == FLASH) {
    fore = obj->d.y;
  }
  if (obj->type == SWITCH || obj->type == SWITCHON) {
    back = obj->d.y % 6 + 1;
    if (back >= COLOR_GREEN) back++;
    if (obj->d.y == 12) back = COLOR_GREEN;
  }
  if (lvl.map.size()) {
    if ((obj->type == PLAYER) || (obj->type == BOULDER) || (obj->type == FLASH)) {
      int mapblock = lvl.map[obj->coord.y][obj->coord.x];
      if ((mapblock != NONE) && (mapblock != NORMAL)) {
        back = CharData[mapblock][2];
        b = 0;
      }
      if (mapblock == LADDER) {
        back = CharData[mapblock][3];
        b = 0;
      }
    }
  }
  return (COLOR_PAIR ((8 * fore) + back) | (WA_BOLD * b));
}

void fillscreen (int ch) {
  XY screen;
  for (screen.y = 0; screen.y < LINES; screen.y++)
    for (screen.x = 0; screen.x < COLS; screen.x++)
      mvaddch (screen.y, screen.x, ch);
}

bool displaystats (level const & lvl) {
  attrset (color_pair(HELPMENU));
  XY upperleft;
  upperleft.y = (LINES - 13) / 2;
  if (lvl.name.length() > 22) {
    upperleft.x = (COLS - lvl.name.length() - 2) / 2;
    fillsquare(upperleft.y, upperleft.x, 13, lvl.name.length() + 2);
  } else {
    upperleft.x = (COLS - 24) / 2;
    fillsquare(upperleft.y, upperleft.x, 13, 24);
  }
  mvprintw (upperleft.y + 1, (COLS - 18) / 2, "Level %d Complete!", lvl.id);
  mvprintw (upperleft.y + 2, (COLS - lvl.name.length()) / 2, "%s",lvl.name.c_str());

  mvprintw (upperleft.y + 4, (COLS - 18) / 2, "%d Game Beats", lvl.stats.numticks);
  mvprintw (upperleft.y + 5, (COLS - 18) / 2, "%d Steps Taken", lvl.stats.numsteps);
  mvprintw (upperleft.y + 6, (COLS - 18) / 2, "%d Portals Used", lvl.stats.numportals);
  mvprintw (upperleft.y + 7, (COLS - 18) / 2, "%d Deaths/Restarts", lvl.stats.numdeaths);

  mvprintw (upperleft.y + 10, (COLS - 16) / 2, "Press R to Retry");
  mvprintw (upperleft.y + 11, (COLS - 19) / 2, "Any Key to Continue");

  refresh ();
  napms (500);
  nodelay(stdscr,0);
  //while (!(getch() == ERR));
  switch (getch()) {
    case 'R' : case 'r':
      nodelay(stdscr,1);
      return 0;
    default :
      nodelay(stdscr,1);
      return 1;
  }
}

inline objiter object_at (level & lvl, XY coord) {
  objiter retval = lvl.objm.NULLOBJ;
  objset *s = &(lvl.objm.objmap[coord.y][coord.x]);
  for (objset::iterator it = s->begin(); it != s->end(); ++it) {
    switch ((*it)->type) {
      case NONE:
      case TEXTTRIGGER:
        break;
      case DOOR:
        if ((*it)->d.x > 0 && retval == lvl.objm.NULLOBJ)
          retval = *it;
        break;
      case PORTAL1:
      case PORTAL2:
        if (retval == lvl.objm.NULLOBJ)
          retval = *it;
        break;
      default:
        retval = *it;
        break;
    }
  }
  return retval;
}

int screenchar(int o) {
  return (CharData[o][1] | color_pair(o) | WA_ALTCHARSET);
}

int obj_screenchar(level const & lvl, objiter obj) {
  int objtype;

  objtype = obj->type;
  if (objtype == DOOR) {
    switch (obj->d.x) {
      case 3: objtype = DOOR3; break;
      case 2: objtype = DOOR2; break;
      case 1: objtype = DOOR1; break;
      case 0: objtype = NONE; break;
    }
    if(objtype == DOOR && obj->d.y >= 6)
      return (4 | obj_color_pair(lvl, obj) | WA_ALTCHARSET);
  }
  if (objtype == SWITCHON && obj->d.y >= 6) objtype = SWITCH;
  else if (objtype == SWITCH && obj->d.y >= 6) objtype = SWITCHON;
  return (CharData[objtype][1] | obj_color_pair(lvl, obj) | WA_ALTCHARSET);
}

// Math library round() is slow for some reason, at least on my system.
// This version is good enough for us.
inline int qround (double x) {
  return (x < 0) ? (x - 0.5) : (x + 0.5);
}

// Fast atan2 for the special case where (x,y) is an integer-valued unit vector.
// Output is in degrees, not radians (duh).
inline int qatan2 (int y, int x) {
  return (x + y + 1) ? (x ? 0 : 90) : (x ? 180 : 270);
}

// greatest integer strictly less than the rational number a/b
inline int div_lt (int a, int b) {
  if (b > 0)
    return ((a > 0) ? (a - 1) : (a - b)) / b;
  else
    return ((a < 0) ? (a + 1) : (a - b)) / b;
}

// least integer strictly greater than the rational number a/b
inline int div_gt (int a, int b) {
  if (b > 0)
    return ((a < 0) ? (a + 1) : (a + b)) / b;
  else
    return ((a > 0) ? (a - 1) : (a + b)) / b;
}

// gets the dimensions of the space behind (type = 1) or in front of (type = 0) a portal
// s = screen (type = 1) or map (type = 0) coords of portal
// d = direction vector of portal
inline void portal_space (level const & lvl, XY& s, XY& d, int type, int& zmax, int& wmin, int& wmax) {
  int xmax = type ? (COLS - 1)  : (lvl.map[0].size() - 1);
  int ymax = type ? (LINES - 1) : (lvl.map.size() - 1);
  int dir  = type ? 1 : -1;
  switch (qatan2 (dir * d.y, dir * d.x)) {
    case 0:
      zmax = s.x;
      wmin = s.y - ymax;
      wmax = s.y;
      break;
    case 90:
      zmax = s.y;
      wmin = -s.x;
      wmax = xmax - s.x;
      break;
    case 180:
      zmax = xmax - s.x;
      wmin = -s.y;
      wmax = ymax - s.y;
      break;
    case 270:
      zmax = ymax - s.y;
      wmin = s.x - xmax;
      wmax = s.x;
      break;
  }
}

// Fills screenmap according to the objects present in the world
void map_screen (level & lvl, vector<vector<chtype> >& screenmap) {
  XY upperleft, screenc;
  screenmap.clear();
  upperleft.x = lvl.objm.player->coord.x - (COLS/2);
  upperleft.y = lvl.objm.player->coord.y - (LINES/2);
  for (screenc.y = 0; screenc.y < LINES; screenc.y++) {
    vector<chtype> screenline(COLS, screenchar(NONSTICK));
    XY mapc;
    mapc.y = upperleft.y + screenc.y;
    if (mapc.y >= 0 && mapc.y < lvl.map.size()) {
      int sxa = (upperleft.x < 0) ? -upperleft.x : 0;
      int sxb = ((upperleft.x + COLS) > lvl.map[0].size()) ? (lvl.map[0].size() - upperleft.x) : COLS;
      for (screenc.x = sxa; screenc.x < sxb; screenc.x++) {
        mapc.x = upperleft.x + screenc.x;
        objiter c_obj = object_at(lvl, mapc);
        screenline[screenc.x] = (c_obj == lvl.objm.NULLOBJ) ? screenchar(lvl.map[mapc.y][mapc.x]) : obj_screenchar(lvl, c_obj);
      }
    }
    screenmap.push_back(screenline);
  }

  XY sight = { lvl.objm.player->coord.x + lvl.aimobject.d.x, lvl.objm.player->coord.y + lvl.aimobject.d.y };
  if (object_at(lvl, sight) == lvl.objm.NULLOBJ) {
    screenc.x = (COLS/2)  + lvl.aimobject.d.x;
    screenc.y = (LINES/2) + lvl.aimobject.d.y;
    if (screenmap[screenc.y][screenc.x] == screenchar(NONE))
#ifndef __NOSDL__
      screenmap[screenc.y][screenc.x] = 250 | color_pair(lvl.aimobject.type) | WA_ALTCHARSET;
#else
    screenmap[screenc.y][screenc.x] = 250 | color_pair(lvl.aimobject.type);
#endif
    else
      screenmap[screenc.y][screenc.x] = (screenmap[screenc.y][screenc.x] & ~A_COLOR) | color_pair(lvl.aimobject.type);
  }

  if (lvl.objm.portals[0] == lvl.objm.NULLOBJ || lvl.objm.portals[1] == lvl.objm.NULLOBJ)
    return;

  // look into portals
  for (int i = 0; i < 2; i++) {
    objiter u = lvl.objm.portals[i];
    objiter v = lvl.objm.portals[i^1];
    XY us = { u->coord.x - upperleft.x, u->coord.y - upperleft.y };
    if (us.x < 0 || us.x >= COLS || us.y < 0 || us.y >= LINES)
      continue;

    // fill portal u with any object in portal v
    objiter check = hitsobj(lvl, v, v->coord.y, v->coord.x);
    if (check != v)
      screenmap[us.y][us.x] = screenchar(check->type);

    XY ud = u->d;
    int a = ((COLS/2) - us.x) * -ud.y + ((LINES/2) - us.y) * ud.x;
    int b = ((COLS/2) - us.x) *  ud.x + ((LINES/2) - us.y) * ud.y;

    while (b > 0 || ((COLS/2) == us.x && (LINES/2) == us.y)) {
      int zmax, wmin, wmax, vzmax, vwmin, vwmax;
      portal_space (lvl, us, ud, 1, zmax, wmin, wmax);
      portal_space (lvl, v->coord, v->d, 0, vzmax, vwmin, vwmax);
      for (int z = 1; z <= zmax; z++) {
        int w1 = b ? (-2 + div_gt ((a - 2) * z, b)) : wmin;
        int w2 = b ? ( 2 + div_lt ((a + 2) * z, b)) : wmax;
        if (w1 < wmin) w1 = wmin;
        if (w2 > wmax) w2 = wmax;
        XY screenc = { us.x - z * ud.x + w1 * ud.y, us.y - z * ud.y - w1 * ud.x };
        XY mapc = { v->coord.x + z * v->d.x - w1 * v->d.y, v->coord.y + z * v->d.y + w1 * v->d.x };
        for (int w = w1; w <= w2; w++) {
          if (z<=vzmax && w>=vwmin && w<=vwmax) {
            objiter c_obj = object_at(lvl, mapc);
            screenmap[screenc.y][screenc.x] = (c_obj == lvl.objm.NULLOBJ) ? screenchar(lvl.map[mapc.y][mapc.x]) : obj_screenchar(lvl, c_obj);
          }
          else
            screenmap[screenc.y][screenc.x] = screenchar(NONSTICK);
          if (cheatview == 1 || animateportal == 2)
            screenmap[screenc.y][screenc.x] = (screenmap[screenc.y][screenc.x] & ~A_COLOR) | color_pair(PORTAL1 + i);
          screenc.x += ud.y;
          screenc.y -= ud.x;
          mapc.x -= v->d.y;
          mapc.y += v->d.x;
        }
      }

#define COLORIZE(w,z) { \
  XY screenc = { us.x - (z) * ud.x + (w) * ud.y, us.y - (z) * ud.y - (w) * ud.x }; \
  if (screenmap[screenc.y][screenc.x] == screenchar(NONE)) screenmap[screenc.y][screenc.x] = screenchar(NONSTICK); \
  screenmap[screenc.y][screenc.x] = (screenmap[screenc.y][screenc.x] & ~A_COLOR) | color_pair(PORTAL1 + i); \
}

      if (cheatview == 2 || animateportal == 1) {
        if (b) {
          if (abs(2-a) <= b) {
            for (int z = 1; z <= zmax; z++) {
              int w = -2 + div_gt ((a - 2) * z, b);
              if (w >= wmin && w <= wmax) COLORIZE(w,z);
            }
          }
          else {
            for (int w = wmin; w <= wmax; w++) {
              int z = (a > 2) ? div_lt (b * (w + 2), a - 2) : div_gt (b * (w + 2), a - 2);
              if (z >= 1 && z <= zmax) COLORIZE(w,z);
            }
          }
          if (abs(2+a) <= b) {
            for (int z = 1; z <= zmax; z++) {
              int w = 2 + div_lt ((a + 2) * z, b);
              if (w >= wmin && w <= wmax) COLORIZE(w,z);
            }
          }
          else {
            for (int w = wmin; w <= wmax; w++) {
              int z = (a < -2) ? div_lt (b * (w - 2), a + 2) : div_gt (b * (w - 2), a + 2);
              if (z >= 1 && z <= zmax) COLORIZE(w,z);
            }
          }
          if (-1 >= wmin) COLORIZE(-1,0);
          if ( 1 <= wmax) COLORIZE( 1,0);
        }
        else {
          for (int w = wmin; w <= wmax; w++)
            if (w) COLORIZE(w,0);
        }
      }

      int z = (u->coord.x - v->coord.x) *  v->d.x + (u->coord.y - v->coord.y) * v->d.y;
      int w = (u->coord.x - v->coord.x) * -v->d.y + (u->coord.y - v->coord.y) * v->d.x;
      if (z > zmax || w < wmin || w > wmax)
        break;
      if ((b*(w+2)-(a-2)*z) <= 0 || (b*(w-2)-(a+2)*z) >= 0)
        break;

      us.x -= z*ud.x - w*ud.y;
      us.y -= z*ud.y + w*ud.x;
      if (check != v)
        screenmap[us.y][us.x] = screenchar(check->type);

      XY temp = { -ud.x * v->d.x - ud.y * v->d.y, 
                   ud.x * v->d.y - ud.y * v->d.x };
      ud.x = temp.x * u->d.x - temp.y * u->d.y;
      ud.y = temp.x * u->d.y + temp.y * u->d.x;

      a = ((COLS/2) - us.x) * -ud.y + ((LINES/2) - us.y) * ud.x;
      b = ((COLS/2) - us.x) *  ud.x + ((LINES/2) - us.y) * ud.y;
    }
  }
}

void draw_map (vector<vector<chtype> > const & map) {
#ifndef __NOSDL__
  for (int y = 0; y < map.size(); ++y)
    mvaddchnstr(y, 0, &map[y][0], map[y].size());
#else
  for (int y = 0; y < map.size(); ++y) {
    // Convert every line to unicode characters
    vector<cchar_t> line;
    line.resize(map[y].size());
    transform(map[y].begin(), map[y].end(), line.begin(), convert_char);

    mvadd_wchnstr(y, 0, &line[0], line.size());
  }
#endif
}

void draw_screen_angle (level & lvl, int angle, vector<vector<chtype> >& screenmap) {
  if (angle) {
    vector<chtype> blankline(COLS, screenchar(NONSTICK));
    vector<vector<chtype> > angledmap(LINES, blankline);
    double c = cos (angle * (M_PI / 180));
    double s = sin (angle * (M_PI / 180));
    int oy = LINES/2;
    int ox = COLS/2;
    for (int y = -oy; y < (LINES - oy); y++) {
      for (int x = -ox; x < (COLS - ox); x++) {
        int nx = qround (c*x - s*y) + ox;
        int ny = qround (s*x + c*y) + oy;
        if (nx >= 0 && nx < COLS && ny >= 0 && ny < LINES)
          angledmap[y+oy][x+ox] = screenmap[ny][nx];
      }
    }
    draw_map (angledmap);
  }
  else {
    draw_map (screenmap);
    lvl.pager.scroll_messages();
  }

  if (animateportal) animateportal--;
  lvl.pager.print_status(lvl.ticks);
  refresh();
}

void draw_screen(level & lvl) {
  vector<vector<chtype> > screenmap;
  map_screen(lvl, screenmap);
  draw_screen_angle (lvl, 0, screenmap);
}

void draw_rotate (level & lvl, int num) { // num is the number of 90 degree rotations necessary;
  int step = 3;
  if (num > 2 || ((num == 2) && (rand() % 2)))
    step = -3;

  vector<vector<chtype> > screenmap;
  map_screen(lvl, screenmap);
  for (int angle = step + 90 * (4 - num); (angle > 0) && (angle < 360); angle += step) {
    draw_screen_angle(lvl, angle, screenmap);
    napms(5);
  }
}

void graphics_init (bool fullscreen, int height, int width) {
#ifndef __NOSDL__
#ifdef __GP2X__
  pdc_screen = SDL_SetVideoMode(320, 200, 16, SDL_SWSURFACE | (SDL_FULLSCREEN * fullscreen));
#else
#ifdef __DINGOO__
  pdc_screen = SDL_SetVideoMode(320, 200, 16, SDL_SWSURFACE | (SDL_FULLSCREEN * fullscreen));
#else
  SDL_Init( SDL_INIT_EVERYTHING );
  if (fullscreen) width = height = 0; // use current resolution
  pdc_screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE | (SDL_FULLSCREEN * fullscreen));
#endif
#endif
  SDL_JoystickOpen( 0 );
#endif

  initscr();

  curs_set (0);
  cbreak ();
  noecho();
  keypad(stdscr,1);
  nodelay(stdscr,1);
  srand (time (NULL));
#ifdef PDCURSES
  PDC_set_title("Cymon's Games - ASCIIPortal " AP_VERSION);
#endif
  start_color();
  for (int d = 0; d < COLORS; d++)
    for (int c = 0; c < COLORS; c++)
      init_pair (c + 8 * d, d, c);
}

void graphics_deinit () {
  endwin ();
#ifndef __NOSDL__
//  SDL_Quit();
#endif
}
