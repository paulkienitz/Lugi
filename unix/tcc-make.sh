#!
# Build Lugi with tcc... no makefile, just compile everything.  The $1 arg is often -D_DEBUG.
pushd ~/lugi
tcc -g -b $1 $2 -o unix/Lugi *.c
# this is a valuable compiler as -b gives it full bounds checking on arrays and mallocs
chmod u=rwxs,go=rx unix/Lugi
popd
