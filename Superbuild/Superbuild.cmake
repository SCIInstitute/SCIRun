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

###########################################
# Set default CMAKE_BUILD_TYPE
# if empty for Unix Makefile builds
IF(CMAKE_GENERATOR MATCHES "Unix Makefiles" AND NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF()

FIND_PACKAGE(Git)

IF(NOT GIT_FOUND)
  MESSAGE(ERROR "Cannot find Git. Git is required for SCIRun's Superbuild")
ENDIF()

INCLUDE( ExternalProject )

###########################################
# DETERMINE ARCHITECTURE
# In order for the code to depend on the architecture settings
###########################################

IF(CMAKE_SIZEOF_VOID_P MATCHES 8)
  SET(SCIRUN_BITS 64)
ELSE()
  SET(SCIRUN_BITS 32)
ENDIF()

###########################################
# Configure test support
OPTION(BUILD_TESTING "Build with tests." OFF)

###########################################
# Configure compilation database generation
OPTION(GENERATE_COMPILATION_DATABASE "Generate Compilation Database." ON)

###########################################
# Configure python
OPTION(BUILD_WITH_PYTHON "Build with python support." ON)

###########################################
# Configure tetgen
OPTION(WITH_TETGEN "Build Tetgen." ON)

###########################################
# Configure ospray
OPTION(WITH_OSPRAY "Build Ospray." OFF)

###########################################
# Configure data
OPTION(BUILD_WITH_SCIRUN_DATA "Svn checkout data" OFF)

###########################################
# Configure Windows executable to run with
# or without the console

IF(WIN32)
  OPTION(SCIRUN_SHOW_CONSOLE "Show Windows console when running SCIRun (useful for debugging)." ON)
  MARK_AS_ADVANCED(SCIRUN_SHOW_CONSOLE)
ENDIF()

###########################################
# Configure headless build
OPTION(BUILD_HEADLESS "Build SCIRun without GUI." OFF)


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
set(Qt_PATH "" CACHE PATH "Path to Qt installation (e.g. C:/Qt/6.7.1/msvc2022_64)")

if (NOT BUILD_HEADLESS)

    if (NOT IS_DIRECTORY "${Qt_PATH}")
        message(FATAL_ERROR "Qt_PATH is invalid. Point it to the Qt install prefix that contains the lib/, bin/, and cmake/ directories.")
    endif()

    # Qt expects the directory containing Qt6Config.cmake or Qt5Config.cmake
    if (SCIRUN_QT_MAJOR STREQUAL "6")
        set(QT_MIN_VERSION ${QT6_MIN_VERSION})
        list(APPEND CMAKE_PREFIX_PATH "${Qt_PATH}")

        find_package(Qt6 ${QT_MIN_VERSION} COMPONENTS
            Core
            Gui
            Widgets
            Network
            Concurrent
            PrintSupport
            Svg
            OpenGL
            OpenGLWidgets
            REQUIRED
        )

        message(STATUS "Using Qt6 found at: ${Qt6_DIR}")

    elseif (SCIRUN_QT_MAJOR STREQUAL "5")
        set(QT_MIN_VERSION ${QT5_MIN_VERSION})
        list(APPEND CMAKE_PREFIX_PATH "${Qt_PATH}")

        find_package(Qt5 ${QT_MIN_VERSION} COMPONENTS
            Core
            Gui
            Widgets
            Network
            Concurrent
            PrintSupport
            Svg
            OpenGL
            REQUIRED
        )

        message(STATUS "Using Qt5 found at: ${Qt5_DIR}")

    else()
        message(FATAL_ERROR "SCIRUN_QT_MAJOR must be '5' or '6'.")
    endif()

    if(SCIRUN_QT_MAJOR STREQUAL "6")
      if(TARGET Qt6::qmake)
        get_target_property(_qmake_path Qt6::qmake LOCATION)
      else()
        # Fallback: derive from Qt6_DIR if the qmake target is missing
        # Qt6_DIR typically points to .../lib/cmake/Qt6
        # qmake is usually under the corresponding bin/
        get_filename_component(_qt6_cmake_dir "${Qt6_DIR}" DIRECTORY)         # .../lib/cmake
        get_filename_component(_qt6_prefix    "${_qt6_cmake_dir}" DIRECTORY)  # .../lib
        get_filename_component(_qt6_root      "${_qt6_prefix}" DIRECTORY)     # ...
        if(WIN32)
          set(_qmake_path "${_qt6_root}/bin/qmake.exe")
        else()
          set(_qmake_path "${_qt6_root}/bin/qmake")
        endif()
      endif()
    elseif(SCIRUN_QT_MAJOR STREQUAL "5")
      if(TARGET Qt5::qmake)
        get_target_property(_qmake_path Qt5::qmake LOCATION)
      else()
        get_filename_component(_qt5_cmake_dir "${Qt5_DIR}" DIRECTORY)
        get_filename_component(_qt5_prefix    "${_qt5_cmake_dir}" DIRECTORY)
        get_filename_component(_qt5_root      "${_qt5_prefix}" DIRECTORY)
        if(WIN32)
          set(_qmake_path "${_qt5_root}/bin/qmake.exe")
        else()
          set(_qmake_path "${_qt5_root}/bin/qmake")
        endif()
      endif()
    endif()
    
    # Validate and publish for QwtExternal.cmake to use
    if(NOT _qmake_path OR NOT EXISTS "${_qmake_path}")
      message(FATAL_ERROR "Could not locate qmake. Checked: ${_qmake_path}")
    endif()
    
    # Make it visible to the external (cache it so EPs see it in their configure env)
    set(QT_QMAKE_EXECUTABLE "${_qmake_path}" CACHE FILEPATH "Path to Qt qmake for building Qwt" FORCE)
    
    message(STATUS "QT_QMAKE_EXECUTABLE = ${QT_QMAKE_EXECUTABLE}")
else()
    add_definitions(-DBUILD_HEADLESS)
endif()


###########################################
# Configure Doxygen documentation
OPTION(BUILD_DOCUMENTATION "Build documentation" OFF)
MARK_AS_ADVANCED(BUILD_DOCUMENTATION)

########################################################################
# Factory code generation options

OPTION(GENERATE_MODULE_FACTORY_CODE "Generate module factory code from config files" ON)
MARK_AS_ADVANCED(GENERATE_MODULE_FACTORY_CODE)

OPTION(REGENERATE_MODULE_FACTORY_CODE "Delete generated module factory code file in order to refresh module list" ON)
MARK_AS_ADVANCED(REGENERATE_MODULE_FACTORY_CODE)

###########################################
# Configure externals

SET( SCIRun_DEPENDENCIES )

MACRO(ADD_EXTERNAL cmake_file external)
  INCLUDE( ${cmake_file} )
  LIST(APPEND SCIRun_DEPENDENCIES ${external})
ENDMACRO()

SET(SUPERBUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
SET(SCIRUN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src CACHE INTERNAL "" FORCE)
SET(SCIRUN_BINARY_DIR ${CMAKE_BINARY_DIR}/SCIRun CACHE INTERNAL "" FORCE)

IF(BUILD_TESTING)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/TestDataConfig.cmake SCIRunTestData_external )
ENDIF()

ADD_EXTERNAL( ${SUPERBUILD_DIR}/EigenExternal.cmake Eigen_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/ZlibExternal.cmake Zlib_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/SQLiteExternal.cmake SQLite_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/TeemExternal.cmake Teem_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/FreetypeExternal.cmake Freetype_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/GLMExternal.cmake GLM_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/SpdLogExternal.cmake SpdLog_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/TnyExternal.cmake Tny_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/LodePngExternal.cmake LodePng_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/Cleaver2External.cmake Cleaver2_external )

IF(WIN32)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/GlewExternal.cmake Glew_external )
ENDIF()

IF(BUILD_WITH_PYTHON)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/PythonExternal.cmake Python_external )
ENDIF()

FIND_PACKAGE(Subversion)
IF(NOT Subversion_FOUND)
  SET(BUILD_WITH_SCIRUN_DATA OFF)
ENDIF()
IF(BUILD_WITH_SCIRUN_DATA)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/SCIRunDataExternal.cmake SCI_data_external)
ENDIF()

IF(WITH_TETGEN)
  MESSAGE(STATUS "Configuring Tetgen library under GPL. The SCIRun InterfaceWithTetGen module can be disabled by setting the CMake build variable WITH_TETGEN to OFF.")
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/TetgenExternal.cmake Tetgen_external )
ENDIF()

IF(WITH_OSPRAY)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/OsprayExternal.cmake Ospray_external )
ENDIF()

IF(NOT BUILD_HEADLESS)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/QwtExternal.cmake Qwt_external )
  #ADD_EXTERNAL( ${SUPERBUILD_DIR}/CtkExternal.cmake Ctk_external )
ENDIF()

ADD_EXTERNAL( ${SUPERBUILD_DIR}/BoostExternal.cmake Boost_external )


# Helper to export a CONFIG package directory to SCIRun_CACHE_ARGS
function(_export_config_dir pkg target subdir_pattern)
  # Get the external's install prefix
  ExternalProject_Get_Property(${target} INSTALL_DIR)
  if(NOT INSTALL_DIR)
    message(FATAL_ERROR "INSTALL_DIR not set for ${target}. Ensure INSTALL is enabled in its ExternalProject_Add.")
  endif()

  # The folder that contains <Pkg>Config.cmake varies slightly by project.
  # Common patterns: lib/cmake/<Pkg>, lib/cmake/<PKG_UPPER>, share/<Pkg>/cmake
  set(_candidates
    "${INSTALL_DIR}/lib/cmake/${pkg}"
    "${INSTALL_DIR}/lib/cmake/${pkg}-*"
    "${INSTALL_DIR}/lib/cmake/${pkg_upper}"
    "${INSTALL_DIR}/share/${pkg}/cmake"
  )

  string(TOUPPER "${pkg}" pkg_upper)
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

  # Export <Pkg>_DIR to the SCIRun configure cache
  set(${pkg}_DIR "${_found_dir}" CACHE PATH "${pkg} config dir" FORCE)
  list(APPEND SCIRun_CACHE_ARGS "-D${pkg}_DIR:PATH=${${pkg}_DIR}")
  set(SCIRun_CACHE_ARGS "${SCIRun_CACHE_ARGS}" PARENT_SCOPE)

  message(STATUS "[superbuild] ${pkg}_DIR = ${${pkg}_DIR}")
endfunction()

# Helper to export just include/lib hints (for header-only or non-config packages)
function(_export_include_lib pkg target)
  ExternalProject_Get_Property(${target} INSTALL_DIR)
  if(NOT INSTALL_DIR)
    message(FATAL_ERROR "INSTALL_DIR not set for ${target}.")
  endif()

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

#get boost properties and pass to SCIRun
ExternalProject_Get_Property(Boost_external INSTALL_DIR)
set(Boost_DIR "${INSTALL_DIR}/lib/cmake/Boost")
set(SCI_BOOST_INCLUDE "${INSTALL_DIR}/include")
set(SCI_BOOST_PREFIX "${INSTALL_DIR}" CACHE PATH "Boost install prefix (produced by Boost_external)" FORCE)
if (WIN32 AND EXISTS "${INSTALL_DIR}/lib64")
  set(SCI_BOOST_LIBRARY_DIR "${INSTALL_DIR}/lib64")
else()
  set(SCI_BOOST_LIBRARY_DIR "${INSTALL_DIR}/lib")
endif()

# CONFIG-package externals (expect *Config.cmake)
# Adjust names if the actual package names differ.
_export_config_dir(Zlib       Zlib_external       "<auto>")
_export_config_dir(Freetype   Freetype_external   "<auto>")
_export_config_dir(SQLite     SQLite_external     "<auto>")

# Qwt: do not try to 'find' files at configure time; define an imported target
# that will point to the expected install location(s) and depend on the EP.

# (Optional) This helper won't find anything for Qwt (no CMake config), but harmless
_export_config_dir(Qwt Qwt_external "<auto>")

# Get the external's install prefix
ExternalProject_Get_Property(Qwt_external INSTALL_DIR)
set(QWT_INSTALL_DIR "${INSTALL_DIR}")

# Include & lib dirs as installed by qmake
set(QWT_INCLUDE_DIR "${QWT_INSTALL_DIR}/include")
set(QWT_LIBRARY_DIR "${QWT_INSTALL_DIR}/lib")

# Create an imported target that *will* exist after the external builds.
# Use per-config locations on Windows to handle qwt.lib (Release) and qwtd.lib (Debug).
add_library(Qwt::Qwt UNKNOWN IMPORTED GLOBAL)
add_dependencies(Qwt::Qwt Qwt_external)

# Include directories (Qwt typically installs headers under include/ or include/qwt-<ver>)
set_property(TARGET Qwt::Qwt PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES
    "${QWT_INCLUDE_DIR}"
)

if(WIN32)
  # Windows/MSVC typically uses a 'd' suffix for Debug builds.
  set_property(TARGET Qwt::Qwt PROPERTY IMPORTED_CONFIGURATIONS "Debug;Release")

  set_property(TARGET Qwt::Qwt PROPERTY
    IMPORTED_LOCATION_RELEASE "${QWT_LIBRARY_DIR}/qwt.lib")
  # Try qwtd.lib first; if your build produces qwt.lib for Debug too, you can
  # set both to qwt.lib and it will still work.
  set_property(TARGET Qwt::Qwt PROPERTY
    IMPORTED_LOCATION_DEBUG   "${QWT_LIBRARY_DIR}/qwtd.lib")

  # If you prefer a single generator expression instead of config-specific props,
  # you could do:
  # set_property(TARGET Qwt::Qwt PROPERTY
  #   IMPORTED_LOCATION "$<IF:$<CONFIG:Debug>,${QWT_LIBRARY_DIR}/qwtd.lib,${QWT_LIBRARY_DIR}/qwt.lib>")
elseif(APPLE)
  # Prefer shared if present, otherwise static (the file will appear at build time)
  set_property(TARGET Qwt::Qwt PROPERTY
    IMPORTED_LOCATION "${QWT_LIBRARY_DIR}/libqwt.dylib")
  # Optional: provide a static fallback for environments that build static
  # set_property(TARGET Qwt::Qwt PROPERTY
  #   IMPORTED_LOCATION "${QWT_LIBRARY_DIR}/libqwt.a")
else()
  # Linux/*nix: prefer shared, static as fallback
  # If you know you're building static-only, you can point straight to libqwt.a
  set_property(TARGET Qwt::Qwt PROPERTY
    IMPORTED_LOCATION "${QWT_LIBRARY_DIR}/libqwt.so")
  # Optional static fallback:
  # set_property(TARGET Qwt::Qwt PROPERTY
  #   IMPORTED_LOCATION "${QWT_LIBRARY_DIR}/libqwt.a")
endif()

# IMPORTANT: Do not 'EXISTS'-check the files at configure time. They'll be created during build.
# Now consumers can safely link:
# target_link_libraries(SCIRunGui PRIVATE Qwt::Qwt)

# GLEW: depends on its CMake; if it exports configs, use config; otherwise include/lib
if(WIN32)
  _export_config_dir(GLEW Glew_external "<auto>")
endif()

# Python: if your PythonExternal builds a CPython with a CMake export, use config;
# otherwise export include/lib and PYTHONHOME-ish prefix for embedding.
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
  # OSPRay typically has a config; try config first, fall back to include/lib
  _export_config_dir(ospray Ospray_external "<auto>")
endif()
if(BUILD_TESTING)
  # Test data external likely doesn't export headers/libs; skip
endif()

###########################################
# Download external data sources
OPTION(DOWNLOAD_TOOLKITS "Download toolkit repositories." ON)
MARK_AS_ADVANCED(DOWNLOAD_TOOLKITS)
IF(DOWNLOAD_TOOLKITS)
  INCLUDE(${SUPERBUILD_DIR}/ToolkitsConfig.cmake)
  EXTERNAL_TOOLKIT(BrainStimulator)
  EXTERNAL_TOOLKIT(FwdInvToolkit)
ENDIF()


# --- SCIRUN CACHE ARGUMENTS (SCIRun internal settings only) ---
set(SCIRUN_CACHE_ARGS
    "-DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}"
    "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
    "-DSCIRUN_BINARY_DIR:PATH=${SCIRUN_BINARY_DIR}"
    "-DSCIRUN_BITS:STRING=${SCIRUN_BITS}"
    "-DBUILD_TESTING:BOOL=${BUILD_TESTING}"
    "-DBUILD_DOCUMENTATION:BOOL=${BUILD_DOCUMENTATION}"
    "-DBUILD_HEADLESS:BOOL=${BUILD_HEADLESS}"
    "-DQT_VERSION_MAJOR:STRING=${QT_VERSION_MAJOR}"
    "-DSCIRUN_TEST_RESOURCE_DIR:PATH=${SCIRUN_TEST_RESOURCE_DIR}"
    "-DBUILD_WITH_PYTHON:BOOL=${BUILD_WITH_PYTHON}"
    "-DUSER_PYTHON_VERSION:STRING=${USER_PYTHON_VERSION}"
    "-DUSER_PYTHON_VERSION_MAJOR:STRING=${USER_PYTHON_VERSION_MAJOR}"
    "-DUSER_PYTHON_VERSION_MINOR:STRING=${USER_PYTHON_VERSION_MINOR}"
    "-DWITH_TETGEN:BOOL=${WITH_TETGEN}"
    "-DWITH_OSPRAY:BOOL=${WITH_OSPRAY}"
    "-DREGENERATE_MODULE_FACTORY_CODE:BOOL=${REGENERATE_MODULE_FACTORY_CODE}"
    "-DGENERATE_MODULE_FACTORY_CODE:BOOL=${GENERATE_MODULE_FACTORY_CODE}"
    "-DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}"
    "-DGENERATE_COMPILATION_DATABASE:BOOL=${GENERATE_COMPILATION_DATABASE}"
    "-DSCIRUN_QT_MAJOR:STRING=${SCIRUN_QT_MAJOR}"
    "-DQt_PATH:PATH=${Qt_PATH}"
)

if(BUILD_WITH_PYTHON)
  list(APPEND SCIRUN_CACHE_ARGS
    "-DPYTHON_EXECUTABLE:FILEPATH=${SCI_PYTHON_EXE}"
  )
endif()

if(WIN32)
  list(APPEND SCIRUN_CACHE_ARGS
    "-DSCIRUN_SHOW_CONSOLE:BOOL=${SCIRUN_SHOW_CONSOLE}"
  )
endif()

if(NOT BUILD_HEADLESS)
  list(APPEND SCIRUN_CACHE_ARGS
    "-DQt_PATH:PATH=${Qt_PATH}"
    "-DQt${QT_VERSION_MAJOR}Core_DIR:PATH=${Qt${QT_VERSION_MAJOR}Core_DIR}"
    "-DQt${QT_VERSION_MAJOR}CoreTools_DIR:PATH=${Qt${QT_VERSION_MAJOR}CoreTools_DIR}"
    "-DQt${QT_VERSION_MAJOR}Gui_DIR:PATH=${Qt${QT_VERSION_MAJOR}Gui_DIR}"
    "-DQt${QT_VERSION_MAJOR}GuiTools_DIR:PATH=${Qt${QT_VERSION_MAJOR}GuiTools_DIR}"
    "-DQt${QT_VERSION_MAJOR}OpenGL_DIR:PATH=${Qt${QT_VERSION_MAJOR}OpenGL_DIR}"
    "-DQt${QT_VERSION_MAJOR}Network_DIR:PATH=${Qt${QT_VERSION_MAJOR}Network_DIR}"
    "-DQt${QT_VERSION_MAJOR}Widgets_DIR:PATH=${Qt${QT_VERSION_MAJOR}Widgets_DIR}"
    "-DQt${QT_VERSION_MAJOR}Concurrent_DIR:PATH=${Qt${QT_VERSION_MAJOR}Concurrent_DIR}"
    "-DMACDEPLOYQT_OUTPUT_LEVEL:STRING=${MACDEPLOYQT_OUTPUT_LEVEL}"
  )
endif()

# Ensure ZLIB_INSTALL_DIR is already cached by ZlibExternal.cmake:
#   set(ZLIB_INSTALL_DIR "<...>/Externals/Install/Zlib_external" CACHE PATH ... FORCE)

list(APPEND SCIRUN_CACHE_ARGS
  "-DCMAKE_PREFIX_PATH:PATH=${ZLIB_INSTALL_DIR}"
  "-DZLIB_ROOT:PATH=${ZLIB_INSTALL_DIR}"
  "-DZLIB_INCLUDE_DIR:PATH=${ZLIB_INSTALL_DIR}/include"
  # Optional preference (keeps things deterministic if both static & shared exist):
  "-DZLIB_USE_STATIC_LIBS:BOOL=ON"
)

ExternalProject_Add( SCIRun_external
  DEPENDS ${SCIRun_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SCIRUN_SOURCE_DIR}
  BINARY_DIR ${SCIRUN_BINARY_DIR}
  CMAKE_CACHE_ARGS ${SCIRUN_CACHE_ARGS}
  DEPENDS Zlib_external
  INSTALL_COMMAND ""
)

ExternalProject_Add_Step(SCIRun_external wait_for_zlib
  COMMAND ${CMAKE_COMMAND} -E echo "Waiting for zlib artifacts before SCIRun configure..."
  DEPENDEES download
  DEPENDERS configure
  DEPENDS
    "${ZLIB_INSTALL_DIR}/include/zlib.h"
    "${ZLIB_INSTALL_DIR}/lib"
)