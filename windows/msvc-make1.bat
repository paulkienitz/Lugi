rem build or check one source file with MSVC compiler
@echo off
if "%LIBPATH%" == "" (
    echo Run this in a VS Developer Command Prompt window.
) else (
    CL /FoWindows\ob\ /c /Zi /RTC1 /guard:cf %1.c
)
