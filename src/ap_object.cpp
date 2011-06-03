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

#include <iostream>
#include <stdio.h>
#include "ap_object.h"

using namespace std;

void ObjectManager::addobj(object& obj) {
  if (obj.coord.x < 0) obj.coord.x = 0;
  if (obj.coord.y < 0) obj.coord.y = 0;
  if (obj.coord.x >= mw) obj.coord.x = mw-1;
  if (obj.coord.y >= mh) obj.coord.y = mh-1;
  objs.push_back(obj);
  objiter it = --objs.end();
  switch (it->type) {
    case SHOT1: shots[0] = it; break;
    case SHOT2: shots[1] = it; break;
  }
  objmap[obj.coord.y][obj.coord.x].push_back(it);
}

objiter ObjectManager::delobj(objiter it) {
  switch (it->type) {
    case PLAYER:  player     = NULLOBJ; break;
    case PORTAL1: portals[0] = NULLOBJ; break;
    case PORTAL2: portals[1] = NULLOBJ; break;
    case SHOT1:   shots[0]   = NULLOBJ; break;
    case SHOT2:   shots[1]   = NULLOBJ; break;
  }
  objmap[it->coord.y][it->coord.x].remove(it);
  return objs.erase(it);
}

void ObjectManager::moveobj(objiter it, int x, int y) {
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x >= mw) x = mw-1;
  if (y >= mh) y = mh-1;
  objmap[it->coord.y][it->coord.x].remove(it);
  objmap[y][x].push_back(it);
  it->coord.y = y;
  it->coord.x = x;
}

void ObjectManager::resetmap(int w, int h) {
  mw = w;
  mh = h;

  NULLOBJ = objs.end();
  player = portals[0] = portals[1] = shots[0] = shots[1] = NULLOBJ;

  vector<objiter> a;
  doors.assign(13, a);
  switches.assign(13, a);
  triggers.assign(9, a);
  
  objset s;
  vector<objset> v(w, s);
  objmap.assign(h, v);

  for (objiter it = objs.begin(); it != objs.end(); it++) {
    objmap[it->coord.y][it->coord.x].push_back(it);
    switch (it->type) {
      case PLAYER:  player     = it; break;
      case PORTAL1: portals[0] = it; break;
      case PORTAL2: portals[1] = it; break;
      case DOOR:
        doors[it->d.y].push_back(it); break;
      case SWITCH:
        switches[it->d.y].push_back(it); break;
      case TEXTTRIGGER:
        triggers[it->d.y].push_back(it); break;
    }
  }
}

void ObjectManager::killtriggers(int n) {
  for (int i = 0; i < triggers[n].size(); i++) {
    objiter it = triggers[n][i];
    objmap[it->coord.y][it->coord.x].remove(it);
    objs.erase(it);
  }
  triggers[n].clear();
}

int ObjectManager::verify() {
#define FAIL(msg) { cout << msg << '\n'; return 0; }
  int nmapobj = 0;
  for (int y = 0; y < objmap.size(); y++)
    for (int x = 0; x < objmap[y].size(); x++)
      nmapobj += objmap[y][x].size();
  if (nmapobj != objs.size()) FAIL("number of objects mismatch");

  objiter pl, por[2], sht[2];
  pl = por[0] = por[1] = sht[0] = sht[1] = NULLOBJ;
  for (objiter it = objs.begin(); it != objs.end(); it++) {
    switch (it->type) {
      case PLAYER:  pl     = it; break;
      case PORTAL1: por[0] = it; break;
      case PORTAL2: por[1] = it; break;
      case SHOT1:   sht[0] = it; break;
      case SHOT2:   sht[1] = it; break;
    }
    int found = 0;
    objset *s = &objmap[it->coord.y][it->coord.x];
    for (objset::iterator sit = s->begin(); sit != s->end(); sit++)
      if (*sit == it) found = 1;
    if (!found) FAIL("object not found in objmap");
  }
  if (player != pl) {
    cout << "player is " << ((player == NULLOBJ) ? "" : "not ") << "null\n";
    cout << "but should be " << ((pl == NULLOBJ) ? "" : "not ") << "null\n";
    FAIL("bad player ref");
  }
  if (portals[0] != por[0]) FAIL("bad portals[0] ref");
  if (shots[0] != sht[0]) FAIL("bad shots[0] ref");
  if (portals[1] != por[1]) FAIL("bad portals[1] ref");
  if (shots[1] != sht[1]) FAIL("bad shots[1] ref");
  return 1;
}

void ObjectManager::dump() {
  printf ("\n========\nOBJECT LIST:\n");
  printf ("  %s %s %s %s %s %s\n", "x", "y", "type", "tick", "d.x", "d.y");
  for (objiter it = objs.begin(); it != objs.end(); it++)
    printf ("  %d %d   %d   %d   %d  %d\n", it->coord.x, it->coord.y, it->type, it->tick, it->d.x, it->d.y);
  printf ("\nOBJECT MAP:\n");
  for (int y = 0; y < objmap.size(); y++) {
    for (int x = 0; x < objmap[y].size(); x++) {
      objset *s = &objmap[y][x];
      if (s->size()) {
        printf ("  at x=%d,y=%d:\n", x, y);
        for (objset::iterator it = s->begin(); it != s->end(); it++) {
          printf ("    %d %d %d %d %d %d\n", (*it)->coord.x, (*it)->coord.y, (*it)->type, (*it)->tick, (*it)->d.x, (*it)->d.y);
          printf ("    addr of coord.x field = %p\n", (void *) &((*it)->coord.x)); 
        }
      }
    }
  }
}
