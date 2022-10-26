@echo off

set cmake="C:\Program Files\CMake\bin\cmake"
:: "C:\Qt5.14\5.14.0\msvc2017_64"
set qt=%1
set cfg=Release

cd bin
%cmake% ..\Superbuild -DCMAKE_BUILD_TYPE=%cfg% -G"Visual Studio 15 2017" -Ax64 -DQt5_PATH:PATH=%qt%
%cmake% --build . --config %cfg% -j6

for %%D in (Qt5Core Qt5Gui Qt5Network Qt5OpenGL Qt5PrintSupport Qt5Svg Qt5Widgets) do (copy %qt%\bin\%%D.dll SCIRun\%cfg%\) 
copy Externals\Build\Qwt_external\src\%cfg%\qwt.dll SCIRun\%cfg%\
copy Externals\Source\Python_external\PCbuild\amd64\python38.dll SCIRun\%cfg%\
cd SCIRun\%cfg%\
mkdir platforms
cd ..\..
xcopy /s %qt%\plugins\platforms SCIRun\%cfg%\platforms\
