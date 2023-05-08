#!
# Build Lugi with clang... no makefile, just compile everything.  This is used for my official release.
# $1 is often -D_DEBUG.  $2 can be -DMAP_TEST_HARNESS.
pushd ~/lugi
clang -g -Wno-dangling-else -Wno-switch $1 $2 -o unix/Lugi *.c
chmod u=rwxs,go=rx unix/Lugi
popd
