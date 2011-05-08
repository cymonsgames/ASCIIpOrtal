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

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>
#include <climits>
using namespace std;
#include "asciiportal.h"
#ifndef __NOSDL__
#include "SDL/SDL.h"
#include "sdl1/pdcsdl.h"
#endif
#include <curses.h>
#ifndef __NOSOUND__
#include "ap_sound.h"
#endif
#include "menu.h"
#include "ap_play.h"
#include "ap_draw.h"

extern const int CharData [MAXColors][5] = // File, Screen, Forground, background, A_BOLD
{{' ', ' ', COLOR_WHITE, COLOR_BLACK, 0}, // NONE
 {'Q', 'E', COLOR_WHITE, COLOR_GREEN, 1}, // GOAL
 {'+', 254, COLOR_BLACK, COLOR_YELLOW, 0}, // LADDER
 {'#', 219, COLOR_WHITE, COLOR_BLACK, 0}, // NORMAL
 {'N', 219, COLOR_BLACK, COLOR_WHITE, 1}, // NONSTICK
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
 {0, 220 | A_PROTECT, COLOR_BLACK, COLOR_GREEN, 1}, // SWITCHON
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

extern vector<vector<int> > map;
extern ObjectManager objm;
extern object aimobject;
extern string lvlname;
extern int ticks;

int cheatview = 2;
int animateportal = 0;
string statusmsg;
string scrolling;
int statustick;
int scrolltick;

int color_pair(int objtype) {
  int fore = CharData[objtype][2];
  int back = CharData[objtype][3];
  return (COLOR_PAIR ((8 * fore) + back) | (A_BOLD * CharData[objtype][4]));
}

int obj_color_pair (objiter obj) {
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
  if (map.size()) {
    if ((obj->type == PLAYER) || (obj->type == BOULDER) || (obj->type == FLASH)) {
      int mapblock = map[obj->coord.y][obj->coord.x];
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
  return (COLOR_PAIR ((8 * fore) + back) | (A_BOLD * b));
}

void fillscreen (int ch) {
  XY screen;
  for (screen.y = 0; screen.y < LINES; screen.y++)
    for (screen.x = 0; screen.x < COLS; screen.x++)
      mvaddch (screen.y, screen.x, ch);
}

int displaystats (statstype stats, int level) {
  attrset (color_pair(HELPMENU));
  XY upperleft;
  upperleft.y = (LINES - 13) / 2;
  if (lvlname.length() > 22) {
    upperleft.x = (COLS - lvlname.length() - 2) / 2;
    fillsquare(upperleft.y, upperleft.x, 13, lvlname.length() + 2);
  } else {
    upperleft.x = (COLS - 24) / 2;
    fillsquare(upperleft.y, upperleft.x, 13, 24);
  }
  mvprintw (upperleft.y + 1, (COLS - 18) / 2, "Level %d Complete!", level + 1);
  mvprintw (upperleft.y + 2, (COLS - lvlname.length()) / 2, "%s",lvlname.c_str());

  mvprintw (upperleft.y + 4, (COLS - 18) / 2, "%d Game Beats", stats.numticks);
  mvprintw (upperleft.y + 5, (COLS - 18) / 2, "%d Steps Taken", stats.numsteps);
  mvprintw (upperleft.y + 6, (COLS - 18) / 2, "%d Portals Used", stats.numportals);
  mvprintw (upperleft.y + 7, (COLS - 18) / 2, "%d Deaths/Restarts", stats.numdeaths);

  mvprintw (upperleft.y + 10, (COLS - 16) / 2, "Press R to Retry");
  mvprintw (upperleft.y + 11, (COLS - 19) / 2, "Any Key to Continue");

  refresh ();
  napms (500);
  while (!(getch() == ERR));
  nodelay(stdscr,0);
  switch (getch()) {
    case 'R' : case 'r':
      nodelay(stdscr,1);
      return 0;
    default :
      nodelay(stdscr,1);
      return 1;
  }
}

void statusmessage (string msg) {
  statustick = ticks + 30;
  statusmsg = msg;
}

void startscrollmessage (string msg) {
  scrolltick = 0;
  scrolling = msg;
}

void stopmessages () {
  scrolling.clear();
  statusmsg.clear();
}

void scrollmessage () {

  if (scrolling.size() && (scrolltick < COLS + (signed)scrolling.size())) {
    int start = COLS - scrolltick;
    int substart = 0;
    int subend = scrolling.size();

    if (start < 0) substart = scrolltick - COLS;
    if (scrolltick < (signed)scrolling.size()) subend = scrolltick;
    if (subend > COLS) subend = COLS;
    attrset(color_pair(NONE) | A_BOLD);
    mvprintw(0, (start < 0) ? 0 : start, "%s", scrolling.substr(substart, subend).c_str());
  }
  scrolltick ++;
}

inline objiter object_at (XY coord) {
  objiter retval = objm.NULLOBJ;
  objset *s = &(objm.objmap[coord.y][coord.x]);
  for (objset::iterator it = s->begin(); it != s->end(); it++) {
    switch ((*it)->type) {
      case NONE:
      case TEXTTRIGGER:
        break;
      case DOOR:
        if ((*it)->d.x > 0 && retval == objm.NULLOBJ)
          retval = *it;
        break;
      case PORTAL1:
      case PORTAL2:
        if (retval == objm.NULLOBJ)
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
  return (CharData[o][1] | color_pair(o) | A_ALTCHARSET);
}

int obj_screenchar(objiter obj) {
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
      return (4 | obj_color_pair(obj) | A_ALTCHARSET);
  }
  if (objtype == SWITCHON && obj->d.y >= 6) objtype = SWITCH;
  else if (objtype == SWITCH && obj->d.y >= 6) objtype = SWITCHON;
  return (CharData[objtype][1] | obj_color_pair(obj) | A_ALTCHARSET);
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
inline void portal_space (XY& s, XY& d, int type, int& zmax, int& wmin, int& wmax) {
  int xmax = type ? (COLS - 1)  : (map[0].size() - 1);
  int ymax = type ? (LINES - 1) : (map.size() - 1);
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

void map_screen (vector<vector<chtype> >& screenmap) {
  XY upperleft, screenc;
  screenmap.clear();
  upperleft.x = objm.player->coord.x - (COLS/2);
  upperleft.y = objm.player->coord.y - (LINES/2);
  for (screenc.y = 0; screenc.y < LINES; screenc.y++) {
    vector<chtype> screenline(COLS, screenchar(NONSTICK));
    XY mapc;
    mapc.y = upperleft.y + screenc.y;
    if (mapc.y >= 0 && mapc.y < map.size()) {
      int sxa = (upperleft.x < 0) ? -upperleft.x : 0;
      int sxb = ((upperleft.x + COLS) > map[0].size()) ? (map[0].size() - upperleft.x) : COLS;
      for (screenc.x = sxa; screenc.x < sxb; screenc.x++) {
        mapc.x = upperleft.x + screenc.x;
        objiter c_obj = object_at (mapc);
        screenline[screenc.x] = (c_obj == objm.NULLOBJ) ? screenchar(map[mapc.y][mapc.x]) : obj_screenchar(c_obj);
      }
    }
    screenmap.push_back(screenline);
  }

  XY sight = { objm.player->coord.x + aimobject.d.x, objm.player->coord.y + aimobject.d.y };
  if (object_at (sight) == objm.NULLOBJ) {
    screenc.x = (COLS/2)  + aimobject.d.x;
    screenc.y = (LINES/2) + aimobject.d.y;
    if (screenmap[screenc.y][screenc.x] == screenchar(NONE))
      screenmap[screenc.y][screenc.x] = 250 | color_pair(aimobject.type) | A_ALTCHARSET;
    else
      screenmap[screenc.y][screenc.x] = (screenmap[screenc.y][screenc.x] & ~A_COLOR) | color_pair(aimobject.type);
  }

  if (objm.portals[0] == objm.NULLOBJ || objm.portals[1] == objm.NULLOBJ)
    return;

  // look into portals
  for (int i = 0; i < 2; i++) {
    objiter u = objm.portals[i];
    objiter v = objm.portals[i^1];
    XY us = { u->coord.x - upperleft.x, u->coord.y - upperleft.y };
    if (us.x < 0 || us.x >= COLS || us.y < 0 || us.y >= LINES)
      continue;

    // fill portal u with any object in portal v
    objiter check = hitsobj(v, v->coord.y, v->coord.x);
    if (check != v)
      screenmap[us.y][us.x] = screenchar(check->type);

    XY ud = u->d;
    int a = ((COLS/2) - us.x) * -ud.y + ((LINES/2) - us.y) * ud.x;
    int b = ((COLS/2) - us.x) *  ud.x + ((LINES/2) - us.y) * ud.y;

    while (b > 0 || ((COLS/2) == us.x && (LINES/2) == us.y)) {
      int zmax, wmin, wmax, vzmax, vwmin, vwmax;
      portal_space (us, ud, 1, zmax, wmin, wmax);
      portal_space (v->coord, v->d, 0, vzmax, vwmin, vwmax);
      for (int z = 1; z <= zmax; z++) {
        int w1 = b ? (-2 + div_gt ((a - 2) * z, b)) : wmin;
        int w2 = b ? ( 2 + div_lt ((a + 2) * z, b)) : wmax;
        if (w1 < wmin) w1 = wmin;
        if (w2 > wmax) w2 = wmax;
        XY screenc = { us.x - z * ud.x + w1 * ud.y, us.y - z * ud.y - w1 * ud.x };
        XY mapc = { v->coord.x + z * v->d.x - w1 * v->d.y, v->coord.y + z * v->d.y + w1 * v->d.x };
        for (int w = w1; w <= w2; w++) {
          if (z<=vzmax && w>=vwmin && w<=vwmax) {
            objiter c_obj = object_at (mapc);
            screenmap[screenc.y][screenc.x] = (c_obj == objm.NULLOBJ) ? screenchar(map[mapc.y][mapc.x]) : obj_screenchar(c_obj);
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

void draw_screen_angle (int angle, vector<vector<chtype> >& screenmap) {
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
    for (int y = 0; y < LINES; y++)
      mvaddchnstr(y, 0, &angledmap[y][0], COLS);
  }
  else {
    for (int y = 0; y < LINES; y++)
      mvaddchnstr(y, 0, &screenmap[y][0], COLS);
    scrollmessage();
  }

  if (animateportal) animateportal--;
  attrset(color_pair(NONE));
  if (statustick > ticks)
    mvprintw(LINES - 1, 0, "%s", statusmsg.c_str());
  if (lvlname.size() && (ticks < 200))
    mvprintw(LINES - 1, COLS - lvlname.size(), "%s", lvlname.c_str());
  refresh();
}

void draw_screen () {
  vector<vector<chtype> > screenmap;
  map_screen (screenmap);
  draw_screen_angle (0, screenmap);
}

void draw_rotate (int num) { // num is the number of 90 degree rotations necessary;
  int step = 3;
  if (num > 2 || ((num == 2) && (rand() % 2)))
    step = -3;

  vector<vector<chtype> > screenmap;
  map_screen (screenmap);
  for (int angle = step + 90 * (4 - num); (angle > 0) && (angle < 360); angle += step) {
    draw_screen_angle (angle, screenmap);
    napms(5);
  }
}

void graphics_init (int def, int fullscreen, int height, int width, string font) {
#ifndef __NOSDL__
#ifdef __GP2X__
  pdc_screen = SDL_SetVideoMode(320, 200, 16, SDL_SWSURFACE | (SDL_FULLSCREEN * fullscreen));
#else
#ifdef __dingoo__
  pdc_screen = SDL_SetVideoMode(320, 200, 16, SDL_SWSURFACE | (SDL_FULLSCREEN * fullscreen));
#else
  if (!def) {
    SDL_Init( SDL_INIT_EVERYTHING );
    if (fullscreen) width = height = 0; // use current resolution
    pdc_screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE | (SDL_FULLSCREEN * fullscreen));
  }
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
  PDC_set_title("Cymon's Games - ASCIIPortal " __DATE__);
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
