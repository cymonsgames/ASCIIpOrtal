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

#include <string>
#include <iostream>
#include <curses.h>

#include "ap_sound.h"

#define NUMBER_VOICES 10

#ifndef __NOSOUND__
#ifndef __NOSDL__

#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
using namespace std;

#include "ap_filemgr.h"

// from main.cpp
extern FileManager filemgr;

const int num_music_files = 5;
string music_files[num_music_files] = {
  "ascii.ogg",
  "ascii_threat2.ogg",
  "ascii_threat3.ogg",
  "yorkiimusic.ogg",
  "ascii_threat4.ogg"
};

string sound_files[MAXSound] = {
  "stub.ogg", // SILENT
  "win.ogg", // WIN
  "sizzle.ogg", // SIZZLE
  "collapseportals.ogg", // COLAPSEPORTALS
  "crush.ogg", // CRUSH
  "switchhit.ogg", // SWITCHHIT
  "dooropen.ogg", // DOOROPEN
  "doorclose.ogg", // DOORCLOSE
  "dispenser.ogg", // DUPLICATE
  "keyhit.ogg", // MENUBEEP
  "menuselect.ogg", // MENUSELECT
  "gunshot.ogg", // GUNSHOT
  "portalshot.ogg", // PORTALCREATE
  "portalmiss.ogg", // PORTALFAIL
  "portalthrough.ogg", // THROUGH
  "glad1.ogg", // VOICE
  "glad2.ogg", // VOICE2
  "glad3.ogg", // VOICE3
  "glad4.ogg", // VOICE
  "glad5.ogg", // VOICE2
  "glad6.ogg", // VOICE3
  "glad7.ogg", // VOICE
  "glad8.ogg", // VOICE2
  "glad9.ogg", // VOICE3
  "glad10.ogg", // VOICE3
};

string current_music = music_files[0];

Mix_Chunk* soundEffects[MAXSound];

Mix_Music* ambience;

int sound_init () {
  //Initialize SDL_mixer
  if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 1024 ) == -1 ) return false;

  for (int c = 0; c < MAXSound; c++)
    soundEffects[c] = Mix_LoadWAV( filemgr.get_media(sound_files[c]).c_str() );

  ambience = Mix_LoadMUS( filemgr.get_media(music_files[0]).c_str() );

  if (ambience == NULL) return 0;
  return 1;
}

int default_ambience (int selection) {
  if ((selection < 0) || (selection > num_music_files)) selection = 0;
  if (current_music == music_files[selection]) return 2;
  if( Mix_PlayingMusic() == 0 ) Mix_HaltMusic();
  Mix_FreeMusic(ambience);
  ambience = Mix_LoadMUS( filemgr.get_media(music_files[selection]).c_str() );
  if (ambience == NULL) return 0;
  current_music = music_files[selection];
  return 1;
}

// Loads a media-located music file.
int load_ambience(string filename) {
  if (filename == current_music) return 2;
  if( Mix_PlayingMusic() == 0 ) Mix_HaltMusic();
  Mix_FreeMusic(ambience);
  
  ambience = Mix_LoadMUS( filemgr.get_media(filename).c_str() );
  if (ambience == NULL) {
    default_ambience(0);
    return 0;
  }
  current_music = filename;
  return 1;
}

// Loads a map pack-located music file.
// Note that upon failure to find the music file in the map pack
// directory, this falls back to the standard media directory.
int load_ambience (MapPack const & mappack, string filename) {
  if (filename == current_music) return 2;
  if( Mix_PlayingMusic() == 0 ) Mix_HaltMusic();
  Mix_FreeMusic(ambience);
  
  ambience = Mix_LoadMUS( mappack.filemgr.get_media(filename).c_str() );
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

int play_voice () {
  return play_sound(VOICE + rand() % NUMBER_VOICES);
}

void deinit_sound () {
  Mix_FreeMusic( ambience );
  for (int c = 0; c < MAXSound; c++) Mix_FreeChunk(soundEffects[c]);
  Mix_CloseAudio();
}
#endif
#endif
