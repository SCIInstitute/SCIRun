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

# SQLiteExternal.cmake (redirect libs + copy headers into Install/include)
set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

set(sqlite_GIT_TAG "v3.0.1")

# Common CMake args
set(_cmake_args
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON

  # Redirect all outputs so install step is unnecessary for libs
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=<INSTALL_DIR>/lib
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=<INSTALL_DIR>/bin

  # Multi-config (VS)
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/lib
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/lib
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG=<INSTALL_DIR>/bin
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE=<INSTALL_DIR>/bin
)

# Single-config generators
if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  list(APPEND _cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

# Superbuild directories
set(_sqlite_src  "${CMAKE_BINARY_DIR}/Externals/Source/SQLite_external")
set(_sqlite_bin  "${CMAKE_BINARY_DIR}/Externals/Build/SQLite_external")
set(_sqlite_inst "${CMAKE_BINARY_DIR}/Externals/Install/SQLite_external")

ExternalProject_Add(SQLite_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/sqlite.git"
  GIT_TAG        ${sqlite_GIT_TAG}
  UPDATE_DISCONNECTED 1

  SOURCE_DIR ${_sqlite_src}
  BINARY_DIR ${_sqlite_bin}

  CMAKE_GENERATOR          "${CMAKE_GENERATOR}"
  CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
  CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}"

  CMAKE_ARGS ${_cmake_args}

  # Skip 'install' to keep the fast redirect flow for libs
  INSTALL_COMMAND ""

  # After build, ensure headers are available in <INSTALL_DIR>/include
  # (sqlite3.h is typically at the source root; sqlite3ext.h may also be present)
  STEP_TARGETS copy_headers
  COMMAND ${CMAKE_COMMAND} -E make_directory "<INSTALL_DIR>/include"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_sqlite_src}/sqlite3.h"     "<INSTALL_DIR>/include/sqlite3.h"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_sqlite_src}/sqlite3ext.h"  "<INSTALL_DIR>/include/sqlite3ext.h"

  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# Export variables for SCIRun
set(SQLITE_SOURCE_DIR  ${_sqlite_src})
set(SQLITE_INSTALL_DIR ${_sqlite_inst})
set(SQLITE_INCLUDE     ${SQLITE_INSTALL_DIR}/include)
set(SQLITE_LIBRARY_DIR ${SQLITE_INSTALL_DIR}/lib)

# Library name note:
# Many SQLite CMake builds produce 'sqlite3' as the lib name.
# Your repo tag v3.0.1 may export 'sqlite' or 'sqlite3' depending on CMakeLists.
# If build fails to link, try switching this to 'sqlite3'.
set(SQLITE_LIBRARY     "sqlite")

message(STATUS "[SQLite_external] INSTALL_DIR=${SQLITE_INSTALL_DIR}")