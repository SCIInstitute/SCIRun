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

cmake_minimum_required(VERSION 3.20)

# =========================
# Global setup / policies
# =========================
set(compress_type "GIT" CACHE INTERNAL "")
set(ep_base "${CMAKE_BINARY_DIR}/Externals" CACHE INTERNAL "")

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW) # ExternalProject downloads
endif()

# Default build type for Unix Makefiles if not set
if(CMAKE_GENERATOR MATCHES "Unix Makefiles" AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

find_package(Git REQUIRED)
include(ExternalProject)

# =========================
# Architecture
# =========================
if(CMAKE_SIZEOF_VOID_P MATCHES 8)
  set(SCIRUN_BITS 64)
else()
  set(SCIRUN_BITS 32)
endif()

# =========================
# Options
# =========================
option(BUILD_TESTING "Build with tests." OFF)
option(GENERATE_COMPILATION_DATABASE "Generate Compilation Database." ON)
option(BUILD_WITH_PYTHON "Build with python support." ON)
option(WITH_TETGEN "Build Tetgen." ON)
option(WITH_OSPRAY "Build Ospray." OFF)
option(BUILD_WITH_SCIRUN_DATA "Svn checkout data" OFF)

if(WIN32)
  option(SCIRUN_SHOW_CONSOLE "Show Windows console when running SCIRun." ON)
  mark_as_advanced(SCIRUN_SHOW_CONSOLE)
endif()

option(BUILD_HEADLESS "Build SCIRun without GUI." OFF)

# =========================
# Prefix collector
# =========================
# Keep one clean CMAKE_PREFIX_PATH for SCIRun via GLOBAL PROPERTY
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

# =========================
# Qt selection
# =========================
set(SCIRUN_QT_MAJOR "6" CACHE STRING "Qt major version to use (5 or 6)")
set_property(CACHE SCIRUN_QT_MAJOR PROPERTY STRINGS 5 6)
set(QT5_MIN_VERSION "5.15.2")
set(QT6_MIN_VERSION "6.3.0")
set(Qt_PATH "" CACHE PATH "Qt install prefix (e.g. C:/Qt/6.10.1/msvc2022_64)")

if (WIN32 AND NOT BUILD_HEADLESS)
  if (NOT Qt_PATH OR NOT IS_DIRECTORY "${Qt_PATH}")
    set(_qt_default "C:/Qt/6.10.1/msvc2022_64")
    if (IS_DIRECTORY "${_qt_default}")
      message(STATUS "Qt_PATH not set or invalid — defaulting to ${_qt_default}")
      set(Qt_PATH "${_qt_default}" CACHE PATH "Qt install prefix" FORCE)
    endif()
  endif()
endif()

# Validate
if (NOT BUILD_HEADLESS)
  if (NOT IS_DIRECTORY "${Qt_PATH}")
    message(FATAL_ERROR
      "Qt_PATH invalid or not set.\n"
      "Point it to the Qt install prefix with lib/, bin/, and lib/cmake/Qt[56].\n"
      "Example: C:/Qt/6.10.1/msvc2022_64")
  endif()

  file(TO_CMAKE_PATH "${Qt_PATH}" _Qt_PREFIX)
  list(PREPEND CMAKE_PREFIX_PATH "${_Qt_PREFIX}")
  sb_prefix_append("${_Qt_PREFIX}")

  if (SCIRUN_QT_MAJOR STREQUAL "6")
    set(QT_MIN_VERSION "${QT6_MIN_VERSION}")
    set(Qt6_DIR "${_Qt_PREFIX}/lib/cmake/Qt6")
    if (NOT EXISTS "${Qt6_DIR}/Qt6Config.cmake")
      message(FATAL_ERROR "Qt6Config.cmake not found at '${Qt6_DIR}'")
    endif()
    find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
      Core Gui Widgets Network Concurrent PrintSupport Svg OpenGL OpenGLWidgets
      HINTS "${Qt_PATH}"
    )
    message(STATUS "[superbuild] Using Qt6_DIR='${Qt6_DIR}'")
  elseif (SCIRUN_QT_MAJOR STREQUAL "5")
    set(QT_MIN_VERSION "${QT5_MIN_VERSION}")
    set(Qt5_DIR "${_Qt_PREFIX}/lib/cmake/Qt5")
    if (NOT EXISTS "${Qt5_DIR}/Qt5Config.cmake")
      message(FATAL_ERROR "Qt5Config.cmake not found at '${Qt5_DIR}'")
    endif()
    find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS
      Core Gui Widgets Network Concurrent PrintSupport Svg OpenGL
      HINTS "${Qt_PATH}"
    )
    message(STATUS "[superbuild] Using Qt5_DIR='${Qt5_DIR}'")
  else()
    message(FATAL_ERROR "SCIRUN_QT_MAJOR must be '5' or '6'. Got: '${SCIRUN_QT_MAJOR}'")
  endif()

  # Locate qmake (needed by Qwt external)
  if (SCIRUN_QT_MAJOR STREQUAL "6")
    if (TARGET Qt6::qmake)
      get_target_property(_qmake_path Qt6::qmake LOCATION)
    else()
      get_filename_component(_qt6_cmake_dir "${Qt6_DIR}" DIRECTORY)
      get_filename_component(_qt6_prefix    "${_qt6_cmake_dir}" DIRECTORY)
      get_filename_component(_qt6_root      "${_qt6_prefix}" DIRECTORY)
      if (WIN32)
        set(_qmake_path "${_qt6_root}/bin/qmake.exe")
      else()
        set(_qmake_path "${_qt6_root}/bin/qmake")
      endif()
    endif()
  else()
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

# =========================
# Doxygen / codegen toggles
# =========================
option(BUILD_DOCUMENTATION "Build documentation" OFF)
mark_as_advanced(BUILD_DOCUMENTATION)
option(GENERATE_MODULE_FACTORY_CODE "Generate module factory code from config files" ON)
mark_as_advanced(GENERATE_MODULE_FACTORY_CODE)
option(REGENERATE_MODULE_FACTORY_CODE "Delete generated module factory to refresh module list" ON)
mark_as_advanced(REGENERATE_MODULE_FACTORY_CODE)

# =========================
# Externals registration
# =========================
set(SCIRUN_DEPENDENCIES)
macro(ADD_EXTERNAL cmake_file external)
  include(${cmake_file})
  list(APPEND SCIRUN_DEPENDENCIES ${external})
endmacro()

# =========================
# Python (superbuild-level detection)
# =========================
if(BUILD_WITH_PYTHON)
  # Prefer modern FindPython module (Interpreter + Development)
  find_package(Python COMPONENTS Interpreter Development REQUIRED)

  message(STATUS "[superbuild] Python executable: ${Python_EXECUTABLE}")
  message(STATUS "[superbuild] Python includes  : ${Python_INCLUDE_DIRS}")
  message(STATUS "[superbuild] Python libraries : ${Python_LIBRARIES}")
  message(STATUS "[superbuild] Python version   : ${Python_VERSION}")

  # Optionally bias CMake to not scan the Windows registry for a different Python
  # set(Python_FIND_REGISTRY NEVER)

  # If you want the inner configure to also see this location via CMAKE_PREFIX_PATH
  # (not required, but harmless), you can append Python's root to prefixes:
  # Try to infer a plausible root from exec/include path:
  get_filename_component(_py_exec_dir "${Python_EXECUTABLE}" DIRECTORY)
  get_filename_component(_py_root     "${_py_exec_dir}" DIRECTORY)
  if(EXISTS "${_py_root}")
    sb_prefix_append("${_py_root}")
  endif()
endif()

set(SUPERBUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
set(SCIRUN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src CACHE INTERNAL "" FORCE)
set(SCIRUN_BINARY_DIR ${CMAKE_BINARY_DIR}/SCIRun CACHE INTERNAL "" FORCE)

if(BUILD_TESTING)
  ADD_EXTERNAL(${SUPERBUILD_DIR}/TestDataConfig.cmake SCIRunTestData_external)
endif()

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

if(WIN32)
  ADD_EXTERNAL(${SUPERBUILD_DIR}/GlewExternal.cmake   Glew_external)
endif()

if(BUILD_WITH_PYTHON)
  ADD_EXTERNAL(${SUPERBUILD_DIR}/PythonExternal.cmake Python_external)
endif()

find_package(Subversion)
if(NOT Subversion_FOUND)
  set(BUILD_WITH_SCIRUN_DATA OFF)
endif()
if(BUILD_WITH_SCIRUN_DATA)
  ADD_EXTERNAL(${SUPERBUILD_DIR}/SCIRunDataExternal.cmake SCI_data_external)
endif()

if(WITH_TETGEN)
  message(STATUS "Configuring Tetgen under GPL. Disable WITH_TETGEN to skip.")
  ADD_EXTERNAL(${SUPERBUILD_DIR}/TetgenExternal.cmake Tetgen_external)
endif()

if(WITH_OSPRAY)
  ADD_EXTERNAL(${SUPERBUILD_DIR}/OsprayExternal.cmake Ospray_external)
endif()

# Keep Boost last so we can compute Boost_DIR & legacy hints afterward
ADD_EXTERNAL(${SUPERBUILD_DIR}/BoostExternal.cmake Boost_external)

# Ensure header copy steps run before SCIRun config/build
if(TARGET Glew_external-copy_headers)
  add_dependencies(SCIRun_external Glew_external-copy_headers)
endif()
if(TARGET Tny_external-copy_headers)
  add_dependencies(SCIRun_external Tny_external-copy_headers)
endif()
if(TARGET SQLite_external-copy_headers)
  add_dependencies(SCIRun_external SQLite_external-copy_headers)
endif()

# =========================
# Exporters (config dir / include & lib dir)
# =========================
function(_export_config_dir pkg target subdir_pattern)
  if(NOT TARGET ${target})
    message(STATUS "[superbuild] ${target} not defined yet; skipping ${pkg}_DIR export.")
    return()
  endif()
  ExternalProject_Get_Property(${target} INSTALL_DIR)
  if(NOT INSTALL_DIR)
    message(FATAL_ERROR "INSTALL_DIR not set for ${target}. Ensure INSTALL is enabled.")
  endif()

  sb_prefix_append("${INSTALL_DIR}")

  string(TOUPPER "${pkg}" pkg_upper)
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
    message(STATUS "[superbuild] ${pkg}: no *Config.cmake under ${INSTALL_DIR} (expected if package doesn't export).")
    return()
  endif()

  set(${pkg}_DIR "${_found_dir}" CACHE PATH "${pkg} config dir" FORCE)
  list(APPEND SCIRUN_CACHE_ARGS "-D${pkg}_DIR:PATH=${${pkg}_DIR}")
  set(SCIRUN_CACHE_ARGS "${SCIRUN_CACHE_ARGS}" PARENT_SCOPE)
  message(STATUS "[superbuild] ${pkg}_DIR = ${${pkg}_DIR}")
endfunction()

function(_sb_export_inc_lib pkg target)
  if(NOT TARGET ${target})
    message(STATUS "[superbuild] ${target} not defined yet; skipping ${pkg} include/lib export.")
    return()
  endif()
  ExternalProject_Get_Property(${target} INSTALL_DIR)
  if(NOT INSTALL_DIR)
    message(FATAL_ERROR "INSTALL_DIR not set for ${target}.")
  endif()

  sb_prefix_append("${INSTALL_DIR}")

  set(_inc "${INSTALL_DIR}/include")
  if(EXISTS "${INSTALL_DIR}/lib64")
    set(_lib "${INSTALL_DIR}/lib64")
  else()
    set(_lib "${INSTALL_DIR}/lib")
  endif()

  # Export include dir (unconditionally; path will exist by build time)
  set(${pkg}_INCLUDE_DIR "${_inc}" CACHE PATH "${pkg} include dir" FORCE)
  list(APPEND SCIRUN_CACHE_ARGS "-D${pkg}_INCLUDE_DIR:PATH=${${pkg}_INCLUDE_DIR}")

  # Export lib dir if present
  if(EXISTS "${_lib}")
    set(${pkg}_LIB_DIR "${_lib}" CACHE PATH "${pkg} lib dir" FORCE)
    list(APPEND SCIRUN_CACHE_ARGS "-D${pkg}_LIB_DIR:PATH=${${pkg}_LIB_DIR}")
  endif()

  set(SCIRUN_CACHE_ARGS "${SCIRUN_CACHE_ARGS}" PARENT_SCOPE)
  message(STATUS "[superbuild] ${pkg}: include=${_inc}  lib=${_lib}")
endfunction()

# =========================
# Boost props/hints for downstream
# =========================
if(TARGET Boost_external)
  ExternalProject_Get_Property(Boost_external INSTALL_DIR)
  set(SCI_BOOST_PREFIX  "${INSTALL_DIR}" CACHE PATH "Boost install prefix" FORCE)
  set(SCI_BOOST_INCLUDE "${INSTALL_DIR}/include" CACHE PATH "Boost include directory" FORCE)
  if (WIN32 AND EXISTS "${INSTALL_DIR}/lib64")
    set(SCI_BOOST_LIBRARY_DIR "${INSTALL_DIR}/lib64")
  else()
    set(SCI_BOOST_LIBRARY_DIR "${INSTALL_DIR}/lib")
  endif()
  set(SCI_BOOST_LIBRARY_DIR "${SCI_BOOST_LIBRARY_DIR}" CACHE PATH "Boost library directory" FORCE)

  # Try versioned Boost-* dir first
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
  sb_prefix_append("${INSTALL_DIR}")
endif()

# =========================
# Export externals (config or include/lib)
# =========================
_export_config_dir(Zlib       Zlib_external       "<auto>")
_export_config_dir(Freetype   Freetype_external   "<auto>")
_export_config_dir(SQLite     SQLite_external     "<auto>")

# Qwt block guarded by GUI flag and target existence
if(NOT BUILD_HEADLESS AND TARGET Qwt_external)
  _export_config_dir(Qwt Qwt_external "<auto>")

  ExternalProject_Get_Property(Qwt_external INSTALL_DIR)
  set(QWT_INSTALL_DIR "${INSTALL_DIR}")
  set(QWT_INCLUDE_DIR "${QWT_INSTALL_DIR}/include")
  if(EXISTS "${QWT_INSTALL_DIR}/lib64")
    set(QWT_LIBRARY_DIR "${QWT_INSTALL_DIR}/lib64")
  else()
    set(QWT_LIBRARY_DIR "${QWT_INSTALL_DIR}/lib")
  endif()

  add_library(Qwt::Qwt UNKNOWN IMPORTED GLOBAL)
  add_dependencies(Qwt::Qwt Qwt_external)
  set_property(TARGET Qwt::Qwt PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIR}")

  if(WIN32)
    set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_CONFIGURATIONS "Debug;Release")
    set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_LOCATION_RELEASE "${QWT_LIBRARY_DIR}/qwt.lib")
    set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_LOCATION_DEBUG   "${QWT_LIBRARY_DIR}/qwtd.lib")
  elseif(APPLE)
    set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_LOCATION "${QWT_LIBRARY_DIR}/libqwt.dylib")
  else()
    set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_LOCATION "${QWT_LIBRARY_DIR}/libqwt.so")
  endif()
else()
  message(STATUS "[superbuild] Qwt not present (BUILD_HEADLESS ON or target missing); skipping Qwt exports.")
endif()

# Prefer config dirs when possible (Windows GLEW sometimes exports one)
if(WIN32 AND TARGET Glew_external)
  _export_config_dir(GLEW Glew_external "<auto>")
endif()

if(BUILD_WITH_PYTHON AND TARGET Python_external)
  _export_config_dir(Python Python_external "<auto>")
endif()

# Include/lib hints (guarded by target existence)
_sb_export_inc_lib(Eigen     Eigen_external)
_sb_export_inc_lib(GLM       GLM_external)
_sb_export_inc_lib(SpdLog    SpdLog_external)
_sb_export_inc_lib(Teem      Teem_external)
_sb_export_inc_lib(Tny       Tny_external)
_sb_export_inc_lib(LodePng   LodePng_external)
_sb_export_inc_lib(Cleaver2  Cleaver2_external)
_sb_export_inc_lib(SQLite     SQLite_external)
if(WITH_TETGEN AND TARGET Tetgen_external)
  _sb_export_inc_lib(Tetgen  Tetgen_external)
endif()
if(WIN32 AND TARGET Glew_external)
  _sb_export_inc_lib(GLEW    Glew_external)
endif()

# =========================
# Build SCIRun cache args
# =========================
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

# =========================
# Forward Python values to SCIRun (inner CMake)
# =========================
if(BUILD_WITH_PYTHON)
  # New (modern) variables for FindPython in SCIRun subdirs
  list(APPEND SCIRUN_CACHE_ARGS
    "-DBUILD_WITH_PYTHON:BOOL=${BUILD_WITH_PYTHON}"
    "-DPython_EXECUTABLE:FILEPATH=${Python_EXECUTABLE}"
    "-DPython_INCLUDE_DIRS:PATH=${Python_INCLUDE_DIRS}"
    "-DPython_LIBRARIES:STRING=${Python_LIBRARIES}"
    # Mirror for FindPython3 in case inner scripts resolve to that module name
    "-DPython3_EXECUTABLE:FILEPATH=${Python_EXECUTABLE}"
    "-DPython3_INCLUDE_DIRS:PATH=${Python_INCLUDE_DIRS}"
    "-DPython3_LIBRARIES:STRING=${Python_LIBRARIES}"
  )
endif()

if(WIN32)
  list(APPEND SCIRUN_CACHE_ARGS "-DSCIRUN_SHOW_CONSOLE:BOOL=${SCIRUN_SHOW_CONSOLE}")
endif()

# Zlib + Boost essential hints (include & lib paths explicitly)
if(TARGET Zlib_external)
  ExternalProject_Get_Property(Zlib_external INSTALL_DIR)
  set(ZLIB_INSTALL_DIR "${INSTALL_DIR}")
  list(APPEND SCIRUN_CACHE_ARGS
    "-DZLIB_ROOT:PATH=${ZLIB_INSTALL_DIR}"
    "-DZLIB_INCLUDE_DIR:PATH=${ZLIB_INSTALL_DIR}/include"
    "-DZLIB_USE_STATIC_LIBS:BOOL=ON"
  )
  if(DEFINED Zlib_DIR)
    list(APPEND SCIRUN_CACHE_ARGS "-DZLIB_DIR:PATH=${Zlib_DIR}")
  else()
    _export_config_dir(Zlib Zlib_external "lib/cmake/zlib")
  endif()
endif()

if(TARGET Boost_external)
  list(APPEND SCIRUN_CACHE_ARGS
    "-DBoost_DIR:PATH=${Boost_DIR}"
    "-DBoost_ROOT:PATH=${SCI_BOOST_PREFIX}"
    "-DBOOST_ROOT:PATH=${SCI_BOOST_PREFIX}"
    "-DSCI_BOOST_PREFIX:PATH=${SCI_BOOST_PREFIX}"
    "-DBoost_INCLUDE_DIR:PATH=${SCI_BOOST_PREFIX}/include"
    "-DBoost_LIBRARY_DIR:PATH=${SCI_BOOST_LIBRARY_DIR}"
    "-DBoost_NO_SYSTEM_PATHS:BOOL=ON"
  )
endif()

# Legacy alias variables (so existing SCIRun CMake picks them up)
# Normalize LodePng var name if needed
if(DEFINED LodePng_INCLUDE_DIR AND NOT DEFINED LODEPNG_INCLUDE_DIR)
  set(LODEPNG_INCLUDE_DIR "${LodePng_INCLUDE_DIR}")
endif()

if(DEFINED Eigen_INCLUDE_DIR)
  # Stable alias that the inner SCIRun configure can use everywhere.
  set(SCIRUN_EIGEN_INCLUDE "${Eigen_INCLUDE_DIR}" CACHE PATH "Alias: Eigen include dir" FORCE)
  list(APPEND SCIRUN_CACHE_ARGS "-DSCIRUN_EIGEN_INCLUDE:PATH=${SCIRUN_EIGEN_INCLUDE}")
endif()
if(DEFINED GLEW_INCLUDE_DIR)
  set(SCI_GLEW_INCLUDE "${GLEW_INCLUDE_DIR}" CACHE PATH "Legacy: GLEW include dir" FORCE)
  list(APPEND SCIRUN_CACHE_ARGS "-DSCI_GLEW_INCLUDE:PATH=${SCI_GLEW_INCLUDE}")
endif()
if(DEFINED LODEPNG_INCLUDE_DIR)
  set(SCI_LODEPNG_INCLUDE "${LODEPNG_INCLUDE_DIR}" CACHE PATH "Legacy: LodePNG include dir" FORCE)
  list(APPEND SCIRUN_CACHE_ARGS "-DSCI_LODEPNG_INCLUDE:PATH=${SCI_LODEPNG_INCLUDE}")
endif()
if(DEFINED TNY_INCLUDE_DIR)
  set(SCI_TNY_INCLUDE "${TNY_INCLUDE_DIR}" CACHE PATH "Legacy: TNY include dir" FORCE)
  list(APPEND SCIRUN_CACHE_ARGS "-DSCI_TNY_INCLUDE:PATH=${SCI_TNY_INCLUDE}")
endif()
if(DEFINED SQLite_INCLUDE_DIR)
  set(SQLite3_INCLUDE_DIR "${SQLite_INCLUDE_DIR}" CACHE PATH "Alias: SQLite3 include dir" FORCE)
  list(APPEND SCIRUN_CACHE_ARGS "-DSQLite3_INCLUDE_DIR:PATH=${SQLite3_INCLUDE_DIR}")
endif()
# --- Ensure SQLite hints are present AFTER SCIRUN_CACHE_ARGS is created ---
if(TARGET SQLite_external)
  ExternalProject_Get_Property(SQLite_external INSTALL_DIR)
  # Compute lib dir (lib64 preferred when present)
  if(EXISTS "${INSTALL_DIR}/lib64")
    set(_sqlite_lib_dir "${INSTALL_DIR}/lib64")
  else()
    set(_sqlite_lib_dir "${INSTALL_DIR}/lib")
  endif()

  # Always pass include + lib dir
  list(APPEND SCIRUN_CACHE_ARGS
    "-DSQLite_INCLUDE_DIR:PATH=${INSTALL_DIR}/include"
    "-DSQLite_LIB_DIR:PATH=${_sqlite_lib_dir}"
    # Alias the include to SQLite3_* for consumers that use that name
    "-DSQLite3_INCLUDE_DIR:PATH=${INSTALL_DIR}/include"
  )

  # Optional: pass the exact library file if it exists (helps consumers)
  if(WIN32)
    set(_sqlite_lib "${_sqlite_lib_dir}/sqlite3.lib")
    if(NOT EXISTS "${_sqlite_lib}" AND EXISTS "${_sqlite_lib_dir}/libsqlite3.lib")
      set(_sqlite_lib "${_sqlite_lib_dir}/libsqlite3.lib")
    endif()
  elseif(APPLE)
    set(_sqlite_lib "${_sqlite_lib_dir}/libsqlite3.dylib")
  else()
    set(_sqlite_lib "${_sqlite_lib_dir}/libsqlite3.so")
  endif()

  if(EXISTS "${_sqlite_lib}")
    list(APPEND SCIRUN_CACHE_ARGS "-DSQLite3_LIBRARY:FILEPATH=${_sqlite_lib}")
    message(STATUS "[superbuild] SQLite library: ${_sqlite_lib}")
  else()
    message(STATUS "[superbuild] SQLite library: (not found yet at configure time)")
  endif()

  message(STATUS "[superbuild] SQLite include: ${INSTALL_DIR}/include")
  message(STATUS "[superbuild] SQLite lib dir: ${_sqlite_lib_dir}")
endif()

# --- Ensure Teem hints are present AFTER SCIRUN_CACHE_ARGS is created ---
if(TARGET Teem_external)
  ExternalProject_Get_Property(Teem_external INSTALL_DIR)

  # Compute Teem lib dir (prefer lib64 when present)
  if(EXISTS "${INSTALL_DIR}/lib64")
    set(_teem_lib_dir "${INSTALL_DIR}/lib64")
  else()
    set(_teem_lib_dir "${INSTALL_DIR}/lib")
  endif()

  # Always pass include + lib dir
  list(APPEND SCIRUN_CACHE_ARGS
    "-DTeem_INCLUDE_DIR:PATH=${INSTALL_DIR}/include"
    "-DTeem_LIB_DIR:PATH=${_teem_lib_dir}"
  )

  # Optional: pass full library path if present (helps consumers)
  if(WIN32)
    set(_teem_lib "${_teem_lib_dir}/teem.lib")
    if(NOT EXISTS "${_teem_lib}" AND EXISTS "${_teem_lib_dir}/libteem.lib")
      set(_teem_lib "${_teem_lib_dir}/libteem.lib")
    endif()
  elseif(APPLE)
    set(_teem_lib "${_teem_lib_dir}/libteem.dylib")
    if(NOT EXISTS "${_teem_lib}" AND EXISTS "${_teem_lib_dir}/libteem.a")
      set(_teem_lib "${_teem_lib_dir}/libteem.a")
    endif()
  else()
    set(_teem_lib "${_teem_lib_dir}/libteem.so")
    if(NOT EXISTS "${_teem_lib}" AND EXISTS "${_teem_lib_dir}/libteem.a")
      set(_teem_lib "${_teem_lib_dir}/libteem.a")
    endif()
  endif()

  if(EXISTS "${_teem_lib}")
    list(APPEND SCIRUN_CACHE_ARGS "-DTeem_LIBRARY:FILEPATH=${_teem_lib}")
    message(STATUS "[superbuild] Teem library: ${_teem_lib}")
  else()
    message(STATUS "[superbuild] Teem library: (not found yet at configure time)")
  endif()

  message(STATUS "[superbuild] Teem include: ${INSTALL_DIR}/include")
  message(STATUS "[superbuild] Teem lib dir: ${_teem_lib_dir}")
endif()

# Compose a single CMAKE_PREFIX_PATH for SCIRun
get_property(_acc GLOBAL PROPERTY SCIRUN_PREFIXES)
set(_joined_prefixes "")
if(_acc)
  list(REMOVE_DUPLICATES _acc)
  string(JOIN ";" _joined_prefixes ${_acc})
endif()
set(_joined_prefixes_escaped "${_joined_prefixes}")
string(REPLACE ";" "\\;" _joined_prefixes_escaped "${_joined_prefixes}")

# Ensure only one -DCMAKE_PREFIX_PATH is passed
set(_filtered "")
foreach(arg IN LISTS SCIRUN_CACHE_ARGS)
  if(NOT arg MATCHES "^-DCMAKE_PREFIX_PATH:PATH=")
    list(APPEND _filtered "${arg}")
  endif()
endforeach()
set(SCIRUN_CACHE_ARGS "${_filtered}")
list(APPEND SCIRUN_CACHE_ARGS "-DCMAKE_PREFIX_PATH:PATH=${_joined_prefixes_escaped}")

# Log what we will pass to SCIRun
message(STATUS "[superbuild] Final CMAKE_PREFIX_PATH for SCIRun: ${_joined_prefixes}")
message(STATUS "Superbuild passing to SCIRun:")
foreach(arg IN LISTS SCIRUN_CACHE_ARGS)
  message(STATUS "  ${arg}")
endforeach()

# =========================
# Helper: gate SCIRun configure on external artifacts
# =========================
function(_sb_scirun_wait_for)
  # Usage:
  #   _sb_scirun_wait_for(NAME <tag> FILES <file1> ... DIRS <dir1> ...)
  cmake_parse_arguments(WAIT "" "NAME" "FILES;DIRS" ${ARGN})
  if(NOT WAIT_NAME)
    message(FATAL_ERROR "_sb_scirun_wait_for requires NAME")
  endif()
  set(_deps)
  foreach(_f IN LISTS WAIT_FILES)
    list(APPEND _deps "${_f}")
  endforeach()
  foreach(_d IN LISTS WAIT_DIRS)
    list(APPEND _deps "${_d}")
  endforeach()
  if(_deps)
    ExternalProject_Add_Step(SCIRun_external "wait_for_${WAIT_NAME}"
      COMMAND ${CMAKE_COMMAND} -E echo "Waiting for ${WAIT_NAME} before SCIRun configure..."
      DEPENDEES download
      DEPENDERS configure
      DEPENDS ${_deps}
    )
  endif()
endfunction()

# =========================
# SCIRun ExternalProject
# =========================
ExternalProject_Add(SCIRun_external
  DEPENDS ${SCIRUN_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SCIRUN_SOURCE_DIR}
  BINARY_DIR ${SCIRUN_BINARY_DIR}
  CMAKE_CACHE_ARGS ${SCIRUN_CACHE_ARGS}
  INSTALL_COMMAND ""
)

# =========================
# BOOST: header staging (no delete; copy only) + waits
# =========================
# Prepare cross-platform commands for b2 bootstrapping and header generation
if(WIN32)
  set(_B2_BOOTSTRAP_CMD cmd /c bootstrap.bat)
  set(_B2_HEADERS_CMD   cmd /c .\\b2 headers)
else()
  set(_B2_BOOTSTRAP_CMD ./bootstrap.sh)
  set(_B2_HEADERS_CMD   ./b2 headers)
endif()

if(TARGET Boost_external)
  # 1) Bootstrap b2
  ExternalProject_Add_Step(Boost_external bootstrap_b2
    COMMAND ${_B2_BOOTSTRAP_CMD}
    WORKING_DIRECTORY <SOURCE_DIR>
    DEPENDEES update
    DEPENDERS configure
    COMMENT "Bootstrapping Boost.Build (b2)"
  )

  # 2) Generate the 'boost/' header tree
  ExternalProject_Add_Step(Boost_external stage_headers
    COMMAND ${_B2_HEADERS_CMD}
    WORKING_DIRECTORY <SOURCE_DIR>
    DEPENDEES bootstrap_b2
    COMMENT "Running 'b2 headers' to generate the boost/ header tree"
  )

  # 3) Copy full headers into install/include WITHOUT deleting first
  if(NOT DEFINED _SB_BOOST_HEADERS_COPY_STEP_DEFINED)
    ExternalProject_Add_Step(Boost_external stage_boost_headers_copy
      COMMAND ${CMAKE_COMMAND} -E make_directory <INSTALL_DIR>/include/boost
      COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/boost <INSTALL_DIR>/include/boost
      DEPENDEES stage_headers
      DEPENDERS build
      BYPRODUCTS "<INSTALL_DIR>/include/boost/array.hpp"
      COMMENT "Staging full Boost headers into <INSTALL_DIR>/include/boost (no delete)"
    )
    set(_SB_BOOST_HEADERS_COPY_STEP_DEFINED TRUE)
  endif()

  # Expose step targets for ordering elsewhere
  ExternalProject_Add_StepTargets(Boost_external stage_headers stage_boost_headers_copy)

  # Gate SCIRun configure on staged headers
  ExternalProject_Get_Property(Boost_external INSTALL_DIR)
  _sb_scirun_wait_for(NAME boost
    FILES
      "${INSTALL_DIR}/include/boost/array.hpp"
      "${INSTALL_DIR}/include/boost/exception/all.hpp"
    DIRS  "${INSTALL_DIR}/include/boost"
  )
  add_dependencies(SCIRun_external Boost_external-stage_boost_headers_copy)
endif()

# =========================
# Gate SCIRun configure on other externals' headers
# =========================
if(TARGET Glew_external)
  ExternalProject_Get_Property(Glew_external INSTALL_DIR)
  _sb_scirun_wait_for(NAME glew
    FILES "${INSTALL_DIR}/include/GL/glew.h"
    DIRS  "${INSTALL_DIR}/include"
  )
endif()

if(TARGET GLM_external)
  ExternalProject_Get_Property(GLM_external INSTALL_DIR)
  _sb_scirun_wait_for(NAME glm
    FILES "${INSTALL_DIR}/include/glm/glm.hpp"
    DIRS  "${INSTALL_DIR}/include"
  )
endif()

if(TARGET LodePng_external)
  ExternalProject_Get_Property(LodePng_external INSTALL_DIR)
  _sb_scirun_wait_for(NAME lodepng
    FILES
      "${INSTALL_DIR}/include/lodepng/lodepng.h"
      "${INSTALL_DIR}/include/lodepng.h"
    DIRS  "${INSTALL_DIR}/include"
  )
endif()

if(TARGET SQLite_external)
  ExternalProject_Get_Property(SQLite_external INSTALL_DIR)
  _sb_scirun_wait_for(NAME sqlite
    FILES "${INSTALL_DIR}/include/sqlite3.h"
    DIRS  "${INSTALL_DIR}/include"
  )
endif()

if(TARGET Tny_external)
  ExternalProject_Get_Property(Tny_external INSTALL_DIR)
  _sb_scirun_wait_for(NAME tny
    FILES "${INSTALL_DIR}/include/tny/tny.hpp"
    DIRS  "${INSTALL_DIR}/include"
  )
endif()

if(TARGET Teem_external)
  ExternalProject_Get_Property(Teem_external INSTALL_DIR)
  _sb_scirun_wait_for(NAME teem
    DIRS "${INSTALL_DIR}/include"
  )
endif()

if(NOT BUILD_HEADLESS AND TARGET Qwt_external)
  ExternalProject_Get_Property(Qwt_external INSTALL_DIR)
  _sb_scirun_wait_for(NAME qwt
    FILES "${INSTALL_DIR}/include/qwt.h"
    DIRS  "${INSTALL_DIR}/include"
  )
endif()
