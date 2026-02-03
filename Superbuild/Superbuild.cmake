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

###########################################
# TODO: build from archive - Git not used
SET(compress_type "GIT" CACHE INTERNAL "")
SET(ep_base "${CMAKE_BINARY_DIR}/Externals" CACHE INTERNAL "")

# Silence CMP0135 warning for ExternalProject downloads (NEW is safer)
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

###########################################
# Set default CMAKE_BUILD_TYPE if empty for Unix Makefile builds
IF(CMAKE_GENERATOR MATCHES "Unix Makefiles" AND NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF()

FIND_PACKAGE(Git)
IF(NOT GIT_FOUND)
  MESSAGE(FATAL_ERROR "Cannot find Git. Git is required for SCIRun's Superbuild")
ENDIF()

INCLUDE(ExternalProject)

###########################################
# DETERMINE ARCHITECTURE
###########################################
IF(CMAKE_SIZEOF_VOID_P MATCHES 8)
  SET(SCIRUN_BITS 64)
ELSE()
  SET(SCIRUN_BITS 32)
ENDIF()

###########################################
# Options
###########################################
OPTION(BUILD_TESTING "Build with tests." OFF)
OPTION(GENERATE_COMPILATION_DATABASE "Generate Compilation Database." ON)
OPTION(BUILD_WITH_PYTHON "Build with python support." ON)
OPTION(WITH_TETGEN "Build Tetgen." ON)
OPTION(WITH_OSPRAY "Build Ospray." OFF)
OPTION(BUILD_WITH_SCIRUN_DATA "Svn checkout data" OFF)
IF(WIN32)
  OPTION(SCIRUN_SHOW_CONSOLE "Show Windows console when running SCIRun (useful for debugging)." ON)
  MARK_AS_ADVANCED(SCIRUN_SHOW_CONSOLE)
ENDIF()
OPTION(BUILD_HEADLESS "Build SCIRun without GUI." OFF)

###########################################
# Prefix collector (build ONE clean CMAKE_PREFIX_PATH for SCIRun)
###########################################
# Use a GLOBAL PROPERTY so values persist across functions/macros.
set_property(GLOBAL PROPERTY SCIRUN_PREFIXES "")

function(sb_prefix_append dir_in)
  if(NOT dir_in)
    return()
  endif()
  file(TO_CMAKE_PATH "${dir_in}" _norm)
  get_property(_acc GLOBAL PROPERTY SCIRUN_PREFIXES)
  if(NOT _acc)
    set(_acc "")
  endif()
  list(APPEND _acc "${_norm}")
  list(REMOVE_DUPLICATES _acc)
  set_property(GLOBAL PROPERTY SCIRUN_PREFIXES "${_acc}")
endfunction()

###########################################
# Configure Qt
###########################################

# Allow user to choose whether to use Qt5 or Qt6
set(SCIRUN_QT_MAJOR "6" CACHE STRING "Qt major version to use (5 or 6)")
set_property(CACHE SCIRUN_QT_MAJOR PROPERTY STRINGS 5 6)

# Minimum versions
set(QT5_MIN_VERSION "5.15.2")
set(QT6_MIN_VERSION "6.3.0")

# User provides Qt base path
set(Qt_PATH "" CACHE PATH "Path to Qt installation (e.g. C:/Qt/6.10.1/msvc2022_64)")

if (NOT BUILD_HEADLESS)

  # 1) Validate Qt_PATH
  if (NOT IS_DIRECTORY "${Qt_PATH}")
    message(FATAL_ERROR
      "Qt_PATH is invalid. Point it to the Qt install prefix that contains lib/, bin/, and (lib/cmake/Qt6 or lib/cmake/Qt5).\n"
      "Example (MSVC 2022 x64): C:/Qt/6.10.1/msvc2022_64"
    )
  endif()

  # 2) Make the prefix visible to find_package (and collect it)
  file(TO_CMAKE_PATH "${Qt_PATH}" _Qt_PREFIX)
  list(PREPEND CMAKE_PREFIX_PATH "${_Qt_PREFIX}")
  sb_prefix_append("${_Qt_PREFIX}")

  # 3) Pick Qt major and set DIR to the exact config folder
  if (SCIRUN_QT_MAJOR STREQUAL "6")
    set(QT_MIN_VERSION "${QT6_MIN_VERSION}")
    set(Qt6_DIR "${_Qt_PREFIX}/lib/cmake/Qt6")
    if (NOT EXISTS "${Qt6_DIR}/Qt6Config.cmake")
      message(FATAL_ERROR
        "Qt6Config.cmake not found at '${Qt6_DIR}'.\n"
        "Verify Qt 6 is installed for your compiler and Qt_PATH is correct."
      )
    endif()

    message(STATUS "[superbuild] Qt_PATH='${Qt_PATH}'")
    message(STATUS "[superbuild] Using Qt6_DIR='${Qt6_DIR}'")
    message(STATUS "[superbuild] CMAKE_PREFIX_PATH='${CMAKE_PREFIX_PATH}'")

    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
      Core Gui Widgets Network Concurrent PrintSupport Svg OpenGL OpenGLWidgets
      HINTS "${Qt_PATH}"
    )
    message(STATUS "Using Qt6 found at: ${Qt6_DIR}")

  elseif (SCIRUN_QT_MAJOR STREQUAL "5")
    set(QT_MIN_VERSION "${QT5_MIN_VERSION}")
    set(Qt5_DIR "${_Qt_PREFIX}/lib/cmake/Qt5")
    if (NOT EXISTS "${Qt5_DIR}/Qt5Config.cmake")
      message(FATAL_ERROR
        "Qt5Config.cmake not found at '${Qt5_DIR}'.\n"
        "Verify Qt 5 is installed for your compiler and Qt_PATH is correct."
      )
    endif()

    message(STATUS "[superbuild] Qt_PATH='${Qt_PATH}'")
    message(STATUS "[superbuild] Using Qt5_DIR='${Qt5_DIR}'")
    message(STATUS "[superbuild] CMAKE_PREFIX_PATH='${CMAKE_PREFIX_PATH}'")

    find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS
      Core Gui Widgets Network Concurrent PrintSupport Svg OpenGL
      HINTS "${Qt_PATH}"
    )
    message(STATUS "Using Qt5 found at: ${Qt5_DIR}")

  else()
    message(FATAL_ERROR "SCIRUN_QT_MAJOR must be '5' or '6'. Got: '${SCIRUN_QT_MAJOR}'")
  endif()

  # 4) Locate qmake for the Qwt build step
  if (SCIRUN_QT_MAJOR STREQUAL "6")
    if (TARGET Qt6::qmake)
      get_target_property(_qmake_path Qt6::qmake LOCATION)
    else()
      get_filename_component(_qt6_cmake_dir "${Qt6_DIR}" DIRECTORY)        # .../lib/cmake
      get_filename_component(_qt6_prefix    "${_qt6_cmake_dir}" DIRECTORY) # .../lib
      get_filename_component(_qt6_root      "${_qt6_prefix}" DIRECTORY)    # ...
      if (WIN32)
        set(_qmake_path "${_qt6_root}/bin/qmake.exe")
      else()
        set(_qmake_path "${_qt6_root}/bin/qmake")
      endif()
    endif()
  else() # Qt5
    if (TARGET Qt5::qmake)
      get_target_property(_qmake_path Qt5::qmake LOCATION)
    else()
      get_filename_component(_qt5_cmake_dir "${Qt5_DIR}" DIRECTORY)
      get_filename_component(_qt5_prefix    "${_qt5_cmake_dir}" DIRECTORY)
      get_filename_component(_qt5_root      "${_qt5_prefix}" DIRECTORY)
      if (WIN32)
        set(_qmake_path "${_qt5_root}/bin/qmake.exe")
      else()
        set(_qmake_path "${_qt5_root}/bin/qmake")
      endif()
    endif()
  endif()

  if (NOT _qmake_path OR NOT EXISTS "${_qmake_path}")
    message(FATAL_ERROR "Could not locate qmake. Checked: ${_qmake_path}")
  endif()

  set(QT_QMAKE_EXECUTABLE "${_qmake_path}" CACHE FILEPATH "Path to Qt qmake for building Qwt" FORCE)
  message(STATUS "QT_QMAKE_EXECUTABLE = ${QT_QMAKE_EXECUTABLE}")

else()
  add_definitions(-DBUILD_HEADLESS)
endif()

###########################################
# Configure Doxygen documentation
###########################################
OPTION(BUILD_DOCUMENTATION "Build documentation" OFF)
MARK_AS_ADVANCED(BUILD_DOCUMENTATION)

###########################################
# Factory code generation options
###########################################
OPTION(GENERATE_MODULE_FACTORY_CODE "Generate module factory code from config files" ON)
MARK_AS_ADVANCED(GENERATE_MODULE_FACTORY_CODE)

OPTION(REGENERATE_MODULE_FACTORY_CODE "Delete generated module factory code file in order to refresh module list" ON)
MARK_AS_ADVANCED(REGENERATE_MODULE_FACTORY_CODE)

###########################################
# Configure externals
###########################################
SET(SCIRun_DEPENDENCIES)

MACRO(ADD_EXTERNAL cmake_file external)
  INCLUDE(${cmake_file})
  LIST(APPEND SCIRun_DEPENDENCIES ${external})
ENDMACRO()

SET(SUPERBUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
SET(SCIRUN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src CACHE INTERNAL "" FORCE)
SET(SCIRUN_BINARY_DIR ${CMAKE_BINARY_DIR}/SCIRun CACHE INTERNAL "" FORCE)

IF(BUILD_TESTING)
  ADD_EXTERNAL(${SUPERBUILD_DIR}/TestDataConfig.cmake SCIRunTestData_external)
ENDIF()

ADD_EXTERNAL(${SUPERBUILD_DIR}/EigenExternal.cmake    Eigen_external)
ADD_EXTERNAL(${SUPERBUILD_DIR}/ZlibExternal.cmake     Zlib_external)
ADD_EXTERNAL(${SUPERBUILD_DIR}/SQLiteExternal.cmake   SQLite_external)
ADD_EXTERNAL(${SUPERBUILD_DIR}/TeemExternal.cmake     Teem_external)
ADD_EXTERNAL(${SUPERBUILD_DIR}/FreetypeExternal.cmake Freetype_external)
ADD_EXTERNAL(${SUPERBUILD_DIR}/GLMExternal.cmake      GLM_external)
ADD_EXTERNAL(${SUPERBUILD_DIR}/SpdLogExternal.cmake   SpdLog_external)
ADD_EXTERNAL(${SUPERBUILD_DIR}/TnyExternal.cmake      Tny_external)
ADD_EXTERNAL(${SUPERBUILD_DIR}/LodePngExternal.cmake  LodePng_external)
ADD_EXTERNAL(${SUPERBUILD_DIR}/Cleaver2External.cmake Cleaver2_external)

IF(WIN32)
  ADD_EXTERNAL(${SUPERBUILD_DIR}/GlewExternal.cmake   Glew_external)
ENDIF()

IF(BUILD_WITH_PYTHON)
  ADD_EXTERNAL(${SUPERBUILD_DIR}/PythonExternal.cmake Python_external)
ENDIF()

FIND_PACKAGE(Subversion)
IF(NOT Subversion_FOUND)
  SET(BUILD_WITH_SCIRUN_DATA OFF)
ENDIF()
IF(BUILD_WITH_SCIRUN_DATA)
  ADD_EXTERNAL(${SUPERBUILD_DIR}/SCIRunDataExternal.cmake SCI_data_external)
ENDIF()

IF(WITH_TETGEN)
  MESSAGE(STATUS "Configuring Tetgen library under GPL. The SCIRun InterfaceWithTetGen module can be disabled by setting the CMake build variable WITH_TETGEN to OFF.")
  ADD_EXTERNAL(${SUPERBUILD_DIR}/TetgenExternal.cmake Tetgen_external)
ENDIF()

IF(WITH_OSPRAY)
  ADD_EXTERNAL(${SUPERBUILD_DIR}/OsprayExternal.cmake Ospray_external)
ENDIF()

IF(NOT BUILD_HEADLESS)
  ADD_EXTERNAL(${SUPERBUILD_DIR}/QwtExternal.cmake Qwt_external)
  # ADD_EXTERNAL(${SUPERBUILD_DIR}/CtkExternal.cmake Ctk_external)
ENDIF()

ADD_EXTERNAL(${SUPERBUILD_DIR}/BoostExternal.cmake Boost_external)

###########################################
# Helpers to export locations to SCIRun and collect prefixes
###########################################

# Helper to export a CONFIG package directory to SCIRun cache args
function(_export_config_dir pkg target subdir_pattern)
  ExternalProject_Get_Property(${target} INSTALL_DIR)
  if(NOT INSTALL_DIR)
    message(FATAL_ERROR "INSTALL_DIR not set for ${target}. Ensure INSTALL is enabled in its ExternalProject_Add.")
  endif()

  # Collect prefix
  sb_prefix_append("${INSTALL_DIR}")

  string(TOUPPER "${pkg}" pkg_upper)

  # Candidates where <Pkg>Config.cmake might live
  set(_candidates
    "${INSTALL_DIR}/lib/cmake/${pkg}"
    "${INSTALL_DIR}/lib/cmake/${pkg}-*"
    "${INSTALL_DIR}/lib/cmake/${pkg_upper}"
    "${INSTALL_DIR}/lib64/cmake/${pkg}"
    "${INSTALL_DIR}/lib64/cmake/${pkg_upper}"
    "${INSTALL_DIR}/share/${pkg}/cmake"
    "${INSTALL_DIR}/cmake/${pkg}"
    "${INSTALL_DIR}/CMake/${pkg}"
  )

  unset(_found_dir)
  foreach(_cand IN LISTS _candidates)
    file(GLOB _hits "${_cand}")
    foreach(_d IN LISTS _hits)
      if(EXISTS "${_d}/${pkg}Config.cmake" OR EXISTS "${_d}/${pkg_upper}Config.cmake")
        set(_found_dir "${_d}")
        break()
      endif()
    endforeach()
    if(DEFINED _found_dir)
      break()
    endif()
  endforeach()

  if(NOT DEFINED _found_dir)
    message(STATUS "[superbuild] ${pkg}: no *Config.cmake found under ${INSTALL_DIR}. "
                   "This may be expected if the package does not export configs. "
                   "We will skip setting ${pkg}_DIR for now.")
    return()
  endif()

  # Export <Pkg>_DIR as a cache entry so SCIRun receives it
  set(${pkg}_DIR "${_found_dir}" CACHE PATH "${pkg} config dir" FORCE)
  list(APPEND SCIRun_CACHE_ARGS "-D${pkg}_DIR:PATH=${${pkg}_DIR}")
  set(SCIRun_CACHE_ARGS "${SCIRun_CACHE_ARGS}" PARENT_SCOPE)

  message(STATUS "[superbuild] ${pkg}_DIR = ${${pkg}_DIR}")
endfunction()

# Helper to export include/lib hints (for header-only or non-config packages)
function(_export_include_lib pkg target)
  ExternalProject_Get_Property(${target} INSTALL_DIR)
  if(NOT INSTALL_DIR)
    message(FATAL_ERROR "INSTALL_DIR not set for ${target}.")
  endif()

  # Collect prefix
  sb_prefix_append("${INSTALL_DIR}")

  # Heuristics: include/ and lib{,64}/ under install prefix
  set(_inc "${INSTALL_DIR}/include")
  if(EXISTS "${INSTALL_DIR}/lib64")
    set(_lib "${INSTALL_DIR}/lib64")
  else()
    set(_lib "${INSTALL_DIR}/lib")
  endif()

  if(EXISTS "${_inc}")
    set(${pkg}_INCLUDE_DIR "${_inc}" CACHE PATH "${pkg} include dir" FORCE)
    list(APPEND SCIRun_CACHE_ARGS "-D${pkg}_INCLUDE_DIR:PATH=${${pkg}_INCLUDE_DIR}")
  endif()
  if(EXISTS "${_lib}")
    set(${pkg}_LIB_DIR "${_lib}" CACHE PATH "${pkg} lib dir" FORCE)
    list(APPEND SCIRun_CACHE_ARGS "-D${pkg}_LIB_DIR:PATH=${${pkg}_LIB_DIR}")
  endif()
  set(SCIRun_CACHE_ARGS "${SCIRun_CACHE_ARGS}" PARENT_SCOPE)

  message(STATUS "[superbuild] ${pkg}: include=${_inc}  lib=${_lib}")
endfunction()

###########################################
# Boost properties and hints (robust DIR)
###########################################
ExternalProject_Get_Property(Boost_external INSTALL_DIR)
set(SCI_BOOST_PREFIX  "${INSTALL_DIR}" CACHE PATH "Boost install prefix (produced by Boost_external)" FORCE)
set(SCI_BOOST_INCLUDE "${INSTALL_DIR}/include")
if (WIN32 AND EXISTS "${INSTALL_DIR}/lib64")
  set(SCI_BOOST_LIBRARY_DIR "${INSTALL_DIR}/lib64")
else()
  set(SCI_BOOST_LIBRARY_DIR "${INSTALL_DIR}/lib")
endif()

# Try to pick the exact config dir (versioned) if present; fall back to unversioned
set(Boost_DIR "")
file(GLOB _boost_cfg_dirs "${INSTALL_DIR}/lib/cmake/Boost*")
foreach(_bd IN LISTS _boost_cfg_dirs)
  if(EXISTS "${_bd}/BoostConfig.cmake")
    set(Boost_DIR "${_bd}")
    break()
  endif()
endforeach()
if(NOT Boost_DIR)
  set(Boost_DIR "${INSTALL_DIR}/lib/cmake/Boost")
endif()

# Collect Boost prefix too
sb_prefix_append("${INSTALL_DIR}")

###########################################
# Export locations for externals
###########################################
# CONFIG-package externals (expect *Config.cmake)
_export_config_dir(Zlib       Zlib_external       "<auto>")
_export_config_dir(Freetype   Freetype_external   "<auto>")
_export_config_dir(SQLite     SQLite_external     "<auto>")

# Qwt: create imported target (does not rely on a config package)
_export_config_dir(Qwt Qwt_external "<auto>") # harmless if no config is present

ExternalProject_Get_Property(Qwt_external INSTALL_DIR)
set(QWT_INSTALL_DIR "${INSTALL_DIR}")
set(QWT_INCLUDE_DIR "${QWT_INSTALL_DIR}/include")
set(QWT_LIBRARY_DIR "${QWT_INSTALL_DIR}/lib")

add_library(Qwt::Qwt UNKNOWN IMPORTED GLOBAL)
add_dependencies(Qwt::Qwt Qwt_external)

set_property(TARGET Qwt::Qwt PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIR}"
)

if(WIN32)
  set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_CONFIGURATIONS "Debug;Release")
  set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_LOCATION_RELEASE "${QWT_LIBRARY_DIR}/qwt.lib")
  set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_LOCATION_DEBUG   "${QWT_LIBRARY_DIR}/qwtd.lib")
elseif(APPLE)
  set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_LOCATION "${QWT_LIBRARY_DIR}/libqwt.dylib")
else()
  set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_LOCATION "${QWT_LIBRARY_DIR}/libqwt.so")
endif()

# GLEW: try config; if none, include/lib hints will still help
if(WIN32)
  _export_config_dir(GLEW Glew_external "<auto>")
endif()

# Python: prefer config if available; otherwise include/lib
if(BUILD_WITH_PYTHON)
  _export_config_dir(Python Python_external "<auto>")
endif()

# Header-only or non-config: export include/lib hints
_export_include_lib(Eigen     Eigen_external)
_export_include_lib(GLM       GLM_external)
_export_include_lib(SpdLog    SpdLog_external)

# Likely non-config or custom installs—export include/lib for now
_export_include_lib(Teem      Teem_external)
_export_include_lib(Tny       Tny_external)
_export_include_lib(LodePng   LodePng_external)
_export_include_lib(Cleaver2  Cleaver2_external)

# Optional sets (only if enabled)
if(WITH_TETGEN)
  _export_include_lib(Tetgen Tetgen_external)
endif()
if(WITH_OSPRAY)
  _export_config_dir(ospray Ospray_external "<auto>")
endif()

###########################################
# Download external data sources
###########################################
OPTION(DOWNLOAD_TOOLKITS "Download toolkit repositories." ON)
MARK_AS_ADVANCED(DOWNLOAD_TOOLKITS)
IF(DOWNLOAD_TOOLKITS)
  INCLUDE(${SUPERBUILD_DIR}/ToolkitsConfig.cmake)
  EXTERNAL_TOOLKIT(BrainStimulator)
  EXTERNAL_TOOLKIT(FwdInvToolkit)
ENDIF()

###########################################
# Build SCIRun cache args (internal settings)
###########################################
set(SCIRUN_CACHE_ARGS
  "-DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}"
  "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
  "-DSCIRUN_BINARY_DIR:PATH=${SCIRUN_BINARY_DIR}"
  "-DSCIRUN_BITS:STRING=${SCIRUN_BITS}"
  "-DBUILD_TESTING:BOOL=${BUILD_TESTING}"
  "-DBUILD_DOCUMENTATION:BOOL=${BUILD_DOCUMENTATION}"
  "-DBUILD_HEADLESS:BOOL=${BUILD_HEADLESS}"
  "-DSCIRUN_TEST_RESOURCE_DIR:PATH=${SCIRUN_TEST_RESOURCE_DIR}"
  "-DBUILD_WITH_PYTHON:BOOL=${BUILD_WITH_PYTHON}"
  "-DUSER_PYTHON_VERSION:STRING=${USER_PYTHON_VERSION}"
  "-DUSER_PYTHON_VERSION_MAJOR:STRING=${USER_PYTHON_VERSION_MAJOR}"
  "-DUSER_PYTHON_VERSION_MINOR:STRING=${USER_PYTHON_VERSION_MINOR}"
  "-DWITH_TETGEN:BOOL=${WITH_TETGEN}"
  "-DWITH_OSPRAY:BOOL=${WITH_OSPRAY}"
  "-DREGENERATE_MODULE_FACTORY_CODE:BOOL=${REGENERATE_MODULE_FACTORY_CODE}"
  "-DGENERATE_MODULE_FACTORY_CODE:BOOL=${GENERATE_MODULE_FACTORY_CODE}"
  "-DGENERATE_COMPILATION_DATABASE:BOOL=${GENERATE_COMPILATION_DATABASE}"
  "-DSCIRUN_QT_MAJOR:STRING=${SCIRUN_QT_MAJOR}"
  "-DQt_PATH:PATH=${Qt_PATH}"
)

if(BUILD_WITH_PYTHON)
  if(SCI_PYTHON_EXE)
    list(APPEND SCIRUN_CACHE_ARGS "-DPYTHON_EXECUTABLE:FILEPATH=${SCI_PYTHON_EXE}")
  endif()
endif()

if(WIN32)
  list(APPEND SCIRUN_CACHE_ARGS "-DSCIRUN_SHOW_CONSOLE:BOOL=${SCIRUN_SHOW_CONSOLE}")
endif()

# (Avoid passing empty Qt component DIR hints; CMAKE_PREFIX_PATH+Qt_PATH are enough)
if(NOT BUILD_HEADLESS)
  if(DEFINED MACDEPLOYQT_OUTPUT_LEVEL)
    list(APPEND SCIRUN_CACHE_ARGS "-DMACDEPLOYQT_OUTPUT_LEVEL:STRING=${MACDEPLOYQT_OUTPUT_LEVEL}")
  endif()
endif()

###########################################
# Explicit Zlib/Boost hints (good to keep)
###########################################
ExternalProject_Get_Property(Zlib_external INSTALL_DIR)
set(ZLIB_INSTALL_DIR "${INSTALL_DIR}")

list(APPEND SCIRUN_CACHE_ARGS
  "-DZLIB_ROOT:PATH=${ZLIB_INSTALL_DIR}"
  "-DZLIB_INCLUDE_DIR:PATH=${ZLIB_INSTALL_DIR}/include"
  "-DZLIB_USE_STATIC_LIBS:BOOL=ON"
  "-DBoost_DIR:PATH=${Boost_DIR}"                 # points to folder containing BoostConfig.cmake
  "-DBoost_ROOT:PATH=${SCI_BOOST_PREFIX}"
  "-DBOOST_ROOT:PATH=${SCI_BOOST_PREFIX}"
  "-DSCI_BOOST_PREFIX:PATH=${SCI_BOOST_PREFIX}"
  "-DBoost_NO_SYSTEM_PATHS:BOOL=ON"
)

# If Zlib_DIR is known from _export_config_dir, pass it (helps config-package resolution)
if(DEFINED Zlib_DIR)
  list(APPEND SCIRUN_CACHE_ARGS "-DZLIB_DIR:PATH=${Zlib_DIR}")
endif()

# Compose ONE clean CMAKE_PREFIX_PATH for SCIRun
get_property(_acc GLOBAL PROPERTY SCIRUN_PREFIXES)
set(_joined_prefixes "")
if(_acc)
  list(REMOVE_DUPLICATES _acc)
  string(JOIN ";" _joined_prefixes ${_acc})
endif()

# ESCAPE the semicolons so the whole value stays ONE list element
set(_joined_prefixes_escaped "${_joined_prefixes}")
string(REPLACE ";" "\\;" _joined_prefixes_escaped "${_joined_prefixes}")

# Remove any earlier -DCMAKE_PREFIX_PATH entries (defensive)
set(_filtered "")
foreach(arg IN LISTS SCIRUN_CACHE_ARGS)
  if(NOT arg MATCHES "^-DCMAKE_PREFIX_PATH:PATH=")
    list(APPEND _filtered "${arg}")
  endif()
endforeach()
set(SCIRUN_CACHE_ARGS "${_filtered}")

# Append exactly ONE CMAKE_PREFIX_PATH argument (escaped)
list(APPEND SCIRUN_CACHE_ARGS "-DCMAKE_PREFIX_PATH:PATH=${_joined_prefixes_escaped}")

# Log what we will pass
message(STATUS "[superbuild] Final CMAKE_PREFIX_PATH for SCIRun: ${_joined_prefixes}")
message(STATUS "Superbuild passing to SCIRun:")
foreach(arg IN LISTS SCIRUN_CACHE_ARGS)
  message(STATUS "  ${arg}")
endforeach()

###########################################
# ExternalProject for SCIRun
###########################################
ExternalProject_Add(SCIRun_external
  DEPENDS ${SCIRun_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SCIRUN_SOURCE_DIR}
  BINARY_DIR ${SCIRUN_BINARY_DIR}
  CMAKE_CACHE_ARGS ${SCIRUN_CACHE_ARGS}
  INSTALL_COMMAND ""
)

# Optional: ensure Zlib artifacts exist before SCIRun config (keeps order tidy)
ExternalProject_Add_Step(SCIRun_external wait_for_zlib
  COMMAND ${CMAKE_COMMAND} -E echo "Waiting for zlib artifacts before SCIRun configure..."
  DEPENDEES download
  DEPENDERS configure
  DEPENDS
    "${ZLIB_INSTALL_DIR}/include/zlib.h"
    "${ZLIB_INSTALL_DIR}/lib"
)
