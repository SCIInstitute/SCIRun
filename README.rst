.. image::  http://www.sci.utah.edu/images/banners/splash-scirun.png
   :height: 245 px
   :width:  495 px
   :align: right

==================
SCIRun 5 Prototype
==================

| https://github.com/SCIInstitute/SCIRunGUIPrototype

.. contents::

Summary
=======

+---------------+----------------------------------------------------------------------+
|  **Warning**  |  SCIRun 5 is pre-alpha software, do not use for real science yet.    |
+---------------+----------------------------------------------------------------------+

Download Location
-----------------
| http://sci.utah.edu/devbuilds/scirun5/

Goals
=====

SCIRun 5 is a complete rewrite of the GUI front end and graphical components of SCIRun 4, including a more stable and 
efficient middle layer, with support for Python scripting.

Features
========

Network Editor
--------------
* Module selector: filterable list of modules that you can drag-and-drop into the editor or double-click to add a module.
* No middle mouse clicks are needed to connect modules: use left click instead.
* To execute a network, click the "Execute All" button or use the Ctrl-E hotkey.
* Execution of individual modules (rather than the entire network) is disabled for now.
* Saving and loading of network files is enabled. The file format is xml, but it is not compatible with SCIRun 4, so a *.srn5 extension is used.
* SCIRun v4 networks are not compatible yet, either. This feature will be implemented in a later milestone release.
* There are many menu options and GUI elements that do nothing yet. They are either grayed out or have an asterisk next to their name.
* You can filter the module selector's list, either with a simple starting string or a wildcard pattern.
* In addition to the basic "Euclidean" connection pipes, a "Cubic Bezier"-style connection is available as an option in the Network Editor tab of the Configuration pane.
* Execution progress is shown in a progress bar as in SCIRun v4.
* Each module widget has an actions button, that contains common actions that used to appear in the right-click menu.
* Double-click a connection to bring up its action menu, including delete.
* Deleting objects from the network editor uses Delete on Windows and Fn-Del on MacOS.
* New working module: SolveLinearSystem--uses the parallel linear algebra library of SCIRun v4 for a fast CG solver method.
* Help mode: click the "What's This" button (or F1 or Ctrl-H) and then click anywhere in the interface for helpful information.
* Networks can be loaded from the command line, and automatically executed with the -e and -E options as in SCIRun v4 (useful for scripting).
* LatVols can be rendered with a brand new rendering engine.
* **New in milestone D**
* Multithreaded network execution. Access this by opening the developer console (hotkey '`'), and choosing "Parallel".
* Python integration, phase 1. Currently only works with Python 3.3. Turn on "BUILD_WITH_PYTHON" in CMake. You get a Python interpreter in the GUI with a few working commands for network building.
* TriSurfMeshes can now be rendered, using a special two-file text import format.
* Network editor provenance: another new window, accessible under the Window menu, allows undo and redo of basic network editor actions.
* The network editor background color can be customized, and is saved as a preference.
* Modules in the network editor now have a notes editor. The notes have a few formatting and position options as in SCIRun v4. They are not saved with the network yet; to be delivered in milestone E.
* Duplicate module is available as a general module action.

Renderer
--------------
* Can render faces and edges of LatVolMeshes and TriSurfMeshes.
* Camera control is still in ImageVis3D mode.

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
* OS X (tested on 10.7 and 10.8)

  - Apple clang 4.1
  - Qt 4.8
  
    + Download from http://releases.qt-project.org/qt4/source/qt-mac-opensource-4.8.4.dmg.

* Windows (tested on Windows 7)

  - Visual Studio C++ 2010
  - Qt 4.8 
  
    + Build from source (see http://scirundocwiki.sci.utah.edu/SCIRunDocs/index.php/CIBC:Seg3D2:Building_Releases#Installing_Qt_on_your_system_and_building_from_scratch for instructions), but be sure to download http://releases.qt-project.org/qt4/source/qt-everywhere-opensource-src-4.8.4.tar.gz.

* Linux (tested on Ubuntu 12.10)

  - gcc 4.6, 4.7
  - Qt 4.8 
  
    + Build from source (http://releases.qt-project.org/qt4/source/qt-everywhere-opensource-src-4.8.4.tar.gz), or use system libraries if available.

* All platforms

  - CMake 2.8

    + Root cmake file is SCIRunGUIPrototype/src/CMakeLists.txt.
    + Building in the source directory is not permitted.
    + Make sure BUILD_SHARED_LIBS is on (default setting).
    + BUILD_WITH_PYTHON works on Windows, not yet (easily) on MacOS.
  

CMake Build Generators
----------------------

* Windows

  - Visual Studio 2010 C++

* OS X (tested on 10.7 and 10.8)

  - Unix Makefiles (Xcode not currently supported)

* Linux (tested on Ubuntu 12.10)

  - Unix Makefiles

Unix Makefiles notes
--------------------

* Boost must be built before the SCIRun libraries.
* Parallel make builds can be used as long the Boost target is built first, for example:

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


