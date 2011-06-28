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
#include <iostream>
#include <fstream>
#include <string>
#include <string>
#include <vector>
#include <ctime>
#include <cmath>
using namespace std;

#include "ap_input.h"

#ifndef __NOSDL__
#include "pdcsdl.h"
#include "SDL/SDL.h"
#endif
#ifndef __NOSOUND__
#include "ap_sound.h"
#endif

//#include "ap_play.h"
//#include "ap_draw.h"
#include "menu.h"

/*
extern statstype levelstats;
extern int level;
extern int maxlevel;
extern int ticks;
extern int cheatview;
extern vector<vector<int> > map;
*/

#ifndef __NOSDL__
SDL_Event event;
int mouseon = 0;
bool blocking = false;
int hasevent = 0;
int oldkey = ERR;

static struct
{
	SDLKey keycode;
	bool numkeypad;
	unsigned short normal;
	unsigned short shifted;
	unsigned short control;
	unsigned short alt;
} key_table[] = // shamelessly copied.
{
/* keycode	keypad	normal	     shifted	   control	alt*/
 {SDLK_LEFT,	FALSE,	KEY_LEFT,    KEY_SLEFT,    CTL_LEFT,	ALT_LEFT},
 {SDLK_RIGHT,	FALSE,	KEY_RIGHT,   KEY_SRIGHT,   CTL_RIGHT,	ALT_RIGHT},
 {SDLK_UP,	FALSE,	KEY_UP,      KEY_SUP,	   CTL_UP,	ALT_UP},
 {SDLK_DOWN,	FALSE,	KEY_DOWN,    KEY_SDOWN,    CTL_DOWN,	ALT_DOWN},
 {SDLK_HOME,	FALSE,	KEY_HOME,    KEY_SHOME,    CTL_HOME,	ALT_HOME},
 {SDLK_RETURN,	FALSE,	'\n',    '\n',  CTL_PADENTER,ALT_PADENTER},
 {SDLK_END,	FALSE,	KEY_END,     KEY_SEND,	   CTL_END,	ALT_END},
 {SDLK_PAGEUP,	FALSE,	KEY_PPAGE,   KEY_SPREVIOUS,CTL_PGUP,	ALT_PGUP},
 {SDLK_PAGEDOWN,FALSE,	KEY_NPAGE,   KEY_SNEXT,    CTL_PGDN,	ALT_PGDN},
 {SDLK_INSERT,	FALSE,	KEY_IC,      KEY_SIC,	   CTL_INS,	ALT_INS},
 {SDLK_DELETE,	FALSE,	KEY_DC,      KEY_SDC,	   CTL_DEL,	ALT_DEL},
 {SDLK_F1,	FALSE,	KEY_F(1),    KEY_F(13),    KEY_F(25),	KEY_F(37)},
 {SDLK_F2,	FALSE,	KEY_F(2),    KEY_F(14),    KEY_F(26),	KEY_F(38)},
 {SDLK_F3,	FALSE,	KEY_F(3),    KEY_F(15),    KEY_F(27),	KEY_F(39)},
 {SDLK_F4,	FALSE,	KEY_F(4),    KEY_F(16),    KEY_F(28),	KEY_F(40)},
 {SDLK_F5,	FALSE,	KEY_F(5),    KEY_F(17),    KEY_F(29),	KEY_F(41)},
 {SDLK_F6,	FALSE,	KEY_F(6),    KEY_F(18),    KEY_F(30),	KEY_F(42)},
 {SDLK_F7,	FALSE,	KEY_F(7),    KEY_F(19),    KEY_F(31),	KEY_F(43)},
 {SDLK_F8,	FALSE,	KEY_F(8),    KEY_F(20),    KEY_F(32),	KEY_F(44)},
 {SDLK_F9,	FALSE,	KEY_F(9),    KEY_F(21),    KEY_F(33),	KEY_F(45)},
 {SDLK_F10,	FALSE,	KEY_F(10),   KEY_F(22),    KEY_F(34),	KEY_F(46)},
 {SDLK_F11,	FALSE,	KEY_F(11),   KEY_F(23),    KEY_F(35),	KEY_F(47)},
 {SDLK_F12,	FALSE,	KEY_F(12),   KEY_F(24),    KEY_F(36),	KEY_F(48)},
 {SDLK_F13,	FALSE,	KEY_F(13),   KEY_F(25),    KEY_F(37),	KEY_F(49)},
 {SDLK_F14,	FALSE,	KEY_F(14),   KEY_F(26),    KEY_F(38),	KEY_F(50)},
 {SDLK_F15,	FALSE,	KEY_F(15),   KEY_F(27),    KEY_F(39),	KEY_F(51)},
 {SDLK_BACKSPACE,FALSE,	0x08,        0x08,	   CTL_BKSP,	ALT_BKSP},
 {SDLK_TAB,	FALSE,	0x09,        KEY_BTAB,	   CTL_TAB,	ALT_TAB},
 {SDLK_PRINT,	FALSE,	KEY_PRINT,   KEY_SPRINT,   KEY_PRINT,	KEY_PRINT},
 {SDLK_PAUSE,	FALSE,	KEY_SUSPEND, KEY_SSUSPEND, KEY_SUSPEND, KEY_SUSPEND},
 {SDLK_CLEAR,	FALSE,	KEY_CLEAR,   KEY_CLEAR,    KEY_CLEAR,	KEY_CLEAR},
 {SDLK_BREAK,	FALSE,	KEY_BREAK,   KEY_BREAK,    KEY_BREAK,	KEY_BREAK},
 {SDLK_HELP,	FALSE,	KEY_HELP,    KEY_SHELP,    KEY_LHELP,	KEY_HELP},
 {SDLK_MENU,	FALSE,	KEY_OPTIONS, KEY_SOPTIONS, KEY_OPTIONS, KEY_OPTIONS},
 {SDLK_ESCAPE,	FALSE,	0x1B,        0x1B,	   0x1B,	ALT_ESC},
 {SDLK_KP_ENTER,TRUE,	'\n',    '\n',	   CTL_PADENTER,ALT_PADENTER},
 {SDLK_KP_PLUS,	TRUE,	PADPLUS,     '+',	   CTL_PADPLUS, ALT_PADPLUS},
 {SDLK_KP_MINUS,TRUE,	PADMINUS,    '-',	   CTL_PADMINUS,ALT_PADMINUS},
 {SDLK_KP_MULTIPLY,TRUE,PADSTAR,     '*',	   CTL_PADSTAR, ALT_PADSTAR},
 {SDLK_KP_DIVIDE,TRUE,	PADSLASH,    '/',	   CTL_PADSLASH,ALT_PADSLASH},
 {SDLK_KP_PERIOD,TRUE,	PADSTOP,     '.',	   CTL_PADSTOP, ALT_PADSTOP},
 {SDLK_KP0,	TRUE,	'0',	     '0',	   CTL_PAD0,	ALT_PAD0},
 {SDLK_KP1,	TRUE,	'1',      '1',	   CTL_PAD1,	ALT_PAD1},
 {SDLK_KP2,	TRUE,	'2',      '2',	   CTL_PAD2,	ALT_PAD2},
 {SDLK_KP3,	TRUE,	'3',      '3',	   CTL_PAD3,	ALT_PAD3},
 {SDLK_KP4,	TRUE,	'4',      '4',	   CTL_PAD4,	ALT_PAD4},
 {SDLK_KP5,	TRUE,	'5',      '5',	   CTL_PAD5,	ALT_PAD5},
 {SDLK_KP6,	TRUE,	'6',      '6',	   CTL_PAD6,	ALT_PAD6},
 {SDLK_KP7,	TRUE,	'7',      '7',	   CTL_PAD7,	ALT_PAD7},
 {SDLK_KP8,	TRUE,	'8',      '8',	   CTL_PAD8,	ALT_PAD8},
 {SDLK_KP9,	TRUE,	'9',      '9',	   CTL_PAD9,	ALT_PAD9},
 {SDLK_WORLD_0,0,0,0,0},
 {SDLK_SPACE,   FALSE,  ' ', ' ', ' ', ' '}
};
#endif

int processevent () { // Convert keys down to PDC key commands. Introduces priority if multiple keys are pressed.
#ifdef __NOSDL__
  //TODO: fix the nasty delay with standard curses
  int tmp = getch();
  flushinput();
  return tmp;
#else
  int key = oldkey;
  bool ismouseevent = false;

  do { // while (ismousevent)
    ismouseevent = false;
    if (blocking)
      hasevent = SDL_WaitEvent( &event );
    else
      if (!hasevent) hasevent = SDL_PollEvent( &event );

    if ( hasevent ) {
      hasevent = 0;
      //      printf("%d\n", event.type);
      switch (event.type) {
      case SDL_QUIT:
        exit(1);
      case SDL_VIDEORESIZE: // shameless copied from sdl1/pdckbd.c
        if (pdc_own_screen && (event.resize.h / pdc_fheight != LINES || event.resize.w / pdc_fwidth != COLS)) {
          pdc_sheight = event.resize.h;
          pdc_swidth = event.resize.w;

          if (!SP->resized) {
            SP->resized = TRUE;
            return KEY_RESIZE;
          }
        }
        break;

      case SDL_MOUSEMOTION:
        //        if (mouseon)
        //          SDL_ShowCursor(SDL_ENABLE);
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEBUTTONDOWN:
      case SDL_ACTIVEEVENT:
        //        oldkey = SDLK_SPACE;
        //        if (SP->_trap_mbe)
        //          return _process_mouse_event();
        key = ERR;
        ismouseevent = true;
        break;
      case SDL_KEYUP :
        key = ERR;
        break;
#ifdef __GP2X__
      case SDL_JOYBUTTONDOWN:
        switch (event.jbutton.button) {
        case 0 : key = KEY_UP; break; // up
        case 1 : key = KEY_LEFT; break; // upleft
        case 2 : key = KEY_LEFT; break; // left
        case 3 : key = KEY_LEFT; break; // downleft
        case 4 : key = KEY_DOWN; break; // down
        case 5 : key = KEY_RIGHT; break; // downright
        case 6 : key = KEY_RIGHT; break; // right
        case 7 : key = KEY_RIGHT; break; // upright
        case 8 : key = KEY_F(1); break; // GP2X start
        case 9 : key = KEY_F(2); break; // GP2X select
        case 10: key = 'z'; break; // GP2X L
        case 11: key = 'x'; break; // GP2X R
        case 12: key = 'z'; break; // GP2X A (left)
        case 13: key = 'x'; break; // GP2X B (right)
        case 14: key = ' '; break; // GP2X X (down)
        case 15: key = 'c'; break; // GP2X y (up)
        case 16: key = '+'; break; // GP2X VolUp
        case 17: key = '-'; break; // CP2X VolDown
        }
#endif
      default :
        //if (mouseon) PDC_mouse_set();
        for (int i = 0; key_table[i].keycode; i++) {
          if (key_table[i].keycode == event.key.keysym.sym) {
            key = key_table[i].normal;
          }
        }

        if (key == ERR) key = event.key.keysym.unicode;

        switch( event.key.keysym.sym ) {
          // handle specalized checking here that will override the default info:
#ifdef __DINGOO__
        case SDLK_UP: key = KEY_UP; break;
        case SDLK_DOWN: key = KEY_DOWN; break;
        case SDLK_LEFT: key = KEY_LEFT; break;
        case SDLK_RIGHT: key = KEY_RIGHT; break;
        case SDLK_LCTRL: key = 'z'; break; // A key (right)
        case SDLK_LALT: key = ' '; break; // B key (down)
        case SDLK_SPACE: key = 'c'; break; // X key (up)
        case SDLK_LSHIFT: key = 'x'; break; // Y key (left)
        case SDLK_TAB: key = 'x'; break; // Left Shoulder
        case SDLK_BACKSPACE: key = 'z'; break; // Right Shoulder
        case SDLK_RETURN: key = KEY_F(1); break; // Start button
        case SDLK_ESCAPE: key = KEY_F(2); break; // Y key (left)
#endif
        default: break;
        }
        break;
      }
    }
    oldkey = key;

    // This section is for keys you do not want repeated if the key is pressed
    switch (oldkey) {
    case ' ':
    case '\n':
    case 'c':
    case 'C':
    case 'x':
    case 'X':
    case 'z':
    case 'Z':
    case '+':
    case '-':
      //case 27: // ESC key
      oldkey = -1;
    }
  //  if (delay) oldkey = -1;
  } while (ismouseevent);

  return key;
#endif //!NOSDL
}

int getinput() {
  int key;

  key = processevent();
  if (key == -1) key = processevent();
  return key;
}

void flushinput () {
#ifndef __NOSDL__
  oldkey = -1;
#endif
  flushinp();
}

/* List of SDL keystates you could be checking:
	keynames[SDLK_BACKSPACE] = "backspace";
	keynames[SDLK_TAB] = "tab";
	keynames[SDLK_CLEAR] = "clear";
	keynames[SDLK_RETURN] = "return";
	keynames[SDLK_PAUSE] = "pause";
	keynames[SDLK_ESCAPE] = "escape";
	keynames[SDLK_SPACE] = "space";
	keynames[SDLK_EXCLAIM]  = "!";
	keynames[SDLK_QUOTEDBL]  = "\"";
	keynames[SDLK_HASH]  = "#";
	keynames[SDLK_DOLLAR]  = "$";
	keynames[SDLK_AMPERSAND]  = "&";
	keynames[SDLK_QUOTE] = "'";
	keynames[SDLK_LEFTPAREN] = "(";
	keynames[SDLK_RIGHTPAREN] = ")";
	keynames[SDLK_ASTERISK] = "*";
	keynames[SDLK_PLUS] = "+";
	keynames[SDLK_COMMA] = ",";
	keynames[SDLK_MINUS] = "-";
	keynames[SDLK_PERIOD] = ".";
	keynames[SDLK_SLASH] = "/";
	keynames[SDLK_0] = "0";
	keynames[SDLK_1] = "1";
	keynames[SDLK_2] = "2";
	keynames[SDLK_3] = "3";
	keynames[SDLK_4] = "4";
	keynames[SDLK_5] = "5";
	keynames[SDLK_6] = "6";
	keynames[SDLK_7] = "7";
	keynames[SDLK_8] = "8";
	keynames[SDLK_9] = "9";
	keynames[SDLK_COLON] = ":";
	keynames[SDLK_SEMICOLON] = ";";
	keynames[SDLK_LESS] = "<";
	keynames[SDLK_EQUALS] = "=";
	keynames[SDLK_GREATER] = ">";
	keynames[SDLK_QUESTION] = "?";
	keynames[SDLK_AT] = "@";
	keynames[SDLK_LEFTBRACKET] = "[";
	keynames[SDLK_BACKSLASH] = "\\";
	keynames[SDLK_RIGHTBRACKET] = "]";
	keynames[SDLK_CARET] = "^";
	keynames[SDLK_UNDERSCORE] = "_";
	keynames[SDLK_BACKQUOTE] = "`";
	keynames[SDLK_a] = "a";
	keynames[SDLK_b] = "b";
	keynames[SDLK_c] = "c";
	keynames[SDLK_d] = "d";
	keynames[SDLK_e] = "e";
	keynames[SDLK_f] = "f";
	keynames[SDLK_g] = "g";
	keynames[SDLK_h] = "h";
	keynames[SDLK_i] = "i";
	keynames[SDLK_j] = "j";
	keynames[SDLK_k] = "k";
	keynames[SDLK_l] = "l";
	keynames[SDLK_m] = "m";
	keynames[SDLK_n] = "n";
	keynames[SDLK_o] = "o";
	keynames[SDLK_p] = "p";
	keynames[SDLK_q] = "q";
	keynames[SDLK_r] = "r";
	keynames[SDLK_s] = "s";
	keynames[SDLK_t] = "t";
	keynames[SDLK_u] = "u";
	keynames[SDLK_v] = "v";
	keynames[SDLK_w] = "w";
	keynames[SDLK_x] = "x";
	keynames[SDLK_y] = "y";
	keynames[SDLK_z] = "z";
	keynames[SDLK_DELETE] = "delete";

	keynames[SDLK_WORLD_0] = "world 0";
	keynames[SDLK_WORLD_1] = "world 1";
	keynames[SDLK_WORLD_2] = "world 2";
	keynames[SDLK_WORLD_3] = "world 3";
	keynames[SDLK_WORLD_4] = "world 4";
	keynames[SDLK_WORLD_5] = "world 5";
	keynames[SDLK_WORLD_6] = "world 6";
	keynames[SDLK_WORLD_7] = "world 7";
	keynames[SDLK_WORLD_8] = "world 8";
	keynames[SDLK_WORLD_9] = "world 9";
	keynames[SDLK_WORLD_10] = "world 10";
	keynames[SDLK_WORLD_11] = "world 11";
	keynames[SDLK_WORLD_12] = "world 12";
	keynames[SDLK_WORLD_13] = "world 13";
	keynames[SDLK_WORLD_14] = "world 14";
	keynames[SDLK_WORLD_15] = "world 15";
	keynames[SDLK_WORLD_16] = "world 16";
	keynames[SDLK_WORLD_17] = "world 17";
	keynames[SDLK_WORLD_18] = "world 18";
	keynames[SDLK_WORLD_19] = "world 19";
	keynames[SDLK_WORLD_20] = "world 20";
	keynames[SDLK_WORLD_21] = "world 21";
	keynames[SDLK_WORLD_22] = "world 22";
	keynames[SDLK_WORLD_23] = "world 23";
	keynames[SDLK_WORLD_24] = "world 24";
	keynames[SDLK_WORLD_25] = "world 25";
	keynames[SDLK_WORLD_26] = "world 26";
	keynames[SDLK_WORLD_27] = "world 27";
	keynames[SDLK_WORLD_28] = "world 28";
	keynames[SDLK_WORLD_29] = "world 29";
	keynames[SDLK_WORLD_30] = "world 30";
	keynames[SDLK_WORLD_31] = "world 31";
	keynames[SDLK_WORLD_32] = "world 32";
	keynames[SDLK_WORLD_33] = "world 33";
	keynames[SDLK_WORLD_34] = "world 34";
	keynames[SDLK_WORLD_35] = "world 35";
	keynames[SDLK_WORLD_36] = "world 36";
	keynames[SDLK_WORLD_37] = "world 37";
	keynames[SDLK_WORLD_38] = "world 38";
	keynames[SDLK_WORLD_39] = "world 39";
	keynames[SDLK_WORLD_40] = "world 40";
	keynames[SDLK_WORLD_41] = "world 41";
	keynames[SDLK_WORLD_42] = "world 42";
	keynames[SDLK_WORLD_43] = "world 43";
	keynames[SDLK_WORLD_44] = "world 44";
	keynames[SDLK_WORLD_45] = "world 45";
	keynames[SDLK_WORLD_46] = "world 46";
	keynames[SDLK_WORLD_47] = "world 47";
	keynames[SDLK_WORLD_48] = "world 48";
	keynames[SDLK_WORLD_49] = "world 49";
	keynames[SDLK_WORLD_50] = "world 50";
	keynames[SDLK_WORLD_51] = "world 51";
	keynames[SDLK_WORLD_52] = "world 52";
	keynames[SDLK_WORLD_53] = "world 53";
	keynames[SDLK_WORLD_54] = "world 54";
	keynames[SDLK_WORLD_55] = "world 55";
	keynames[SDLK_WORLD_56] = "world 56";
	keynames[SDLK_WORLD_57] = "world 57";
	keynames[SDLK_WORLD_58] = "world 58";
	keynames[SDLK_WORLD_59] = "world 59";
	keynames[SDLK_WORLD_60] = "world 60";
	keynames[SDLK_WORLD_61] = "world 61";
	keynames[SDLK_WORLD_62] = "world 62";
	keynames[SDLK_WORLD_63] = "world 63";
	keynames[SDLK_WORLD_64] = "world 64";
	keynames[SDLK_WORLD_65] = "world 65";
	keynames[SDLK_WORLD_66] = "world 66";
	keynames[SDLK_WORLD_67] = "world 67";
	keynames[SDLK_WORLD_68] = "world 68";
	keynames[SDLK_WORLD_69] = "world 69";
	keynames[SDLK_WORLD_70] = "world 70";
	keynames[SDLK_WORLD_71] = "world 71";
	keynames[SDLK_WORLD_72] = "world 72";
	keynames[SDLK_WORLD_73] = "world 73";
	keynames[SDLK_WORLD_74] = "world 74";
	keynames[SDLK_WORLD_75] = "world 75";
	keynames[SDLK_WORLD_76] = "world 76";
	keynames[SDLK_WORLD_77] = "world 77";
	keynames[SDLK_WORLD_78] = "world 78";
	keynames[SDLK_WORLD_79] = "world 79";
	keynames[SDLK_WORLD_80] = "world 80";
	keynames[SDLK_WORLD_81] = "world 81";
	keynames[SDLK_WORLD_82] = "world 82";
	keynames[SDLK_WORLD_83] = "world 83";
	keynames[SDLK_WORLD_84] = "world 84";
	keynames[SDLK_WORLD_85] = "world 85";
	keynames[SDLK_WORLD_86] = "world 86";
	keynames[SDLK_WORLD_87] = "world 87";
	keynames[SDLK_WORLD_88] = "world 88";
	keynames[SDLK_WORLD_89] = "world 89";
	keynames[SDLK_WORLD_90] = "world 90";
	keynames[SDLK_WORLD_91] = "world 91";
	keynames[SDLK_WORLD_92] = "world 92";
	keynames[SDLK_WORLD_93] = "world 93";
	keynames[SDLK_WORLD_94] = "world 94";
	keynames[SDLK_WORLD_95] = "world 95";

	keynames[SDLK_KP0] = "[0]";
	keynames[SDLK_KP1] = "[1]";
	keynames[SDLK_KP2] = "[2]";
	keynames[SDLK_KP3] = "[3]";
	keynames[SDLK_KP4] = "[4]";
	keynames[SDLK_KP5] = "[5]";
	keynames[SDLK_KP6] = "[6]";
	keynames[SDLK_KP7] = "[7]";
	keynames[SDLK_KP8] = "[8]";
	keynames[SDLK_KP9] = "[9]";
	keynames[SDLK_KP_PERIOD] = "[.]";
	keynames[SDLK_KP_DIVIDE] = "[/]";
	keynames[SDLK_KP_MULTIPLY] = "[*]";
	keynames[SDLK_KP_MINUS] = "[-]";
	keynames[SDLK_KP_PLUS] = "[+]";
	keynames[SDLK_KP_ENTER] = "enter";
	keynames[SDLK_KP_EQUALS] = "equals";

	keynames[SDLK_UP] = "up";
	keynames[SDLK_DOWN] = "down";
	keynames[SDLK_RIGHT] = "right";
	keynames[SDLK_LEFT] = "left";
	keynames[SDLK_DOWN] = "down";
	keynames[SDLK_INSERT] = "insert";
	keynames[SDLK_HOME] = "home";
	keynames[SDLK_END] = "end";
	keynames[SDLK_PAGEUP] = "page up";
	keynames[SDLK_PAGEDOWN] = "page down";

	keynames[SDLK_F1] = "f1";
	keynames[SDLK_F2] = "f2";
	keynames[SDLK_F3] = "f3";
	keynames[SDLK_F4] = "f4";
	keynames[SDLK_F5] = "f5";
	keynames[SDLK_F6] = "f6";
	keynames[SDLK_F7] = "f7";
	keynames[SDLK_F8] = "f8";
	keynames[SDLK_F9] = "f9";
	keynames[SDLK_F10] = "f10";
	keynames[SDLK_F11] = "f11";
	keynames[SDLK_F12] = "f12";
	keynames[SDLK_F13] = "f13";
	keynames[SDLK_F14] = "f14";
	keynames[SDLK_F15] = "f15";

	keynames[SDLK_NUMLOCK] = "numlock";
	keynames[SDLK_CAPSLOCK] = "caps lock";
	keynames[SDLK_SCROLLOCK] = "scroll lock";
	keynames[SDLK_RSHIFT] = "right shift";
	keynames[SDLK_LSHIFT] = "left shift";
	keynames[SDLK_RCTRL] = "right ctrl";
	keynames[SDLK_LCTRL] = "left ctrl";
	keynames[SDLK_RALT] = "right alt";
	keynames[SDLK_LALT] = "left alt";
	keynames[SDLK_RMETA] = "right meta";
	keynames[SDLK_LMETA] = "left meta";
	keynames[SDLK_LSUPER] = "left super";	// "Windows" keys
	keynames[SDLK_RSUPER] = "right super";
	keynames[SDLK_MODE] = "alt gr";
	keynames[SDLK_COMPOSE] = "compose";

	keynames[SDLK_HELP] = "help";
	keynames[SDLK_PRINT] = "print screen";
	keynames[SDLK_SYSREQ] = "sys req";
	keynames[SDLK_BREAK] = "break";
	keynames[SDLK_MENU] = "menu";
	keynames[SDLK_POWER] = "power";
	keynames[SDLK_EURO] = "euro";
	keynames[SDLK_UNDO] = "undo";
*/

void restms(int mils) { // a sort of busy wait to catch keyboard input

//#ifdef __NOSDL__
  napms (mils);
//#else
//
//  while (mils > 0) {
//    if (!hasevent) hasevent = SDL_PollEvent( &event );
//    napms(50);
//    mils -= 50;
//  }
//#endif
}

int pollevent () {
#ifdef __NOSDL__
  return !(getch() == ERR);
#else
  if (!hasevent) hasevent = SDL_PollEvent( &event );
 return hasevent;
#endif
}

void setblocking(bool _blocking) {
#ifndef __NOSDL__
  blocking = _blocking;
#else
  if (_blocking)
    nodelay(stdscr, 0); // blocking mode
  else
    nodelay(stdscr, 1); // non-blocking mode
#endif
}
