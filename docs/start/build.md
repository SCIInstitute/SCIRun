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
      ```
      brew install qt@5
      ```
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
  - Tested on Ubuntu 16.04 LTS, 18.04 LTS, 22.04 LTS, 24.04 LTS, OpenSUSE Leap 42.1, Arch Linux
  - gcc 7+
  - Qt 5.15 or later
    + [Download](https://download.qt.io/archive/qt/) and run the desired Qt 5.x or 6.x installer.  Make sure to turn off other versions and system configurations to save space and build time. 
      Optionally, install Qt through your distro's package manager instead(apt on Ubuntu/Debian, pacman on Arch). 

      To install Qt 5, the command on Ubuntu (20.04 LTS or earlier) or Debian is:
      ```
      sudo apt-get install qt5-default
      ```
      or, on Ubuntu 22.04 LTS and later
      ```
      sudo apt-get install qtbase5-dev libqt5svg5-dev
      ```
      To install Qt 6, the command on Ubuntu/Debian is:
      ```
      sudo apt-get install qt6-base-dev libqt6svg6-dev
      ```
      The installation directory for the CMake variable `Qt_PATH` will then be `/usr/lib/x86_64-linux-gnu/cmake/`.

      To install Qt 5, the command on Arch is:
      ```
      sudo pacman -S qt5-base
      ```
      To install Qt 6, the command on Arch is:
      ```
      sudo pacman -S qt6-base
      ```
      The installation directory for the CMake variable `Qt_PATH` will then be `/usr/lib/cmake/`.

#### All Platforms
  - [CMake](https://cmake.org/) (platform independent configuring system that is used for generating Makefiles, Visual Studio project files, or Xcode project files)
    + 3.21 or newer
    + Root cmake file is Superbuild/CMakeLists.txt.
    + Building in source directories is not permitted.
    + Make sure BUILD_SHARED_LIBS is on (default setting).

### CMake Build Generators
* Windows
  - Visual Studio 2019 & 2022
* OS X
  - Unix Makefiles
  - Ninja
  - Xcode
* Linux
  - Unix Makefiles
  - Ninja

## Configuring with presets (recommended)

`Superbuild/CMakePresets.json` ships the configurations we build, so the only thing you have to write down is where Qt is. Put that in `Superbuild/CMakeUserPresets.json` -- it is ignored by git and never committed -- starting from the example:

```
cp Superbuild/CMakeUserPresets.json.example Superbuild/CMakeUserPresets.json
```

Edit it to point `Qt_PATH` at your Qt install (see the Qt install steps above for the directory) and pick which shipped presets to inherit. `cmake --list-presets` in `Superbuild/` shows what is available:

* `release`, `debug`, `headless` -- CMake's default generator for your platform
* `make-*`, `ninja-*`, `vs2022-*`, `vs2019-*` -- the same, with an explicit generator
* `qt6` -- add to `inherits` to build against Qt 6 (the shipped presets default to Qt 5)
* `testing` -- add to `inherits` to build the unit and regression tests

Earlier entries in `inherits` win, so list `qt6` / `testing` before the generator preset, or set the variables directly in your preset's `cacheVariables`, which always take precedence.

Then, from `Superbuild/`:

```
cmake --preset mine
cmake --build --preset mine
ctest --preset mine
```

The build directory defaults to `bin/<preset name>` under the repository root. On Windows override `binaryDir` in your user preset with something short such as `C:/SR`: the Superbuild nests deeply and the default location can exceed the 260-character path limit.

## Configuring CMake by hand
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

### Build options

Pass these as `-DOPTION=ON|OFF` to the Superbuild (`cmake -DBUILD_TESTING=ON ../Superbuild`).

| Option | Default | What it does |
|---|---|---|
| `BUILD_TESTING` | OFF | Build unit and regression tests |
| `BUILD_DOCUMENTATION` | OFF | Build the documentation |
| `BUILD_WITH_PYTHON` | ON | Python API and the Python modules |
| `BUILD_HEADLESS` | OFF | Build without Qt / the GUI |
| `WITH_TETGEN` | ON | TetGen mesh generation (GPL; see the InterfaceWithTetGen module) |
| `BUILD_OSPRAY` | OFF | Download and build OSPRay for the OsprayViewer module |
| `PREBUILT_OSPRAY` | OFF | Use an already-installed OSPRay instead of building one |
| `WITH_VTK` | OFF | VTK renderer backend |
| `Qt_PATH` | | Location of the Qt installation (see below) |
| `SCIRUN_QT_MIN_VERSION` | 5.15.2 | Set to `6.3.1` to build against Qt 6 |

Option names follow one rule: the prefix says what kind of knob it is.

- `WITH_<DEP>` — an optional third-party dependency. The Superbuild fetches and builds it; the inner build compiles the code that uses it.
- `BUILD_<THING>` — an extra artifact SCIRun emits, consistent with CMake's own `BUILD_TESTING` and `BUILD_SHARED_LIBS`.
- `ENABLE_`, `RUN_`, `GENERATE_`, `DOWNLOAD_` — behavior knobs.

`BUILD_WITH_` is retired and no new options use it. `BUILD_WITH_PYTHON`, `BUILD_HEADLESS` and `BUILD_OSPRAY` predate the rule and are being renamed to `WITH_PYTHON`, `WITH_GUI` and `WITH_OSPRAY`; the old spellings will keep working for one release with a deprecation warning.

### Configuring SCIRun with Qt 5

Building SCIRun with Qt 5 requires additional input. Use the `Qt_PATH` CMake variable to point to the Qt 5 build location. Look at the Qt install steps above for information about the directory. This can be done through the command line with a command similar to:
```
cmake -DQt_PATH=path_to_Qt5_build/ ../Superbuild/
```
Or they can be set in the CMake GUI or with the `ccmake` function.

The command will be similar to the following:
```
cmake -DQt_PATH=path_to_Qt5/5.15.1/clang_64/ ../Superbuild/
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


### Configuring SCIRun with OSPRay
To use the OsprayViewer module, SCIRun needs to download and install Ospray during the build process, which is off by default. This is enabled with the `BUILD_OSPRAY` flag. In the command line, it would look like:
```
cmake -DBUILD_OSPRAY=True ../Superbuild/
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

### Configuring SCIRun with Prebuilt OSPRay

To use the OsprayViewer module with a prebuilt binary, you need to download from the [releases page](https://github.com/ospray/OSPRay/releases) and extract the zip. Only OSPRay 2.4 and 2.10 have been tested. Then add the directory to your shell's PATH variable. CMake will automatically find it when the flag `PREBUILT_OSPRAY` is enabled.

## Tagging Releases
On an OSX system, run script `release.sh` in the `src` directory with the release name in format ***beta.XX*** as a parameter.
