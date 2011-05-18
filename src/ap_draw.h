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

#ifndef AP_DRAW_H_INCLUDED
#define AP_DRAW_H_INCLUDED

#include <utility>
#include <list>
#include <vector>
#include <curses.h>
#include "ap_object.h"

using namespace std;

typedef list< pair<string, int> > pairlist;

// Used to display both status messages, level name and scrolling messages
class Pager {
 private:
  // <message to scroll, tick>
  list< pair<string, int> > messages;
  // minimum space between two messages
  int scrolling_space;

  string status;
  int status_tick;
  string levelname;
 public:
  Pager();
  // Scrolling messages handling
  void add_scrolling(string);
  void scroll_messages();
  // Status message handling
  void set_status(string);
  void set_levelname(string);
  void print_status();
  void clear();
};

int color_pair(int);

void fillscreen (int);

int displaystats (statstype, int);

void statusmessage (std::string);

void stopmessages ();

int screenchar(int);

void draw_map(std::vector< std::vector<chtype> > const &);

void map_screen (std::vector<std::vector<chtype> >&);

void draw_screen_angle (int angle);

void draw_rotate (int);

void draw_screen ();

// fullscreen, height, width
void graphics_init (bool, int, int);

void graphics_deinit ();

#endif // AP_DRAW_H_INCLUDED
