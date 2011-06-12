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

#include <vector>
#include <curses.h>

#include "asciiportal.h"

#include "ap_maps.h"

using namespace std;

int color_pair(int);

void fillscreen (int);

// True if we want to go to the next level, false if we want to retry
bool displaystats (const level&);

int screenchar(int);

void draw_map(std::vector< std::vector<chtype> > const &);

void map_screen(level &, std::vector<std::vector<chtype> >&);

void draw_screen_angle (int angle);

void draw_rotate (level &, int num);

void draw_screen (level &);

void graphics_init (bool fullscreen, int height, int width, bool usePureAscii);

void graphics_deinit ();

#endif // AP_DRAW_H_INCLUDED
