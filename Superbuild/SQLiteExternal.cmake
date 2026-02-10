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

# SQLiteExternal.cmake
# - Clones CIBC-Internal/sqlite at a tag/branch that contains the amalgamation at repo root:
#     sqlite3.c, sqlite3.h, (optional) sqlite3ext.h
# - Verifies presence, builds static lib 'sqlite3', installs headers+lib to <INSTALL_DIR>
# - Exports cache variables for SCIRun

cmake_minimum_required(VERSION 3.15)

# Keep ExternalProject data under the superbuild tree (provided by the top-level superbuild)
set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

set(sqlite_GIT_TAG "v3.51.2")

# Superbuild layout
set(_sqlite_src  "${CMAKE_BINARY_DIR}/Externals/Source/SQLite_external")
set(_sqlite_bin  "${CMAKE_BINARY_DIR}/Externals/Build/SQLite_external")
set(_sqlite_inst "${CMAKE_BINARY_DIR}/Externals/Install/SQLite_external")
file(MAKE_DIRECTORY "${_sqlite_inst}")

include(ExternalProject)

# ---------- Wrapper project (written into the build tree) ----------
set(_wrapper_dir "${CMAKE_BINARY_DIR}/Externals/SQLiteWrapper")
file(MAKE_DIRECTORY "${_wrapper_dir}")

file(WRITE "${_wrapper_dir}/CMakeLists.txt" "
cmake_minimum_required(VERSION 3.15)
# Force VS to use $(Configuration) instead of <CONFIG>
set(CMAKE_CFG_INTDIR \"$(Configuration)\" CACHE STRING \"VS config placeholder\" FORCE)

project(SQLiteExternal C)

if(NOT SQLite_SOURCE_DIR)
  message(FATAL_ERROR \"SQLiteWrapper requires -DSQLite_SOURCE_DIR=<path>\")
endif()

# Require amalgamation present at the directory we were told (repo root)
if(NOT EXISTS \"\${SQLite_SOURCE_DIR}/sqlite3.c\")
  message(FATAL_ERROR \"sqlite3.c not found in \${SQLite_SOURCE_DIR}\")
endif()
if(NOT EXISTS \"\${SQLite_SOURCE_DIR}/sqlite3.h\")
  message(FATAL_ERROR \"sqlite3.h not found in \${SQLite_SOURCE_DIR}\")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build static library from amalgamation
add_library(sqlite3 STATIC \"\${SQLite_SOURCE_DIR}/sqlite3.c\")
target_include_directories(sqlite3 PUBLIC \"\${SQLite_SOURCE_DIR}\")

# Install lib + headers
install(TARGETS sqlite3
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(FILES \"\${SQLite_SOURCE_DIR}/sqlite3.h\" DESTINATION include)
if(EXISTS \"\${SQLite_SOURCE_DIR}/sqlite3ext.h\")
  install(FILES \"\${SQLite_SOURCE_DIR}/sqlite3ext.h\" DESTINATION include)
endif()
")

# ---------- Fast pre-check (fail early if files aren't there) ----------
set(_precheck "${CMAKE_BINARY_DIR}/Externals/SQLitePrecheck.cmake")
file(WRITE "${_precheck}" "
  cmake_minimum_required(VERSION 3.15)
  if(NOT src)
    message(FATAL_ERROR \"SQLitePrecheck.cmake requires -Dsrc\")
  endif()
  file(TO_CMAKE_PATH \"\${src}\" src)
  get_filename_component(src \"\${src}\" ABSOLUTE)
  if(NOT EXISTS \"\${src}/sqlite3.c\")
    message(FATAL_ERROR \"[SQLite_precheck] Missing: \${src}/sqlite3.c\")
  endif()
  if(NOT EXISTS \"\${src}/sqlite3.h\")
    message(FATAL_ERROR \"[SQLite_precheck] Missing: \${src}/sqlite3.h\")
  endif()
  message(STATUS \"[SQLite_precheck] Found amalgamation at: \${src}\")
")

# Common args for wrapper configure (control outputs; multi-config aware)
set(_cfg_args
  -DSQLite_SOURCE_DIR:PATH=<SOURCE_DIR>
  -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=<INSTALL_DIR>/lib
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=<INSTALL_DIR>/bin
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=<INSTALL_DIR>/lib
  -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG:PATH=<INSTALL_DIR>/lib
  -DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE:PATH=<INSTALL_DIR>/lib
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG:PATH=<INSTALL_DIR>/bin
  -DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE:PATH=<INSTALL_DIR>/bin
  -DCMAKE_CFG_INTDIR:STRING=$(Configuration)
)
if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  list(APPEND _cfg_args -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
endif()

# Forward generator settings only if set
set(_gen_fwd CMAKE_GENERATOR "${CMAKE_GENERATOR}")
if(CMAKE_GENERATOR_PLATFORM)
  list(APPEND _gen_fwd CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}")
endif()
if(CMAKE_GENERATOR_TOOLSET)
  list(APPEND _gen_fwd CMAKE_GENERATOR_TOOLSET  "${CMAKE_GENERATOR_TOOLSET}")
endif()

ExternalProject_Add(SQLite_external
  GIT_REPOSITORY "https://github.com/CIBC-Internal/sqlite.git"
  GIT_TAG        ${sqlite_GIT_TAG}
  GIT_SHALLOW    TRUE
  GIT_PROGRESS   TRUE

  # Prevent the 'update' step from running 'git' in SOURCE_DIR (we pin to a tag)
  UPDATE_COMMAND ""

  UPDATE_DISCONNECTED 1

  SOURCE_DIR ${_sqlite_src}
  BINARY_DIR ${_sqlite_bin}

  # Configure / Build / Install wrapper
  CONFIGURE_COMMAND
    ${CMAKE_COMMAND} -S "${_wrapper_dir}" -B "<BINARY_DIR>/wrap" ${_cfg_args}

  BUILD_COMMAND
    ${CMAKE_COMMAND} --build "<BINARY_DIR>/wrap" --config Release

  INSTALL_COMMAND
    ${CMAKE_COMMAND} --build "<BINARY_DIR>/wrap" --target install --config Release

  ${_gen_fwd}

  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

# Pre-configure step: verify amalgamation exists at repo root
ExternalProject_Add_Step(SQLite_external verify_amalgamation
  COMMAND ${CMAKE_COMMAND} -Dsrc=<SOURCE_DIR> -P "${_precheck}"
  DEPENDEES download
  DEPENDERS configure
  COMMENT "Verifying sqlite3.c/sqlite3.h exist at <SOURCE_DIR> (amalgamation at repo root)"
)

# ---------- Export variables for SCIRun ----------
ExternalProject_Get_Property(SQLite_external INSTALL_DIR)
set(SQLITE_INSTALL_DIR "${INSTALL_DIR}")

# Cache the exported vars so SCIRun configure sees stable values
set(SQLITE_SOURCE_DIR  "${_sqlite_src}"                CACHE PATH   "SQLite source dir (clone)" FORCE)
set(SQLITE_INSTALL_DIR "${SQLITE_INSTALL_DIR}"         CACHE PATH   "SQLite install dir"        FORCE)
set(SQLite_INCLUDE_DIR "${SQLITE_INSTALL_DIR}/include" CACHE PATH   "SQLite include dir"        FORCE)
set(SQLite_LIBRARY_DIR "${SQLITE_INSTALL_DIR}/lib"     CACHE PATH   "SQLite library dir"        FORCE)

# Basename built by wrapper
if(NOT DEFINED SQLITE_LIBRARY)
  set(SQLITE_LIBRARY "sqlite3" CACHE STRING "SQLite library base name (sqlite vs sqlite3)" FORCE)
endif()

message(STATUS "[SQLite_external] INSTALL_DIR=${SQLITE_INSTALL_DIR}")
message(STATUS "[SQLite_external] Include dir=${SQLite_INCLUDE_DIR}")
message(STATUS "[SQLite_external] Lib dir=${SQLite_LIBRARY_DIR}")
message(STATUS "[SQLite_external] Library base=${SQLITE_LIBRARY}")