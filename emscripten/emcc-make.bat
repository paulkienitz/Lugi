@rem build Lugi with emcc for Windows... no makefile, just compile everything... the $1 arg is often either -D_DEBUG or -O3.
@rem (On my system, and maybe yours, this needs to be run with administrator privileges.)

emcc -Wno-dangling-else -Wno-switch -sENVIRONMENT=web -sASYNCIFY %1 -o emscripten\Lugi.js lugi.c diction.c drop.c embassy.c fiddle.c gothere.c obey.c player.c put.c roomz.c take.c upchuck.c basics.c
