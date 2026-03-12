#!
# Build Lugi with gcc... no makefile, just compile everything.  $1 is often -D_DEBUG.
pushd ~/lugi/C
echo 'Compiling with gcc -g'
gcc -g $1 $2 -o unix/Lugi *.c
chmod u=rwxs,go=rx unix/Lugi
ls -l unix/Lugi
popd
