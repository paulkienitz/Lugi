# Lugi
### an old text adventure game from the eighties, revived

##      About the Game

Lugi is a text adventure game written way back in 1980-81 by me and my friend Jay Wilson, who started the project.
For those who don’t know, text adventures are an ancient genre of computer game which has no graphics.
The program describes your character’s surroundings, and you type in commands to tell it what your character does, such as to walk in a given direction or to pick up or use some item.
This type of game is also more pretentiously known as interactive fiction, but that term was not in use back in those days.

Traditional text adventures like Colossal Cave Adventure, Mystery Mansion, or the Zork series generally give the user a rather slow-paced game experience centered around learning the environment and solving puzzles.
Once all puzzles are solved, there is relatively little replay value.
Lugi is different.
First of all, the map is not the same from one run to the next.
And second, you have limited time — only a certain number of minutes at the keyboard to complete a run of the game.
This means that if you take the time to carefully write a map, as is common practice in classical text adventures, you are unlikely to complete it before you character expires.
But if you don’t use one, you are never going to find everything.
This makes finishing the game all about balancing risk and reward, with every run having a different outcome.
It’s interactive fiction for speedrunners.
Play is expected to be messy and chaotic, so the game does not in any way take itself seriously.

Lugi can be run from a command prompt on many different systems from my old Amiga to my current phone, but of course for it to reach the public nowadays, it has to be on the web.
So I made a web page that emulates a primitive scrolling text display like we used to use back in the timesharing days, with Lugi running in it.
This page is included in the repo as play.html in the emscripten folder.
(The page referenced by the "about the game" link in it is not included.
That page's content is similar to the About the Game section you are now reading.)
_Be aware that refreshing that page restarts the game._

The source code is designed to be easily ported to any platform old or new, though it will not fit in something like a Commodore 64 — an 8088 PC is probably about as primitive as you’d want to go.
It’s written in C, though the eighties original was in Pascal.
Much of the C translation was done in the early nineties, but it wasn’t finished then.
I will ask that you please don’t read the source code just to figure out the tricks in the game.
Lots of players back then discovered them through gameplay (it was pretty popular), and one or two even outplayed me at it.

##      About the Code

The basic design idea of the source archive is that everything is platform-independent, except for a primitive layer in the file `basics.c`, and a few conditionals in the `lugi.h` include file.
In `basics.c` we actually redefine some familiar stdio functions to pass them through our own output stream, which knows how to check the window it's running in for width, height, and ANSI color support.
These checks are specific to each targeted platform.
With this knowledge, it word-wraps the text to fit the current width, and uses “`-- MORE --`” prompts to break up output taller than the current height.
In the case of the web target, the actual IO does not go through stdio at all, but through JavaScript functions.

Each target platform has a subdirectory in the source archive, but these do not contain any .c or .h source files.
They only contain makefiles or scripts for building an executable for that target.
In some cases, more than one compiler is supported.

* The unix folder has simple shell scripts to build with gcc, with clang, or with tcc.
* The windows folder has a batch script for MSVC, a makefile for Open Watcom, and a Visual Stupido project.
(It also has a batch script for TCC, but this is currently broken as it does not support the Windows APIs needed for console analysis.)
* The msdos folder only supports Open Watcom (another compiler was there in the past but is long gone).
* The amiga folder only supports Manx Aztec C, not SAS/C.
* Finally, the emscripten folder supports emcc with a shell script, which produces a `Lugi.wasm` module accompanied by a `Lugi.js` wrapper.

Each folder also has an executable in the repo.
In the case of the windows folder, this was produced with the MSVC script.
In the case of the unix folder, this was produced with the clang script, with the implicit target being `x86_64-pc-linux-gnu`.
(A target of `aarch64-unknown-linux-android24` has also been tested.)

Each executable attempts to load and save high scores in a file within its own home directory.
In some cases on legacy platforms, it may be unable to determine this directory.
If this fails in MS-DOS I’m not sure where it might end up — the user’s current directory, maybe.
If this fails on the Amiga it will use the `S:` folder, where assorted scripts and configuration files are stored.
