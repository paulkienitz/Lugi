#!
# Build Lugi with emcc... no makefile, just compile everything.  $1 is often -D_DEBUG but defaults to -O3.
pushd ~/lugi
echo using compilation option ${1:--O3}
emcc -Wno-dangling-else -Wno-switch -sENVIRONMENT=web -sASYNCIFY ${1:--O3} -o emscripten/Lugi.js *.c
popd
# This is now the primary build of Lugi that is visible to the public.
# See http://paulkienitz.net/Lugi/ to play that build.
