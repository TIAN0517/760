@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cl /LD /MD /EHsc /I"." /D_UNICODE /DUNICODE LauncherDll\*.cpp LauncherDll\hook\*.cpp LauncherDll\Utility\*.cpp /link /DEF:LauncherDll\LauncherDll.def ws2_32.lib advapi32.lib /OUT:LauncherDll.dll
