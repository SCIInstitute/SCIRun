---
title: Build
category: info
tags: build
layout: default_toc
---

<link rel="stylesheet" href="css/main.css">

## Platform Notes

**At least C++11 64-bit compiler support is required.**

### Build requirements

#### OS X
  - Tested on 10.10 - 10.13
  - Apple clang 7.2 or newer
  - Qt 4.8 or 5.12
    + Download [Qt 4.8 DMG](https://download.qt.io/archive/qt/4.8/4.8.4/qt-mac-opensource-4.8.4.dmg).
    + The above dmg will not work if using OS X 10.11. In this case Qt 4.8 can be acquired through macports using the command 'sudo port install qt4-mac' - more on macports here: https://www.macports.org/install.php
    + Qt 4.8 can also be installed with homebrew.  The instructions are found [here](https://github.com/cartr/homebrew-qt4).
    + [Download](https://download.qt.io/archive/qt/) and run the Qt 5.12 installer.  Make sure to turn off other versions and system configurations to save space and build time.  Configure CMake for Qt 5.

#### Windows
  - Tested on Windows 10
  - Visual Studio 2013 & 2017
  - Qt 5.12 or later
    + [Download](https://download.qt.io/archive/qt/) and run the Qt 5.12 installer.  Make sure to turn off other versions and system configurations to save space and build time.


#### Linux
  - Tested on Ubuntu 14.04 LTS, OpenSUSE Leap 42.1
  - gcc 4.8
  - Qt 5.12 or later
    + [Download](https://download.qt.io/archive/qt/) and run the Qt 5.12 installer.  Make sure to turn off other versions and system configurations to save space and build time.  Add the Qt 5 path to CMake configuration.

#### All Platforms
  - [CMake](https://cmake.org/) (platform independent configuring system that is used for generating Makefiles, Visual Studio project files, or Xcode project files)
    + Tested with 2.8 - 3.4
    + Root cmake file is Superbuild/CMakeLists.txt.
    + Building in source directories is not permitted.
    + Make sure BUILD_SHARED_LIBS is on (default setting).

### CMake Build Generators
* Windows
  - Visual Studio 2013 & 2017
* OS X
  - Unix Makefiles
  - Xcode
* Linux
  - Unix Makefiles

## Compiling SCIRun

Run CMake from your build (bin or other build directory of your choice) directory and give a path to the CMake Superbuild directory containing the master CMakeLists.txt file.
For example, on the command line if building from the default SCIRun bin directory:

```
cd bin
cmake ../Superbuild
```

The console version `ccmake`, or GUI version can also be used.
You may be prompted to specify your location of the Qt installation.
If you installed Qt in the default location, it should find Qt automatically.
After configuration is done, generate the make files or project files for your favorite
development environment and build.

Following the previous example, the SCIRun application will be built in bin/SCIRun.

A bash build script (`build.sh`) is also available for Linux and Mac OS X to simplify the process.
Usage information is available using the ***--help*** flag:

```
./build.sh --help
```

### Building SCIRun with Qt 5

Building SCIRun with Qt 5 requires additional input. Use the `Qt5_PATH` CMake variable to point to the Qt 5 build location.  This can be done through the command line with a command similar to:
```
cmake -DQt5_PATH=path_to_Qt5_build/ ../Superbuild/
```
Or they can be set in the CMake GUI or with the `ccmake` function.

In OS X builds, the `QT5_BUILD` CMake flag will also need to be on.  The command will be similar to the following:
```
cmake -DQT5_BUILD=True -DQt5_PATH=path_to_Qt5/5.12.1/clang_64/ ../Superbuild/
```

### Building with Tetgen

To use the InterfaceWithTetgen module, SCIRun needs to be built with Tetgen, which is off by default.  Turn on the `WITH_TETGEN` flag to enable tegen.  This will download and install Tetgen with SCIRun. In the command line, it would look like:
```
cmake -DWITH_TETGEN=True ../Superbuild/
```

## Tagging Releases
On an OSX system, run script `release.sh` in the `src` directory with the release name in format ***beta.XX*** as a parameter.
