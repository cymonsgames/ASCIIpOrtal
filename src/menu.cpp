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
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <algorithm>

#include "menu.h"

#ifndef __NOSDL__
#include "SDL/SDL.h"
#endif
#ifndef __NOSOUND__
#include "ap_sound.h"
#endif

#include "ap_filemgr.h"
#include "ap_draw.h"
#include "ap_input.h"
#include "ap_play.h"
#include "ap_pager.h"

using namespace std;
extern const int CharData[MAXColors][6];

// from main.cpp
extern FileManager filemgr;


int chartoscreen(char o) {
  for (int c = 0; c < MAXObjects; ++c)
    if (CharData[c][0] == o)
      return screenchar(c);
  return screenchar(NONE);
}

void fillsquare(int y1, int x1, int height, int width) {
  for (int yy = y1; yy < height + y1; yy++)
    for (int xx = x1; xx < width + x1; xx++)
      mvaddch (yy, xx, ' ');
}

void wfillsquare(WINDOW* win, int y1, int x1, int height, int width) {
  for (int yy = y1; yy < height + y1; yy++)
    for (int xx = x1; xx < width + x1; xx++)
      mvwaddch(win, yy, xx, ' ');
}

void help_menu () {
  int maxwidth = 0;
#ifdef __DINGOO__
  int numkeys = 13;
  string keys[] = {
    " How to play ASCIIpOrtal ",
    "=========================",
    "Use the following keys:",
    "Left/Right ........ Move Left/Right",
    "Up/Down ........... Move Up/Down Ladders",
    "                    or adjust aim.",
    "R Shoulder/A....... Fire Blue Portal",
    "L Shoulder/Y....... Fire Yellow Portal",
    "B ................. Fire 'Next' Portal",
    "X ................. Cancel Portals",
    "",
    "Start ............. Menu",
    "Select ............ Portal View Options"
  };
#else
#ifdef __GP2X__
  int numkeys = 12;
  string keys[] = {
    " How to play ASCIIpOrtal ",
    "=========================",
    "Use the following keys:",
    "Left/Right ........ Move Left/Right",
    "Up/Down ........... Move Up/Down Ladders",
    "                    or adjust aim.",
    "L Shoulder/A....... Fire Blue Portal",
    "R Shoulder/B....... Fire Yellow Portal",
    "X/Y ............... Fire 'Next' Portal",
    "",
    "Vol up ............ Game Speed Up",
    "Vol down .......... Game Speed Down",
    "Start ............. Menu",
    "Select ............ Portal View Options"
  };
#else
  int numkeys = 18;
  string keys[] = {
    " How to play ASCIIpOrtal ",
    "=========================",
    "Left/Right Arrows . Move Left/Right",
    "Up/Down Arrows .... Move Up/Down Ladders",
    "                    or adjust aim.",
    "Number Pad Keys ... Quick Aim.",
    "Z/0 ............... Fire Blue Portal",
    "X/PERIOD .......... Fire Yellow Portal",
    "SPACE/5 ........... Fire 'Next' Portal",
    "C ................. Clear Portals",
    "",
    "ESC/F1 ............ Menu",
    "P ................. In-game Pause",
    "F2 ................ Change Portal Render",
    "F3 ................ Set to Default Speed",
    "+ ................. Increase Game Speed",
    "- ................. Decrease Game Speed",
    "R ................. Restart Level",
    "M ................. Toggle music"
  };
#endif
#endif


  setblocking(true);

  for (int yy = 0; yy < numkeys; yy++)
    if ((signed)keys[yy].size() > maxwidth) maxwidth = (signed)keys[yy].size();
  XY upperleft;
  upperleft.y = (LINES - numkeys - 2) / 2; upperleft.x = (COLS - maxwidth) / 2;
  attrset (color_pair(HELPMENU));
  fillsquare(upperleft.y - 1, upperleft.x - 1, numkeys + 5, maxwidth + 2);
  for (int yy = 0; yy < numkeys; yy++)
    mvprintw (upperleft.y + yy, upperleft.x, "%s", keys[yy].c_str());
  mvprintw (upperleft.y + numkeys + 2, upperleft.x, "Press Any Key to Return...");
  refresh();
  restms(100);
  flushinput();
  int input = getinput();
  while (1) {
    switch (input) {
#ifdef PDCURSES
      case KEY_RESIZE: resize_term(0,0); break;
#endif
      case ERR : break;
      default :
        setblocking(false);
        return;
    }
    input = getinput ();
  }

  setblocking(false);
}

int menu (vector <string>& items, int selection) {
  int input;

#ifndef __NOSOUND__
  play_sound(MENUBEEP);
#endif
  setblocking(true);
  XY s;
  int w = 0;
  attrset (color_pair(MENUDIM));
  for (s.y = 0; s.y < (signed)items.size(); s.y++)
    if ((signed)items[s.y].size() > w) w = (signed)items[s.y].size();
  w += 5;

  while (1) {
    fillsquare(LINES - 2 - items.size() * 2, 1, items.size() * 2 + 1, w);
    for (int i = 0; i < (signed)items.size(); ++i) {
      if (i != selection) attrset (color_pair(MENUDIM));
      else attrset (color_pair(MENUSELECT));
      mvprintw (LINES - 2 * (items.size() - i) - 1, 1, " %d) %s ", i + 1, items[i].c_str());
    }
    refresh ();
//    restms(1); // don't know why the menu doesn't want to show up without this. Ah well.
    do { restms(10); input = getinput(); } while (input == ERR);
    if ((input >= '1') && (input < '1' + (signed)items.size())) selection = input - '1';
    flushinput();
    switch (input) {
#ifndef __NOSOUND__
      case 'M' :
      case 'm' : toggle_ambience (); break;
#endif
#ifdef PDCURSES
      case KEY_RESIZE: resize_term(0,0); break;
#endif
      case KEY_UP:
        selection--;
        if (selection < 0) selection = items.size() - 1;
#ifndef __NOSOUND__
        play_sound(MENUBEEP);
#endif
        break;
      case KEY_DOWN:
        selection++;
#ifndef __NOSOUND__
        play_sound(MENUBEEP);
#endif
        break;
      case 27: // ESC key
#ifndef __NOSOUND__
        play_sound(MENUCHOICE);
#endif
        setblocking(false);
        return -1;
        break;
      case KEY_ENTER:
      case KEY_RIGHT:
      case '\n':
      case ' ':
#ifndef __NOSOUND__
        play_sound(MENUCHOICE);
#endif
        setblocking(false);
        return selection;
    }
    selection %= items.size();
  }
}

int display_main_menu (MapPack &mappack) {
  string line, name;
  int maxwidth = 0;
  int input;
  vector<string> inscreen;
  vector<string> options;
  Pager pager;

#ifndef __NOSOUND__
  load_ambience ("menumusic.ogg");
#endif

  // first load the 'inscreen' file
  name = mappack.filemgr.get_inscreen();
  ifstream mapfile(name.c_str());

  if (mapfile.is_open()) {
    vector<string> map;
    while (! mapfile.eof() ) {
      getline (mapfile, line);
      if ((signed)line.length() > maxwidth) maxwidth = (signed)line.length();
      if (line.find("music") == 0) {
#ifndef __NOSOUND__
        // 'mappack' isn't really suitable here, since we might well be
        // reading the default 'inscreen' file...
        // Anyway, load_ambience is clever enough to find the music file.
        load_ambience (mappack, line.substr (6, line.size() - 6));
#endif
      }
      if (line.find("message") == 0)
        pager.add_scrolling(line.substr (8, line.size() - 8));
      else inscreen.push_back(line);
    }
    mapfile.close();
  }

  XY upperleft;

  // Convert the inscreen vector into a drawable one, centering it
  // in the process.
  vector< vector<chtype> > inscreen_draw;
  vector<chtype> draw_line(COLS, chartoscreen(' '));
  upperleft.x = (COLS - maxwidth)/2;
  upperleft.y = (LINES - inscreen.size())/2;
  for (int i = 0; i < LINES; ++i) {
    draw_line.assign(COLS, chartoscreen(' '));
    // there is a line to draw
    if ((i - upperleft.y >= 0) && (i - upperleft.y < inscreen.size())) {
      for (int j = 0; j < COLS; ++j) {
        if ((j - upperleft.x >= 0) && (j - upperleft.x < inscreen[i-upperleft.y].length())) {
          draw_line[j] = chartoscreen(inscreen[i - upperleft.y][j - upperleft.x]);
        }
      }
    }
    inscreen_draw.push_back(draw_line);
  }

#ifndef __NOSOUND__
  start_ambience ();
#endif

  // Draw the inscreen image
  attrset (color_pair(NONE));
  clear();

  draw_map(inscreen_draw);

  attrset(color_pair(NONE) | WA_BOLD);
  mvprintw (LINES - 2, COLS - 28, "v%s (%s)", AP_VERSION, __DATE__);

  pager.set_status(0, mappack.name);

  do {
    pager.scroll_messages();
    pager.print_status(0);
    refresh ();
    restms (100);
    input = getinput();
#ifdef PDCURSES
    if (input == KEY_RESIZE) {
      resize_term(0,0);
      input = ERR;
    }
#endif
  } while (input == ERR);

  for (int yy = 0; yy < (signed)inscreen.size(); yy++)
    inscreen[yy].clear();
  inscreen.clear();

  options.push_back("Begin");
  options.push_back("Select Level");
  options.push_back("Change Map Set");
  options.push_back("Instructions");
  options.push_back("Credits");
  options.push_back("QUIT");

  int opt=menu(options, 0);
  options.clear();
  return opt;
}

int display_pause_menu () {
  vector<string> options;
  int yy;

  restms(200);
  options.push_back("Resume");
  options.push_back("Restart");
  options.push_back("Select Level");
  options.push_back("Help");
  options.push_back("Exit to main menu");

  for (yy = 0; yy < LINES; yy++)
    mvchgat (yy, 0, -1, WA_BOLD, 8 * CharData[PAUSE][3] + CharData[PAUSE][4], NULL);
  return menu(options, 0);
}


// This gets called forever by the main function
int main_menu(MapPack& mappack) {
  int level;
  cerr << "Main menu. Mappack name is " << mappack.name << endl;
  switch (display_main_menu (mappack)) {
    case 0: // Play
      play(mappack); break;
    case 1: // Select Level
      level = select_level(mappack);
      if (level != -1) {
        mappack.set_currentlevel(level);
        play(mappack);
      }
      break;
    case 2: { // Change Map Set
      MapPack newmappack = select_mappack(mappack);
      if (&newmappack != &mappack) { // the user actually changed the mappack
        /* attrset(color_pair(HELPMENU));
           fillsquare(LINES / 2 - 3, (COLS - 26) / 2, 7, 26);
           if (newmappack.name == "") {
           mvprintw (LINES / 2, (COLS - 16) / 2, "Invalid Map Pack"); refresh ();
           restms (150);
           getch();
           } else {
           mvprintw (LINES / 2, (COLS - 15) / 2, "Map Pack Loaded!"); refresh ();
           restms (150);
           getch();*/
        mappack = newmappack;
        mappack.reload_level();
        //mappack.dump();
        //}
      }
    }
      break;
    case 3: // Instructions
      help_menu (); break;
    case 4: // Credits
      roll_credits (mappack); break;
    default: // QUIT
      return -1;
  }
  return 0;
}


// handles in-game menu.
int pause_menu(MapPack & mappack) {
  int pause = display_pause_menu();
  switch (pause) {
  case 0 : case -1 : break; // Resume
  case 1 : // Restart
    mappack.reload_level();
    break;
  case 2 : { // Select level
    int newlvl = select_level(mappack);
    if (newlvl == -1) // Back to the menu (ESC key or something similar)
      pause_menu(mappack);
    else if (newlvl != mappack.get_currentlevel()) { // Change level
      mappack.set_currentlevel(newlvl);
      play(mappack);
      return -1;
    }
    break;
  }
  case 3 : help_menu (); break; // Help
  default : return -1; break; // Quit
  }
  return 0;
}


int select_level (MapPack const & mappack) {
  int input;
  int level = mappack.get_currentlevel();
  int maxlevel = mappack.get_maxlevel() + 1;

  if (maxlevel > mappack.get_number_maps())
    maxlevel = mappack.get_number_maps();
  if (level == 0)
    level = mappack.get_lastlevel();

  setblocking(true);
  fillsquare(LINES / 2 - 2, (COLS - 16) / 2, 4, 16);
  do {
    mvprintw(LINES / 2 - 1, (COLS - 14) / 2, "Choose a Level");
    mvprintw(LINES / 2, (COLS - 7) / 2, "< %3d >", level);
    refresh();
    restms(10);
    input = getinput();
    if (input == 27) return -1; // Back to the menu
#ifdef PDCURSES
    if (input == KEY_RESIZE) resize_term(0,0);
#endif
#ifndef __NOSOUND__
    if ((input == 'M') || (input == 'm')) toggle_ambience ();
#endif
    if ((input >= '0') && (input <= '9')) {
      level = level * 10 + input - '0';
#ifndef __NOSOUND__
      play_sound(MENUBEEP);
#endif
    }
    if (input == KEY_RIGHT) {
#ifndef __NOSOUND__
      play_sound(MENUBEEP);
#endif
      level ++;
    }
          if (input == KEY_LEFT) {
#ifndef __NOSOUND__
      play_sound(MENUBEEP);
#endif
            level --;
          }
          if (level > maxlevel) {
            if ((input >= '0') && (input <= '9')) level = input - '0';
            else level = maxlevel;
          }
    if (level < 1) level = 1;
  } while ((input != '\n') && (input != ' ') && (input != 'z') && (input != 'x'));

  setblocking(false);
#ifndef __NOSOUND__
  play_sound(MENUCHOICE);
#endif
  return level;
}

void roll_credits (MapPack const & mappack) {
  const int numcredits = 13;
  char basecredits[numcredits][50] = {
    "ASCIIpOrtal by Joe Larson",
    "",
    "Inspired by Increpare's Portile game",
    "http://www.increpare.com/2008/11/portile/",
    "",
    "And Super Serif Bros",
    "http://foon.co.uk/farcade/ssb/",
    "",
    "And Valve's Portal.",
    "",
    "Sound Design by Steve Fenton.",
    "Additional code by ivanq and zorun.",
    ""};
  vector <string> credits;
  string line;

  for (int l = 0; l < numcredits; l++) {
    line = basecredits[l];
    credits.push_back(line);
  }

  ifstream mapfile( mappack.filemgr.get_credits().c_str() );
  if (mapfile.is_open()) {
    vector<string> map;
    while (! mapfile.eof() ) {
      getline (mapfile, line);
      if (line.find("music") == 0) {
#ifndef __NOSOUND__
        load_ambience (mappack, line.substr (6, line.size() - 6));
#endif
      } else {
        credits.push_back(line);
        line.clear();
      }
    }
  }

  for (int c = 0; c < (LINES / 2); c++) credits.push_back("");
  credits.push_back("Thank you for playing.");

  attrset (color_pair(MENUSELECT));
  clear ();
  for (int c = 0; c < (LINES / 4) + (signed)credits.size(); c++) {
    clear ();
    for (int d = 0; d <= c; d++) {
      if ((d < (signed)credits.size()) && (LINES - 1 - 2 * c + 2 * d >= 0))
        mvprintw (LINES - 1 - 2 * c + 2 * d, (COLS - (signed)credits[d].length()) / 2, "%s", credits[d].c_str());
    }
    refresh ();
    restms (250);
    clear();
    setblocking(false);
    int input = getinput();
#ifdef PDCURSES
    if (input == KEY_RESIZE) {
      resize_term(0,0);
      input = ERR;
    }
#endif
    if (input != ERR)  {
      for (int yy = 0; yy < (signed)credits.size(); yy++)
        credits[yy].clear();
      credits.clear();
      return;
    }
    for (int d = 0; d <= c; d++) {
      if ((d < (signed)credits.size()) && (LINES - 2 - 2 * c + 2 * d >= 0))
        mvprintw (LINES - 2 - 2 * c + 2 * d, (COLS - credits[d].length()) / 2, "%s", credits[d].c_str());
    }
    refresh ();
    restms (250);
    input = getinput();
#ifdef PDCURSES
    if (input == KEY_RESIZE) {
      resize_term(0,0);
      input = ERR;
    }
#endif
    if (input != ERR) {
      for (int yy = 0; yy < (signed)credits.size(); yy++)
        credits[yy].clear();
      credits.clear();
      return;
    }
  }

  setblocking(true);
  int input = getinput();
  do {
#ifdef PDCURSES
    if (input == KEY_RESIZE) {
      resize_term(0,0);
      input = ERR;
    }
#endif
    input = getinput();
  } while (input == ERR);
  setblocking(false);
}


bool describe_mappack (MapPack const & mp) {

  int const width = 60;
  int const height = 16;
  int input;
  string tmp;
  WINDOW *win = subwin(stdscr, height, width, (LINES - height) / 2, (COLS - width) / 2);

  wattrset(win, color_pair(TEXTFIELD));
  wfillsquare(win, 0, 0, height, width);
  wattrset(win, color_pair(MENUSELECT));
  wfillsquare(win, 1, 1, 1, width-2);
  tmp = mp.properties.name + " (" + mp.properties.version + ")";
  mvwprintw(win, 1, (width - tmp.size()) / 2, tmp.c_str());

  wattrset(win, color_pair(TEXTFIELD));
  mvwprintw(win, 3, 1, "Difficulty: ");
  int diff = mp.properties.difficulty;
  for (int i = 0; i < diff; ++i)
    waddch(win, 3 | WA_ALTCHARSET);
  tmp.clear();
  if (diff < 2) tmp = "Numby Pumby";
  else if (diff < 4) tmp = "Quite easy";
  else if (diff < 6) tmp = "Achievable";
  else if (diff < 8) tmp = "Challenging";
  else if (diff < 10) tmp = "Hardcore";
  else tmp = "Impossible";
  wprintw(win, (" " + tmp).c_str());

  mvwprintw(win, 4, 1, "Author: ");
  wprintw(win, mp.properties.author.c_str());

  mvwprintw(win, 5, 1, "Short name: ");
  wprintw(win, mp.name.c_str());

  mvwprintw(win, 6, 1, "Description:");
  mvwprintw(win, 7, 2, mp.properties.description.c_str());

  do {
    wrefresh(win);
    flushinput();
    setblocking(true);
    restms(100);
    input = getinput();
    switch (input) {
      case KEY_ENTER :
      case KEY_RIGHT :
      case '\n':
      case ' ':
#ifndef __NOSOUND__
        play_sound(MENUCHOICE);
#endif
        delwin(win);
        return true;
        break;
      case 27 : // ESC key
      case KEY_LEFT :
#ifndef __NOSOUND__
        play_sound(MENUCHOICE);
#endif
        setblocking(false);
        delwin(win);
        return false;
        break;
      default: break;
    }
  } while (1);

}


MapPack select_mappack (MapPack const & current) {
  vector<string> _mappacks = filemgr.list_mappacks();
  vector<MapPack> mappacks;
  int const displayed_entries = 6;
  // Maximum displayed size of a map pack name
  int const name_maxwidth = 30;
  int width = name_maxwidth + 9;
  int input;

  // first, get available mappacks
  for (vector<string>::iterator i = _mappacks.begin(); i != _mappacks.end(); ++i) {
    MapPack tmp(*i);
    if (tmp.name != "")
      mappacks.push_back(tmp);
  }

  // sort it
  sort(mappacks.begin(), mappacks.end());

  cout << mappacks.size() << " mappacks detected" << endl;

  setblocking(true);

  int position = 0;
  int top_map_displayed = 0;
  int size = mappacks.size();
  stringstream namestream, levelstream, tmpstream;
  string _name;

  do {
    attrset(color_pair(HELPMENU));
    fillsquare(LINES / 2 - 3, (COLS - (width + 2)) / 2, 4 + displayed_entries, width + 2);
    attrset(color_pair(MENUDIM));
    mvprintw (LINES / 2 - 2, (COLS - width) / 2, "Select your map pack:");
    for (int i = 0; i < displayed_entries && i < size; ++i) {
      if (i == position - top_map_displayed) {
        attrset(color_pair(MENUSELECT));
        fillsquare(LINES / 2 + i, (COLS - width) / 2, 1, width);
      }
      else {
        attrset(color_pair(TEXTFIELD));
        fillsquare(LINES / 2 + i, (COLS - width) / 2, 1, width);
      }
      namestream.str("");
      levelstream.str("");
      tmpstream.str("");
      namestream << left << setw(name_maxwidth)
                 << setfill(' ');
      levelstream << right << setw(width - name_maxwidth - 1)
                  << setfill(' ');
      if (mappacks[i+top_map_displayed].properties.name.size() > name_maxwidth)
        _name = mappacks[i+top_map_displayed].properties.name.substr(0, name_maxwidth);
      else
        _name = mappacks[i+top_map_displayed].properties.name;
      namestream << _name;
      tmpstream << "(" << mappacks[i+top_map_displayed].get_maxlevel() << "/";
      tmpstream << mappacks[i+top_map_displayed].get_number_maps() << ")";
      levelstream << tmpstream.str();
      mvprintw (LINES / 2 + i, (COLS - width) / 2, (" " + namestream.str() + levelstream.str()).c_str());
    }
    refresh();
    flushinput();
    restms(100);
    setblocking(true);
    input = getinput();
    setblocking(false);
    switch (input) {
      case KEY_UP :
#ifndef __NOSOUND__
        play_sound(MENUBEEP);
#endif
        if (position > 0) position--;
        if ((position <= top_map_displayed) && (top_map_displayed > 0))
          top_map_displayed--;
        break;
      case KEY_DOWN :
#ifndef __NOSOUND__
        play_sound(MENUBEEP);
#endif
        if (position + 1 < size) position++;
        if ((position + 1 >= displayed_entries)
            && (top_map_displayed < size - displayed_entries))
          top_map_displayed++;
        break;
      case KEY_ENTER :
      case KEY_RIGHT :
      case '\n':
      case ' ':
#ifndef __NOSOUND__
        play_sound(MENUCHOICE);
#endif
        setblocking(false);
        if (describe_mappack(mappacks[position]))
          return mappacks[position];
        break;
      case 27 : // ESC key
      case KEY_LEFT :
#ifndef __NOSOUND__
        play_sound(MENUCHOICE);
#endif
        setblocking(false);
        return current;
        break;
    }
  } while (1);

  return current;
}
