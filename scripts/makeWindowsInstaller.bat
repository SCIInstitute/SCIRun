@echo off

title SCIRun 5 Windows Installer
echo Building SCIRun 5 Windows Installer

set tag=%1
set buildDir="installerbuild_%tag%"

echo %1
echo %tag%
echo %buildDir%

mkdir %buildDir%
cd %buildDir%

git clone https://github.com/SCIInstitute/SCIRun.git
cd SCIRun
git checkout %tag%
cd "C:\"
mkdir iBin
cd iBin
"C:\Program Files\CMake\bin\cmake" -DBUILD_TESTING:BOOL=OFF -DWITH_TETGEN:BOOL=OFF -G "Visual Studio 12 2013 Win64" -DQT_QMAKE_EXECUTABLE:FILE="C:/Dev/Qt-4.8.6-2013/bin/qmake.exe" %buildDir%\SCIRun\Superbuild 
REM "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe" Superbuild.sln /Build Release /log superLog.txt
REM cd SCIRun
REM "C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe" SCIRun.sln /Build Release /log srLog.txt 
REM rm LATEST_TAG.txt
REM echo $VERSION >> LATEST_TAG.txt
REM git add LATEST_TAG.txt
REM git commit -m "Tagged release $VERSION"
REM git push origin master
REM git tag -a $VERSION -m "Release $VERSION"
REM git push --tags
