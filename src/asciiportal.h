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

#ifndef ASCIIPORTAL_H_INCLUDED
#define ASCIIPORTAL_H_INCLUDED

#include <string>

// This gets defined by the Makefile at compile time
#ifndef AP_VERSION
#define AP_VERSION "1.3"
#endif

// level file syntax
#ifndef MAPS_PROTOCOL
#define MAPS_PROTOCOL 1
#endif

// Note that the Makefile handles this
#ifndef __NOSDL__
//#define __NOSDL__
#endif
#ifndef __NOSOUND__
//#define __NOSOUND__
#endif
#ifndef __DINGOO__
//#define __DINGOO__
#endif
#ifndef __GP2X__
//#define __GP2X__
#endif

unsigned long long int get_microseconds();

enum GameObjs {
  NONE, GOAL, LADDER, NORMAL, NONSTICK, LTREAD, RTREAD,
  FFIELD, PFIELD, XFIELD, SPIKE,
  MAXWall,
  DUPLICATOR, SHOT1, SHOT2, PORTAL1, PORTAL2,
  SWITCH, SWITCHON, DOOR, DOOR3, DOOR2, DOOR1,
  BOX, BOULDER, PLAYER, TEXTTRIGGER, FLASH,
  MAXObjects, // defined for ease of reference
  PAUSE, MENUDIM, MENUSELECT, HELPMENU, TEXTFIELD,
  MAXColors
};

struct XY { // very simple struct for keeping XY cooridnate pairs.
  int x; int y;
};

int loadmaps (std::string);

std::string get_env_var(std::string const &);

void debug(std::string);

#endif // ASCIIPORTAL_H_INCLUDED
