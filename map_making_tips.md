# Guide For Map Makers

Making maps in ASCIIpOrtal is very simple to understand, but hard to master.

The following guide is intended for those writing their levels by hand. If
you are using a map editor like the one by Mads Lund there are still many
useful tips and tricks in this guide that will enhance you level designing.

---
## BASICS

Maps are simply text files that all have the same root name with incrementing
numbers after the root name in a directory with the same root name. By
default `maps` is the root name ASCIIpOrtal looks in first for maps unless
you pass it another root name to look in when you run it. So the first map it
will load will be `maps/maps001.txt`. The next is `maps/maps002.txt` and so
on. You can place your levels in the "extralevels" directory and test them
using the "ASCIIpOrtal_extra_levels.bat" file.

I recommend "Notepad++" for text editing. If you already have a text editor
that will probably work okay. If you are using plain vanilla notepad please
do yourself a favor and try something else out, it will help.

Each map must have one and only one starting point for the player indicated
by an "@", and at least one exit marked by a "Q". If there are more or less
"@"s the level will not start. If there are no "Q"s the level will never end.

There are two basic wall types. The first is the normal wall that portals can
stick to. In the map file the character for these is "#". You can also have
non-stick walls that portals can not stick to. These are signified by "N" in
the map file.

With this you have all you need to create a level. The most basic level in
ASCIIpOrtal would look like this:
```
#########
# @   Q #
#########
```
Tho this isn't very interesting. To really get the game going you need to add
some objects.

---
## TABLE OF CHARACTERS

Below is the list of all objects recognized in ASCIIpOrtal and a short
description of each.:
```
' ' - Nothing, empty space. You can move un-obstructed through it.
'Q' - Goal, where the player ultimately is trying to go.
'+' - Ladder. This allows for movement vertically.
'#' - Normal wall. Portals stick to it. Nothing goes through it.
'N' - Nonstick wall. Like the normal wall but with a finish that portals
      don't like.
'(' - Left Conveyer. The player or any companion cubes on a treadmill will be
      moved automatically every turn.
')' - Right Conveyer. Similar to the left conveyer in many respects.
'$' - Chain Link. Portals can fire through, but objects will not pass through.
'%' - Particle Field. These special fields will destroy any object save the
      player that pass through them, and they will disrupt the portal gun as it
      passes through, closing all open portals.
'"' - Laser Field. Touching these is ill-advised. You can shoot portals
      through them.
'X' - Spike, deadly to the player, boulders and boxes will sit on top of them
      safely. You can shoot through them.
'=' - Dispenser. If a boulder or companion cube is on top of the dispenser
      then a copy will appear below the dispenser at regular intervals. The
	  copy will have all the attributes of the object it is a copy of
	  including, in the case of boulders, the direction traveling.
'A' .. 'F' - Toggle Switch. If all toggle switches of the same identifier are
      depressed then all doors of the same identifier will open, and will
	  permanently remain open.
'G' .. 'M' - Momentary Switches. As long as all momentary switches of the
      same identifier are depressed then the doors of the same identifier
	  will remain open. Once one of the switches is not depressed the door
	  will close.
'a' ... 'm' - The doors that correspond to the switches above.
'&' - A companion cube. These can be pushed by the player or a boulder.
'O' - Boulder. Constantly in motion, if the boulder hits the player it will
      crush them. However under certain conditions the player can stand and
	  walk across the tops of boulders. Using this in your map file the
	  boulder will choose in initial random direction.
'<' - Left traveling boulder. Same as above except you control in this way
      the initial direction.
'>' - Right traveling boulder.
'@' - Where they player starts.
'1'..'9' Text message triggers. When any one of a type of text trigger is
      touched by the player all of that type of text trigger will be removed
	  from the map.
```
---
## NUANCES AND SUBTELTIES

There are certain interactions that a map maker may need to be aware of:

* There is no limit to how tall or wide or short or narrow a map can be.

* Portals are unidirectional. As such they will not appear on blocks that
  make up corners or bends. Only where there is a block to the left and right
  of the block that is shot at will they appear. You need to be sure to
  include enough flat surface if you intend for a portal to be shot.

* Conveyer belts count both as surfaces portals will appear on as well as for
  calculating whether a portal can appear.

* Doors affected by the toggle switches ('A' through 'F') do not close once
  they've been triggered.

* For the momentary switches ('G' through 'M') the doors will only remain
  open while there is something on top of them. Doors also take a count of 4
  to open. Consequently boulders do not open these doors very well.

* Switches can be triggered by the player, companion cubes, boulders, or even
  other doors.

* All switches of the same type must be depressed at the same time before the
  corresponding doors will open, so multiple switches will require the
  coordinated use of boxes, boulders, the and or the player.

* The dispenser only works if there's a box or a boulder above it and nothing
  beneath it. If the way is obstructed by a door, a boulder, the player, or
  anything else it will not operate.

* Players and boulders can push companion cubes until the cube's paths is
  obstructed.

* Players will step up on immovable boxes, walls or even doors that are a
  maximum of 1 block high. In this way doors could be used to build temporary
  stair cases that will disappear when a switch is triggered.

* The fling works by building up momentum from a fall, then falling directly
  into a portal that exits in a non-down direction. The object or player will
  then coast for a distance related to how far they fell but limited by a
  max, before gravity will take over again.

* A ladder will stop a player's fling, but not cubes or boulders as they
  don't have hands to grab hold.

* When encountering a ladder 50% of the time a boulder will take the ladder
  and 50% of the time it will roll over it. When it lands it will continue in
  the same direction it started in.

* If a boulder falls on a ladder it will simply continue falling through it
  until it hits solid ground.

* When passing through portals that are not aligned a boulder may change
  directions. The direction it chooses to go is random.

* Switches can also be triggered by text triggers, tho the behavior is a bit
  confusing at first. Text triggers are invisible and once one of a type is
  triggered all of that type are removed from the map. If one of that type is
  sitting on a momentary switch ("G" through "M") the switch, which had been
  on because the text event was on top of it, will be turned off, closing
  whatever door it's related to. Since doors can trigger switches you'd then
  need 2 switches to activate a switch when a text trigger is activated. Map
  001 and 002 have examples of this.

* Place your own notes here and send the changes to joealarson@gmail.com.

---
## SPECIAL LINES

At the start of each level a message can be displayed across the top, banner
style. This message is taken from the map data. You must start a line with
the word "message:" (case sensitive) and the proceed with the message you
want to be displayed. While the message line can occur anywhere in the map
file it makes most sense to put it in the first or last line.

Additionally you can have text trigger messages that appear whenever the
corresponding text trigger is touched by the player. In the map data the
triggers are simply numbers and the messages are identified by a line that
starts "message1:", "message2:" etc. Once a message is triggered the message
will be displayed and all message objects of the same type will be removed
from the map, so don't feel bad about putting a whole block of text triggers
to make sure your message gets across.

A line that starts "music:" will tell the game to look in the map pack
directory for an MP3 or OGG file and play that as background music. There
are 5 default musics to choose from. "music:defaut" or "music:defaut0" will
play the first default level music. "music:defaut1" through "music:defaut5"
will play the others.  If a level does not have a "music:" line it will
simply continue playing whatever background music it was playing, even if
that music is from a later level that the player level selected into. In
other words the game makes no attempt to figure out what music the level
"should" be playing if you don't tell it. If the mp3 described is already
playing it will simply continue playing it without interruption.

---
## FILE NAMES

If ASCIIpOrtal is passed the runtime paraamter of '-m ' followed by the name
of a directory upon run time it will look for that directory and begin
reading files from there. If it has not had any parameter passed to it the
default directory is "maps" with "001.txt" being the first file it looks for.
It will continue incrementing the file games until it stops finding maps.
