// ASCIIpOrtal ver 1.2 by Joseph Larson
// Copyright (c) 2009 Joseph Larson
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

#include <string>
#include <curses.h>
#include "asciiportal.h"

#ifndef __NOSOUND__
#ifndef __NOSLD__

#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
using namespace std;
#include "ap_sound.h"

// from main.cpp
extern string basepath;
extern string userpath;

const int num_music_files = 5;
string music_files[num_music_files] = {
  "ascii.ogg",
  "ascii_threat2.ogg",
  "ascii_threat3.ogg",
  "yorkiimusic.ogg",
  "ascii_threat4.ogg"
};

string sound_files[MAXSound] = {
  "stub.wav", // SILENT
  "win.wav", // WIN
  "sizzle.wav", // SIZZLE
  "collapseportals.wav", // COLAPSEPORTALS
  "crush.wav", // CRUSH
  "switchhit.wav", // SWITCHHIT
  "dooropen.wav", // DOOROPEN
  "doorclose.wav", // DOORCLOSE
  "dispenser.wav", // DUPLICATE
  "keyhit.wav", // MENUBEEP
  "menuselect.wav", // MENUSELECT
  "gunshot.wav", // GUNSHOT
  "portalshot.wav", // PORTALCREATE
  "portalmiss.wav", // PORTALFAIL
  "portalthrough.wav", // THROUGH
  "glad1.wav", // VOICE
  "glad2.wav", // VOICE2
  "glad3.wav", // VOICE3
  "glad4.wav", // VOICE
  "glad5.wav", // VOICE2
  "glad6.wav", // VOICE3
  "glad7.wav", // VOICE
  "glad8.wav", // VOICE2
  "glad9.wav", // VOICE3
  "glad10.wav", // VOICE3
};

string current_music = music_files[0];

Mix_Chunk* soundEffects[MAXSound];

Mix_Music* ambience;

int sound_init () {
  string name;
#ifdef WIN32
  string mediapath = basepath + "media\\";
#else
  string mediapath = basepath + "media/";
#endif

  //Initialize SDL_mixer
  if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 1024 ) == -1 ) return false;

  for (int c = 0; c < MAXSound; c++) {
    name = mediapath + sound_files[c];
    soundEffects[c] = Mix_LoadWAV(name.c_str());
    name.clear();
  }

  name = mediapath + music_files[0];
  ambience = Mix_LoadMUS(name.c_str ());
  name.clear();
  if (ambience == NULL) return 0;
  return 1;
}

int default_ambience (int selection) {
  string name;

  if ((selection < 0) || (selection > num_music_files)) selection = 0;
  if (current_music == music_files[selection]) return 2;
  if( Mix_PlayingMusic() == 0 ) Mix_HaltMusic();
  Mix_FreeMusic(ambience);
#ifdef WIN32
    name = basepath + "media\\" + music_files[selection];
#else
    name = basepath + "media/" + music_files[selection];
#endif
  ambience = Mix_LoadMUS(name.c_str ());
  if (ambience == NULL) return 0;
  current_music = music_files[selection];
  return 1;
}

int load_ambience (string mappack, string filename) {
  string name;

  if (filename == current_music) return 2;
  if( Mix_PlayingMusic() == 0 ) Mix_HaltMusic();
  Mix_FreeMusic(ambience);
#ifdef WIN32
  name = userpath + mappack + "\\" + filename;
#else
  name = userpath + mappack + "/" + filename;
  // handles custom maps in user directory and default maps
  ambience = Mix_LoadMUS(name.c_str ());
  if (ambience == NULL)
    name = basepath + mappack + "/" + filename;
#endif
  ambience = Mix_LoadMUS(name.c_str ());
  if (ambience == NULL) {
    default_ambience(0);
    return 0;
  }
  current_music = filename;
  return 1;
}

int start_ambience () {
  if (Mix_PlayingMusic() == 1 ) return 2;
  if (Mix_PlayMusic( ambience, -1 ) == -1 ) return 0;
  return 1;
}

void stop_ambience () {
  Mix_HaltMusic();
}

void toggle_ambience () {
  if( Mix_PausedMusic() == 1 ) Mix_ResumeMusic();
  else Mix_PauseMusic();
}

int play_sound (int c) {
  if (Mix_PlayChannel( -1, soundEffects[c], 0 ) == -1) return 0;
  return 1;
}

void deinit_sound () {
  Mix_FreeMusic( ambience );
  for (int c = 0; c < MAXSound; c++) Mix_FreeChunk(soundEffects[c]);
  Mix_CloseAudio();
}
#endif
#endif
