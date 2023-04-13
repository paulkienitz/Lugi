#!
# Build Lugi with emcc... no makefile, just compile everything.  $1 is often -D_DEBUG.
pushd ~/lugi
emcc -Wno-dangling-else -Wno-switch -sENVIRONMENT=web -sASYNCIFY $1 -o emscripten/Lugi.js *.c
popd
