#ifndef AP_OBJECT_H_INCLUDED
#define AP_OBJECT_H_INCLUDED

#include <vector>
#include <list>
#include "asciiportal.h"

using namespace std;

class object {
  public:
  XY coord;
  int type, tick;
  XY d;
};

typedef list<object>::iterator objiter;
typedef list<objiter> objset;

class ObjectManager {
  public:
  list<object> objs;
  vector< vector<objset> > objmap;
  objiter NULLOBJ;
  objiter player;
  objiter portals[2];
  objiter shots[2];
  vector< vector<objiter> > doors;
  vector< vector<objiter> > switches;
  vector< vector<objiter> > triggers;
  int mw, mh;
  void resetmap(int w, int h);
  void addobj(object& obj);
  objiter delobj(objiter it);
  void moveobj(objiter it, int x, int y);
  void killtriggers(int n);
  int verify();
  void dump();
};

#endif // AP_OBJECT_H_INCLUDED
