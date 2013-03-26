.. image::  http://www.sci.utah.edu/images/banners/splash-scirun.png
   :height: 245 px
   :width:  495 px
   :align: right

==================
SCIRun 5 Prototype
==================

| https://github.com/SCIInstitute/SCIRunGUIPrototype
| <b>`Downloads: Click Here! <http://sci.utah.edu/devbuilds/scirun5/>`_ </b>

.. contents::

Summary
=======

TODO

+---------------+----------------------------------------------------------------------+
|  **Warning**  |  SCIRun 5 is pre-alpha software, do not use for real science yet.    |
+---------------+----------------------------------------------------------------------+

Goals
=====

SCIRun 5 is a complete rewrite of the GUI front end and graphical components of SCIRun 4, including a more stable and 
efficient middle layer, with support for Python scripting.

Features
========

Network Editor
--------------
* Module selector: filterable list of modules that you can drag-and-drop into the editor or double-click to add a module.
* No middle mouse clicks are needed to connect modules.
* Choice of connection drawing method.

Renderer
--------------
* Can render faces and edges of LatVolMeshes and TriSurfMeshes.

In Progress
===========

* Python console, to allow scripted network editing

Planned Features for Next Release
================

* Saving module notes.
* Connection notes.
* Full Python support on all platforms, including ability to run headless without building Qt.
* Better geometry rendering, coloring and shading.

Limitations
===========

* Cannot load SCIRun 4 network files
* Limited set of modules at the moment

Other Information
=================

Platform Notes
==============

Build requirements
------------------
  - OS X (tested on 10.7 and 10.8):
    - Apple clang 4.1
    - Qt 4.8. Download from http://releases.qt-project.org/qt4/source/qt-mac-opensource-4.8.4.dmg.

  - Windows (tested on Windows 7)
	- Visual Studio C++ 2010
    - Qt 4.8. Build from source (see http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php/CIBC:Seg3D2:Building_Releases#Installing_Qt_on_your_system_and_building_from_scratch for instructions),
      but be sure to download http://releases.qt-project.org/qt4/source/qt-everywhere-opensource-src-4.8.4.tar.gz.

  - Linux (tested on Ubuntu 12.10)
	- gcc 4.6, 4.7
    - Qt 4.8. Build from source (http://releases.qt-project.org/qt4/source/qt-everywhere-opensource-src-4.8.4.tar.gz), or use system libraries if available.

  -	All platforms
    - CMake 2.8. 
      - Root cmake file is SCIRunGUIPrototype/src/CMakeLists.txt.
      - Building in the source directory is not permitted.
      - Make sure BUILD_SHARED_LIBS is on (default setting).
	  - BUILD_WITH_PYTHON works on Windows, not yet (easily) on MacOS.
	  

CMake Build Generators
----------------------
  - Windows
    - Visual Studio 2010 C++

  - OS X (tested on 10.7 and 10.8):
    - Unix Makefiles (Xcode not currently supported)

  - Linux (tested on Ubuntu 12.10):
    - Unix Makefiles

Unix Makefiles notes
--------------------
  - Boost must be built before the SCIRun libraries.
  - Parallel make builds can be used as long the Boost target is built first, for example:
    - make -j4 Boost_external && make -j4


Questions and Answers
=====================

For help, email the testing mailing list at scirun5-testers@sci.utah.edu.

License and Credits
===================

  For more information, please see: http://software.sci.utah.edu
 
  The MIT License
 
  Copyright (c) 2013 Scientific Computing and Imaging Institute,
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


