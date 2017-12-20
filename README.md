# ASCIIpOrtal

**By Joe Larson**

**Sound Design by Steve Fenton.**

Inspired by [Increpare's Portile game](http://www.increpare.com/2008/11/portile/) and [Super Serif Bros](http://foon.co.uk/farcade/ssb/) and Valve's Portal.

Check http://cymonsgames.com/asciiportal for updates or our Github page
at https://github.com/cymonsgames/ASCIIpOrtal

---
## HOW TO PLAY

Grab your hand-held portal device and enter the test chambers for a
non-euclidean good time.

ASCIIpOrtal is a text based puzzle game inspired by the popular video game.
In ASCIIpOrtal you overcome challenges by placing portal way-points, joining
two points in the map. If they player or any object passes through one portal
way-point it will seamlessly exit the other. Since both way-points are the
same point on the map the player's view through the portal reflects this and
space warps around you as you pass through the portal.

From the main menu you can choose to 'Begin' from the first map, 'Select
Level' to visit any level you've already beaten, 'Change Map Set' to load a
new set of levels from any map set subdirectory in your ASCIIpOrtal
directory, Read the 'Instructions', view the map set's 'Credits', or 'Quit'
the game.
```
Within the game you can use the following keys to play:
Left/Right Arrows . Move Left/Right
Up/Down Arrows .... Move Up/Down Ladders
                    or adjust aim.
Number Pad Keys ... Quick Aim.
Z/0 ............... Fire Blue Portal
X/PERIOD .......... Fire Yellow Portal
SPACE/5 ........... Fire 'Next' Portal
C ................. Clear Portals

ESC/F1 ............ Menu
P ................. In-game Pause
F2 ................ Change Portal Render
F3 ................ Set to Default Speed
+ ................. Increase Game Speed
- ................. Decrease Game Speed
R ................. Restart Level
M ................. Mute/Unmute music
? ................. This menu
```
The goal of each level is to make it to one of exits on the level. Exits
appear as an E on a green field.

---
## USING PORTALS

Portals link 2 places on the map and make them as one place. Remember that
you can aim diagonally to aid portal placement, except when on ladders. While
on ladders your aim remains steady and can only be adjusted using the Number
Pad for quick aim.

When passing through portals if the in direction and out direction are
different then any object passing through the portal will exit it rotated,
preserving momentum. This opens up several techniques:

**HOROZONTAL FLING** - Fall into a portal placed on the floor from a height and
exit a portal on the wall. You will travel a horizontal distance relative to
how far you fell before gravity will take over.

**VERTICAL FLING** - Fall into a portal placed on the floor from a height and
exit a portal placed on the floor. You will travel a distance up relative to
how far you fell. It's important to note that the distance you fall up will
be less than the height you fell from, so vertical re-flings will eventually
fall to zero.

**OBJECT RELOCATION** - Occasionally you will need get objects from where they
are and pushing them either will not work (in the case of a box stuck in the
corner) or is not possible (in the case of boulders). Placing a portal under
or in the path of an object can aid in getting things where you want them.

For any other use of portals experimentation is key.

---
## STATS

After each level you will be given your stats for that level. They are:

**Game beats** - How many in game "turns" it took you.

**Portals Used** - How many portals you opened in the last successful run. Missed
               placements that don't open don't count against your score.

**Deaths/Restarts** - How many times you had to try to get that last successful run.

---
## MAKING YOUR OWN CHALLENGES

Please see the included `MAP_MAKING_TIPS.md` for the how-to on making your own
challenging maps.

---
## COMMAND LINE PROMPTS

```
 -?                  This help menu
 -m [mappackname]    load a map pack
 -r <width>x<height> Choose the screen resolution (default: 640x480)
 -f                  Fullscreen mode
 -w                  Windowed mode (default)
```
Please visit Cymon's Games and register on the forums
(http://cymonsgames.com/forum).

Any comments or critisism you can either post on the forums or e-mail me
directly at joealarson@gmail.com.

---
## KNOWN ISSUES

* If a portal is on one side of a wall, and you attempt to place a portal on
  the other side of the same wall in the same time you will not be allowed.
  This is because both portals, despite not being invisible to each other,
  will be trying to occupy the same space. Does not warrant the time to fix it.
* See the issue tracker on Github

---
## THANK YOU

Thanks again to Erin Robinson who without which I would not have been as well
known as quickly. It was her tweet that got me on Kotaku, Boing Boing, Wired,
Joystiq, and just about every other games news website.

Thanks to Nicholas McGuire, Mads Lund, arminwagner, Sparky, and i300 for
designing levels during the beta that were used in the initial public
release. Thank you to everyone else who helped with the beta testing.
