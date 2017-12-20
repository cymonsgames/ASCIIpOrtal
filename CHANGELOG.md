1.3 (not released yet)
* /!\ Now features recursive drawing of portals!
* New object management that should be a lot more efficient.
* A lot of bugfixes in the menus (were taking 100% CPU).
* Added the long-awaited menu to select a map pack.
* 14 map packs bundled.
* Now rely on Yaml for save data and config files; most notably, map
  packs now have a lot of associated data: description, author, difficulty...
* Made things a lot more unix-friendly: uses '~/.asciiportal/' to store
  local data and optionally '/usr/share/asciiportal/' for game data.
* Save data is persistent across reinstallation (%APPDATA% on win32,
  ~/.asciiportal otherwise)
* Map pack's 'credits.txt' now gets added to the default credits.
* The nosdl port (truly ASCII!) should be roughly working now.
* OS X port with the latest version.

1.2c bugfixes.

1.2a (Nov 18 2009)
* WADS keys added.
* Music files changed to OGG.
* Menu item for changing map set disabled.
* 5th default ambient music added.
* Number-pad input is fixed.
* Objects in portals when they collapse are more properly delt with.

1.2 (Nov 14 2009)
* Controls finally fixed to react instantly. For real this time.
* Original levels reduced from 50 to 25. Extra levels moved to their own pack.
* 4 default musics available for custom map makers.
* Visual effect added when portals change.
* "Next" for portal gun increments when portal is created.
* Key to turn off all portals ('C') added.
* Game speed is now variable.
* Code hopefully compiles nativly for dingoo and GP2X devices.
* There is a known issue involving mouse movement being caught as input.

1.1 (Oct 21 2009) - Bug fix release:
* Physics bugs concerning boulder movements have been corrected.
* Player will pass through portals determined by aiming direction.
* The keyboard delay pause has been considerably scaled back. (You still may
  need to turn before moving, so there may be a pause there.)
* Some levels have been adjusted slightly.
* Comes with all known extra levels and new map packs.
* Player will now "catch" ladders mid fling.
* Level stats no longer try to keep track of time.
* You can now adjust your aim if at the top or bottom of a ladder.
* Portals will no longer count doors as valid walls when calculating normals.
* Portals will no longer open up straight to a chain link wall.

1.0 (Sept 16 2009) - First public release.
