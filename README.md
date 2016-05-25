![SCIRun Banner](http://www.sci.utah.edu/images/software/SCIRun/scirun.png "")

## SCIRun 5
https://github.com/SCIInstitute/SCIRun

[![Build Status](https://travis-ci.org/SCIInstitute/SCIRun.svg)](https://travis-ci.org/SCIInstitute/SCIRun)
[![Coverage Status](https://coveralls.io/repos/SCIInstitute/SCIRun/badge.png)](https://coveralls.io/r/SCIInstitute/SCIRun)

##### [Contents](#user-content-scirun-5-prototype "generated with DocToc(http://doctoc.herokuapp.com/)")

- [SCIRun 5](#user-content-scirun-5)
	- [Summary](#user-content-summary)
	- [Goals](#user-content-goals)
	- [Documentation](#user-content-documentation)
	- [Platform Notes](#user-content-platform-notes)
		- [Build requirements](#user-content-build-requirements)
		- [CMake Build Generators](#user-content-cmake-build-generators)
		- [Unix Makefiles notes](#user-content-unix-makefiles-notes)
	- [Questions and Answers](#user-content-questions-and-answers)
	- [License and Credits](#user-content-license-and-credits)

### Summary

| Warning! |  SCIRun 5 is alpha software, you may use for real science but beware of instability.  |
|:--------:|:-------------------------------------------------------------------------------------:|

###Goals
SCIRun 5 is a complete rewrite of the GUI front end and graphical components of SCIRun 4, including a more stable and
efficient middle layer, with support for Python scripting.

### Documentation
For documentation, please see: http://sciinstitute.github.io/SCIRun/

### Platform Notes
#### Build requirements
* OS X (tested on 10.7-10.11)
  - Apple clang 5.1
  - Qt 4.8
    + Download from http://releases.qt-project.org/qt4/source/qt-mac-opensource-4.8.4.dmg.
    + The above dmg will not work if using OS X 10.11. In this case Qt 4.8 can be acquired through macports using the command 'sudo port install qt4-mac' - more on macports here: https://www.macports.org/install.php
* Windows (tested on Windows 7, 8, 10)
  - Visual Studio 2013, 2015
  - Qt 4.8
    + Build from source (see http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php/CIBC:Seg3D2:Building_Releases#Installing_Qt_on_your_system_and_building_from_scratch for instructions), but be sure to download http://releases.qt-project.org/qt4/source/qt-everywhere-opensource-src-4.8.4.tar.gz.
* Linux (tested on Ubuntu 12.10, OpenSUSE Leap)
  - gcc 4.6, 4.7, 4.8
  - Qt 4.8
    + Build from source (http://releases.qt-project.org/qt4/source/qt-everywhere-opensource-src-4.8.4.tar.gz), or use system libraries if available.
* All platforms
  - CMake 2.8
    + Root cmake file is SCIRun/Superbuild/CMakeLists.txt.
    + Building in the source directory is not permitted.
    + Make sure BUILD_SHARED_LIBS is on (default setting).

#### CMake Build Generators
* Windows
  - Visual Studio 2013
* OS X (tested on 10.7 and 10.8 and 10.10)
  - Unix Makefiles
  - Xcode
* Linux (tested on Ubuntu 12.10)
  - Unix Makefiles

#### Unix Makefiles notes
* Build Superbuild first (src/Superbuild), then subsequently, SCIRun can be built by itself (src/).
* Included `build.sh` takes care of all this for you.

### Tagging releases
On an OSX system, run script `release.sh` in the `src` directory with the release name in format `alpha.XX` as a parameter.

### Questions and Answers
For help, email the testing mailing list at scirun5-testers@sci.utah.edu.

### License and Credits
  For more information, please see: http://software.sci.utah.edu

  The MIT License

  Copyright (c) 2016 Scientific Computing and Imaging Institute,
  University of Utah.


  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
