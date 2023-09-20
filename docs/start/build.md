# Build

## Platform Notes

**At least C++11 64-bit compiler support is required.**

### Build requirements

#### OS X
  - MacOS 11 or newer
  - Apple clang 9.0.0 or newer
  - Qt 5.15 or later
    + [Download](https://download.qt.io/archive/qt/) and run the desired Qt 5.x or 6.x installer.  Make sure to turn off other versions and system configurations to save space and build time.  Configure CMake for Qt 5.
      Optionally, install Qt through the package manager, [brew](https://brew.sh/).

      To install Qt 5, the command is:
      ```bash
      brew install qt@5
      ```bash
      To install Qt 6, the command is:
      ```
      brew install qt@6
      ```
      The installation directory for the CMake variable `Qt_PATH` will then be `/usr/local/Cellar/Qt/_TYPE_QT_VERSION_HERE_`.

#### Windows
  - Tested on Windows 10
  - Visual Studio 2017 or 2019
    + Using 2017, be sure to change the CMake platform to x64.
  - Qt 5.15 or later
    + [Download](https://download.qt.io/archive/qt/) and run the Qt 5.15 installer.  Make sure to turn off other versions and system configurations to save space and build time.


#### Linux
  - Tested on Ubuntu 16.04 LTS, 18.04 LTS, 22.04 LTS, OpenSUSE Leap 42.1, Arch Linux
  - gcc 7+
  - Qt 5.15 or later
    + [Download](https://download.qt.io/archive/qt/) and run the desired Qt 5.x or 6.x installer.  Make sure to turn off other versions and system configurations to save space and build time. 
      Optionally, install Qt through your distro's package manager instead(apt on Ubuntu/Debian, pacman on Arch). 

      To install Qt 5, the command on Ubuntu/Debian is:
      ```
      sudo apt-get install qt5-default
      ```
      To install Qt 6, the command on Ubuntu/Debian is:
      ```
      sudo apt-get install qt6-base-dev libqt6svg6-dev
      ```
      The installation directory for the CMake variable `Qt5_DIR`(for Qt5) or `Qt_PATH`(for Qt6) will then be `/usr/lib/x86_64-linux-gnu/cmake/`.

      To install Qt 5, the command on Arch is:
      ```
      sudo pacman -S qt5-base
      ```
      To install Qt 6, the command on Arch is:
      ```
      sudo pacman -S qt6-base
      ```
      The installation directory for the CMake variable `Qt5_DIR`(for Qt5) or `Qt_PATH`(for Qt6) will then be `/usr/lib/cmake/`.

#### All Platforms
  - [CMake](https://cmake.org/) (platform independent configuring system that is used for generating Makefiles, Visual Studio project files, or Xcode project files)
    + Tested with 3.4 and newer
    + Root cmake file is Superbuild/CMakeLists.txt.
    + Building in source directories is not permitted.
    + Make sure BUILD_SHARED_LIBS is on (default setting).

### CMake Build Generators
* Windows
  - Visual Studio 2017 & 2019
* OS X
  - Unix Makefiles
  - Xcode
* Linux
  - Unix Makefiles
  
## Configuring CMake
Run CMake from your build (bin or other build directory of your choice) directory and give a path to the CMake Superbuild directory containing the master CMakeLists.txt file.

A bash build script (`build.sh`) is also available for Linux and Mac OS X to simplify the process.
Usage information is available using the ***--help*** flag:

```
./build.sh --help
```

For example, on the command line if building from the default SCIRun bin directory:

```
cd bin
cmake ../Superbuild
```

The console version `ccmake`, or GUI version can also be used.
You may be prompted to specify your location of the Qt installation.
If you installed Qt in the default location, it should find Qt automatically.

### Configuring SCIRun with Qt 5

Building SCIRun with Qt 5 requires additional input. Use the `Qt5_PATH` CMake variable to point to the Qt 5 build location. Look at the Qt install steps above for information about the directory. This can be done through the command line with a command similar to:
```
cmake -DQt5_PATH=path_to_Qt5_build/ ../Superbuild/
```
Or they can be set in the CMake GUI or with the `ccmake` function.

The command will be similar to the following:
```
cmake -DQt5_PATH=path_to_Qt5/5.15.1/clang_64/ ../Superbuild/
```

### Configuring SCIRun with Qt 6

Building SCIRun with Qt 6 requires additional input. Set the Cmake variable `SCIRUN_QT_MIN_VERSION` to 6.X.X, where the version is less than the installed Qt 6 version. Use the `Qt_PATH` CMake variable to point to the Qt 6 build location. Look at the Qt install steps above for information about the directory. This can be done through the command line with a command similar to:
```
cmake -DQt_PATH=path_to_Qt6_build/ ../Superbuild/
```
Or they can be set in the CMake GUI or with the `ccmake` function.

The command will be similar to the following:
```
cmake -DQt_PATH=path_to_Qt6/6.4.2/clang_64/ -DSCIRUN_QT_MIN_VERSION="6.3.1" ../Superbuild/
```

```

### Configuring SCIRun with OSPRay
To use the OsprayViewer module, SCIRun needs to download and install Ospray during the build process, which is off by default. This is enabled with the `WITH_OSPRAY` flag. In the command line, it would look like:
```
cmake -DWITH_OSPRAY=True ../Superbuild/
```

## Building SCIRun
After configuration is done, generate the make files or project files for your favorite development environment and build.

From the same command line, you can build with:
```
make
```

Append `-jN`, where N is the number of threads, to build multi-threaded.
Note: A common problem during the first build is the Python build fails with multi-threading. If this happens, rebuild single-threaded.

Following the previous example, the SCIRun application will be built in `bin/SCIRun`.

## Partial Rebuild
After SCIRun has finished building externals from the `bin/` folder, you can recompile just the internal SCIRun code by building in the `bin/SCIRun` folder. In the command line, it would look like:
```
cd bin/SCIRun
make
```
Append `-jN`, where N is the number of threads, to build multi-threaded.

## Tagging Releases
On an OSX system, run script `release.sh` in the `src` directory with the release name in format ***beta.XX*** as a parameter.
