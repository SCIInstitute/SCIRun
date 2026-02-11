#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2020 Scientific Computing and Imaging Institute,
#  University of Utah.
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
#

# --- Sources ---
set(Core_DatabaseManager_SRCS
  DatabaseManager.cc
)

set(Core_DatabaseManager_HEADERS
  DatabaseManager.h
  share.h
)

SCIRUN_ADD_LIBRARY(Core_DatabaseManager
  ${Core_DatabaseManager_HEADERS}
  ${Core_DatabaseManager_SRCS}
)

if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_Core_DatabaseManager)
endif()

# =========================
# SQLite3 discovery
# =========================
# 1) Try config package
find_package(SQLite3 CONFIG QUIET)
if(TARGET SQLite::SQLite3)
  set(_sqlite_target SQLite::SQLite3)
else()
  # 2) If superbuild passed an exact library file, use it
  if(SQLite3_LIBRARY AND EXISTS "${SQLite3_LIBRARY}")
    add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
    set_target_properties(SQLite::SQLite3 PROPERTIES
      IMPORTED_LOCATION "${SQLite3_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${SQLite3_INCLUDE_DIR}"
    )
    set(_sqlite_target SQLite::SQLite3)
  else()
    # 3) Build from hints: include dir and lib dir
    if(NOT SQLite3_INCLUDE_DIR)
      message(FATAL_ERROR "SQLite3_INCLUDE_DIR not set; superbuild should pass it (path to folder with sqlite3.h).")
    endif()

    # Try to locate the library:
    # 3a) If SQLite_LIB_DIR is provided, compute the canonical name(s)
    set(_sqlite_lib "")
    if(SQLite_LIB_DIR)
      if(WIN32)
        foreach(_cand sqlite3.lib libsqlite3.lib)
          if(EXISTS "${SQLite_LIB_DIR}/${_cand}")
            set(_sqlite_lib "${SQLite_LIB_DIR}/${_cand}")
            break()
          endif()
        endforeach()
      elseif(APPLE)
        if(EXISTS "${SQLite_LIB_DIR}/libsqlite3.dylib")
          set(_sqlite_lib "${SQLite_LIB_DIR}/libsqlite3.dylib")
        endif()
      else()
        if(EXISTS "${SQLite_LIB_DIR}/libsqlite3.so")
          set(_sqlite_lib "${SQLite_LIB_DIR}/libsqlite3.so")
        endif()
      endif()
    endif()

    # 3b) If still not found, fall back to find_library with hints
    if(NOT _sqlite_lib)
      find_library(SQLite3_LIBRARY
        NAMES sqlite3 libsqlite3
        HINTS "${SQLite_LIB_DIR}"
              "${CMAKE_PREFIX_PATH}"
              "${CMAKE_PREFIX_PATH}/lib"
              "${CMAKE_PREFIX_PATH}/lib64"
      )
      if(SQLite3_LIBRARY)
        set(_sqlite_lib "${SQLite3_LIBRARY}")
      endif()
    endif()

    if(NOT _sqlite_lib OR NOT EXISTS "${_sqlite_lib}")
      message(FATAL_ERROR
        "SQLite3 library not found.\n"
        "  SQLite3_INCLUDE_DIR='${SQLite3_INCLUDE_DIR}'\n"
        "  SQLite_LIB_DIR='${SQLite_LIB_DIR}'\n"
        "  CMAKE_PREFIX_PATH='${CMAKE_PREFIX_PATH}'\n"
        "Ensure the SQLite external installs a library (sqlite3.lib/.so/.dylib).")
    endif()

    add_library(SQLite::SQLite3 UNKNOWN IMPORTED)
    set_target_properties(SQLite::SQLite3 PROPERTIES
      IMPORTED_LOCATION "${_sqlite_lib}"
      INTERFACE_INCLUDE_DIRECTORIES "${SQLite3_INCLUDE_DIR}"
    )
    set(_sqlite_target SQLite::SQLite3)
  endif()
endif()

target_link_libraries(Core_DatabaseManager
  PRIVATE
    ${_sqlite_target}
    ${SCI_BOOST_LIBRARY}
)

# Tests
SCIRUN_ADD_TEST_DIR(Tests)