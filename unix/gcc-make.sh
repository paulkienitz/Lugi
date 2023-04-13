#!
# Build Lugi with gcc... no makefile, just compile everything.  $1 is often -D_DEBUG.
pushd ~/lugi
gcc -g $1 $2 -o unix/Lugi *.c
chmod u=rwxs,go=rx unix/Lugi
popd
