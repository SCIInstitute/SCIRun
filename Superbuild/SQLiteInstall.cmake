# SQLiteInstall.cmake
# Usage:
#   cmake -Dsrc=<source_dir> -Ddst=<install_include_dir> -P SQLiteInstall.cmake
#
# Copies SQLite public headers into the install include directory in a robust way.
# - Supports both amalgamation layout (headers in repo root) and CMake "include/" layout.
# - Fails fast if sqlite3.h cannot be found in any expected location.

cmake_minimum_required(VERSION 3.15)

if(NOT DEFINED src OR NOT DEFINED dst)
  message(FATAL_ERROR "SQLiteInstall.cmake requires -Dsrc and -Ddst (got src='${src}', dst='${dst}')")
endif()

# Normalize paths
file(TO_CMAKE_PATH "${src}" _SRC)
file(TO_CMAKE_PATH "${dst}" _DST)

# Create destination include dir
file(MAKE_DIRECTORY "${_DST}")

# Candidate locations to probe for public headers
# 1) Amalgamation in project root:   <src>/sqlite3.h, sqlite3ext.h
# 2) CMake-style include directory:  <src>/include/sqlite3.h
# 3) Rare alternative layouts:       <src>/inc/sqlite3.h
set(_CANDIDATES
  "${_SRC}"
  "${_SRC}/include"
  "${_SRC}/inc"
)

# Find a candidate that actually contains sqlite3.h
set(_FOUND_SOURCE "")
foreach(_C IN LISTS _CANDIDATES)
  if(EXISTS "${_C}/sqlite3.h")
    set(_FOUND_SOURCE "${_C}")
    break()
  endif()
endforeach()

if(NOT _FOUND_SOURCE)
  # Print helpful diagnostics
  message(STATUS "[SQLiteInstall.cmake] Probed candidates:")
  foreach(_C IN LISTS _CANDIDATES)
    message(STATUS "  - ${_C}")
  endforeach()
  message(FATAL_ERROR "Could not find 'sqlite3.h' under any candidate. "
                      "Searched in: ${_CANDIDATES}")
endif()

message(STATUS "[SQLiteInstall.cmake] Using header source dir: ${_FOUND_SOURCE}")
message(STATUS "[SQLiteInstall.cmake] Destination include dir: ${_DST}")

# Copy sqlite3.h (required)
file(COPY "${_FOUND_SOURCE}/sqlite3.h" DESTINATION "${_DST}")

# Copy sqlite3ext.h if present (optional extension API)
if(EXISTS "${_FOUND_SOURCE}/sqlite3ext.h")
  file(COPY "${_FOUND_SOURCE}/sqlite3ext.h" DESTINATION "${_DST}")
else()
  message(STATUS "[SQLiteInstall.cmake] Optional header not found: ${_FOUND_SOURCE}/sqlite3ext.h")
endif()

# Verify copy succeeded
if(NOT EXISTS "${_DST}/sqlite3.h")
  message(FATAL_ERROR "Copy failed: '${_DST}/sqlite3.h' not found after copy.")
endif()

message(STATUS "[SQLiteInstall.cmake] Copied headers into: ${_DST}")