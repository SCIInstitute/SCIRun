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
_sb_export_inc_lib(Freetype  Freetype_external)
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
# Forward Python values to SCIRun (inner CMake), version-agnostic
# =========================
if(BUILD_WITH_PYTHON)
  # Base paths from your external layout
  set(_PY_SRC        "${ep_base}/Source/Python_external")
  set(_PY_PCBUILD    "${_PY_SRC}/PCbuild/amd64")
  set(_PY_INC        "${_PY_SRC}/Include")
  set(_PY_PC_INC     "${_PY_SRC}/PC")
  set(_PY_EXE        "${_PY_PCBUILD}/python.exe")

  # Derive MAJOR.MINOR from the built interpreter
  set(_PY_MAJ "")
  set(_PY_MIN "")
  if(EXISTS "${_PY_EXE}")
    execute_process(
      COMMAND "${_PY_EXE}" -c "import sys; print(f'{sys.version_info[0]}.{sys.version_info[1]}')"
      OUTPUT_VARIABLE _PY_VER_SHORT
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
    if(_PY_VER_SHORT MATCHES "^([0-9]+)\\.([0-9]+)$")
      set(_PY_MAJ "${CMAKE_MATCH_1}")
      set(_PY_MIN "${CMAKE_MATCH_2}")
    endif()
  endif()

  # Fallback: if the interpreter didn’t run (first configure), try to read cached values you exported
  if(NOT _PY_MAJ OR NOT _PY_MIN)
    if(DEFINED PY_MAJOR AND DEFINED PY_MINOR)
      set(_PY_MAJ "${PY_MAJOR}")
      set(_PY_MIN "${PY_MINOR}")
    endif()
  endif()

  # Construct the Windows import library names: pythonXY(.lib) and pythonXY_d.lib
  set(_PY_DIGITS "")
  if(_PY_MAJ AND _PY_MIN)
    set(_PY_DIGITS "${_PY_MAJ}${_PY_MIN}")     # e.g. "313"
  else()
    # As a last resort, try to detect digits from filename presence in the lib dir
    # (See Approach B below for a fuller glob)
    message(STATUS "[Python wiring] Could not determine Python version via interpreter; will try folder scan later.")
  endif()

  # Resolve actual files in PCbuild/amd64
  set(_PY_LIB_REL "")
  set(_PY_LIB_DBG "")
  if(_PY_DIGITS AND EXISTS "${_PY_PCBUILD}/python${_PY_DIGITS}.lib")
    set(_PY_LIB_REL "${_PY_PCBUILD}/python${_PY_DIGITS}.lib")
  endif()
  if(_PY_DIGITS AND EXISTS "${_PY_PCBUILD}/python${_PY_DIGITS}_d.lib")
    set(_PY_LIB_DBG "${_PY_PCBUILD}/python${_PY_DIGITS}_d.lib")
  endif()

  # If still missing, fall back to a glob (see Approach B)
  if(NOT _PY_LIB_REL OR NOT _PY_LIB_DBG)
    file(GLOB _py_rel_cand "${_PY_PCBUILD}/python3*.lib")
    file(GLOB _py_dbg_cand "${_PY_PCBUILD}/python3*_d.lib")
    list(SORT _py_rel_cand)
    list(SORT _py_dbg_cand)
    list(REVERSE _py_rel_cand)
    list(REVERSE _py_dbg_cand)
    if(NOT _PY_LIB_REL AND _py_rel_cand)
      list(GET _py_rel_cand 0 _PY_LIB_REL)
    endif()
    if(NOT _PY_LIB_DBG AND _py_dbg_cand)
      list(GET _py_dbg_cand 0 _PY_LIB_DBG)
    endif()
  endif()

  if(NOT (EXISTS "${_PY_LIB_REL}" AND EXISTS "${_PY_LIB_DBG}"))
    message(WARNING "[Python wiring] Could not resolve both Python import libs under ${_PY_PCBUILD}. "
                    "REL='${_PY_LIB_REL}' DBG='${_PY_LIB_DBG}'. "
                    "First configure after a clean may hit this; they will exist after Python_external builds.")
  endif()

  # Append cache args for SCIRun inner build: no version hard-coded
  list(APPEND SCIRUN_CACHE_ARGS
    "-DBUILD_WITH_PYTHON:BOOL=${BUILD_WITH_PYTHON}"
    "-DPYTHON_INCLUDE_DIR:PATH=${_PY_INC}"
    "-DPYTHON_PC_INCLUDE_DIR:PATH=${_PY_PC_INC}"
    "-DPYTHON_EXECUTABLE:FILEPATH=${_PY_EXE}"
    "-DPYTHON_RUNTIME_DIR:PATH=${_PY_PCBUILD}"
    "-DPYTHON_LIBRARY_DEBUG:FILEPATH=${_PY_LIB_DBG}"
    "-DPYTHON_LIBRARY_RELEASE:FILEPATH=${_PY_LIB_REL}"

    # Optional hints for legacy find modules
    "-DPython_EXECUTABLE:FILEPATH=${_PY_EXE}"
    "-DPython_INCLUDE_DIRS:PATH=${_PY_INC};${_PY_PC_INC}"
    "-DPython3_EXECUTABLE:FILEPATH=${_PY_EXE}"
    "-DPython3_INCLUDE_DIRS:PATH=${_PY_INC};${_PY_PC_INC}"

    # Boost.Python wiring remains as-is
    "-DSCI_BOOST_LIBRARY_DIR:PATH=${SCI_BOOST_LIBRARY_DIR}"
    "-DSCIRUN_EXPLICIT_BOOST_PYTHON_LINK:BOOL=ON"

    # === Strong hints for FindPython to stay inside the superbuild tree ===
    "-DPython_ROOT_DIR:PATH=${_PY_SRC}"
    "-DPython3_ROOT_DIR:PATH=${_PY_SRC}"
    
    # === Explicit paths used by the module-level override (imported target) ===
    "-DPY_EXT_LIB_DIR:PATH=${_PY_PCBUILD}"
    "-DPY_INCLUDE_DIR:PATH=${_PY_INC}"

    # Bias CMake's FindPython inside the inner build to honor our root and NOT the registry/system
    "-DPython_FIND_REGISTRY:STRING=NEVER"
    "-DPython_FIND_STRATEGY:STRING=LOCATION"
    # Fence typical system install path(s) so they don't creep in via any other find logic
    "-DCMAKE_IGNORE_PREFIX_PATH:PATH=C:/Program Files/Python313;C:/Program Files (x86)/Python*"
  )
endif()

if(WIN32)
  list(APPEND SCIRUN_CACHE_ARGS "-DSCIRUN_SHOW_CONSOLE:BOOL=${SCIRUN_SHOW_CONSOLE}")
endif()

# FreeType essential hints (include & lib paths explicitly)
if(TARGET Freetype_external)
  ExternalProject_Get_Property(Freetype_external INSTALL_DIR)
  set(FREETYPE_INSTALL_DIR "${INSTALL_DIR}")

  # Primary include (contains ft2build.h) and the 'freetype2' sub-include
  set(Freetype_INCLUDE_DIR       "${FREETYPE_INSTALL_DIR}/include")
  set(FREETYPE_INCLUDE_DIR2      "${FREETYPE_INSTALL_DIR}/include/freetype2")

  # Library directory (freetype.lib / libfreetype.{a,so,dylib})
  # If you set CMAKE_INSTALL_LIBDIR=lib in the external, this is stable:
  set(Freetype_LIB_DIR           "${FREETYPE_INSTALL_DIR}/lib")

  # Append cache args consumed by the inner SCIRun configure
  list(APPEND SCIRUN_CACHE_ARGS
    "-DFreetype_INCLUDE_DIR:PATH=${Freetype_INCLUDE_DIR}"
    "-DFREETYPE_INCLUDE_DIR2:PATH=${FREETYPE_INCLUDE_DIR2}"
    "-DFreetype_LIB_DIR:PATH=${Freetype_LIB_DIR}"
  )

  # If the external produced a *Config.cmake, pass its DIR like you do for Zlib.
  # Prefer an explicit Freetype_DIR if the superbuild has one; else export the common location.
  if(DEFINED Freetype_DIR)
    list(APPEND SCIRUN_CACHE_ARGS "-DFreetype_DIR:PATH=${Freetype_DIR}")
  else()
    # Helper you already use for Zlib to export a config dir that the inner find_package() can pick up.
    # Adjust the suffix if your FreeType external installs config files elsewhere.
    _export_config_dir(Freetype Freetype_external "lib/cmake/freetype")
  endif()

  # (Optional) If you also maintain legacy alias variables for consistency with older code:
  set(SCI_FREETYPE_INCLUDE "${Freetype_INCLUDE_DIR}" CACHE PATH "Legacy: FreeType include dir (ft2build.h)" FORCE)
  set(SCI_FREETYPE_INCLUDE2 "${FREETYPE_INCLUDE_DIR2}" CACHE PATH "Legacy: FreeType include dir (freetype2)" FORCE)
  set(SCI_FREETYPE_LIBRARY_DIR "${Freetype_LIB_DIR}" CACHE PATH "Legacy: FreeType library dir" FORCE)

  # And push those legacy names too if you want them available in the inner cache:
  list(APPEND SCIRUN_CACHE_ARGS
    "-DSCI_FREETYPE_INCLUDE:PATH=${SCI_FREETYPE_INCLUDE}"
    "-DSCI_FREETYPE_INCLUDE2:PATH=${SCI_FREETYPE_INCLUDE2}"
    "-DSCI_FREETYPE_LIBRARY_DIR:PATH=${SCI_FREETYPE_LIBRARY_DIR}"
  )
endif()

# Zlib + Boost essential hints (include & lib paths explicitly)
if(TARGET Zlib_external)
  ExternalProject_Get_Property(Zlib_external INSTALL_DIR)
  set(ZLIB_INSTALL_DIR "${INSTALL_DIR}")

  # Existing hints you already pass:
  list(APPEND SCIRUN_CACHE_ARGS
    "-DZLIB_ROOT:PATH=${ZLIB_INSTALL_DIR}"
    "-DZLIB_INCLUDE_DIR:PATH=${ZLIB_INSTALL_DIR}/include"
    "-DZLIB_USE_STATIC_LIBS:BOOL=ON"
  )

  # New: pass the ACTUAL library file (full path). Adjust filename to your build.
  if(WIN32)
    # If you built static zlib as zlibstatic.lib, set that; otherwise zlib.lib or zlib1.lib
    if(EXISTS "${ZLIB_INSTALL_DIR}/lib/zlibstatic.lib")
      set(_zlib_lib "${ZLIB_INSTALL_DIR}/lib/zlibstatic.lib")
    elseif(EXISTS "${ZLIB_INSTALL_DIR}/lib/zlib.lib")
      set(_zlib_lib "${ZLIB_INSTALL_DIR}/lib/zlib.lib")
    elseif(EXISTS "${ZLIB_INSTALL_DIR}/lib/zlib1.lib")
      set(_zlib_lib "${ZLIB_INSTALL_DIR}/lib/zlib1.lib")
    endif()
  elseif(APPLE)
    if(EXISTS "${ZLIB_INSTALL_DIR}/lib/libz.a")
      set(_zlib_lib "${ZLIB_INSTALL_DIR}/lib/libz.a")
    elseif(EXISTS "${ZLIB_INSTALL_DIR}/lib/libz.dylib")
      set(_zlib_lib "${ZLIB_INSTALL_DIR}/lib/libz.dylib")
    endif()
  else() # Linux/Unix
    if(EXISTS "${ZLIB_INSTALL_DIR}/lib/libz.a")
      set(_zlib_lib "${ZLIB_INSTALL_DIR}/lib/libz.a")
    elseif(EXISTS "${ZLIB_INSTALL_DIR}/lib/libz.so")
      set(_zlib_lib "${ZLIB_INSTALL_DIR}/lib/libz.so")
    elseif(EXISTS "${ZLIB_INSTALL_DIR}/lib64/libz.so")
      set(_zlib_lib "${ZLIB_INSTALL_DIR}/lib64/libz.so")
    endif()
  endif()

  if(DEFINED _zlib_lib AND EXISTS "${_zlib_lib}")
    list(APPEND SCIRUN_CACHE_ARGS "-DZLIB_LIBRARY:FILEPATH=${_zlib_lib}")
    message(STATUS "[superbuild] Zlib library: ${_zlib_lib}")
  else()
    message(WARNING "[superbuild] Could not resolve ZLIB_LIBRARY at configure time; inner build may still find it via ZLIB_ROOT/CMAKE_PREFIX_PATH.")
  endif()

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

# ===== Eigen (special-case exporter to prefer include/eigen3) =====
if(TARGET Eigen_external)
  ExternalProject_Get_Property(Eigen_external INSTALL_DIR)

  # Prefer the CMake-installed Eigen layout
  set(_eigen_inc "${INSTALL_DIR}/include/eigen3")
  if(NOT EXISTS "${_eigen_inc}/Eigen/Dense")
    # Legacy fallback when copy-only install was used.
    # Strongly recommend switching your Eigen external to CMake install.
    set(_eigen_inc "${INSTALL_DIR}/include")
  endif()

  # Pass normalized include to SCIRun (it will also verify 'unsupported' exists)
  list(APPEND SCIRUN_CACHE_ARGS
    "-DEigen_INCLUDE_DIR:PATH=${_eigen_inc}"
    "-DSCIRUN_EIGEN_INCLUDE:PATH=${_eigen_inc}"
  )

  # Ensure SCIRun sees Eigen's prefix in CMAKE_PREFIX_PATH
  sb_prefix_append("${INSTALL_DIR}")
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
# ---- Cleaver2 uppercase aliases for inner CMake (legacy/consistent names) ----
if(DEFINED Cleaver2_INCLUDE_DIR)
  list(APPEND SCIRUN_CACHE_ARGS
    "-DCLEAVER2_INCLUDE:PATH=${Cleaver2_INCLUDE_DIR}"
    "-DCLEAVER2_INCLUDE_DIR:PATH=${Cleaver2_INCLUDE_DIR}"
  )
endif()
if(DEFINED Cleaver2_LIB_DIR)
  list(APPEND SCIRUN_CACHE_ARGS
    "-DCLEAVER2_LIBRARY_DIR:PATH=${Cleaver2_LIB_DIR}"
    "-DCLEAVER2_LIB_DIR:PATH=${Cleaver2_LIB_DIR}"
  )
endif()
# Library logical name (import lib on Windows will be resolved by consumers)
list(APPEND SCIRUN_CACHE_ARGS
  "-DCLEAVER2_LIBRARY:STRING=cleaver2"
)
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

if(TARGET Boost_external AND TARGET Python_external)
  add_dependencies(Boost_external Python_external)
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

# --- Wait for Eigen headers (including unsupported/Tensor) before SCIRun configure ---
if(TARGET Eigen_external)
  ExternalProject_Get_Property(Eigen_external INSTALL_DIR)
  set(_eigen_inc "${INSTALL_DIR}/include/eigen3")  # assume modern layout produced by CMake install

  # Do not check EXISTS here—let ExternalProject step handle availability at build time.
  _sb_scirun_wait_for(NAME eigen
    FILES
      "${_eigen_inc}/Eigen/Dense"
      "${_eigen_inc}/unsupported/Eigen/CXX11/Tensor"
    DIRS  "${_eigen_inc}"
  )
endif()

# --- Gate SCIRun configure on Cleaver2 headers (vec3.h) ---
if(TARGET Cleaver2_external)
  # Expose a phony target for the copy step so we can depend on it
  ExternalProject_Add_StepTargets(Cleaver2_external copy_headers)

  ExternalProject_Get_Property(Cleaver2_external INSTALL_DIR)
  _sb_scirun_wait_for(NAME cleaver2
    FILES "${INSTALL_DIR}/include/cleaver2/vec3.h"
    DIRS  "${INSTALL_DIR}/include/cleaver2"
  )

  # Ensure SCIRun waits specifically for the header copy step to complete
  add_dependencies(SCIRun_external Cleaver2_external-copy_headers)
endif()
