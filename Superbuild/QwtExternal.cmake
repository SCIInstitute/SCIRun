#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
#  University of Utah.
#
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.


# Assume you already define these at the top-level (as in our previous message):
# set(SCIRUN_QT_MAJOR "6" CACHE STRING "Qt major version to use (5 or 6)")
# set_property(CACHE SCIRUN_QT_MAJOR PROPERTY STRINGS 5 6)
# set(Qt_PATH "" CACHE PATH "Qt install prefix, e.g. C:/Qt/6.3.1/msvc2019_64")

set_property(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

# Common cache args for the ExternalProject
set(QWT_CACHE_ARGS
  "-DCMAKE_POLICY_VERSION_MINIMUM:STRING=3.5"
  "-DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}"
  "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
  "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON"
)

# Sanity check Qt_PATH once, fail early if not set correctly
if (NOT IS_DIRECTORY "${Qt_PATH}")
  message(FATAL_ERROR "Qt_PATH is invalid or not set: ${Qt_PATH}")
endif()

# Help the sub-build find Qt by giving it the correct prefix path.
# On Windows Qt installs, ${Qt_PATH} contains bin/, lib/, plugins/, and cmake/Qt6 (or Qt5).
list(APPEND QWT_CACHE_ARGS "-DCMAKE_PREFIX_PATH:PATH=${Qt_PATH}")

# Pass version-specific hints. Keep them minimal and valid for the chosen major version.
if (SCIRUN_QT_MAJOR STREQUAL "6")
  # Primary hint for Qt6
  list(APPEND QWT_CACHE_ARGS "-DQt6_DIR:PATH=${Qt_PATH}/lib/cmake/Qt6")

  # OPTIONAL: if Qwt’s CMake looks for module-specific config packages, you can add these:
  # (Only if actually needed by their CMake; otherwise leave them out to avoid over-constraining.)
  # list(APPEND QWT_CACHE_ARGS "-DQt6Core_DIR:PATH=${Qt_PATH}/lib/cmake/Qt6")
  # list(APPEND QWT_CACHE_ARGS "-DQt6Gui_DIR:PATH=${Qt_PATH}/lib/cmake/Qt6")
  # list(APPEND QWT_CACHE_ARGS "-DQt6Widgets_DIR:PATH=${Qt_PATH}/lib/cmake/Qt6")

  # Choose a Qt6 branch/tag for Qwt
  set(qwt_GIT_TAG "origin/qt6-static-6.2.0")
elseif (SCIRUN_QT_MAJOR STREQUAL "5")
  # Primary hint for Qt5
  list(APPEND QWT_CACHE_ARGS "-DQt5_DIR:PATH=${Qt_PATH}/lib/cmake/Qt5")

  # OPTIONAL: same note as above—only pass if Qwt expects these variables.
  # list(APPEND QWT_CACHE_ARGS "-DQt5Core_DIR:PATH=${Qt_PATH}/lib/cmake/Qt5Core")
  # list(APPEND QWT_CACHE_ARGS "-DQt5Gui_DIR:PATH=${Qt_PATH}/lib/cmake/Qt5Gui")
  # list(APPEND QWT_CACHE_ARGS "-DQt5Widgets_DIR:PATH=${Qt_PATH}/lib/cmake/Qt5Widgets")

  set(qwt_GIT_TAG "origin/qt5-static-6.1.5")
else()
  message(FATAL_ERROR "SCIRUN_QT_MAJOR must be '5' or '6'. Current value: ${SCIRUN_QT_MAJOR}")
endif()

# If Qwt’s subbuild needs to know which major to target (some CMakeLists choose behavior by this),
# pass a simple cache variable it can use.
list(APPEND QWT_CACHE_ARGS "-DSCIRUN_QT_MAJOR:STRING=${SCIRUN_QT_MAJOR}")

# Remove legacy/fragile arguments that caused trouble:
#  -DQt_PATH
#  -DQt5_PATH
#  -DQt${QT_VERSION_MAJOR}Core_DIR
#  -DQt${QT_VERSION_MAJOR}CoreTools_DIR
#  -DQt${QT_VERSION_MAJOR}Gui_DIR
#  -DQt${QT_VERSION_MAJOR}GuiTools_DIR
#
# These often become empty or point to non-existent packages (e.g., *Tools on Qt6),
# which derails the subbuild's find_package() calls.

ExternalProject_Add(Qwt_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/Qwt.git"
  GIT_TAG ${qwt_GIT_TAG}
  PATCH_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS ${QWT_CACHE_ARGS}
)

ExternalProject_Get_Property(Qwt_external BINARY_DIR)
set(QWT_DIR ${BINARY_DIR} CACHE PATH "")
message(STATUS "QWT_DIR: ${QWT_DIR}")
