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

set(sqlite_GIT_TAG "v3.51.2")

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

  # Let our script act as the "install" step
  INSTALL_COMMAND
    ${CMAKE_COMMAND}
      -Dsrc=<SOURCE_DIR>         # this is Externals/Source/SQLite_external
      -Ddst=${_sqlite_inst}/include
      -P ${CMAKE_CURRENT_LIST_DIR}/SQLiteInstall.cmake

  LOG_CONFIGURE 1
  LOG_BUILD     1
  LOG_INSTALL   1
)

ExternalProject_Get_Property(SQLite_external INSTALL_DIR)
set(SQLITE_INSTALL_DIR "${INSTALL_DIR}")
set(SQLITE_INCLUDE     "${SQLITE_INSTALL_DIR}/include")
set(SQLITE_LIBRARY_DIR "${SQLITE_INSTALL_DIR}/lib")

# Detect sqlite library name after build (sqlite vs sqlite3) with multi-config awareness
# We’ll prefer sqlite3.*, then sqlite.*
set(_sqlite_lib_names "sqlite3" "sqlite")
set(_sqlite_lib_exts)
if(WIN32)
  list(APPEND _sqlite_lib_exts ".lib")
else()
  list(APPEND _sqlite_lib_exts ".a" ".so" ".dylib")
endif()

# Small CMake script to resolve the name after build
set(_sqlite_check "${CMAKE_BINARY_DIR}/check_sqlite_lib_name.cmake")
file(WRITE "${_sqlite_check}" "
  set(LIBDIR \"${SQLITE_LIBRARY_DIR}\")
  set(names sqlite3 sqlite)
  if(WIN32)
    set(exts .lib)
  else()
    set(exts .a .so .dylib)
  endif()
  set(found \"\")
  foreach(n IN LISTS names)
    foreach(e IN LISTS exts)
      file(GLOB hits \"${SQLITE_LIBRARY_DIR}/${n}*${e}\")
      if(hits)
        set(found \"${n}\")
        break()
      endif()
    endforeach()
    if(found)
      break()
    endif()
  endforeach()
  if(NOT found)
    message(FATAL_ERROR \"SQLite lib not found in ${SQLITE_LIBRARY_DIR}. Expected sqlite3 or sqlite.\")
  endif()
  # Persist the detected base name
  set(SQLITE_LIBRARY \"${found}\" CACHE STRING \"SQLite library base name (sqlite vs sqlite3)\" FORCE)
  message(STATUS \"[SQLite_external] Detected library basename: ${found}\")
")

ExternalProject_Add_Step(SQLite_external verify_lib_name
  COMMAND ${CMAKE_COMMAND} -P "${_sqlite_check}"
  DEPENDEES build
  COMMENT "Verifying SQLite library name and caching SQLITE_LIBRARY"
)

# Export variables for SCIRun
set(SQLITE_SOURCE_DIR  ${_sqlite_src})
set(SQLITE_INSTALL_DIR ${_sqlite_inst})
set(SQLITE_INCLUDE     ${SQLITE_INSTALL_DIR}/include)
set(SQLITE_LIBRARY_DIR ${SQLITE_INSTALL_DIR}/lib)

message(STATUS "[SQLite_external] INSTALL_DIR=${SQLITE_INSTALL_DIR}")