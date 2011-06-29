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

#ifndef AP_PLAY_H_INCLUDED
#define AP_PLAY_H_INCLUDED

#include "asciiportal.h"
#include "ap_object.h"
#include "ap_maps.h"

// Groups functions dedicated to the game itself.
// This is instanciated for every level.
class Game {
private:
  level &lvl;
  objiter &NULLOBJ;
  // Commodity: calls pager.set_status with a proper argument
  void set_status(string);
  void fireportal(int por);
  objiter in_portal();
  int switch_in_portal();
  int por_col(int yy, int xx);
  int will_hit(objiter c);
  int applyd(objiter c);
  void collapse_portals();
  int move_object(objiter c);
  int move_player();

public:
  Game(level&);
  // whether the user requested the pause menu or not
  bool pause;
  // in-game lightweight pause menu
  bool light_pause;
  bool still_alive();
  bool has_won();
  int physics();
};

int sc(int x);
int hitswall(level const & lvl, int yy, int xx);
objiter hitsobj(level & lvl, objiter, int, int);
int play (MapPack&);

#endif // AP_PLAY_H_INCLUDED
