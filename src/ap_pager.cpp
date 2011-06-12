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

#include "ap_pager.h"

using namespace std;

extern const int CharData [MAXColors][6];

extern int color_pair(int);

/* Pager implementation */
Pager::Pager() {
  scrolling_space = 8;
  status.clear();
  messages.clear();
  levelname.clear();
  status_time = 0;
}

void Pager::add_scrolling(string message) {
  // We add a trailing space in case the screen isn't cleared between
  // each scroll_messages()
  pair <string, int> mess(message + " ", 0);

  // shift the previous messages to let enough space for this one
  int shift = 0;
  for (pairlist::iterator i = messages.begin(); i != messages.end(); ++i) {
    int size = i->first.size(); // nasty unsigned int...

    i->second += shift;
    if (i->second - size < scrolling_space) {
      shift = scrolling_space - i->second + size;
      i->second += shift;
    }
  }
  messages.push_front(mess);
}

void Pager::scroll_messages() {
  for (pairlist::iterator i = messages.begin(); i != messages.end(); ++i) {
    int tick = i->second;
    if (tick < COLS + i->first.size() - 2) {
      // display message (i->first)
      int start = COLS - tick;
      int substart = 0;
      int subend = i->first.size();

      if (start < 0) substart = tick - COLS;
      if (tick < i->first.size()) subend = tick;
      if (subend > COLS) subend = COLS;
      attrset(color_pair(NONE) | WA_BOLD);
      mvprintw(0, (start < 0) ? 0 : start, "%s", i->first.substr(substart, subend).c_str());
      i->second++;
    }
    else //delete element at position i and following
      i = messages.erase(i, messages.end());
  } // for
} // scroll_messages

void Pager::set_status(int ticks, string status_msg) {
  status = status_msg;
  status_time = ticks;
}

void Pager::set_levelname(string lvlname) {
  levelname = lvlname;
}

void Pager::print_status(int ticks) {
  attrset(color_pair(NONE));

  if (status.size() && status_time <= ticks + 30)
    mvprintw(LINES - 1, 0, "%s", status.c_str());
 
  if (levelname.size() && ticks < 200)
    mvprintw(LINES - 1, COLS - levelname.size(), "%s", levelname.c_str());
}

void Pager::clear() {
  messages.clear();
  status.clear();
  levelname.clear();
}

