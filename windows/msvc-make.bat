@rem Build Lugi for Windows command line with MSVC... used for official release.
@echo off
if "%LIBPATH%" == "" (
    echo Run this in a VS Developer Command Prompt window.
) else (
    del Windows\vc140.pdb
    rem don't bother to set up dependencies in a makefile or equivalent - just build em all
    CL /FoWindows\ob\ /FeWindows\Lugi.exe /Zi /RTC1 /guard:cf advapi32.lib shlwapi.lib *.c
    move vc140.pdb Windows
)
