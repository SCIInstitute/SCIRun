@echo off

set cmake="C:\Program Files\CMake\bin\cmake"
:: "C:/Qt5.14/5.14.0/msvc2017_64"
set qt=%1
set cfg=Release

cd bin
%cmake% ..\Superbuild -DCMAKE_BUILD_TYPE=%cfg% -G"Visual Studio 15 2017" -Ax64 -DQt5_PATH:PATH=%qt%
%cmake% --build . --config %cfg% -j6
