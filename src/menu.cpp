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
#include <cctype>
#include "asciiportal.h"
#ifndef __NOSDL__
#include "SDL/SDL.h"
#endif
#ifndef __NOSOUND__
#include "ap_sound.h"
#endif
#include "ap_filemgr.h"
#include "menu.h"
#include "ap_draw.h"
#include "ap_input.h"

using namespace std;
extern const int CharData[MAXColors][5];

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
    "ESC/P/F1 .......... Pause Game Menu",
    "F2 ................ Change Portal Render",
    "F3 ................ Set to Default Speed",
    "+ ................. Increase Game Speed",
    "- ................. Decrease Game Speed",
    "R ................. Restart Level",
    "M ................. Toggle music"
  };
#endif
#endif


  pauserun(1);

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
  nodelay(stdscr, 0);
  //int input = getinput();
  int input = getch();
  while (1) {
    switch (input) {
#ifdef PDCURSES
      case KEY_RESIZE: resize_term(0,0); break;
#endif
      case ERR : break;
      default :
        pauserun(0);
        return;
    }
    //input = getinput ();
    input = getch();
  }

  pauserun(0);
}

int menu (vector <string>& items, int selection) {
  int input;

#ifndef __NOSOUND__
  play_sound(MENUBEEP);
#endif
  pauserun(1);
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
        pauserun(0);
        return -1;
        break;
      case KEY_ENTER:
      case KEY_RIGHT:
      case '\n':
      case ' ':
#ifndef __NOSOUND__
        play_sound(MENUCHOICE);
#endif
        pauserun(0);
        return selection;
    }
    selection %= items.size();
  }
}

int main_menu (string mappack) {
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
  name = filemgr.get_inscreen(mappack);
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

  pager.set_status(mappack);

  do { 
    pager.scroll_messages();
    pager.print_status();
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
    mvchgat (yy, 0, -1, WA_BOLD, 8 * CharData[PAUSE][2] + CharData[PAUSE][3], NULL);
  return menu(options, 0);
}

int select_level (int maxlevel, int level){
  int input;

  level ++;
  pauserun(1);
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
          if (level > maxlevel + 1) {
            if ((input >= '0') && (input <= '9')) level = input - '0';
            else level = maxlevel + 1;
          }
    if (level < 1) level = 1;
  } while ((input != '\n') && (input != ' ') && (input != 'z') && (input != 'x'));

  pauserun(0);
#ifndef __NOSOUND__
  play_sound(MENUCHOICE);
#endif
  return level - 1;
}

void roll_credits (string mappack) {
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

  ifstream mapfile( filemgr.get_credits(mappack).c_str() );
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
    nodelay(stdscr, 1);
    int input = getch();
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
    input = getch();
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

  nodelay(stdscr, 0);
  int input = getch();
  do {
#ifdef PDCURSES
    if (input == KEY_RESIZE) {
      resize_term(0,0);
      input = ERR;
    }
#endif
    input = getch();
  } while (input == ERR);
}

string select_mappack () {
  vector<string> mappacks, custommappacks;
  int displayed_entries = 6;
  int input;
  
  // first, get available mappacks
  mappacks = filemgr.list_official_mappacks();
  custommappacks = filemgr.list_custom_mappacks();
  mappacks.insert(mappacks.end(), custommappacks.begin(), custommappacks.end());

  cout << mappacks.size() << " mappacks detected" << endl;

  nodelay(stdscr,0);
  pauserun(1);

  int position = 0;
  int top_map_displayed = 0;
  int size = mappacks.size();

  do {
    attrset(color_pair(HELPMENU));
    fillsquare(LINES / 2 - 3, (COLS - 28) / 2, 4 + displayed_entries, 28);
    attrset(color_pair(MENUDIM));
    mvprintw (LINES / 2 - 2, (COLS - 26) / 2, "Select your map pack:");
    for (int i = 0; i < displayed_entries && i < size; ++i) {
      if (i == position - top_map_displayed) {
        attrset(color_pair(MENUSELECT));
        fillsquare(LINES / 2 + i, (COLS - 26) / 2, 1, 26);
      }
      else {
        attrset(color_pair(TEXTFIELD));
        fillsquare(LINES / 2 + i, (COLS - 26) / 2, 1, 26);
      }
      mvprintw (LINES / 2 + i, (COLS - 26) / 2, (" " + mappacks[i+top_map_displayed]).c_str());
    }
    refresh();
    flushinput();
    restms(50);
    input = getinput();
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
        pauserun(0);
        return mappacks[position];
        break;
      case 27 : // ESC key
      case KEY_LEFT :
#ifndef __NOSOUND__
        play_sound(MENUCHOICE);
#endif
        pauserun(0);
        return "";
        break;
    }
  } while (1);

  return "";
}
