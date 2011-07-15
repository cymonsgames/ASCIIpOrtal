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

#ifndef AP_SOUND_H_INCLUDED
#define AP_SOUND_H_INCLUDED

#include "asciiportal.h"
#include "ap_maps.h"

enum Sounds {
  SILENT, WIN, SIZZLE, PORTALCOLLAPSE, CRUSH, SWITCHHIT, DOOROPEN, DOORCLOSE, DUPLICATE,
  MENUBEEP, MENUCHOICE, GUNSHOT, PORTALCREATE, PORTALFAIL, THROUGH,
  VOICE, VOICE2, VOICE3, VOICE4, VOICE5, VOICE6, VOICE7, VOICE8, VOICE9, VOICE0,
  MAXSound
};

int sound_init ();

int default_ambience (int);

int load_ambience (std::string filename);
int load_ambience (MapPack const & mappack, std::string filename);

int start_ambience ();

void stop_ambience ();

void toggle_ambience ();

int play_sound (int);

int play_voice ();

void deinit_sound ();

#endif
