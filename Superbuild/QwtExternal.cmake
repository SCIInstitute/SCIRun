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


# QwtExternal.cmake — build Qwt 6.3.0 with qmake from the superbuild

set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

# Your mirror/tag
set(qwt_GIT_TAG "v6.3.0")

# Superbuild directories
set(_qwt_src  "${CMAKE_BINARY_DIR}/Externals/Source/Qwt_external")
set(_qwt_bin  "${CMAKE_BINARY_DIR}/Externals/Build/Qwt_external")
set(_qwt_inst "${CMAKE_BINARY_DIR}/Externals/Install/Qwt_external")

# -------- Select build tool per platform --------
if(WIN32)
  # Prefer jom if available; otherwise nmake
  find_program(JOM_EXECUTABLE NAMES jom PATHS ENV PATH NO_DEFAULT_PATH)
  if(NOT JOM_EXECUTABLE)
    find_program(JOM_EXECUTABLE NAMES jom)
  endif()
  if(JOM_EXECUTABLE)
    set(_MAKE_TOOL "${JOM_EXECUTABLE}")
  else()
    set(_MAKE_TOOL "nmake")
  endif()
else()
  # make or gmake; ExternalProject will run it in the source tree where qmake generates Makefiles
  find_program(MAKE_EXECUTABLE NAMES make gmake)
  if(NOT MAKE_EXECUTABLE)
    message(FATAL_ERROR "No 'make' tool found for building Qwt.")
  endif()
  set(_MAKE_TOOL "${MAKE_EXECUTABLE}")
endif()

# -------- Require the Qt qmake path --------
if(NOT DEFINED QT_QMAKE_EXECUTABLE OR NOT EXISTS "${QT_QMAKE_EXECUTABLE}")
  message(FATAL_ERROR "QT_QMAKE_EXECUTABLE is not set to a valid Qt qmake path (Qt 5/6).")
endif()

# Qwt’s qmake accepts PREFIX/INSTALLBASE to control where it installs.
# We pass both to cover different setups.
set(_QMAKE_CONFIG
  "${QT_QMAKE_EXECUTABLE}"
  "qwt.pro"
  "PREFIX=${_qwt_inst}"
  "INSTALLBASE=${_qwt_inst}"
)

# Note:
#  - We *don’t* set CMAKE_*_OUTPUT_DIRECTORY here—qmake controls build dirs.
#  - We *do* run an actual install so downstream consumers get include/lib/plugin layout.

ExternalProject_Add(Qwt_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/Qwt.git"
  GIT_TAG        ${qwt_GIT_TAG}            # v6.3.0 in your setup
  UPDATE_DISCONNECTED 1

  SOURCE_DIR ${_qwt_src}
  BINARY_DIR ${_qwt_bin}                   # not used by qmake; kept for symmetry/logs

  # --- Configure: run qmake in the source dir so it finds qwt.pro ---
  CONFIGURE_COMMAND
    ${CMAKE_COMMAND} -E chdir ${_qwt_src}
    "${QT_QMAKE_EXECUTABLE}" qwt.pro
      "PREFIX=${_qwt_inst}"
      "INSTALLBASE=${_qwt_inst}"

  # --- Build: run the chosen make tool in the same source dir ---
  BUILD_COMMAND
    ${CMAKE_COMMAND} -E chdir ${_qwt_src} ${_MAKE_TOOL}

  # --- Install: same directory (qmake’s Makefile target `install`) ---
  INSTALL_COMMAND
    ${CMAKE_COMMAND} -E chdir ${_qwt_src} ${_MAKE_TOOL} install

  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# ------- Export variables for SCIRun / downstream -------
set(QWT_SOURCE_DIR  ${_qwt_src})
set(QWT_INSTALL_DIR ${_qwt_inst})

# Qwt installs headers under include/ (often include/qwt-6.3.0) and libs under lib/
set(QWT_INCLUDE     "${QWT_INSTALL_DIR}/include")
set(QWT_LIBRARY_DIR "${QWT_INSTALL_DIR}/lib")

# Library base name (actual file is qwt.lib / libqwt.a / libqwt.{so,dylib})
set(QWT_LIBRARY     "qwt")

message(STATUS "[Qwt_external] INSTALL_DIR=${QWT_INSTALL_DIR}")