@rem build Lugi with tcc for Windows... no makefile, just compile everything... the $1 arg is often -D_DEBUG.

@echo THIS IS NOT CURRENTLY EXPECTED TO WORK, due to limited support for Windows DLLs.
@rem  It used to work, before I added some API calls.
@rem  Whoever wants to try to get the .h and .def files set up has some work cut out for them.

tcc -g -b %1 -o windows\Lugi.exe -ladvapi32 -lshlwapi *.c
@rem this is a valuable compiler as -b gives it full bounds checking on arrays and mallocs
