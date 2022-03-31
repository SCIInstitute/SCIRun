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

SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})

SET(QWT_CACHE_ARGS
  "-DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}"
  "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
  "-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON"
  )

LIST(APPEND QWT_CACHE_ARGS
  "-DQt_PATH:PATH=${Qt_PATH}"
  "-DQt5_PATH:PATH=${Qt_PATH}"
  "-DQt${QT_VERSION_MAJOR}Core_DIR:PATH=${Qt${QT_VERSION_MAJOR}Core_DIR}"
  "-DQt${QT_VERSION_MAJOR}CoreTools_DIR:PATH=${Qt${QT_VERSION_MAJOR}CoreTools_DIR}"
  "-DQt${QT_VERSION_MAJOR}Gui_DIR:PATH=${Qt${QT_VERSION_MAJOR}Gui_DIR}"
  "-DQt${QT_VERSION_MAJOR}GuiTools_DIR:PATH=${Qt${QT_VERSION_MAJOR}GuiTools_DIR}"
)

if (${QT_VERSION_MAJOR} STREQUAL "5")
  SET(qwt_GIT_TAG "origin/qt5-static-6.1.5")
else()
  SET(qwt_GIT_TAG "origin/qt6-static-6.2.0")
endif()

# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(Qwt_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/Qwt.git"
  GIT_TAG ${qwt_GIT_TAG}
  PATCH_COMMAND ""
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS ${QWT_CACHE_ARGS}
)

ExternalProject_Get_Property(Qwt_external BINARY_DIR)
SET(QWT_DIR ${BINARY_DIR} CACHE PATH "")

MESSAGE(STATUS "QWT_DIR: ${QWT_DIR}")
