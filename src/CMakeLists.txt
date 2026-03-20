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

CMAKE_MINIMUM_REQUIRED(VERSION 3.15 FATAL_ERROR)

PROJECT(SCIRun)

# Honor CMAKE_MSVC_RUNTIME_LIBRARY for VS generators
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif()

# Use proper CRT per configuration (Debug -> /MDd, others -> /MD)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

IF(GENERATE_COMPILATION_DATABASE)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
ENDIF()

# -------------------------------
# SCIRun path diagnostics (what we actually received from the superbuild)
# -------------------------------
message(STATUS "[SCIRun] CMAKE_PREFIX_PATH = '${CMAKE_PREFIX_PATH}'")
if(CMAKE_PREFIX_PATH)
  message(STATUS "[SCIRun] CMAKE_PREFIX_PATH entries:")
  foreach(_p IN LISTS CMAKE_PREFIX_PATH)
    file(TO_CMAKE_PATH "${_p}" _p_norm)
    if(EXISTS "${_p_norm}")
      message(STATUS "  ✓ ${_p_norm}")
    else()
      message(STATUS "  ✗ ${_p_norm} (does not exist at configure time)")
    endif()
  endforeach()
endif()
message(STATUS "[SCIRun] Qt_PATH='${Qt_PATH}'")
message(STATUS "[SCIRun] Boost_DIR='${Boost_DIR}' ; Boost_ROOT='${Boost_ROOT}' ; BOOST_ROOT='${BOOST_ROOT}' ; SCI_BOOST_PREFIX='${SCI_BOOST_PREFIX}'")
message(STATUS "[SCIRun] ZLIB_ROOT='${ZLIB_ROOT}' ; ZLIB_DIR='${ZLIB_DIR}' ; ZLIB_INCLUDE_DIR='${ZLIB_INCLUDE_DIR}'")

# --- Boost (Windows-only: b2-built libs from b2/--layout=versioned) ---
# If SCIRUN_EXPLICIT_BOOST_LINK=ON (from superbuild), we DISABLE MSVC autolink
# via BOOST_ALL_NO_LIB and link required Boost components explicitly.
if(SCIRUN_EXPLICIT_BOOST_LINK)
  add_compile_definitions(BOOST_ALL_NO_LIB)
endif()

if(WIN32)
  if(DEFINED SCI_BOOST_PREFIX)
    set(_boost_use "${SCI_BOOST_PREFIX}/UseBoost.cmake")
    if(EXISTS "${_boost_use}")
      include("${_boost_use}")  # adds include + lib search dirs, keeps autolink ON
    else()
      # Fallback: add include and library search dirs from exported vars
      if(DEFINED SCI_BOOST_INCLUDE AND IS_DIRECTORY "${SCI_BOOST_INCLUDE}")
        include_directories(SYSTEM "${SCI_BOOST_INCLUDE}")
      endif()
      if(DEFINED SCI_BOOST_LIBRARY_DIR AND IS_DIRECTORY "${SCI_BOOST_LIBRARY_DIR}")
        # Directory-wide link search path so MSVC finds libboost_*‑vc143‑... .lib files
        link_directories("${SCI_BOOST_LIBRARY_DIR}")
      endif()
    endif()

    # Optional sanity check: warn if no versioned Boost libs were found
    if(DEFINED SCI_BOOST_LIBRARY_DIR AND IS_DIRECTORY "${SCI_BOOST_LIBRARY_DIR}")
      file(GLOB _boost_ver_libs "${SCI_BOOST_LIBRARY_DIR}/libboost_*vc*.lib")
      if(NOT _boost_ver_libs)
        message(WARNING
          "[SCIRun] No versioned Boost .lib files detected under '${SCI_BOOST_LIBRARY_DIR}'.\n"
          "         Ensure your external Boost build used b2 with '--layout=versioned' and staged/installed libs.")
      endif()
      unset(_boost_ver_libs)
    endif()
  else()
    message(WARNING "[SCIRun] SCI_BOOST_PREFIX is not defined; Windows auto-link may fail to locate Boost libs.")
  endif()
endif()

# --------------------------------------------------------------------
# Boost explicit linking (Windows + SCIRUN_EXPLICIT_BOOST_LINK)
# - Collect concrete .lib files per component and append to SCI_BOOST_LIBRARY.
# - Handles Debug (-gd) and Release variants; falls back if one variant missing.
# --------------------------------------------------------------------
if(SCIRUN_EXPLICIT_BOOST_LINK AND WIN32 AND DEFINED SCI_BOOST_LIBRARY_DIR AND IS_DIRECTORY "${SCI_BOOST_LIBRARY_DIR}")

  # Helper: pick first match from a list of patterns
  function(_pick_first out_var)
    set(_res "")
    foreach(_pat IN LISTS ARGN)
      file(GLOB _cands "${_pat}")
      if(_cands)
        list(SORT _cands)
        list(GET _cands 0 _res)
        break()
      endif()
    endforeach()
    set(${out_var} "${_res}" PARENT_SCOPE)
  endfunction()

  # Reset the aggregate to avoid duplicates across re-configures
  set(SCI_BOOST_LIBRARY "" CACHE STRING "Aggregate Boost libs" FORCE)

  # ---- Generic component adder (release + debug) ----
  function(_boost_pick_component comp)
    if(NOT EXISTS "${SCI_BOOST_LIBRARY_DIR}")
      message(WARNING "[SCIRun] _boost_pick_component: SCI_BOOST_LIBRARY_DIR is invalid.")
      return()
    endif()

    # Release candidates (versioned names from b2 --layout=versioned)
    set(_rel "")
    _pick_first(_rel
      "${SCI_BOOST_LIBRARY_DIR}/libboost_${comp}*-vc*-mt-x64-*.lib"    # preferred x64 MSVC layout
      "${SCI_BOOST_LIBRARY_DIR}/boost_${comp}*.lib"                    # fallback
      "${SCI_BOOST_LIBRARY_DIR}/libboost_${comp}*.lib")                # fallback
    # Debug candidates (-gd)
    set(_dbg "")
    _pick_first(_dbg
      "${SCI_BOOST_LIBRARY_DIR}/libboost_${comp}*-vc*-mt-gd-x64-*.lib" # preferred x64 Debug
      "${SCI_BOOST_LIBRARY_DIR}/boost_${comp}*-gd*.lib"
      "${SCI_BOOST_LIBRARY_DIR}/libboost_${comp}*-gd*.lib")

    if(NOT _rel AND NOT _dbg)
      message(WARNING "[SCIRun] Boost component '${comp}' not found under '${SCI_BOOST_LIBRARY_DIR}'")
      return()
    endif()

    # If a variant is missing, fall back to the one we have
    if(NOT _rel AND _dbg)
      set(_rel "${_dbg}")
    endif()
    if(NOT _dbg AND _rel)
      set(_dbg "${_rel}")
    endif()

    # Append with config mapping so MSVC picks the right one
    list(APPEND SCI_BOOST_LIBRARY
      optimized "${_rel}" debug "${_dbg}")

    # Save to cache so subdirs see the updated list
    set(SCI_BOOST_LIBRARY "${SCI_BOOST_LIBRARY}" CACHE STRING "Aggregate Boost libs" FORCE)
    message(STATUS "[SCIRun] Boost ${comp}: rel='${_rel}' dbg='${_dbg}'")
  endfunction()

  # ---- Pick all required components explicitly ----
  # Filesystem needs System; Thread usually needs System and Atomic.
  _boost_pick_component(filesystem)
  _boost_pick_component(system)
  _boost_pick_component(program_options)
  _boost_pick_component(thread)
  _boost_pick_component(atomic)
  _boost_pick_component(serialization)

  # Construct the versioned Boost.Python component name: python313 for Python 3.13
  set(_py_comp "python${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
  
  # Try the versioned component first (common with b2 builds), then fall back to unversioned "python"
  # We call _boost_pick_component on both; the function tolerates missing libs and will warn if not found.
  _boost_pick_component("${_py_comp}")
  _boost_pick_component("python")

endif()

# --------------------------------------------------------------------
# Imported targets for Zlib and Teem (transitive wiring)
#   - Canonical name: ZLIB::ZLIB (provide Zlib::Zlib alias)
#   - Canonical name: Teem::teem (links transitively to ZLIB::ZLIB)
# --------------------------------------------------------------------

# Normalize/bridge variables for includes used elsewhere
if(DEFINED ZLIB_INCLUDE_DIR AND NOT DEFINED SCI_ZLIB_INCLUDE)
  set(SCI_ZLIB_INCLUDE "${ZLIB_INCLUDE_DIR}")
endif()
# SCI_TEEM_INCLUDE is assumed to be set elsewhere by the superbuild

# ------------------------
# 1) Zlib imported target
# ------------------------
# Goal: ensure ZLIB::ZLIB always exists; provide Zlib::Zlib alias for compatibility.

if(NOT TARGET ZLIB::ZLIB)
  # Try a config package first (may define ZLIB::ZLIB)
  # Hints leverage ZLIB_ROOT/ZLIB_DIR/CMAKE_PREFIX_PATH already
  find_package(ZLIB QUIET CONFIG)

  if(NOT TARGET ZLIB::ZLIB)
    # Fall back to module find (classic FindZLIB)
    find_package(ZLIB QUIET)

    if(NOT TARGET ZLIB::ZLIB)
      # No package target; build imported target from variables.
      # Prefer debug/release split if available.
      if((ZLIB_LIBRARY_RELEASE OR ZLIB_LIBRARY) AND ZLIB_INCLUDE_DIR)
        add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
        if(ZLIB_LIBRARY_RELEASE)
          set_target_properties(ZLIB::ZLIB PROPERTIES
            IMPORTED_LOCATION_RELEASE         "${ZLIB_LIBRARY_RELEASE}"
            INTERFACE_INCLUDE_DIRECTORIES     "${ZLIB_INCLUDE_DIR}"
          )
        endif()
        if(ZLIB_LIBRARY_DEBUG)
          set_target_properties(ZLIB::ZLIB PROPERTIES
            IMPORTED_LOCATION_DEBUG           "${ZLIB_LIBRARY_DEBUG}"
          )
        endif()
        if(NOT ZLIB_LIBRARY_RELEASE AND ZLIB_LIBRARY)
          # Single lib used for all configs
          set_target_properties(ZLIB::ZLIB PROPERTIES
            IMPORTED_LOCATION                 "${ZLIB_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES     "${ZLIB_INCLUDE_DIR}"
          )
        endif()
      elseif(ZLIB_LIBRARIES AND ZLIB_INCLUDE_DIRS)
        add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
        set_target_properties(ZLIB::ZLIB PROPERTIES
          IMPORTED_LOCATION                 "${ZLIB_LIBRARIES}"
          INTERFACE_INCLUDE_DIRECTORIES     "${ZLIB_INCLUDE_DIRS}"
        )
      else()
        message(WARNING "[SCIRun] Zlib not fully configured yet (need ZLIB_LIBRARY{_DEBUG/_RELEASE} and ZLIB_INCLUDE_DIR). "
                        "This can be expected before externals are installed.")
      endif()
    endif()
  endif()
endif()

# Provide compatibility alias if canonical exists
if(TARGET ZLIB::ZLIB AND NOT TARGET Zlib::Zlib)
  add_library(Zlib::Zlib ALIAS ZLIB::ZLIB)
endif()

# ------------------------
# 2) Teem imported target
# ------------------------
# Goal: ensure Teem::teem exists; link transitively to ZLIB::ZLIB when available.

if(NOT TARGET Teem::teem)
  # Prefer a config-provided target if present
  find_package(Teem QUIET CONFIG)

  if(TARGET Teem::teem)
    # Already provided by package—augment linkage if needed
    if(TARGET ZLIB::ZLIB)
      # Append zlib transitively; don't overwrite existing interface links
      get_target_property(_teem_links Teem::teem INTERFACE_LINK_LIBRARIES)
      if(NOT _teem_links)
        set(_teem_links "")
      endif()
      list(APPEND _teem_links ZLIB::ZLIB)
      list(REMOVE_DUPLICATES _teem_links)
      set_target_properties(Teem::teem PROPERTIES INTERFACE_LINK_LIBRARIES "${_teem_links}")
    endif()
  else()
    # Create imported from explicit paths (superbuild should provide Teem_LIBRARY/Teem_INCLUDE_DIR)
    if(DEFINED Teem_LIBRARY AND EXISTS "${Teem_LIBRARY}" AND DEFINED Teem_INCLUDE_DIR AND EXISTS "${Teem_INCLUDE_DIR}/teem/ten.h")
      add_library(Teem::teem UNKNOWN IMPORTED)
      set_target_properties(Teem::teem PROPERTIES
        IMPORTED_LOCATION                 "${Teem_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES     "${Teem_INCLUDE_DIR}"
        # Make consumers treat Teem headers as static-linked (prevents dllimport)
        INTERFACE_COMPILE_DEFINITIONS     "TEEM_STATIC"
      )
      if(TARGET ZLIB::ZLIB)
        set_property(TARGET Teem::teem APPEND PROPERTY INTERFACE_LINK_LIBRARIES ZLIB::ZLIB)
      endif()
    else()
      message(WARNING "[SCIRun] Teem not fully configured yet (need Teem_LIBRARY and Teem_INCLUDE_DIR). "
                      "This can be expected before externals are built.")
    endif()
  endif()
endif()

# --- LodePNG imported target fallback (manual, cross‑platform) ---
if(NOT TARGET LodePNG::lodepng)
  set(_LPNG_ROOT "${CMAKE_BINARY_DIR}/../Externals/Install/LodePng_external")
  set(_LPNG_INC  "${_LPNG_ROOT}/include")
  set(_LPNG_LIBDIR "${_LPNG_ROOT}/lib")

  # Cross‑platform library detection
  set(_LPNG_LIB "")

  if(WIN32)
    if(EXISTS "${_LPNG_LIBDIR}/lodepngd.lib")
      set(_LPNG_LIB "${_LPNG_LIBDIR}/lodepngd.lib")
    elseif(EXISTS "${_LPNG_LIBDIR}/lodepng.lib")
      set(_LPNG_LIB "${_LPNG_LIBDIR}/lodepng.lib")
    endif()
  elseif(APPLE)
    # macOS: static or shared
    if(EXISTS "${_LPNG_LIBDIR}/liblodepng.a")
      set(_LPNG_LIB "${_LPNG_LIBDIR}/liblodepng.a")
    elseif(EXISTS "${_LPNG_LIBDIR}/liblodepng.dylib")
      set(_LPNG_LIB "${_LPNG_LIBDIR}/liblodepng.dylib")
    endif()
  else()
    # Linux: static or shared
    if(EXISTS "${_LPNG_LIBDIR}/liblodepng.a")
      set(_LPNG_LIB "${_LPNG_LIBDIR}/liblodepng.a")
    elseif(EXISTS "${_LPNG_LIBDIR}/liblodepng.so")
      set(_LPNG_LIB "${_LPNG_LIBDIR}/liblodepng.so")
    endif()
  endif()

  if(_LPNG_LIB)
    add_library(LodePNG::lodepng UNKNOWN IMPORTED)

    set_target_properties(LodePNG::lodepng PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${_LPNG_INC}"
      IMPORTED_LOCATION "${_LPNG_LIB}"
    )

    message(STATUS
      "[SCIRun] LodePNG::lodepng imported:"
      " inc='${_LPNG_INC}', lib='${_LPNG_LIB}'"
    )
  else()
    message(WARNING
      "[SCIRun] LodePNG not found under '${_LPNG_LIBDIR}'. "
      "Build LodePng_external first."
    )
  endif()
endif()

###############################################################################
# Don't allow the user to build in the src directory...
#
IF(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
  MESSAGE( "" )
  MESSAGE( "Building SCIRun in the source directory is not allowed." )
  MESSAGE( "(In order to clean up, you now need to run: rm -rf CMakeFiles CMakeCache.txt" )
  MESSAGE( "" )
  MESSAGE( FATAL_ERROR "Goodbye." )
ENDIF()

include(CheckCCompilerFlag)

########################################################################
# Build shared libraries by default

OPTION(BUILD_SHARED_LIBS "Build shared libraries" ON)
MARK_AS_ADVANCED(BUILD_SHARED_LIBS)

########################################################################
# Set default CMAKE_BUILD_TYPE
# if empty for Unix Makefile builds

IF (CMAKE_GENERATOR MATCHES "Unix Makefiles" AND NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF (CMAKE_GENERATOR MATCHES "Unix Makefiles" AND NOT CMAKE_BUILD_TYPE)

########################################################################
# Git revision details

# Appends the cmake/modules path to MAKE_MODULE_PATH variable.
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules ${CMAKE_MODULE_PATH})

# Make a version file containing the current version from git.
include(GetGitRevisionDescription)
git_describe(VERSION_TAG --tag)
git_describe(VERSION_BRANCH --all)
get_git_head_revision(VERSION_REFSPEC VERSION_HASHVAR)

IF(VERSION_TAG MATCHES "HEAD-HASH-NOTFOUND")
  FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/LATEST_TAG.txt VERSION_TAG)
  STRING(STRIP ${VERSION_TAG} VERSION_TAG)
  MESSAGE(STATUS "Found tag file for version info: " "${VERSION_TAG}")
ENDIF()

MESSAGE(STATUS "Git version tag: " "${VERSION_TAG}")
MESSAGE(STATUS "Git version branch: " "${VERSION_BRANCH}")
MESSAGE(STATUS "Git version commit hash: " "${VERSION_HASHVAR}")

#parse the version information into pieces.
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION_TAG}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION_TAG}")
#TODO: make generic
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\-(beta.[A-Za-z0-9_]+)\\-?.*" "\\1" VERSION_PATCH "${VERSION_TAG}")
#TODO: parse SHA
#string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${VERSION}")
#set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

#MESSAGE(STATUS "Git version major: " "${VERSION_MAJOR}")
#MESSAGE(STATUS "Git version minor: " "${VERSION_MINOR}")
#MESSAGE(STATUS "Git version patch name: " "${VERSION_PATCH}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules/Version.cc.in
 ${CMAKE_CURRENT_BINARY_DIR}/Core/Application/Version.cc)
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/Core/Application/Version.cc")

########################################################################
# SCIRUN version number.

SET(SCIRUN_VERSION_MAJOR "${VERSION_MAJOR}")
SET(SCIRUN_VERSION_MINOR "${VERSION_MINOR}")
SET(SCIRUN_VERSION_PATCH "${VERSION_PATCH}")

SET(SCIRUN_VERSION_STRING "${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")

########################################################################
# APPLICATION Information (used by CPack)
#
# Based on Seg3D settings...

SET(APPLICATION_NAME "SCIRun" CACHE INTERNAL "Application name" FORCE)
# TODO: temporary hack to allow testing
SET(TEST_APPLICATION_NAME "SCIRun_test" CACHE INTERNAL "Application name" FORCE)
# add others?
SET(APPLICATION_EXTENSION ".srn5" CACHE INTERNAL "Network file extension" FORCE)
SET(APPLICATION_URL "www.scirun.org" CACHE INTERNAL "Project URL" FORCE)
SET(APPLICATION_VENDOR "SCI" CACHE INTERNAL "Vendor information" FORCE)
SET(APPLICATION_CONTACT "scirun-users@sci.utah.edu" CACHE INTERNAL "Contact information" FORCE)
SET(APPLICATION_WINDOWS_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons\\scirun5_32.ico" CACHE INTERNAL "Windows icon" FORCE)

SET(APPLICATION_WINDOWS_RC "${CMAKE_SOURCE_DIR}/Resources/Icons/SCIRun.rc" CACHE INTERNAL "Windows icon" FORCE)
SET(APPLICATION_OSX_ICON_FULL_PATH "${CMAKE_SOURCE_DIR}/Resources/Icons/scirun5.icns" CACHE INTERNAL "OSX icon" FORCE)
SET(APPLICATION_OSX_ICON "scirun5.icns" CACHE INTERNAL "OSX icon" FORCE)

########################################################################
# Standard system and compiler tests

INCLUDE(CMakeBackwardCompatibilityC)
INCLUDE(CMakeBackwardCompatibilityCXX)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckCXXSourceCompiles)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(ExternalProject)

IF(WIN32)
  ADD_DEFINITIONS("/wd4251 /wd4275 -D_CRT_SECURE_NO_WARNINGS")
ENDIF(WIN32)

########################################################################
# External project utilities

OPTION(DISABLE_ExternalProject_UPDATE "Turn off ExternalProject UPDATE_COMMAND property (if not connected to the internet, for example" ON)
MARK_AS_ADVANCED(DISABLE_ExternalProject_UPDATE)
IF(DISABLE_ExternalProject_UPDATE)
  # do nothing commands...
  IF(WIN32)
    SET(DISABLED_UPDATE "UPDATE_COMMAND" "rem")
  ELSE()
    SET(DISABLED_UPDATE "UPDATE_COMMAND" "echo")
  ENDIF()
ENDIF()

########################################################################
# Configure test support

INCLUDE( CTest )

# ON by default
IF(BUILD_TESTING)
  ENABLE_TESTING()

  # Hack BUILDNAME to differentiate from regular SCIRun builds
  # TODO: remove when transitioning to production code
  SET(BUILDNAME "${BUILDNAME_SCIRun}" CACHE STRING "Name of build on the dashboard")
  MARK_AS_ADVANCED(BUILDNAME)

  OPTION(RUN_UNIT_TESTS "Run gtest unit tests" ON)
  OPTION(RUN_BASIC_REGRESSION_TESTS "Run basic regression tests" ON)
  OPTION(RUN_IMPORT_TESTS "Run v4 import tests" ON)

  IF(NOT EXISTS "${SCIRUN_TEST_RESOURCE_DIR}")
    MESSAGE( WARNING "Test resource path does not exist. Please set it correctly to run all the unit and regression tests. Clone this github repo to get all the files: https://github.com/CIBC-Internal/SCIRunTestData" )
  ENDIF()

  ADD_DEFINITIONS(-DTEST_RESOURCE_ROOT_DIR="${SCIRUN_TEST_RESOURCE_DIR}" -DBUILD_TESTING)
ENDIF()

#########################################################################
# Configure Spdlog
set (cxxfeaturelist ${CMAKE_CXX_COMPILE_FEATURES})

list (FIND cxxfeaturelist "cxx_thread_local" _index)
if (${_index} GREATER -1)
  #message("Found cxx_thread_local")
else()
  message(WARNING "Did NOT find cxx_thread_local--turning off feature of spdlog")
  add_definitions(-DDISABLE_SPDLOG_TLS=1)
endif()

#########################################################################
# External projects (configure-time discovery of externals)
#
# Robust keyword-based macro + calls with clear diagnostics.
#########################################################################

# --- Keyword-based CONFIG_STANDARD_EXTERNAL macro ---
include(CMakeParseArguments)

# Initialize global summary storage once (safe to include multiple times)
get_property(_sum_init GLOBAL PROPERTY SCIRUN_CONFIG_STD_SUMMARY SET)
if(NOT _sum_init)
  set_property(GLOBAL PROPERTY SCIRUN_CONFIG_STD_SUMMARY "")
endif()
unset(_sum_init)

# CONFIG_STANDARD_EXTERNAL(name [DIR <path>] [ROOT <prefix>] [CFG <file>] [CFGS <file>...]
#                          [CONFIG_ONLY] [REQUIRED] [QUIET] [VERBOSE])
#
# - name: CMake package name (e.g., ZLIB, Freetype, Boost).
# - DIR:  Either an exact config directory (contains *Config.cmake) OR a prefix to probe.
# - ROOT: Install prefix to probe (lib/cmake/<name>*, lib64/cmake/<name>*, share/<name>/cmake, etc).
# - CFG:  Primary config filename to look for (e.g., ZLIBConfig.cmake).
# - CFGS: Alternative config filenames to try if CFG not present (case variants).
# - CONFIG_ONLY: Prefer config packages (sets CMAKE_FIND_PACKAGE_PREFER_CONFIG ON during the call).
# - REQUIRED: Fail configure if not found (but optionally gated when in superbuild mode).
# - QUIET: Less chatter.
# - VERBOSE: Print every probed path when not found.
#
macro(CONFIG_STANDARD_EXTERNAL name)
  # Parse keywords
  set(_cse_one DIR ROOT CFG)
  set(_cse_multi CFGS)
  set(_cse_bool REQUIRED CONFIG_ONLY QUIET VERBOSE)
  cmake_parse_arguments(_CSE "${_cse_bool}" "" "${_cse_one};${_cse_multi}" ${ARGN})

  # Derive case variants
  set(_cse_name "${name}")
  string(TOUPPER "${_cse_name}" _cse_UP)
  string(TOLOWER "${_cse_name}" _cse_LOW)

  # Build candidate config filename list
  set(_cse_cfgs "")
  if(_CSE_CFG)
    list(APPEND _cse_cfgs "${_CSE_CFG}")
  endif()
  if(_CSE_CFGS)
    list(APPEND _cse_cfgs ${_CSE_CFGS})
  endif()
  if(NOT _cse_cfgs)
    list(APPEND _cse_cfgs
      "${_cse_name}Config.cmake"
      "${_cse_UP}Config.cmake"
      "${_cse_LOW}Config.cmake"
      "${_cse_LOW}-config.cmake"
      "${_cse_UP}-config.cmake"
    )
  endif()

  # Normalized hints
  set(_cse_dir  "${_CSE_DIR}")
  set(_cse_root "${_CSE_ROOT}")

  # For diagnostics
  unset(_cse_pkg_dir)
  set(_cse_tried_dirs "")

  # Detect superbuild mode heuristically if not already set by the project
  if(NOT DEFINED SCIRUN_SUPERBUILD_MODE)
    string(FIND "${CMAKE_PREFIX_PATH}" "Externals/Install" _cse_sb_idx)
    if(_cse_sb_idx GREATER -1)
      set(SCIRUN_SUPERBUILD_MODE ON)
    else()
      set(SCIRUN_SUPERBUILD_MODE OFF)
    endif()
    unset(_cse_sb_idx)
  endif()

  # Helpers
  macro(_cse_mark _dir)
    if(NOT _dir)
      # skip
    else()
      set(_hit "")
      foreach(_cfg IN LISTS _cse_cfgs)
        if(EXISTS "${_dir}/${_cfg}")
          set(_hit "✓")
          break()
        endif()
      endforeach()
      if(_hit)
        list(APPEND _cse_tried_dirs "  ✓ ${_dir} (${_cse_cfgs})")
        set(_cse_pkg_dir "${_dir}")
      else()
        list(APPEND _cse_tried_dirs "    ${_dir} (${_cse_cfgs})")
      endif()
    endif()
  endmacro()

  function(_cse_probe_prefix _prefix)
    if(NOT _prefix)
      return()
    endif()

    # Common locations (with lib64, cmake/CMake, share)
    set(_cse_candidate_dirs
      "${_prefix}/lib/cmake/${_cse_name}"
      "${_prefix}/lib/cmake/${_cse_UP}"
      "${_prefix}/lib64/cmake/${_cse_name}"
      "${_prefix}/lib64/cmake/${_cse_UP}"
      "${_prefix}/cmake/${_cse_name}"
      "${_prefix}/cmake/${_cse_UP}"
      "${_prefix}/CMake/${_cse_name}"
      "${_prefix}/CMake/${_cse_UP}"
      "${_prefix}/share/${_cse_name}/cmake"
      "${_prefix}/share/${_cse_UP}/cmake"
      "${_prefix}/share/${_cse_name}"
      "${_prefix}/share/${_cse_UP}"
    )

    # Versioned siblings under lib/cmake and lib64/cmake
    foreach(_base IN ITEMS
      "${_prefix}/lib/cmake/${_cse_name}"
      "${_prefix}/lib/cmake/${_cse_UP}"
      "${_prefix}/lib64/cmake/${_cse_name}"
      "${_prefix}/lib64/cmake/${_cse_UP}"
    )
      if(EXISTS "${_base}")
        _cse_mark("${_base}")
        if(DEFINED _cse_pkg_dir)
          set(_cse_pkg_dir "${_cse_pkg_dir}" PARENT_SCOPE)
          return()
        endif()
      endif()

      file(GLOB _sib "${_base}-*")
      foreach(_cand IN LISTS _sib)
        _cse_mark("${_cand}")
        if(DEFINED _cse_pkg_dir)
          set(_cse_pkg_dir "${_cse_pkg_dir}" PARENT_SCOPE)
          return()
        endif()
      endforeach()
    endforeach()

    foreach(_cand IN LISTS _cse_candidate_dirs)
      _cse_mark("${_cand}")
      if(DEFINED _cse_pkg_dir)
        set(_cse_pkg_dir "${_cse_pkg_dir}" PARENT_SCOPE)
        return()
      endif()
    endforeach()
  endfunction()

  # Banner
  if(NOT _CSE_QUIET)
    message(STATUS "[CONFIG_STANDARD_EXTERNAL] Probing ${_cse_name} ("
                   "DIR='${_cse_dir}', ROOT='${_cse_root}', "
                   "${_cse_name}_DIR='${${_cse_name}_DIR}', ${_cse_UP}_DIR='${${_cse_UP}_DIR}', "
                   "${_cse_name}_ROOT='${${_cse_name}_ROOT}', ${_cse_UP}_ROOT='${${_cse_UP}_ROOT}')")
  endif()

  # 0) Pre-set *_DIR
  if(DEFINED ${_cse_name}_DIR)
    _cse_mark("${${_cse_name}_DIR}")
  endif()
  if(NOT DEFINED _cse_pkg_dir AND DEFINED ${_cse_UP}_DIR)
    _cse_mark("${${_cse_UP}_DIR}")
  endif()

  # 1) *_ROOT and explicit ROOT=
  if(NOT DEFINED _cse_pkg_dir AND DEFINED ${_cse_name}_ROOT)
    _cse_probe_prefix("${${_cse_name}_ROOT}")
  endif()
  if(NOT DEFINED _cse_pkg_dir AND DEFINED ${_cse_UP}_ROOT)
    _cse_probe_prefix("${${_cse_UP}_ROOT}")
  endif()
  if(NOT DEFINED _cse_pkg_dir AND _cse_root)
    _cse_probe_prefix("${_cse_root}")
  endif()

  # 2) Interpret DIR as exact config dir, then as prefix
  if(NOT DEFINED _cse_pkg_dir AND _cse_dir)
    _cse_mark("${_cse_dir}")

    if(NOT DEFINED _cse_pkg_dir)
      if("${_cse_dir}" MATCHES "/lib64?/cmake/${_cse_name}$"
         OR "${_cse_dir}" MATCHES "/lib64?/cmake/${_cse_UP}$")
        file(GLOB _sib_dir "${_cse_dir}-*")
        foreach(_cand IN LISTS _sib_dir)
          _cse_mark("${_cand}")
          if(DEFINED _cse_pkg_dir)
            break()
          endif()
        endforeach()
      endif()
    endif()

    if(NOT DEFINED _cse_pkg_dir)
      _cse_probe_prefix("${_cse_dir}")
    endif()
  endif()

  # 3) CMAKE_PREFIX_PATH
  if(NOT DEFINED _cse_pkg_dir AND CMAKE_PREFIX_PATH)
    foreach(_cpp IN LISTS CMAKE_PREFIX_PATH)
      _cse_probe_prefix("${_cpp}")
      if(DEFINED _cse_pkg_dir)
        break()
      endif()
    endforeach()
  endif()

  # CONFIG_ONLY → prefer config packages during this call
  if(_CSE_CONFIG_ONLY)
    set(_cse_prev_pref "${CMAKE_FIND_PACKAGE_PREFER_CONFIG}")
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
  endif()

  # 4) Always attempt find_package() (even if _cse_pkg_dir wasn't detected).
  #    Assemble HINTS list from detected dir + all available hints.
  set(_cse_hints "")
  if(DEFINED _cse_pkg_dir)
    list(APPEND _cse_hints "${_cse_pkg_dir}")
    # Cache *_DIR to stabilize subsequent configures
    set(${_cse_name}_DIR "${_cse_pkg_dir}" CACHE PATH "${_cse_name} package directory" FORCE)
    set(${_cse_UP}_DIR   "${_cse_pkg_dir}" CACHE PATH "${_cse_name} package directory (upper)" FORCE)
    if(NOT _CSE_QUIET)
      list(JOIN _cse_cfgs ", " _cfgs_join)
      message(STATUS "[CONFIG_STANDARD_EXTERNAL] ${_cse_name}: using '${_cfgs_join}' in '${_cse_pkg_dir}'")
    endif()
  endif()
  if(_cse_dir)
    list(APPEND _cse_hints "${_cse_dir}")
  endif()
  if(DEFINED ${_cse_name}_DIR)
    list(APPEND _cse_hints "${${_cse_name}_DIR}")
  endif()
  if(DEFINED ${_cse_UP}_DIR)
    list(APPEND _cse_hints "${${_cse_UP}_DIR}")
  endif()
  if(DEFINED ${_cse_name}_ROOT)
    list(APPEND _cse_hints "${${_cse_name}_ROOT}")
  endif()
  if(DEFINED ${_cse_UP}_ROOT)
    list(APPEND _cse_hints "${${_cse_UP}_ROOT}")
  endif()
  if(_cse_root)
    list(APPEND _cse_hints "${_cse_root}")
  endif()
  foreach(_cpp IN LISTS CMAKE_PREFIX_PATH)
    list(APPEND _cse_hints "${_cpp}")
  endforeach()
  if(_cse_hints)
    list(REMOVE_DUPLICATES _cse_hints)
  endif()

  # Call find_package() in CONFIG mode quietly and let defaults work (no NO_DEFAULT_PATH).
  if(_cse_hints)
    find_package(${_cse_name} CONFIG QUIET HINTS ${_cse_hints})
  else()
    find_package(${_cse_name} CONFIG QUIET)
  endif()

  # Determine FOUND state robustly
  set(_cse_found FALSE)
  if(DEFINED ${_cse_name}_FOUND AND ${_cse_name}_FOUND)
    set(_cse_found TRUE)
  elseif(DEFINED ${_cse_UP}_FOUND AND ${_cse_UP}_FOUND)
    set(_cse_found TRUE)
  else()
    # Fallback for well-known packages: check imported targets
    if("${_cse_name}" STREQUAL "Boost")
      if(TARGET Boost::headers OR TARGET Boost::boost)
        set(_cse_found TRUE)
      endif()
    endif()
  endif()

  # Update summary (GLOBAL property to avoid parent-scope warnings)
  get_property(_sum GLOBAL PROPERTY SCIRUN_CONFIG_STD_SUMMARY)
  if(NOT _sum)
    set(_sum "")
  endif()

  if(_cse_found)
    if(NOT _CSE_QUIET)
      if(DEFINED _cse_pkg_dir)
        message(STATUS "[CONFIG_STANDARD_EXTERNAL] Found ${_cse_name} at '${_cse_pkg_dir}'")
      else()
        message(STATUS "[CONFIG_STANDARD_EXTERNAL] Found ${_cse_name} (via CMake search paths)")
      endif()
    endif()
    if(DEFINED _cse_pkg_dir)
      string(APPEND _sum "  ${_cse_name}: FOUND at '${_cse_pkg_dir}'\n")
    else()
      string(APPEND _sum "  ${_cse_name}: FOUND\n")
    endif()
  else()
    # Not found → decide whether to fail
    if(_CSE_REQUIRED AND NOT SCIRUN_SUPERBUILD_MODE)
      if(NOT _CSE_QUIET)
        message(STATUS "[CONFIG_STANDARD_EXTERNAL] ${_cse_name} not found. Probed:\n${_cse_tried_dirs}")
      endif()
      set_property(GLOBAL PROPERTY SCIRUN_CONFIG_STD_SUMMARY "${_sum}")
      # Fatal error for non-superbuild REQUIRED case
      message(FATAL_ERROR
        "[CONFIG_STANDARD_EXTERNAL] ${_cse_name} not found.\n"
        "  Tried locations for (${_cse_cfgs}):\n${_cse_tried_dirs}\n"
        "  Hints:\n"
        "    - ${_cse_name}_DIR='${${_cse_name}_DIR}'\n"
        "    - ${_cse_UP}_DIR='${${_cse_UP}_DIR}'\n"
        "    - ${_cse_name}_ROOT='${${_cse_name}_ROOT}'\n"
        "    - ${_cse_UP}_ROOT='${${_cse_UP}_ROOT}'\n"
        "    - DIR='${_cse_dir}'\n"
        "    - ROOT='${_cse_root}'\n"
        "    - CMAKE_PREFIX_PATH='${CMAKE_PREFIX_PATH}'\n"
        "  Hint: ensure the external is built/installed and re-run CMake."
      )
    else()
      if(_CSE_VERBOSE AND NOT _CSE_QUIET)
        message(STATUS "[CONFIG_STANDARD_EXTERNAL] ${_cse_name} not found. Probed:\n${_cse_tried_dirs}")
      elseif(NOT _CSE_QUIET)
        message(STATUS "[CONFIG_STANDARD_EXTERNAL] ${_cse_name} not found yet. This can be expected before externals are built.")
      endif()
      string(APPEND _sum "  ${_cse_name}: NOT FOUND\n")
    endif()
  endif()

  set_property(GLOBAL PROPERTY SCIRUN_CONFIG_STD_SUMMARY "${_sum}")
  unset(_sum)

  # Restore prefer-config policy if we changed it
  if(_CSE_CONFIG_ONLY)
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG "${_cse_prev_pref}")
    unset(_cse_prev_pref)
  endif()

  # Cleanup
  unset(_cse_pkg_dir)
  unset(_cse_tried_dirs)
  unset(_cse_hints)
endmacro()

# --- Calls for each external ---

# ZLIB
CONFIG_STANDARD_EXTERNAL(ZLIB
  ROOT "${ZLIB_ROOT}"
  DIR  "${ZLIB_DIR}"
  CFGS "ZLIBConfig.cmake;zlibConfig.cmake;zlib-config.cmake"
  # VERBOSE
)

# SQLite (SQLite vs SQLite3)
CONFIG_STANDARD_EXTERNAL(SQLite
  DIR  "${SQLite_DIR}"
  CFGS "SQLite3Config.cmake;SQLiteConfig.cmake"
)

# Teem (if a config is exported by your superbuild)
CONFIG_STANDARD_EXTERNAL(Teem
  DIR  "${Teem_DIR}"
  CFG  "TeemConfig.cmake"
)

# FreeType (case varies)
CONFIG_STANDARD_EXTERNAL(Freetype
  DIR  "${Freetype_DIR}"
  CFGS "FreetypeConfig.cmake;FreeTypeConfig.cmake"
)

# Cleaver2
CONFIG_STANDARD_EXTERNAL(Cleaver2
  DIR  "${CLEAVER2_DIR}"
  CFG  "Cleaver2Config.cmake"
)

# Boost — REQUIRED on non-Windows (prefer config package).
# On Windows with b2-built Boost, skip CONFIG mode: rely on auto-link + link_directories above.
if(WIN32)
  if(SCIRUN_EXPLICIT_BOOST_LINK)
    message(STATUS "[SCIRun] Boost: explicit component linking (BOOST_ALL_NO_LIB) with b2 versioned libs.")
  else()
    message(STATUS "[SCIRun] Boost: MSVC autolink enabled with b2 versioned libs.")
  endif()
else()
  CONFIG_STANDARD_EXTERNAL(Boost
    ROOT "${SCI_BOOST_PREFIX}"
    DIR  "${Boost_DIR}"
    CFG  "BoostConfig.cmake"
    CONFIG_ONLY
    REQUIRED
  )
endif()

# Tiny (Tny)
CONFIG_STANDARD_EXTERNAL(Tny
  DIR  "${TNY_DIR}"
  CFG  "TnyConfig.cmake"
)

# LodePNG
CONFIG_STANDARD_EXTERNAL(LodePNG
  DIR  "${LodePNG_DIR}"
  CFGS "LodePNGConfig.cmake"
)

# Tetgen (optional)
if(WITH_TETGEN)
  # Try package config first (does nothing if no TetgenConfig.cmake in CMAKE_PREFIX_PATH)
  #CONFIG_STANDARD_EXTERNAL(Tetgen
  #  DIR  "${Tetgen_DIR}"
  #  CFG  "TetgenConfig.cmake"
  #)

  # --- Fallback when Tetgen doesn't export a CMake package ---
  # Create an imported target from superbuild-provided cache hints.
  if(NOT TARGET Tetgen::Tetgen)
    # Heal include dir: if it doesn't contain tetgen.h, try '<dir>/src'
    set(_TETGEN_INC "${TETGEN_INCLUDE_DIR}")
    if(_TETGEN_INC AND NOT EXISTS "${_TETGEN_INC}/tetgen.h" AND EXISTS "${_TETGEN_INC}/src/tetgen.h")
      set(_TETGEN_INC "${_TETGEN_INC}/src")
    endif()

    # Create imported target if we have a real library file and a valid include dir
    if(TETGEN_LIBRARY AND EXISTS "${TETGEN_LIBRARY}" AND _TETGEN_INC AND EXISTS "${_TETGEN_INC}/tetgen.h")
      add_library(Tetgen::Tetgen UNKNOWN IMPORTED)
      set_target_properties(Tetgen::Tetgen PROPERTIES
        IMPORTED_LOCATION               "${TETGEN_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES   "${_TETGEN_INC}"
        # Define TETLIBRARY for all consumers (TetGen's header expects this when used as a lib)
        INTERFACE_COMPILE_DEFINITIONS   "TETLIBRARY"
      )
      message(STATUS "[SCIRun] Created imported target Tetgen::Tetgen (LIB='${TETGEN_LIBRARY}', INC='${_TETGEN_INC}')")
    else()
      message(STATUS "[SCIRun] Tetgen package not found and cache hints insufficient yet. "
                     "TETGEN_LIBRARY='${TETGEN_LIBRARY}', TETGEN_INCLUDE_DIR='${TETGEN_INCLUDE_DIR}'")
    endif()
  endif()
endif()

# GLEW (Windows)
if(WIN32)
  CONFIG_STANDARD_EXTERNAL(GLEW
    DIR  "${GLEW_DIR}"
    CFGS "GLEWConfig.cmake;GlewConfig.cmake"
  )
endif()

# Qwt (unless headless) - prefer config packages and fail (outside superbuild) if missing
if(NOT BUILD_HEADLESS)
  CONFIG_STANDARD_EXTERNAL(Qwt
    DIR         "${QWT_DIR}"
    CFG         "QwtConfig.cmake"
    CONFIG_ONLY         # prefer config packages
    REQUIRED            # outside superbuild, error out if missing
  )
endif()

# One-shot summary (nice in CI logs)
if(DEFINED _CONFIG_STANDARD_SUMMARY)
  message(STATUS "[CONFIG_STANDARD_EXTERNAL] Summary:\n${_CONFIG_STANDARD_SUMMARY}")
  unset(_CONFIG_STANDARD_SUMMARY)
endif()

########################################################################
# Custom compiler flags and definitions

# TODO: Better compiler check for OS X?
# Try CMAKE_<LANG>_COMPILER_ID MATCHES Clang? Or check for Intel, GNU?
IF(UNIX)
  IF(NOT APPLE)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed -ldl -lrt")
  ENDIF()
ENDIF()

########################################################################
# Find and configure Git

FIND_PACKAGE(Git)
IF(NOT GIT_FOUND)
  MESSAGE(FATAL_ERROR "Git not found.")
ENDIF()

OPTION(BUILD_BUNDLE "Build Mac bundle" FALSE)
MARK_AS_ADVANCED(BUILD_BUNDLE)

########################################################################
# Find and configure Awk & Sed  {(OS X only), needed to build App bundle

# should be installed by default, searching system paths only
IF(APPLE)
  FIND_PROGRAM(MACDEPLOYQT
    HINTS ${Qt_PATH} ${Qt_PATH}/bin
    NAMES macdeployqt
    NO_DEFAULT_PATH
  )
  MARK_AS_ADVANCED(MACDEPLOYQT)

  SET(MACDEPLOYQT_OUTPUT_LEVEL 0 CACHE STRING "Set macdeployqt output level (0-3)")
  MARK_AS_ADVANCED(MACDEPLOYQT_OUTPUT_LEVEL)

  FIND_PROGRAM(SED
    NAMES sed
  )
  MARK_AS_ADVANCED(SED)

  FIND_PROGRAM(AWK
    NAMES awk
  )
  MARK_AS_ADVANCED(AWK)
ENDIF()

# 1) Real interface target
add_library(SCIRunCompileConfig INTERFACE)

# 2) Namespaced alias (optional, for nice linking syntax)
add_library(SCIRun::CompileConfig ALIAS SCIRunCompileConfig)

# 3) Put your global compile definitions here
#    (Use your authoritative Qt selector: SCIRUN_QT_MAJOR is already in your top-level)
if (SCIRUN_QT_MAJOR STREQUAL "6")
  target_compile_definitions(SCIRunCompileConfig INTERFACE SCIRUN_QT6_ENABLED)
endif()

# You also set QT_NO_KEYWORDS globally in a few places—centralize it here:
target_compile_definitions(SCIRunCompileConfig INTERFACE QT_NO_KEYWORDS)

########################################################################
# Configure documentation

OPTION(BUILD_DOXYGEN_DOCUMENTATION "Generate Doxygen-based documentation." OFF)
MARK_AS_ADVANCED(BUILD_DOXYGEN_DOCUMENTATION)

IF(BUILD_DOXYGEN_DOCUMENTATION)
  FIND_PACKAGE(Doxygen)

  IF(DOXYGEN_FOUND)
    OPTION(COPY_DOXYGEN_TO_SERVER "Copy Doxygen documentation to server." OFF)
    MARK_AS_ADVANCED(COPY_DOXYGEN_TO_SERVER)
    ADD_SUBDIRECTORY(Documentation/Doxygen)
  ELSE()
    SET(BUILD_DOXYGEN_DOCUMENTATION OFF CACHE BOOL "Generate doxygen-based documentation." FORCE)
  ENDIF()
ENDIF()

########################################################################
# Configure ospray module build

IF(WITH_OSPRAY)
  ADD_DEFINITIONS(-DWITH_OSPRAY)
  SET(OSPRAY_LIBRARIES ospray)
ENDIF()

########################################################################
# Detailed renderer logging

OPTION(RENDERER_TRACE_ON "Enable detailed renderer logging." OFF)
MARK_AS_ADVANCED(RENDERER_TRACE_ON)
IF(RENDERER_TRACE_ON)
  ADD_DEFINITIONS(-DRENDERER_TRACE_ON)
ENDIF()

########################################################################
# Matlab i/o option--see issue #1727 and #1692

OPTION(MATLAB_STRING_OUTPUT_SIGNED_TYPE_FIX_FOR_SCIPY "Toggle quasi-fix for Issue #1692." OFF)
MARK_AS_ADVANCED(MATLAB_STRING_OUTPUT_SIGNED_TYPE_FIX_FOR_SCIPY)

########################################################################
# Copy Spire-SCIRun specific assets and shaders

set(OUTPUT_SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/Shaders)
set(OUTPUT_FONT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Fonts)
set(OUTPUT_ASSET_DIR ${CMAKE_CURRENT_BINARY_DIR}/Assets)

function (CopyShaders src_shader_dir dest_shader_dir)
  if (NOT dest_shader_dir STREQUAL "")
    file(COPY ${src_shader_dir}/ DESTINATION ${dest_shader_dir}
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE)
  endif()
endfunction()

function (CopyFonts src_font_dir dest_font_dir)
  if (NOT dest_font_dir STREQUAL "")
    file(COPY ${src_font_dir}/ DESTINATION ${dest_font_dir}
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE)
  endif()
endfunction()

function (CopyAssets src_asset_dir dest_asset_dir)
  if (NOT dest_asset_dir STREQUAL "")
    file(COPY ${src_asset_dir}/ DESTINATION ${dest_asset_dir}
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
      DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE)
  endif()
endfunction()

if (DEFINED OUTPUT_SHADER_DIR)
  CopyShaders(${CMAKE_CURRENT_SOURCE_DIR}/Interface/Modules/Render/ES/shaders ${OUTPUT_SHADER_DIR})
else()
  message(WARNING "Not copying shader files. OUTPUT_SHADER_DIR variable not found.")
endif()

if (DEFINED OUTPUT_FONT_DIR)
  CopyFonts(${CMAKE_CURRENT_SOURCE_DIR}/Interface/Modules/Render/ES/fonts ${OUTPUT_FONT_DIR})
else()
  message(WARNING "Not copying font files. OUTPUT_FONT_DIR variable not found.")
endif()

if (DEFINED OUTPUT_ASSET_DIR)
  CopyAssets(${CMAKE_CURRENT_SOURCE_DIR}/Interface/Modules/Render/ES/assets ${OUTPUT_ASSET_DIR})
else()
  message(WARNING "Not copying asset files. OUTPUT_ASSET_DIR variable not found.")
endif()

########################################################################
# Find and configure OpenGL

FIND_PACKAGE(OpenGL REQUIRED)

########################################################################
# Find and configure Qt

IF(NOT BUILD_HEADLESS)

  if (SCIRUN_QT_MAJOR STREQUAL "6")
    find_package(Qt6 REQUIRED COMPONENTS
      DBus Core Gui Widgets Network OpenGL Concurrent
      PrintSupport Svg OpenGLWidgets
      HINTS ${Qt_PATH})
  elseif (SCIRUN_QT_MAJOR STREQUAL "5")
    find_package(Qt5 REQUIRED COMPONENTS
      Core Gui Widgets Network OpenGL Concurrent
      PrintSupport Svg
      HINTS ${Qt_PATH})
  else()
    message(FATAL_ERROR "Unsupported Qt major version: ${SCIRUN_QT_MAJOR}")
  endif()

  if (Qt6_FOUND OR Qt5_FOUND)
    message(STATUS "Found Qt version: ${Qt${QT_VERSION_MAJOR}_VERSION}")
  else()
    message(FATAL_ERROR "Qt is required for building the SCIRun GUI")
  endif()

  add_definitions(-DQT_NO_KEYWORDS)

  if (QT_VERSION_MAJOR EQUAL 5)
    macro(QT_WRAP_UI arg1 arg2)
      qt5_wrap_ui(${arg1} ${arg2})
    endmacro()

    macro(QT_WRAP_CPP arg1 arg2)
      qt5_wrap_cpp(${arg1} ${arg2})
    endmacro()

    macro(QT_ADD_RESOURCES arg1 arg2)
      qt5_add_resources(${arg1} ${arg2})
    endmacro()

    set(QT_GRAPHICS_LIBRARIES Qt5::OpenGL)
    set(QT_NETWORK_LIBRARIES Qt5::Network)
    set(QT_LIBRARIES Qt5::Widgets)
  else()
    set(QT_GRAPHICS_LIBRARIES Qt::OpenGL Qt::OpenGLWidgets)
    set(QT_NETWORK_LIBRARIES Qt::Network)
    set(QT_LIBRARIES Qt::Widgets)
  endif()

ENDIF()

if (QT_VERSION_MAJOR EQUAL 6)
  add_compile_definitions(SCIRUN_QT6_ENABLED)
endif()

if (QT_VERSION_MAJOR EQUAL 5 AND Qt5Widgets_VERSION VERSION_LESS 5.13.0)
  add_definitions(-DOLDER_QT_SUPPORT_NEEDED)
endif()

########################################################################
# Find and configure dl library

IF(UNIX)
  FIND_LIBRARY(DL_LIBRARY dl)
ENDIF()

########################################################################
# Includes

SET(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

# ---------- Diagnostics + robust include discovery ----------

# Normalize superbuild-provided names to what SCIRun uses internally
if(DEFINED LodePng_INCLUDE_DIR AND NOT DEFINED LODEPNG_INCLUDE_DIR)
  set(LODEPNG_INCLUDE_DIR "${LodePng_INCLUDE_DIR}")
endif()
if(DEFINED Tny_INCLUDE_DIR AND NOT DEFINED TNY_INCLUDE_DIR)
  set(TNY_INCLUDE_DIR "${Tny_INCLUDE_DIR}")
endif()
if(DEFINED Teem_INCLUDE_DIR AND NOT DEFINED SCI_TEEM_INCLUDE)
  set(SCI_TEEM_INCLUDE "${Teem_INCLUDE_DIR}")
endif()
if(DEFINED TEEM_INCLUDE AND NOT DEFINED SCI_TEEM_INCLUDE)
  set(SCI_TEEM_INCLUDE "${TEEM_INCLUDE}")
endif()
if(DEFINED SCIRUN_EIGEN_INCLUDE AND NOT DEFINED Eigen_INCLUDE_DIR)
  set(Eigen_INCLUDE_DIR "${SCIRUN_EIGEN_INCLUDE}")
endif()

# Helper: validate a cached include dir (VAR) by checking it contains REL_HEADER.
# If empty or incorrect, auto-derive from CMAKE_PREFIX_PATH and cache the fix.
function(_ensure_header _var_name _rel_header)
  set(_val "${${_var_name}}")
  set(_valid FALSE)
  if(_val AND EXISTS "${_val}/${_rel_header}")
    set(_valid TRUE)
  endif()

  if(NOT _valid)
    # Try to derive from CMAKE_PREFIX_PATH by probing <prefix>/include
    set(_candidates)
    if(CMAKE_PREFIX_PATH)
      foreach(_p IN LISTS CMAKE_PREFIX_PATH)
        list(APPEND _candidates "${_p}/include")
      endforeach()
    endif()
    set(_found "")
    foreach(_inc IN LISTS _candidates)
      if(EXISTS "${_inc}/${_rel_header}")
        set(_found "${_inc}")
        break()
      endif()
    endforeach()

    if(_found)
      set(${_var_name} "${_found}" CACHE PATH "${_var_name}" FORCE)
      message(STATUS "[SCIRun] Fixed ${_var_name} -> '${_found}' (found '${_rel_header}')")
    else()
      if(_val)
        message(WARNING "[SCIRun] ${_var_name} ('${_val}') does not contain '${_rel_header}'. Clearing.")
      endif()
      unset(${_var_name} CACHE)
      unset(${_var_name})
    endif()
  else()
    set(${_var_name} "${_val}" CACHE PATH "${_var_name}" FORCE)
  endif()
endfunction()

# Validate / auto-fix externals we depend on:
#   GLEW:     GL/glew.h
#   LodePNG:  lodepng/lodepng.h
#   SQLite3:  sqlite3.h
#   Tny:      tny/tny.hpp
#   Spdlog:   spdlog/spdlog.h
#   GLM:      glm/glm.hpp
# If the package created the imported target, prefer it and sync the include var
# --- GLEW include discovery (target-first, robust fallback) ---
set(_SCIRUN_USE_GLEW_TARGET FALSE)
set(_glew_inc_from_target "")
find_package(GLEW REQUIRED)
# 1) If the package created GLEW::GLEW, prefer its include path
if(TARGET GLEW::GLEW)
  get_target_property(_glew_inc_from_target GLEW::GLEW INTERFACE_INCLUDE_DIRECTORIES)
  if(_glew_inc_from_target)
    list(GET _glew_inc_from_target 0 _glew_inc_from_target)  # first entry if it's a list
  endif()
endif()

# 2) If the target had a valid include and the header exists there, use it
if(_glew_inc_from_target AND EXISTS "${_glew_inc_from_target}/GL/glew.h")
  set(GLEW_INCLUDE_DIR "${_glew_inc_from_target}" CACHE PATH "GLEW include dir (from imported target)" FORCE)
  set(_SCIRUN_USE_GLEW_TARGET TRUE)
else()
  # 3) Fallback: try to derive from CMAKE_PREFIX_PATH
  _ensure_header(GLEW_INCLUDE_DIR "GL/glew.h")

  # 4) Last-resort: force the well-known superbuild path
  if(NOT GLEW_INCLUDE_DIR)
    set(_sb_glew_inc "${CMAKE_BINARY_DIR}/Externals/Install/Glew_external/include")
    if(EXISTS "${_sb_glew_inc}/GL/glew.h")
      set(GLEW_INCLUDE_DIR "${_sb_glew_inc}" CACHE PATH "GLEW include dir (superbuild)" FORCE)
    endif()
  endif()
endif()

message(STATUS "GLEW_INCLUDE_DIR='${GLEW_INCLUDE_DIR}'")
if(TARGET GLEW::GLEW)
  message(STATUS "GLEW::GLEW INTERFACE_INCLUDE_DIRECTORIES='${_glew_inc_from_target}'")
endif()

# --- Freetype imported target (robust on Windows) ---
# 1) Try a config package (superbuild may have installed one)
find_package(Freetype QUIET CONFIG)

if(NOT TARGET Freetype::Freetype)
  # 2) Synthesize if config not available
  if(DEFINED Freetype_LIB_DIR AND IS_DIRECTORY "${Freetype_LIB_DIR}")
    set(_ft_rel "")
    set(_ft_dbg "")

    file(GLOB _cand_rel
      "${Freetype_LIB_DIR}/freetype.lib"     "${Freetype_LIB_DIR}/libfreetype.lib")
    file(GLOB _cand_dbg
      "${Freetype_LIB_DIR}/freetyped.lib"    "${Freetype_LIB_DIR}/libfreetyped.lib")

    if(_cand_rel)
      list(SORT _cand_rel)
      list(GET _cand_rel 0 _ft_rel)
    endif()
    if(_cand_dbg)
      list(SORT _cand_dbg)
      list(GET _cand_dbg 0 _ft_dbg)
    endif()

    if(NOT _ft_rel AND _ft_dbg)
      set(_ft_rel "${_ft_dbg}")
    endif()
    if(NOT _ft_dbg AND _ft_rel)
      set(_ft_dbg "${_ft_rel}")
    endif()

    if(_ft_rel OR _ft_dbg)
      add_library(Freetype::Freetype UNKNOWN IMPORTED)
      # FreeType expects both roots: ft2build.h and freetype2 headers
      set_property(TARGET Freetype::Freetype PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES
          "${Freetype_INCLUDE_DIR};${FREETYPE_INCLUDE_DIR2}"
      )
      if(_ft_rel)
        set_property(TARGET Freetype::Freetype PROPERTY IMPORTED_LOCATION_RELEASE "${_ft_rel}")
      endif()
      if(_ft_dbg)
        set_property(TARGET Freetype::Freetype PROPERTY IMPORTED_LOCATION_DEBUG   "${_ft_dbg}")
      endif()

      message(STATUS "[SCIRun] Freetype::Freetype: rel='${_ft_rel}' dbg='${_ft_dbg}' "
                     "inc1='${Freetype_INCLUDE_DIR}' inc2='${FREETYPE_INCLUDE_DIR2}'")
    else()
      message(WARNING "[SCIRun] Could not find freetype{,d}.lib in '${Freetype_LIB_DIR}'")
    endif()
  else()
    message(WARNING "[SCIRun] Freetype_LIB_DIR not set or invalid; cannot synthesize Freetype::Freetype")
  endif()
endif()

_ensure_header(LODEPNG_INCLUDE_DIR   "lodepng/lodepng.h")
_ensure_header(SQLite3_INCLUDE_DIR   "sqlite3.h")
_ensure_header(TNY_INCLUDE_DIR "tny/tny.h")
if(NOT TNY_INCLUDE_DIR)
  _ensure_header(TNY_INCLUDE_DIR "tny/tny.hpp")
endif()
_ensure_header(SpdLog_INCLUDE_DIR    "spdlog/spdlog.h")
_ensure_header(GLM_INCLUDE_DIR       "glm/glm.hpp")
_ensure_header(SCI_TEEM_INCLUDE "teem/ten.h")
_ensure_header(Freetype_INCLUDE_DIR  "ft2build.h")

# --- Robust Eigen include discovery (handles eigen3 layout and unsupported) ---
# Try to honor a pre-set Eigen_INCLUDE_DIR first
set(_eigen_ok FALSE)
if(Eigen_INCLUDE_DIR)
  if(EXISTS "${Eigen_INCLUDE_DIR}/Eigen/Dense" AND EXISTS "${Eigen_INCLUDE_DIR}/unsupported/Eigen/CXX11/Tensor")
    set(_eigen_ok TRUE)
  endif()
endif()

# If not OK, auto-probe from CMAKE_PREFIX_PATH (prefer eigen3 layout)
if(NOT _eigen_ok)
  set(_candidates "")
  foreach(_p IN LISTS CMAKE_PREFIX_PATH)
    list(APPEND _candidates
      "${_p}/include/eigen3"
      "${_p}/include"           # legacy fallback
    )
  endforeach()

  set(_found "")
  foreach(_inc IN LISTS _candidates)
    if(EXISTS "${_inc}/Eigen/Dense" AND EXISTS "${_inc}/unsupported/Eigen/CXX11/Tensor")
      set(_found "${_inc}")
      break()
    endif()
  endforeach()

  if(_found)
    set(Eigen_INCLUDE_DIR "${_found}" CACHE PATH "Eigen include root (expects Eigen/ and unsupported/)" FORCE)
    message(STATUS "[SCIRun] Eigen_INCLUDE_DIR -> '${Eigen_INCLUDE_DIR}' (contains Eigen and unsupported)")
    set(_eigen_ok TRUE)
  endif()
endif()

# If we still don’t have unsupported, warn loudly (this will cause compile errors later)
if(NOT _eigen_ok)
  # Try to at least find Eigen/Dense to help with diagnostics
  set(_fallback "")
  foreach(_p IN LISTS CMAKE_PREFIX_PATH)
    foreach(_inc IN ITEMS "${_p}/include/eigen3" "${_p}/include")
      if(EXISTS "${_inc}/Eigen/Dense")
        set(_fallback "${_inc}")
        break()
      endif()
    endforeach()
    if(_fallback)
      break()
    endif()
  endforeach()

  if(_fallback)
    set(Eigen_INCLUDE_DIR "${_fallback}" CACHE PATH "Eigen include root (no unsupported found)" FORCE)
    message(WARNING
      "[SCIRun] Found Eigen at '${Eigen_INCLUDE_DIR}', but 'unsupported/Eigen/CXX11/Tensor' is missing.\n"
      "         Ensure the Eigen external installs the 'unsupported' directory (Eigen 3.4 install puts both under include/eigen3).")
  else()
    # Clear incorrect cache if present
    if(Eigen_INCLUDE_DIR)
      message(WARNING "[SCIRun] Eigen_INCLUDE_DIR='${Eigen_INCLUDE_DIR}' does not contain Eigen/Dense. Clearing.")
      unset(Eigen_INCLUDE_DIR CACHE)
      unset(Eigen_INCLUDE_DIR)
    endif()
  endif()
endif()

# ---- Cleaver2 normalization + validation ----
# Superbuild exports one of:
#   Cleaver2_INCLUDE_DIR / Cleaver2_LIB_DIR
#   CLEAVER2_INCLUDE / CLEAVER2_INCLUDE_DIR / CLEAVER2_LIBRARY_DIR
# Normalize to SCIRun’s internal names for consistency.
if(DEFINED Cleaver2_INCLUDE_DIR AND NOT DEFINED CLEAVER2_INCLUDE)
  set(CLEAVER2_INCLUDE "${Cleaver2_INCLUDE_DIR}")
endif()
if(DEFINED Cleaver2_LIB_DIR AND NOT DEFINED CLEAVER2_LIBRARY_DIR)
  set(CLEAVER2_LIBRARY_DIR "${Cleaver2_LIB_DIR}")
endif()
_ensure_header(CLEAVER2_INCLUDE "cleaver2/vec3.h")
# Create easy-to-use internal aliases (like you do for other externals)
if(DEFINED CLEAVER2_INCLUDE)
  set(SCI_CLEAVER2_INCLUDE "${CLEAVER2_INCLUDE}" CACHE PATH "Cleaver2 include root (contains 'cleaver2/')" FORCE)
endif()
if(DEFINED CLEAVER2_LIBRARY_DIR)
  set(SCI_CLEAVER2_LIBRARY_DIR "${CLEAVER2_LIBRARY_DIR}" CACHE PATH "Cleaver2 library directory" FORCE)
endif()

# Print diagnostics
message(STATUS "CLEAVER2_INCLUDE='${CLEAVER2_INCLUDE}'")
message(STATUS "CLEAVER2_LIBRARY_DIR='${CLEAVER2_LIBRARY_DIR}'")
message(STATUS "Boost_INCLUDE_DIR='${Boost_INCLUDE_DIR}'")
message(STATUS "GLEW_INCLUDE_DIR='${GLEW_INCLUDE_DIR}'")
message(STATUS "LODEPNG_INCLUDE_DIR='${LODEPNG_INCLUDE_DIR}'")
message(STATUS "SQLite3_INCLUDE_DIR='${SQLite3_INCLUDE_DIR}'")
message(STATUS "TNY_INCLUDE_DIR='${TNY_INCLUDE_DIR}'")
message(STATUS "SpdLog_INCLUDE_DIR='${SpdLog_INCLUDE_DIR}'")
message(STATUS "GLM_INCLUDE_DIR='${GLM_INCLUDE_DIR}'")
message(STATUS "Eigen_INCLUDE_DIR='${Eigen_INCLUDE_DIR}'")

# Quick existence confirmations
if(GLEW_INCLUDE_DIR AND EXISTS "${GLEW_INCLUDE_DIR}/GL/glew.h")
  message(STATUS "GLEW header OK: ${GLEW_INCLUDE_DIR}/GL/glew.h")
endif()
if(LODEPNG_INCLUDE_DIR AND EXISTS "${LODEPNG_INCLUDE_DIR}/lodepng/lodepng.h")
  message(STATUS "LodePNG header OK: ${LODEPNG_INCLUDE_DIR}/lodepng/lodepng.h")
endif()
if(SQLite3_INCLUDE_DIR AND EXISTS "${SQLite3_INCLUDE_DIR}/sqlite3.h")
  message(STATUS "SQLite3 header OK: ${SQLite3_INCLUDE_DIR}/sqlite3.h")
endif()
if(TNY_INCLUDE_DIR AND EXISTS "${TNY_INCLUDE_DIR}/tny/tny.hpp")
  message(STATUS "TNY header OK: ${TNY_INCLUDE_DIR}/tny/tny.hpp")
endif()
if(SpdLog_INCLUDE_DIR AND EXISTS "${SpdLog_INCLUDE_DIR}/spdlog/spdlog.h")
  message(STATUS "Spdlog header OK: ${SpdLog_INCLUDE_DIR}/spdlog/spdlog.h")
endif()
if(GLM_INCLUDE_DIR AND EXISTS "${GLM_INCLUDE_DIR}/glm/glm.hpp")
  message(STATUS "GLM header OK: ${GLM_INCLUDE_DIR}/glm/glm.hpp")
endif()

# ---- Qwt (STATIC) detection + imported target (place before _SCIRUN_INCLUDE_DIRS) ----
if(NOT BUILD_HEADLESS)
  # 1) Include dir: prefer install/include/qwt so '#include <qwt_plot.h>' works directly
  if(NOT DEFINED SCI_QWT_INCLUDE OR NOT EXISTS "${SCI_QWT_INCLUDE}/qwt_plot.h")
    if(DEFINED QWT_INSTALL_DIR AND EXISTS "${QWT_INSTALL_DIR}/include/qwt/qwt_plot.h")
      set(SCI_QWT_INCLUDE "${QWT_INSTALL_DIR}/include/qwt" CACHE PATH "Qwt include dir" FORCE)
    elseif(DEFINED QWT_INCLUDE_DIR)
      if(EXISTS "${QWT_INCLUDE_DIR}/qwt/qwt_plot.h")
        set(SCI_QWT_INCLUDE "${QWT_INCLUDE_DIR}/qwt" CACHE PATH "Qwt include dir" FORCE)
      elseif(EXISTS "${QWT_INCLUDE_DIR}/qwt_plot.h")
        set(SCI_QWT_INCLUDE "${QWT_INCLUDE_DIR}" CACHE PATH "Qwt include dir" FORCE)
      endif()
    else()
      # Probe from prefix path
      foreach(_p IN LISTS CMAKE_PREFIX_PATH)
        if(EXISTS "${_p}/include/qwt/qwt_plot.h")
          set(SCI_QWT_INCLUDE "${_p}/include/qwt" CACHE PATH "Qwt include dir" FORCE)
          break()
        endif()
      endforeach()
    endif()
  endif()

  # 2) Library dir (from superbuild) + locate static libs
  if(NOT DEFINED QWT_LIBRARY_DIR AND DEFINED QWT_INSTALL_DIR)
    if(EXISTS "${QWT_INSTALL_DIR}/lib64")
      set(QWT_LIBRARY_DIR "${QWT_INSTALL_DIR}/lib64")
    else()
      set(QWT_LIBRARY_DIR "${QWT_INSTALL_DIR}/lib")
    endif()
  endif()

  set(_qwt_rel_lib "")
  set(_qwt_dbg_lib "")
  if(DEFINED QWT_LIBRARY_DIR AND IS_DIRECTORY "${QWT_LIBRARY_DIR}")
    # Typical MSVC names for Qwt static (or single-config import lib when DLLs are absent)
    file(GLOB _rel_cands "${QWT_LIBRARY_DIR}/qwt.lib"  "${QWT_LIBRARY_DIR}/libqwt.lib")
    file(GLOB _dbg_cands "${QWT_LIBRARY_DIR}/qwtd.lib" "${QWT_LIBRARY_DIR}/libqwtd.lib")

    if(_rel_cands)
      list(SORT _rel_cands)
      list(GET _rel_cands 0 _qwt_rel_lib)  # safe: list is non-empty
    endif()
    
    if(_dbg_cands)
      list(SORT _dbg_cands)
      list(GET _dbg_cands 0 _qwt_dbg_lib)  # safe: list is non-empty
    endif()

    # If only one variant exists, use it for both configs (common in static-only installs)
    if(NOT _qwt_rel_lib AND _qwt_dbg_lib)
      set(_qwt_rel_lib "${_qwt_dbg_lib}")
    endif()
    if(NOT _qwt_dbg_lib AND _qwt_rel_lib)
      set(_qwt_dbg_lib "${_qwt_rel_lib}")
    endif()
  endif()

  # 3) Imported STATIC target (preferred)
  if(NOT TARGET Qwt::Qwt
     AND ( _qwt_rel_lib OR _qwt_dbg_lib )
     AND DEFINED SCI_QWT_INCLUDE AND EXISTS "${SCI_QWT_INCLUDE}/qwt_plot.h")

    add_library(Qwt::Qwt STATIC IMPORTED)
    set_property(TARGET Qwt::Qwt PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${SCI_QWT_INCLUDE}")
    if(_qwt_rel_lib)
      set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_LOCATION_RELEASE "${_qwt_rel_lib}")
    endif()
    if(_qwt_dbg_lib)
      set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_LOCATION_DEBUG   "${_qwt_dbg_lib}")
    endif()

    # Consumers of static Qwt: do NOT define QWT_DLL; define QWT_STATIC if Qwt headers look for it.
    target_compile_definitions(Qwt::Qwt INTERFACE QWT_STATIC)

    message(STATUS "[SCIRun] Qwt::Qwt (static): rel='${_qwt_rel_lib}', dbg='${_qwt_dbg_lib}', inc='${SCI_QWT_INCLUDE}'")
  endif()

  # 4) Legacy variable for modules that still link ${SCI_QWT_LIBRARY}
  #    Provide an optimized/debug list mapped to the same static libraries.
  set(SCI_QWT_LIBRARY "" CACHE STRING "Qwt libraries (optimized/debug)" FORCE)
  if(_qwt_rel_lib OR _qwt_dbg_lib)
    set(_pair "")
    if(_qwt_rel_lib)
      list(APPEND _pair optimized "${_qwt_rel_lib}")
    endif()
    if(_qwt_dbg_lib)
      list(APPEND _pair debug     "${_qwt_dbg_lib}")
    endif()
    set(SCI_QWT_LIBRARY "${_pair}" CACHE STRING "Qwt libraries (optimized/debug)" FORCE)
    unset(_pair)
  endif()
endif()

# ---------- Build include list with guards to avoid stray '/include' ----------

set(_SCIRUN_INCLUDE_DIRS
  ${CMAKE_CURRENT_BINARY_DIR}
  ${SCIRun_SOURCE_DIR}/
  ${SCIRun_SOURCE_DIR}/include
  ${SCIRun_SOURCE_DIR}/Externals/libxml2/include
  ${SCIRun_SOURCE_DIR}/Externals/spire/
  ${SCIRun_SOURCE_DIR}/Externals/submodules/googletest/googletest/include
  ${SCIRun_SOURCE_DIR}/Externals/submodules/googletest/googlemock/include
  ${CMAKE_CURRENT_BINARY_DIR}/Externals/libxml2/include
  ${SCIRun_SOURCE_DIR}/Externals/
  ${SCIRun_BINARY_DIR}/
  ${SCIRun_BINARY_DIR}/include
  ${SCI_TEEM_INCLUDE}
  ${SCI_ZLIB_INCLUDE}
  ${SCI_QWT_INCLUDE}
  ${SCI_CLEAVER2_INCLUDE}
)

# If the second include dir wasn't set, derive it from the first
if(NOT FREETYPE_INCLUDE_DIR2 OR NOT EXISTS "${FREETYPE_INCLUDE_DIR2}/freetype/config/ftheader.h")
  if(Freetype_INCLUDE_DIR AND EXISTS "${Freetype_INCLUDE_DIR}/freetype2/freetype/config/ftheader.h")
    set(FREETYPE_INCLUDE_DIR2 "${Freetype_INCLUDE_DIR}/freetype2" CACHE PATH "FREETYPE_INCLUDE_DIR2" FORCE)
  endif()
endif()

# Append both include roots (only if they exist)
if(Freetype_INCLUDE_DIR AND EXISTS "${Freetype_INCLUDE_DIR}/ft2build.h")
  list(APPEND _SCIRUN_INCLUDE_DIRS "${Freetype_INCLUDE_DIR}")
endif()
if(FREETYPE_INCLUDE_DIR2 AND EXISTS "${FREETYPE_INCLUDE_DIR2}/freetype/config/ftheader.h")
  list(APPEND _SCIRUN_INCLUDE_DIRS "${FREETYPE_INCLUDE_DIR2}")
endif()

# Boost (legacy var)
if(Boost_INCLUDE_DIR AND EXISTS "${Boost_INCLUDE_DIR}/boost")
  list(APPEND _SCIRUN_INCLUDE_DIRS "${Boost_INCLUDE_DIR}")
endif()

# GLEW
if(GLEW_INCLUDE_DIR AND EXISTS "${GLEW_INCLUDE_DIR}/GL/glew.h")
  list(APPEND _SCIRUN_INCLUDE_DIRS "${GLEW_INCLUDE_DIR}")
endif()

# SQLite
if(SQLite3_INCLUDE_DIR AND EXISTS "${SQLite3_INCLUDE_DIR}/sqlite3.h")
  list(APPEND _SCIRUN_INCLUDE_DIRS "${SQLite3_INCLUDE_DIR}")
endif()

# Spdlog (IMPORTANT: do NOT append '/include'; var already points to include dir)
if(SpdLog_INCLUDE_DIR AND EXISTS "${SpdLog_INCLUDE_DIR}/spdlog/spdlog.h")
  list(APPEND _SCIRUN_INCLUDE_DIRS "${SpdLog_INCLUDE_DIR}")
endif()

# GLM
if(GLM_INCLUDE_DIR AND EXISTS "${GLM_INCLUDE_DIR}/glm/glm.hpp")
  list(APPEND _SCIRUN_INCLUDE_DIRS "${GLM_INCLUDE_DIR}")
endif()

# Tny
if(TNY_INCLUDE_DIR AND EXISTS "${TNY_INCLUDE_DIR}/tny/tny.hpp")
  list(APPEND _SCIRUN_INCLUDE_DIRS "${TNY_INCLUDE_DIR}")
endif()

# LodePNG
if(LODEPNG_INCLUDE_DIR AND EXISTS "${LODEPNG_INCLUDE_DIR}/lodepng/lodepng.h")
  list(APPEND _SCIRUN_INCLUDE_DIRS "${LODEPNG_INCLUDE_DIR}")
endif()

# Eigen: add only if the directory contains Eigen/Dense
if(Eigen_INCLUDE_DIR AND EXISTS "${Eigen_INCLUDE_DIR}/Eigen/Dense")
  list(APPEND _SCIRUN_INCLUDE_DIRS "${Eigen_INCLUDE_DIR}")
endif()

# De-duplicate & print for visibility
list(REMOVE_DUPLICATES _SCIRUN_INCLUDE_DIRS)

function(print_list name)
  message(STATUS "${name}:")
  foreach(item IN LISTS ${name})
    message(STATUS "  - ${item}")
  endforeach()
endfunction()
print_list("_SCIRUN_INCLUDE_DIRS")

include_directories(${_SCIRUN_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(SYSTEM ${SCI_BOOST_INCLUDE})

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

SET(QT_USE_QTOPENGL TRUE)

########################################################################
# Macros

MACRO(SCIRUN_ADD_UNIT_TEST test)
  ADD_EXECUTABLE(${test} ${ARGN})
  LIST(APPEND tmp ${test} ${UNIT_TEST_LIST})
  SET(UNIT_TEST_LIST ${tmp} CACHE INTERNAL "List of unit tests." FORCE)
  TARGET_COMPILE_DEFINITIONS(${test} PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY)
ENDMACRO()

MACRO(SCIRUN_ADD_TEST_DIR directory)
  IF(BUILD_TESTING)
    ADD_SUBDIRECTORY(${directory})
  ENDIF()
ENDMACRO()

MACRO(SCIRUN_ADD_LIBRARY library)
  ADD_LIBRARY(${library} ${ARGN})

  # Attach global compile definitions (Qt6, QT_NO_KEYWORDS, etc.)
  if (SCIRUN_QT_MAJOR STREQUAL "6")
    target_compile_definitions(${library} PUBLIC SCIRUN_QT6_ENABLED)
  endif()
  target_compile_definitions(${library} PUBLIC QT_NO_KEYWORDS)

  if(MSVC)
    # TODO:
    # target_compile_options(${library} PRIVATE /W4 /WX)
  else()
    # TODO:
    if(APPLE)
      target_compile_options(${library} PRIVATE -Wall  -Wunused-parameter -Winfinite-recursion)
          # TODO: put back -Werror
          #eventually: -Wall -Wextra -pedantic -Werror)
    else()
      target_compile_options(${library} PRIVATE -Wall -Wunused-parameter)
    endif()
  endif()

  # Set up installer package (Release build only)
  IF(WIN32)
    INSTALL(TARGETS ${library} RUNTIME DESTINATION bin CONFIGURATIONS Release)
  ENDIF()
ENDMACRO()

########################################################################
# Output directories.

SET(SCIRun_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SCIRun_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SCIRun_BINARY_DIR} CACHE INTERNAL "Single output directory for building all executables.")
MARK_AS_ADVANCED(CMAKE_LIBRARY_OUTPUT_DIRECTORY CMAKE_RUNTIME_OUTPUT_DIRECTORY)

SET(BUILD_FROM_TOP_LEVEL 1)

ADD_SUBDIRECTORY(Externals)
IF(NOT BUILD_HEADLESS)
  ADD_SUBDIRECTORY(Interface)
ELSE()
  ADD_DEFINITIONS(-DBUILD_HEADLESS=1)
ENDIF()
ADD_SUBDIRECTORY(Core)
ADD_SUBDIRECTORY(Dataflow)
ADD_SUBDIRECTORY(Modules)
ADD_SUBDIRECTORY(Graphics)

ADD_SUBDIRECTORY(Main)

ADD_SUBDIRECTORY(Testing)

########################################################################
# sci_defs variables
# TODO: temporarily included to support legacy code

# sci_defs/error_defs.h
SET(THROW_LEVEL 0 CACHE STRING "Set throw level N(0-3)")
SET(ASSERTION_LEVEL 1 CACHE STRING "Set assertion level N(0-3)")
IF(CMAKE_BUILD_TYPE MATCHES "Debug")
  SET(ASSERTION_LEVEL 3)
ENDIF(CMAKE_BUILD_TYPE MATCHES "Debug")

# sci_defs/environment_defs.h
SET(scirun_src_dir "${SCIRun_SOURCE_DIR}" CACHE INTERNAL "Path to SCIRun's src directory")
# DV Technically, I don't have access to this variable once we fix building
# these projets separately!!!
SET(scirun_obj_dir ${SCIRun_BINARY_DIR} CACHE INTERNAL "Path to SCIRun's binary directory")

MARK_AS_ADVANCED(THROW_LEVEL ASSERTION_LEVEL DEF_HASH)
# DV Add with_exc stuff from configure.ac
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
CHECK_INCLUDE_FILE(limits.h HAVE_LIMITS)
IF(HAVE_LIMITS)
  SET(HAVE_LIMITS_H 1)
ELSE(HAVE_LIMITS)
  SET(HAVE_LIMITS_H 0)
ENDIF(HAVE_LIMITS)

CHECK_INCLUDE_FILE(sys/select.h HAVE_SYS_SELECT)
IF(HAVE_SYS_SELECT)
  SET(HAVE_SYS_SELECT_H 1)
ELSE(HAVE_SYS_SELECT)
  SET(HAVE_SYS_SELECT_H 0)
ENDIF(HAVE_SYS_SELECT)

CHECK_INCLUDE_FILE(sys/time.h HAVE_SYS_TIME)
IF(HAVE_SYS_TIME)
  SET(HAVE_SYS_TIME_H 1)
ELSE(HAVE_SYS_TIME)
  SET(HAVE_SYS_TIME_H 0)
ENDIF(HAVE_SYS_TIME)

# BW Fix to figure out if we have ext/algorithm
#TRY_COMPILE(HAVE_EXT_ALGORITHM
#  ${CMAKE_CURRENT_BINARY_DIR}/CMakeTmp
#  ${SCIRUN_CORE_SOURCE_DIR}/CMake/SCIRunTestExtAlgorithm.cc
#  OUTPUT_VARIABLE OUTPUT)
IF(HAVE_EXT_ALGORITHM)
  SET(HAVE_EXT_ALGORITHM_H 1)
ELSE(HAVE_EXT_ALGORITHM)
  SET(HAVE_EXT_ALGORITHM_H 0)
ENDIF(HAVE_EXT_ALGORITHM)

CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT)
IF(HAVE_STDINT)
  SET(HAVE_STDINT_H 1)
ELSE(HAVE_STDINT)
  SET(HAVE_STDINT_H 0)
ENDIF(HAVE_STDINT)

CHECK_INCLUDE_FILE(inttypes.h HAVE_INTTYPES)
IF(HAVE_INTTYPES)
  SET(HAVE_INTTYPES_H 1)
ELSE(HAVE_INTTYPES)
  SET(HAVE_INTTYPES_H 0)
ENDIF(HAVE_INTTYPES)

SET(HAVE_SYS_UUID_H 0)
SET(HAVE_UUID_UUID_H 0)

ADD_SUBDIRECTORY(include)

########################################################################
# Organize projects into folders for Visual Studio projects

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET_PROPERTY(TARGET Algorithms_Base         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_DataIO       PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Math         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Field         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Describe         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_BrainStimulator         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Legacy_Inverse         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Factory   PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_Fields         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_Forward         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Visualization         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_FiniteElements         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_Geometry         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_DataIO         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_Converter         PROPERTY FOLDER "Core/Algorithms")

SET_PROPERTY(TARGET Dataflow_Network         PROPERTY FOLDER "Dataflow")
SET_PROPERTY(TARGET Core_Datatypes         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Mesh         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Base         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Field         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Nrrd         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Bundle         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Basis         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Persistent         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_DatabaseManager         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Logging         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_ConsoleApplication         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_CommandLine         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Command         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Thread         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_ImportExport         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_IEPlugin         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Geometry_Primitives         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Application         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Application_Preferences         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Application_Session         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Utils         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Math         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Matlab         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Parser         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_XMLUtil         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Exceptions_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Util_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Containers         PROPERTY FOLDER "Core")
#SET_PROPERTY(TARGET Core_Thread_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Serialization_Network         PROPERTY FOLDER "Dataflow/Serialization")
SET_PROPERTY(TARGET Core_Serialization_Network_Importer         PROPERTY FOLDER "Dataflow/Serialization")
SET_PROPERTY(TARGET bundle_toolkit         PROPERTY FOLDER "Dataflow/Serialization")
SET_PROPERTY(TARGET Engine_Network         PROPERTY FOLDER "Dataflow/Engine")
SET_PROPERTY(TARGET Engine_Scheduler         PROPERTY FOLDER "Dataflow/Engine")
SET_PROPERTY(TARGET Dataflow_State         PROPERTY FOLDER "Dataflow")
SET_PROPERTY(TARGET ${SPIRE_LIBRARY_NAME} PROPERTY FOLDER "Graphics")

IF(NOT BUILD_HEADLESS)
  SET_PROPERTY(TARGET Interface_Modules_Base   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_DataIO   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Math   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Visualization   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Render   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_FiniteElements   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_BrainStimulator   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_String   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Fields   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Bundle   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Matlab   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Forward   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Inverse   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Teem   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Modules_Factory   PROPERTY FOLDER "Interface/Modules")
  SET_PROPERTY(TARGET Interface_Application   PROPERTY FOLDER "Interface/Application")
ENDIF()

SET_PROPERTY(TARGET Modules_Basic   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Math   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Legacy_Math   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Fields   PROPERTY FOLDER "Modules/Fields")
SET_PROPERTY(TARGET Modules_DataIO   PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_Render   PROPERTY FOLDER "Modules/Render")
SET_PROPERTY(TARGET Modules_String   PROPERTY FOLDER "Modules/String")
SET_PROPERTY(TARGET Modules_Legacy_Bundle   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Visualization   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Modules_BrainStimulator   PROPERTY FOLDER "Modules/Brain")
SET_PROPERTY(TARGET Modules_Legacy_FiniteElements   PROPERTY FOLDER "Modules/FiniteElements")
SET_PROPERTY(TARGET Modules_Legacy_Fields PROPERTY FOLDER "Modules/Fields")
SET_PROPERTY(TARGET Modules_Legacy_Converters PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Legacy_Forward PROPERTY FOLDER "Modules/Forward")
SET_PROPERTY(TARGET Modules_Legacy_String PROPERTY FOLDER "Modules/String")
SET_PROPERTY(TARGET Modules_Legacy_Inverse PROPERTY FOLDER "Modules/Inverse")
SET_PROPERTY(TARGET Modules_Legacy_Matlab_DataIO PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_Legacy_Visualization   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Modules_Legacy_Teem_DataIO   PROPERTY FOLDER "Modules/Teem")
SET_PROPERTY(TARGET Modules_Legacy_Teem_Misc   PROPERTY FOLDER "Modules/Teem")
SET_PROPERTY(TARGET Modules_Legacy_Teem_Tend   PROPERTY FOLDER "Modules/Teem")
SET_PROPERTY(TARGET Modules_Legacy_Teem_Converters   PROPERTY FOLDER "Modules/Teem")
SET_PROPERTY(TARGET Modules_Factory   PROPERTY FOLDER "Modules")

SET_PROPERTY(TARGET Graphics_Glyphs   PROPERTY FOLDER "Graphics")
SET_PROPERTY(TARGET Graphics_Widgets   PROPERTY FOLDER "Graphics")
SET_PROPERTY(TARGET Graphics_Datatypes   PROPERTY FOLDER "Graphics")
SET_PROPERTY(TARGET spire   PROPERTY FOLDER "Graphics")

SET_PROPERTY(TARGET pugixml   PROPERTY FOLDER "Externals")

IF(BUILD_TESTING)
  SET_PROPERTY(TARGET gmock         PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET gmock_main    PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET gtest         PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET gtest_main    PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET Testing_Utils PROPERTY FOLDER "Testing Support")
  SET_PROPERTY(TARGET Testing_ModuleTestBase PROPERTY FOLDER "Testing Support")
ENDIF()

IF(BUILD_TESTING)

  #SET_PROPERTY(TARGET SCIRunTestData_external   PROPERTY FOLDER "Testing Support")

  SET_PROPERTY(TARGET Algorithms_Base_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_DataIO_Tests PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_BrainStimulator_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_Math_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_Field_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_Describe_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithms_FiniteElements_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Algorithm_Layer_Test   PROPERTY FOLDER "Core/Algorithms/Tests")
  SET_PROPERTY(TARGET Core_Application_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Application_Session_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Basis_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Command_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_CommandLine_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Containers_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Datatypes_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Datatypes_Legacy_Base_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Datatypes_Legacy_Field_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Datatypes_Mesh_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Geometry_Primitives_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Logging_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Math_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Serialization_Network_Tests         PROPERTY FOLDER "Dataflow/Serialization/Tests")
  SET_PROPERTY(TARGET Core_Thread_Tests   PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Utils_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_XMLUtil_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_Parser_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_DatabaseManager_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_ImportExport_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Core_IEPlugin_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET Graphics_Widgets_Tests         PROPERTY FOLDER "Graphics/Tests")
  SET_PROPERTY(TARGET Graphics_Datatypes_Tests PROPERTY FOLDER "Graphics/Tests")

  SET_PROPERTY(TARGET Engine_Network_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")
  SET_PROPERTY(TARGET Engine_Scheduler_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")

  SET_PROPERTY(TARGET Dataflow_Network_Tests   PROPERTY FOLDER "Dataflow/Tests")
  SET_PROPERTY(TARGET Dataflow_State_Tests   PROPERTY FOLDER "Dataflow/Tests")

  SET_PROPERTY(TARGET Modules_Basic_Tests   PROPERTY FOLDER "Modules/Basic")
  SET_PROPERTY(TARGET Modules_Bundle_Tests   PROPERTY FOLDER "Modules/Basic")
  SET_PROPERTY(TARGET Modules_BrainStimulator_Tests   PROPERTY FOLDER "Modules/Brain")
  SET_PROPERTY(TARGET Modules_DataIO_Tests   PROPERTY FOLDER "Modules/DataIO")
  SET_PROPERTY(TARGET Modules_Teem_DataIO_Tests   PROPERTY FOLDER "Modules/Teem")
  SET_PROPERTY(TARGET Modules_Teem_Misc_Tests   PROPERTY FOLDER "Modules/Teem")
  SET_PROPERTY(TARGET Modules_Teem_Converters_Tests   PROPERTY FOLDER "Modules/Teem")
  SET_PROPERTY(TARGET Modules_Fields_Tests   PROPERTY FOLDER "Modules/Fields")
  SET_PROPERTY(TARGET Modules_Legacy_Inverse_Tests   PROPERTY FOLDER "Modules/Inverse")
  SET_PROPERTY(TARGET Modules_Math_Tests   PROPERTY FOLDER "Modules/Math")
  SET_PROPERTY(TARGET Modules_Render_Tests   PROPERTY FOLDER "Modules/Render")
  SET_PROPERTY(TARGET Modules_String_Tests   PROPERTY FOLDER "Modules/String")
  SET_PROPERTY(TARGET Modules_Visualization_Tests   PROPERTY FOLDER "Modules/Visualization")
  SET_PROPERTY(TARGET Modules_Legacy_Matlab_DataIO_Tests PROPERTY FOLDER "Modules/DataIO")
  SET_PROPERTY(TARGET Modules_Factory_Tests   PROPERTY FOLDER "Modules/Basic")

  IF(NOT BUILD_HEADLESS)
    SET_PROPERTY(TARGET Interface_Modules_Render_Tests PROPERTY FOLDER "Interface/Tests")
    SET_PROPERTY(TARGET Modules_Dialog_Factory_Tests PROPERTY FOLDER "Interface/Tests")
  ENDIF()
  IF(BUILD_WITH_PYTHON)
    SET_PROPERTY(TARGET Core_Python_Tests     PROPERTY FOLDER "Core/Tests")
    SET_PROPERTY(TARGET Engine_Python_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")
    SET_PROPERTY(TARGET Modules_Python_Tests  PROPERTY FOLDER "Modules/Python")
  ENDIF()
ENDIF()

SET_PROPERTY(TARGET xml2 PROPERTY FOLDER "Externals")

IF(BUILD_WITH_PYTHON)
  SET_PROPERTY(TARGET Core_Python         PROPERTY FOLDER "Core")
  SET_PROPERTY(TARGET SCIRunPythonAPI     PROPERTY FOLDER "Dataflow/Engine")
  SET_PROPERTY(TARGET Modules_Python      PROPERTY FOLDER "Modules/Python")
  IF(NOT BUILD_HEADLESS)
    SET_PROPERTY(TARGET Interface_Modules_Python PROPERTY FOLDER "Interface/Modules")
  ENDIF()
ENDIF()

########################################################################
# Custom CMake settings for CPack

########################################################################
# CPack

########################################################################
# PACKAGING Information

SET(PACKAGING_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt" CACHE INTERNAL "Where to find the license file." FORCE )

SET(PACKAGING_DESCRIPTION "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Templates/CPack.GenericDescription.txt" CACHE INTERNAL "Where to find the package description file." FORCE )
SET(PACKAGING_README "${CMAKE_CURRENT_SOURCE_DIR}/CMake/ReadMe.txt" CACHE INTERNAL "Where to find the readme file." FORCE )
SET(PACKAGING_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Welcome.txt" CACHE INTERNAL "Where to find the welcome file." FORCE )

# TODO: need release notes
#SET(PACKAGING_RELEASENOTES "${CMAKE_CURRENT_SOURCE_DIR}/ReleaseNotes.txt" CACHE INTERNAL "Where to find the releasenotes file." FORCE )

SET(PACKAGING_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons\\scirun5_256.png" CACHE INTERNAL "Packing icon." FORCE )

MESSAGE(STATUS "Configuring SCIRun packaging")

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_NAME "${APPLICATION_NAME}")

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${APPLICATION_NAME}")
SET(CPACK_PACKAGE_VENDOR "${APPLICATION_VENDOR}")

# TODO: actually used? looks like CPACK_RESOURCE_FILE_README gets used in OS X installer
# check NSIS installer once it exists...
SET(CPACK_PACKAGE_DESCRIPTION_FILE ${PACKAGING_DESCRIPTION})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${APPLICATION_NAME} ${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}.${SCIRUN_VERSION_PATCH} is a problem solving environment.")

SET(CPACK_RESOURCE_FILE_LICENSE ${PACKAGING_LICENSE})
SET(CPACK_RESOURCE_FILE_README ${PACKAGING_README})
SET(CPACK_RESOURCE_FILE_WELCOME ${PACKAGING_WELCOME})

SET(CPACK_PACKAGE_VERSION_MAJOR "${SCIRUN_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${SCIRUN_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${SCIRUN_VERSION_PATCH}")
SET(CPACK_PACKAGE_VERSION "${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}.${SCIRUN_VERSION_PATCH}")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${APPLICATION_NAME}_${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")

# other platform settings will be built up from here
IF(APPLE)
  SET(CPACK_GENERATOR "productbuild;TGZ")
  SET(CPACK_TOPLEVEL_TAG "")
  IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   SET(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
   SET(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
  ENDIF()
  # productbuild packaging repair hack
  # call repair_package.sh after running package target in build
  # Tip: can inspect packages using the pkgutil --expand command
  IF(CPACK_GENERATOR MATCHES "productbuild")
    # make sure there are paths to packaging tools
    FIND_PROGRAM(PKGBUILD_EXE pkgbuild)
    FIND_PROGRAM(PRODUCTBUILD_EXE productbuild)
    FIND_PROGRAM(PKGUTIL_EXE pkgutil)
    SET(PKG_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-Darwin")
    SET(PKG_BUILD_DIR "${CMAKE_BINARY_DIR}/_CPack_Packages/productbuild/${PKG_FILE_NAME}")
    SET(PKG_ROOT_DIR "${PKG_BUILD_DIR}/Applications")

    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/CMake/Modules/CPack.Distribution.dist.in
                   ${CMAKE_BINARY_DIR}/CPack.Distribution.dist)
    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/CMake/scripts/repair_package.sh.in
                   ${CMAKE_BINARY_DIR}/repair_package.sh)
  ENDIF()
ELSEIF(WIN32 AND NOT UNIX)

  # There is a bug in NSIS that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backslashes.
  SET(CPACK_PACKAGE_ICON "${PACKAGING_ICON}")

  IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    SET(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
    SET(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
  ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

  IF(CMAKE_SIZEOF_VOID_P MATCHES 8)
    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  ELSE(CMAKE_SIZEOF_VOID_P MATCHES 8)
    SET(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
  ENDIF(CMAKE_SIZEOF_VOID_P MATCHES 8)

  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\${APPLICATION_NAME}")
  SET(CPACK_NSIS_MUI_ICON "${APPLICATION_WINDOWS_ICON}")
  SET(CPACK_NSIS_DISPLAY_NAME "${APPLICATION_NAME} ${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\${APPLICATION_URL}")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\${APPLICATION_URL}")
  SET(CPACK_NSIS_CONTACT ${APPLICATION_CONTACT} )
  SET(CPACK_NSIS_MODIFY_PATH ON)
  SET(CPACK_PACKAGE_EXECUTABLES "${APPLICATION_NAME}" "${APPLICATION_NAME}")
  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        WriteRegStr HKCR '${APPLICATION_EXTENSION}' '' '${APPLICATION_NAME}'
        WriteRegStr HKCR '${APPLICATION_NAME}\\shell' '' 'open'
        WriteRegStr HKCR '${APPLICATION_NAME}\\DefaultIcon' '' '$INSTDIR\\bin\\${APPLICATION_NAME}.exe,0'
        WriteRegStr HKCR '${APPLICATION_NAME}\\shell\\open\\command' '' '$INSTDIR\\bin\\${APPLICATION_NAME}.exe \\\"%1\\\"'
        WriteRegStr HKCR '${APPLICATION_NAME}\\shell\\edit\\command' '' '$INSTDIR\\bin\\${APPLICATION_NAME}.exe \\\"%1\\\"'
        System::Call 'Shell32::SHChangeNotify( i 0x800000, i 0, i 0, i 0)'
  ")
  SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        DeleteRegKey HKCR '${APPLICATION_EXTENSION}'
        DeleteRegKey HKCR '${APPLICATION_NAME}'
  ")

ENDIF()

INCLUDE(CPack)

########################################################################
#GCov

OPTION(ENABLE_GCOV_DATA_FILES "Creates .gcda and .gcno files for coverage" OFF)
MARK_AS_ADVANCED(ENABLE_GCOV_DATAFILES)

IF(ENABLE_GCOV_DATA_FILES)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF()