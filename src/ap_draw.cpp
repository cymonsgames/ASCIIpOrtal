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

#include <cstdlib>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>
#include <limits>
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

int CharData [MAXColors][5] = // File, Screen, Forground, background, A_BOLD
{{' ', ' ',COLOR_WHITE, COLOR_BLACK, 0}, // NONE
 {'Q', 'E',COLOR_WHITE, COLOR_GREEN, 1}, // GOAL
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
 {0, 220, COLOR_BLACK, COLOR_GREEN, 1}, // SWITCHON
 {'a', 249, COLOR_BLACK, COLOR_BLUE, 0}, // DOOR
 {0, 254, COLOR_BLUE, COLOR_BLACK, 0}, // DOOR3,
 {0, 249, COLOR_BLUE, COLOR_BLACK, 0}, // DOOR2
 {0, 250, COLOR_BLUE, COLOR_BLACK, 0}, // DOOR1,
 {'&', 3, COLOR_RED, COLOR_CYAN, 0}, // BOX
 {'O', 9, COLOR_MAGENTA, COLOR_BLACK, 1}, // BOULDER
 {'@', 143, COLOR_GREEN, COLOR_BLACK, 1}, // PLAYER
 {'1', ' ', COLOR_WHITE, COLOR_BLACK, 0}, // TEXTTRIGGER
 {0, 15, COLOR_YELLOW, COLOR_BLACK, 0}, // FLASH
 {0, 0, COLOR_WHITE, COLOR_BLACK, 0}, // MAXObj
 {0, 0, COLOR_BLACK, COLOR_GREEN, 0}, // PAUSE
 {0, 0, COLOR_WHITE, COLOR_BLACK, 0}, // MENUDIM
 {0, 0, COLOR_YELLOW, COLOR_BLACK, 1}, // MENUSELECT
 {0, 0, COLOR_YELLOW, COLOR_BLUE, 1}, // HELPMENU
 {0, 0, COLOR_BLACK, COLOR_WHITE, 0} // TEXTFIELD
};

extern vector<vector<int> > map;
extern vector <object> objs;
extern string lvlname;
extern int ticks;

int cheatview = 2;
int animateportal = 0;
string statusmsg;
string scrolling;
int statustick;
int scrolltick;

int color_pair(int obj) {
  int fore = CharData[obj][2];
  int back = CharData[obj][3];
  return (COLOR_PAIR ((8 * fore) + back) | (A_BOLD * CharData[obj][4]));
}

int obj_color_pair (int obj) {
  int fore = CharData[objs[obj].type][2];
  int back = CharData[objs[obj].type][3];
  int b = CharData[objs[obj].type][4];

  if (objs.size()) {
    if (objs[obj].type == DOOR) {
      if (objs[obj].d.x == 4) {
        fore = objs[obj].d.y % 6 + 1;
        if (fore >= COLOR_GREEN) fore++;
      } else {
        fore = CharData[DOOR1][2];
        back = CharData[DOOR1][3];
      }
    }
    if (objs[obj].type == FLASH) {
      fore = objs[obj].d.y;
    }
    if (objs[obj].type == SWITCH) {
      back = objs[obj].d.y % 6 + 1;
      if (back >= COLOR_GREEN) back++;
    }
  }
  if (map.size()) {
    if ((objs[obj].type == PLAYER) || (objs[obj].type == BOULDER) || (objs[obj].type == FLASH)) {
      int mapblock = map[objs[obj].coord.y][objs[obj].coord.x];
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

int findobj(XY coord) { // returns object number of hit object or num favoring portals
  int val = 0;

  for (unsigned int c = 1; c < objs.size(); c++) {
    if ((objs[c].coord.x == coord.x) && (objs[c].coord.y == coord.y)) {
      if (objs[c].type == TEXTTRIGGER) continue;
      if (objs[c].type == DOOR) {
        if ((objs[c].d.x > 0) && (!val)) val = c;
      } else if (objs[c].type != NONE) val = c;
      if ((objs[c].type == PORTAL1) || (objs[c].type == PORTAL2))
        return val;
    }
  }
  return val;
}

int screenchar(int o) {
  return (CharData[o][1] | color_pair(o) | A_ALTCHARSET);
}

int obj_screenchar(int obj) {
  int objtype;

  objtype = objs[obj].type;
  if (objtype == DOOR) {
    switch (objs[obj].d.x) {
      case 3: objtype = DOOR3; break;
      case 2: objtype = DOOR2; break;
      case 1: objtype = DOOR1; break;
      case 0: objtype = NONE; break;
    }
  }
  return (CharData[objtype][1] | obj_color_pair(obj) | A_ALTCHARSET);
}

void map_screen (int player, XY center, XY offset, int angle, vector<vector<int> >& screenmap, int r) {
  while (angle > 360) angle -= 360;
  while (angle < 0) angle += 360;

  double sin_rot, cos_rot;
  XY upperleft, screen, temp, neo, c;
  XY porXY[2]; // gotta remember portal information.
  int portals[2], porvisible[2];

  for (c.y = 0; c.y < (signed)screenmap.size(); c.y++)  // clear the old.
    screenmap[c.y].clear();
  screenmap.clear();

  if (r++ == 2) return;

  upperleft.x = (int)objs[player].coord.x - (COLS / 2);
  upperleft.y = (int)objs[player].coord.y - (LINES / 2);

  sin_rot = sin((double)angle * M_PI / 180.0);
  cos_rot = cos((double)angle * M_PI / 180.0);

  portals[0] = portals[1] = 0;
  porvisible[0] = porvisible[1] = 0;
  for (int p = 1; p < (signed)objs.size(); p++) {  // find the portals
    if (objs[p].type == PORTAL1) portals[0] = p;
    if (objs[p].type == PORTAL2) portals[1] = p;
  }

  for (screen.y = 0; screen.y < LINES; screen.y++) {
    vector<int> screenline(COLS, screenchar(NONSTICK));
    for (screen.x = 0; screen.x < COLS; screen.x++) {
      temp.x = upperleft.x + screen.x - center.x;
      temp.y = upperleft.y + screen.y - center.y;
      neo.x = round((double)temp.x * cos_rot - (double)temp.y * sin_rot);
      neo.y = round((double)temp.x * sin_rot + (double)temp.y * cos_rot);
      neo.x += offset.x; neo.y += offset.y;
      if ((neo.x < (signed)map[0].size()) && (neo.x >= 0) && (neo.y < (signed)map.size()) && (neo.y >= 0)) {
        int c_obj = findobj(neo);
        if (objs[c_obj].type == PORTAL1) {
          porXY[0].x = screen.x; porXY[0].y = screen.y;
        }
        if (objs[c_obj].type == PORTAL2) {
          porXY[1].x = screen.x; porXY[1].y = screen.y;
        }
        int useobj = 0;
        if (c_obj) {
          useobj = 1;
          if (objs[c_obj].type == PORTAL1) porvisible[0] = 1;
          else if (objs[c_obj].type == PORTAL2) { porvisible[1] = 1; useobj = 2; }
          if ((objs[c_obj].type == PORTAL1) || (objs[c_obj].type == PORTAL2)) {
            // Hide portals facing away from player
            if ((screen.x > COLS / 2) && (objs[c_obj].d.x > 0)) {
              porvisible[useobj - 1] = 0;
              useobj = 0;
            }
            if ((screen.x < COLS / 2) && (objs[c_obj].d.x < 0)) {
              porvisible[useobj - 1] = 0;
              useobj = 0;
            }
            if ((screen.y > LINES/ 2) && (objs[c_obj].d.y > 0)) {
              porvisible[useobj - 1] = 0;
              useobj = 0;
            }
            if ((screen.y < LINES/ 2) && (objs[c_obj].d.y < 0))  {
              porvisible[useobj - 1] = 0;
              useobj = 0;
            }
          }
        }
        if (useobj) {
          screenline[screen.x] = obj_screenchar(c_obj);
        } else screenline[screen.x] = screenchar(map[neo.y][neo.x]);
      }
    }
    screenmap.push_back(screenline);
  }

  if (portals[0] && portals[1]) { // look into portals
    for (int p = 0; p < 2; p++) {
      int check = objs[hitsobj(portals[p], objs[portals[p]].coord.y, objs[portals[p]].coord.x)].type;
      if ((check != PORTAL1) && (check != PORTAL2)) { // fill portal with any object in the portals
        if (porvisible[0])
          screenmap[porXY[0].y][porXY[0].x] = screenchar(check);
        if (porvisible[1])
          screenmap[porXY[1].y][porXY[1].x] = screenchar(check);
      }
      if (porvisible[p]) {
        vector<vector<int> > portalmap;
        int porangle = 180 - round((atan2(objs[portals[p]].d.y,objs[portals[p]].d.x)
          - atan2(objs[portals[(p+1)%2]].d.y,objs[portals[(p+1)%2]].d.x)) * 180.0 / M_PI);
        map_screen (player, objs[portals[p]].coord, objs[portals[(p+1)%2]].coord, angle + porangle, portalmap, r);
        if (portalmap.size()) {
          double theta = (angle * M_PI / 180) + atan2(objs[portals[p]].d.y, objs[portals[p]].d.x);
          double m = 1.0 / tan(theta);
          if (m == numeric_limits<float>::infinity()) m = 8165889364191922.00; // to avoid infinite slope
          float mxb = m * (double)porXY[p].x - porXY[p].y;
          int greater = 0;
          if ((angle > 90) && (angle < 270)) {
            if (mxb < (m * (double)((COLS/2) + objs[portals[p]].d.x) - (LINES / 2) - objs[portals[p]].d.y))
              greater = 1;
          } else {
            if (mxb > (m * (double)((COLS/2) + objs[portals[p]].d.x) - (LINES / 2) - objs[portals[p]].d.y))
              greater = 1;
          }
          for (screen.y = 0; screen.y < (signed)portalmap.size(); screen.y++)
            for (screen.x = 0; screen.x < (signed)portalmap[0].size(); screen.x++) {
              if (greater) {
                if ((m * (double)screen.x) - screen.y <= mxb)
                  portalmap[screen.y][screen.x] = MAXWall;
              } else {
                if ((m * (double)screen.x) - screen.y >= mxb)
                  portalmap[screen.y][screen.x] = MAXWall;
              }
            }
          // cut out edges of wedges with linear algebra!
          double m1, m2, mxb1, mxb2, denom;

          if ((objs[player].coord.x != objs[portals[p]].coord.x)
            || (objs[player].coord.y != objs[portals[p]].coord.y)) {

            denom = objs[player].coord.x - (objs[portals[p]].coord.x + 2 * abs(objs[portals[p]].d.y));
            if (abs(denom) < 0.0001) m1 = 8165889364191922.00;
            else m1 = (objs[player].coord.y - (objs[portals[p]].coord.y - 2 * abs(objs[portals[p]].d.x))) / denom;
            denom = objs[player].coord.x - (objs[portals[p]].coord.x - 2 * abs(objs[portals[p]].d.y));
            if (abs(denom) < 0.0001) m2 = 8165889364191922.00;
            else m2 = (objs[player].coord.y - (objs[portals[p]].coord.y + 2 * abs(objs[portals[p]].d.x))) / denom;

            if (abs(m1 - m2) < 0.0001) continue;

            int greater1, greater2;
            mxb1 = m1 * (double)(COLS / 2) - LINES / 2;
            mxb2 = m2 * (double)(COLS / 2) - LINES / 2;

            if (mxb1 > (m1 * porXY[p].x - porXY[p].y)) greater1 = 1;
            else greater1 = 0;
            if (mxb2 > (m2 * porXY[p].x - porXY[p].y)) greater2 = 1;
            else greater2 = 0;

            for (screen.y = 0; screen.y < (signed)portalmap.size(); screen.y++)
              for (screen.x = 0; screen.x < (signed)portalmap[0].size(); screen.x++) {
                if (greater1 && greater2) {
                  if ((mxb1 <= m1 * (double)screen.x - screen.y)
                    || (mxb2 <= m2 * (double)screen.x - screen.y))
                    portalmap[screen.y][screen.x] = MAXWall;
                }
                if (greater1 && !greater2) {
                  if ((mxb1 <= m1 * (double)screen.x - screen.y)
                    || (mxb2 >= m2 * (double)screen.x - screen.y))
                    portalmap[screen.y][screen.x] = MAXWall;
                }
                if (!greater1 && greater2) {
                  if ((mxb1 >= m1 * (double)screen.x - screen.y)
                    || (mxb2 <= m2 * (double)screen.x - screen.y))
                    portalmap[screen.y][screen.x] = MAXWall;
                }
                if (!greater1 && !greater2) {
                  if ((mxb1 >= m1 * (double)screen.x - screen.y)
                    || (mxb2 >= m2 * (double)screen.x - screen.y))
                    portalmap[screen.y][screen.x] = MAXWall;
                }
              }
          }

          for (screen.y = 0; screen.y < (signed)portalmap.size(); screen.y++)
            for (screen.x = 0; screen.x < (signed)portalmap[0].size(); screen.x++) {
              if (porvisible[0] && (screen.y == porXY[0].y) && (screen.x == porXY[0].x)) continue;
              if (porvisible[1] && (screen.y == porXY[1].y) && (screen.x == porXY[1].x)) continue;
              if (portalmap[screen.y][screen.x] == MAXWall) continue;
              screenmap[screen.y][screen.x] = portalmap[screen.y][screen.x];
              if (cheatview || animateportal) {
                int edge = 0;
                if ((cheatview == 1) || (animateportal == 2)) edge = 1;
                if ((screen.y > 0) && (portalmap[screen.y - 1][screen.x] == MAXWall)) edge = 1;
                if ((screen.y < LINES - 1) && (portalmap[screen.y + 1][screen.x] == MAXWall)) edge = 1;
                if ((screen.x > 0) && (portalmap[screen.y][screen.x - 1] == MAXWall)) edge = 1;
                if ((screen.x < COLS - 1) && (portalmap[screen.y][screen.x + 1] == MAXWall)) edge = 1;
                if (edge) {
                  if (screenmap[screen.y][screen.x] == screenchar(NONE))
                    screenmap[screen.y][screen.x] = screenchar(NONSTICK) & 0x00ff | color_pair(PORTAL1 + p) | A_ALTCHARSET;
                  else screenmap[screen.y][screen.x] = screenmap[screen.y][screen.x] & 0x00ff | color_pair(PORTAL1 + p) | A_ALTCHARSET;
                }
              }
            }

          for (c.y = 0; c.y < (signed)portalmap.size(); c.y++)  // clear the old.
            portalmap[c.y].clear();
          portalmap.clear();
        }
      }
    }
  }
}

void draw_screen_angle (int player, int angle) {
  vector<vector<int> > screenmap;
  XY screen;

  fillscreen (' ');

  map_screen (player, objs[player].coord, objs[player].coord, angle, screenmap, 0);
  for (screen.y = 0; screen.y < (signed)screenmap.size(); screen.y++)
    for (screen.x = 0; screen.x < (signed)screenmap[0].size(); screen.x++) {
      move(screen.y, screen.x);
      int mapchar = screenmap[screen.y][screen.x];
      addch(mapchar);
    }

  // drawing aiming sight
  XY sight;
  sight.x = objs[player].coord.x + objs[0].d.x;
  sight.y = objs[player].coord.y + objs[0].d.y;
  if (!findobj(sight)) {
    screen.x = (COLS / 2) + objs[0].d.x;
    screen.y = (LINES / 2)+ objs[0].d.y;
    move (screen.y, screen.x);
    attrset(color_pair(objs[0].type));
    if (screenmap[screen.y][screen.x] == screenchar(NONE))
      addch (250 | A_ALTCHARSET);
    else {
      int targetblock = screenmap[screen.y][screen.x] & 0x00ff;
      addch (targetblock | A_ALTCHARSET);
    }
  }
  mvaddch (LINES / 2, COLS / 2, obj_screenchar(player));
  if (animateportal) animateportal--;
  attrset(color_pair(NONE));
  if (statustick > ticks) mvprintw(LINES - 1, 0, "%s", statusmsg.c_str());
  if (lvlname.size() && (ticks < 200)) {
    mvprintw(LINES - 1, COLS - lvlname.size(), "%s", lvlname.c_str());
  }
  scrollmessage();
  refresh();
}

void draw_screen (int player) {
  draw_screen_angle (player, 0);
}

void draw_rotate (int player, int num) { // num is the number of 90 degree rotations necessary;
  int step = 3;
  if (num > 2 || ((num == 2) && (rand() % 2)))
    step = -3;

  for (int angle = step + 90 * (4 - num); (angle > 0) && (angle < 360); angle += step) {
    draw_screen_angle (player, angle);
    napms(10);
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
  SDL_Quit();
#endif
}
