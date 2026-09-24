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
# Shims for renamed/removed options. Must run before any option is read.
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/DeprecatedFlags.cmake)
scirun_removed_option(BUILD_WITH_SCIRUN_DATA  # removed 2026-09; drop shim after next release
  "Its SVN source (gforge.sci.utah.edu) is gone and nothing read the result; see #2672.")
scirun_renamed_option(BUILD_WITH_PYTHON WITH_PYTHON)  # renamed 2026-09 (#2670)
scirun_renamed_option(BUILD_OSPRAY WITH_OSPRAY)        # renamed 2026-09 (#2749)
scirun_removed_option(BUILD_HEADLESS                    # inverted 2026-09 (#2671); FATAL because a stale ON would silently build the GUI
  "It was replaced by WITH_GUI with the opposite sense. Use -DWITH_GUI=OFF for a headless build." FATAL)

###########################################
# TODO: build from archive - Git not used
SET(compress_type "GIT" CACHE INTERNAL "")
SET(ep_base "${CMAKE_BINARY_DIR}/Externals" CACHE INTERNAL "")
SET_PROPERTY(DIRECTORY PROPERTY "EP_BASE" ${ep_base})
SET_PROPERTY(DIRECTORY PROPERTY "EP_UPDATE_DISCONNECTED" TRUE)

###########################################
# Configure python
OPTION(WITH_PYTHON "Build with python support." ON)

###########################################
# Force superbuild Python, prevent system Python binding
###########################################

if(WITH_PYTHON)

  # This is where PythonExternal.cmake will install Python
  set(_SB_PYTHON_PREFIX "${ep_base}/Python_external")

  # Prevent CMake from picking up /usr/bin/python3.x
  set(Python3_FIND_SYSTEM_ONLY OFF CACHE BOOL "" FORCE)
  set(Python3_FIND_REGISTRY NEVER CACHE STRING "" FORCE)
  set(Python3_FIND_UNVERSIONED_NAMES NEVER CACHE STRING "" FORCE)
  set(Python3_FIND_STRATEGY LOCATION CACHE STRING "" FORCE)

  # Predeclare Python location (even before it exists)
  set(Python3_ROOT_DIR "${_SB_PYTHON_PREFIX}" CACHE PATH "" FORCE)

  # These stop FindPython / FindPython3 from falling back
  set(Python_ROOT_DIR "${_SB_PYTHON_PREFIX}" CACHE PATH "" FORCE)

  # Do NOT set Python3_EXECUTABLE yet — it doesn't exist during first configure
  # We only block system discovery here.

endif()

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
# Parallelism for build steps that bypass the make jobserver
#
# Externals that recurse via $(MAKE) inherit the top-level jobserver, so the
# -j passed to the outer make already bounds them and they must NOT be given a
# second, independent job budget. Boost is the exception: b2 is not a make, so
# without an explicit -j it builds every library serially no matter what the
# outer make was told (issue #2617).
#
# build.sh exports CMAKE_BUILD_PARALLEL_LEVEL to match its own -j; honour that
# when set so the two stay in agreement, and fall back to the core count for
# people who configure the Superbuild with cmake directly.
###########################################
IF(NOT DEFINED SUPERBUILD_PARALLEL_JOBS)
  IF(DEFINED ENV{CMAKE_BUILD_PARALLEL_LEVEL})
    SET(_sb_jobs $ENV{CMAKE_BUILD_PARALLEL_LEVEL})
  ELSE()
    INCLUDE(ProcessorCount)
    ProcessorCount(_sb_jobs)
  ENDIF()
  IF(NOT _sb_jobs OR _sb_jobs LESS 1)
    SET(_sb_jobs 1)
  ENDIF()
  SET(SUPERBUILD_PARALLEL_JOBS ${_sb_jobs} CACHE STRING "Parallel jobs for external build steps that do not inherit the make jobserver")
ENDIF()
MESSAGE(STATUS "Superbuild parallel jobs (non-jobserver steps): ${SUPERBUILD_PARALLEL_JOBS}")

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
IF(BUILD_TESTING)
  # Tests live in the inner SCIRun project. Let "ctest" (and ctest --preset)
  # run from this directory too, as it would after add_subdirectory().
  FILE(WRITE "${CMAKE_BINARY_DIR}/CTestTestfile.cmake" "subdirs(\"SCIRun\")\n")
ENDIF()

###########################################
# Configure code coverage (forwarded to the inner SCIRun build)
OPTION(ENABLE_COVERAGE "Build with Clang source-based code coverage instrumentation" OFF)

###########################################
# Configure compilation database generation
OPTION(GENERATE_COMPILATION_DATABASE "Generate Compilation Database." ON)

###########################################
# Configure tetgen
OPTION(WITH_TETGEN "Build Tetgen." ON)

###########################################
# Configure ospray
OPTION(WITH_OSPRAY "Build with OSPRay support (OsprayViewer module)." OFF)
OPTION(PREBUILT_OSPRAY "With WITH_OSPRAY: use an installed OSPRay (find_package) instead of building the external." OFF)

# PREBUILT_OSPRAY alone used to imply the dependency; keep that working.
IF(PREBUILT_OSPRAY AND NOT WITH_OSPRAY)
  SET(WITH_OSPRAY ON CACHE BOOL "Build with OSPRay support (OsprayViewer module)." FORCE)
ENDIF()

###########################################
# Configure vtk
OPTION(WITH_VTK "build VTK" OFF)

###########################################
# Configure Windows executable to run with
# or without the console

IF(WIN32)
  OPTION(SCIRUN_SHOW_CONSOLE "Show Windows console when running SCIRun (useful for debugging)." ON)
  MARK_AS_ADVANCED(SCIRUN_SHOW_CONSOLE)
ENDIF()

###########################################
# Configure GUI (OFF = headless build, no Qt required)
OPTION(WITH_GUI "Build the SCIRun GUI." ON)

###########################################
# Configure Qt

SET(DEFAULT_QT_MIN_VERSION "5.15.2")

set(SCIRUN_QT_MIN_VERSION ${DEFAULT_QT_MIN_VERSION} CACHE STRING "Qt version")
set_property(CACHE SCIRUN_QT_MIN_VERSION PROPERTY STRINGS 5.12.8 5.15.2 6.3.1)
string(REPLACE "." ";" SCIRUN_QT_MIN_VERSION_LIST ${SCIRUN_QT_MIN_VERSION})
list(GET SCIRUN_QT_MIN_VERSION_LIST 0 QT_VERSION_MAJOR)
list(GET SCIRUN_QT_MIN_VERSION_LIST 1 QT_VERSION_MINOR)
list(GET SCIRUN_QT_MIN_VERSION_LIST 2 QT_VERSION_PATCH)

IF(WITH_GUI)

  SET(Qt_PATH "" CACHE PATH
      "Path to directory where Qt is installed. Directory should contain lib and bin subdirectories.")

  # ------------------------------------------------------------
  # Qt package discovery
  # ------------------------------------------------------------
  IF(IS_DIRECTORY "${Qt_PATH}")
    # HINTS reaches Qt6Config but not its nested find_dependency() calls, so
    # Qt 6.3 fails on Qt6CoreTools unless the prefix is on CMAKE_PREFIX_PATH.
    LIST(APPEND CMAKE_PREFIX_PATH "${Qt_PATH}")
    if (QT_VERSION_MAJOR STREQUAL "6")
      FIND_PACKAGE(Qt${QT_VERSION_MAJOR} ${SCIRUN_QT_MIN_VERSION}
        COMPONENTS
          DBus DBusTools
          Core Gui Widgets Network OpenGL Concurrent PrintSupport Svg
          CoreTools GuiTools WidgetsTools OpenGLWidgets
        REQUIRED
        HINTS ${Qt_PATH})
    else()
      FIND_PACKAGE(Qt${QT_VERSION_MAJOR} ${SCIRUN_QT_MIN_VERSION}
        COMPONENTS
          Core Gui Widgets Network OpenGL Concurrent PrintSupport Svg
        REQUIRED
        HINTS ${Qt_PATH})
    endif()
  ELSE()
    MESSAGE(SEND_ERROR
      "Set Qt_PATH to the Qt install prefix (with bin/ and lib/) or set WITH_GUI=OFF.")
  ENDIF()

  # ------------------------------------------------------------
  # macOS-only settings
  # ------------------------------------------------------------
  IF(APPLE)
    SET(MACDEPLOYQT_OUTPUT_LEVEL 0 CACHE STRING
        "Set macdeployqt output level (0–3)")
    MARK_AS_ADVANCED(MACDEPLOYQT_OUTPUT_LEVEL)
  ENDIF()

ENDIF()

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

# Central dependency manifest (pinned versions + source URLs). Must be included
# before any ADD_EXTERNAL call so the *External.cmake files can consume its
# variables. See VERSIONS.cmake for the update process.
INCLUDE( ${SUPERBUILD_DIR}/VERSIONS.cmake )

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
ADD_EXTERNAL( ${SUPERBUILD_DIR}/Libxml2External.cmake Libxml2_external )

IF(WIN32)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/GlewExternal.cmake Glew_external )
ENDIF()

IF(WITH_PYTHON)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/PythonExternal.cmake Python_external )
ENDIF()

IF(WITH_TETGEN)
  MESSAGE(STATUS "Configuring Tetgen library under GPL. The SCIRun InterfaceWithTetGen module can be disabled by setting the CMake build variable WITH_TETGEN to OFF.")
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/TetgenExternal.cmake Tetgen_external )
ENDIF()

IF(WITH_OSPRAY)
  IF(PREBUILT_OSPRAY)
    find_package(ospray 2.10.0 REQUIRED)
  ELSE()
    #INCLUDE(${SUPERBUILD_DIR}/TBBExternal.cmake)
    #INCLUDE(${SUPERBUILD_DIR}/RKCommonExternal.cmake)
    #INCLUDE(${SUPERBUILD_DIR}/EmbreeExternal.cmake)
    ADD_EXTERNAL(${SUPERBUILD_DIR}/OsprayExternal.cmake Ospray_external)
  ENDIF()
ENDIF()

IF(WITH_GUI)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/QwtExternal.cmake Qwt_external )
  #ADD_EXTERNAL( ${SUPERBUILD_DIR}/deprecated/CtkExternal.cmake Ctk_external )
ENDIF()

ADD_EXTERNAL( ${SUPERBUILD_DIR}/BoostExternal.cmake Boost_external )

IF(WITH_VTK)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/VtkExternal.cmake VTK_external )
ENDIF()

###########################################
# Download external data sources
OPTION(DOWNLOAD_TOOLKITS "Download toolkit repositories." ON)
MARK_AS_ADVANCED(DOWNLOAD_TOOLKITS)
IF(DOWNLOAD_TOOLKITS)
  INCLUDE(${SUPERBUILD_DIR}/ToolkitsConfig.cmake)
  EXTERNAL_TOOLKIT(BrainStimulator)
  EXTERNAL_TOOLKIT(FwdInvToolkit)
ENDIF()

SET(SCIRUN_CACHE_ARGS
    "-DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}"
    "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
    "-DCMAKE_C_COMPILER_LAUNCHER:STRING=${CMAKE_C_COMPILER_LAUNCHER}"
    "-DCMAKE_CXX_COMPILER_LAUNCHER:STRING=${CMAKE_CXX_COMPILER_LAUNCHER}"
    "-DSCIRUN_BINARY_DIR:PATH=${SCIRUN_BINARY_DIR}"
    "-DSCIRUN_BITS:STRING=${SCIRUN_BITS}"
    "-DBUILD_TESTING:BOOL=${BUILD_TESTING}"
    "-DENABLE_COVERAGE:BOOL=${ENABLE_COVERAGE}"
    "-DBUILD_DOCUMENTATION:BOOL=${BUILD_DOCUMENTATION}"
    "-DWITH_GUI:BOOL=${WITH_GUI}"
    "-DQT_VERSION_MAJOR:STRING=${QT_VERSION_MAJOR}"
    "-DSCIRUN_TEST_RESOURCE_DIR:PATH=${SCIRUN_TEST_RESOURCE_DIR}"
    "-DWITH_PYTHON:BOOL=${WITH_PYTHON}"
    "-DUSER_PYTHON_VERSION:STRING=${USER_PYTHON_VERSION}"
    "-DUSER_PYTHON_VERSION_MAJOR:STRING=${USER_PYTHON_VERSION_MAJOR}"
    "-DUSER_PYTHON_VERSION_MINOR:STRING=${USER_PYTHON_VERSION_MINOR}"
    "-DWITH_TETGEN:BOOL=${WITH_TETGEN}"
    "-DWITH_OSPRAY:BOOL=${WITH_OSPRAY}"
    "-DWITH_VTK:BOOL=${WITH_VTK}"
    "-DREGENERATE_MODULE_FACTORY_CODE:BOOL=${REGENERATE_MODULE_FACTORY_CODE}"
    "-DGENERATE_MODULE_FACTORY_CODE:BOOL=${GENERATE_MODULE_FACTORY_CODE}"
    "-DEigen_DIR:PATH=${Eigen_DIR}"
    "-DZlib_DIR:PATH=${Zlib_DIR}"
    "-DSQLite_DIR:PATH=${SQLite_DIR}"
    "-DBoost_DIR:PATH=${Boost_DIR}"
    "-DTeem_DIR:PATH=${Teem_DIR}"
    "-DFreetype_DIR:PATH=${Freetype_DIR}"
    "-DGLM_DIR:PATH=${GLM_DIR}"
    "-DSPDLOG_DIR:PATH=${SPDLOG_DIR}"
    "-DTNY_DIR:PATH=${TNY_DIR}"
    "-DGLEW_DIR:PATH=${Glew_DIR}"
    "-DLODEPNG_DIR:PATH=${LODEPNG_DIR}"
    "-DCLEAVER2_DIR:PATH=${CLEAVER2_DIR}"
    "-DLibXML2_DIR:PATH=${LibXML2_DIR}"
    "-DGENERATE_COMPILATION_DATABASE:BOOL=${GENERATE_COMPILATION_DATABASE}"
)

IF(WITH_PYTHON)
  LIST(APPEND SCIRUN_CACHE_ARGS
    "-DPython_DIR:PATH=${Python_DIR}"
    "-DPYTHON_EXECUTABLE:FILEPATH=${SCI_PYTHON_EXE}"
  )
ENDIF()

IF(WITH_TETGEN)
  LIST(APPEND SCIRUN_CACHE_ARGS
    "-DTetgen_DIR:PATH=${Tetgen_DIR}"
  )
ENDIF()

IF(WITH_OSPRAY)
  LIST(APPEND SCIRUN_CACHE_ARGS
    "-DOspray_External_Dir:PATH=${OSPRAY_BUILD_DIR}"
  )
ENDIF()

IF(WITH_VTK)
  LIST(APPEND SCIRUN_CACHE_ARGS
    "-DVTK_External_Dir:PATH=${VTK_INSTALL_DIR}"
  )
ENDIF()

IF(WIN32)
  LIST(APPEND SCIRUN_CACHE_ARGS
    "-DSCIRUN_SHOW_CONSOLE:BOOL=${SCIRUN_SHOW_CONSOLE}"
  )
ENDIF()

IF(WITH_GUI)
  LIST(APPEND SCIRUN_CACHE_ARGS
    "-DQt_PATH:PATH=${Qt_PATH}"
    "-DCMAKE_PREFIX_PATH:PATH=${Qt_PATH}"
    "-DQt${QT_VERSION_MAJOR}Core_DIR:PATH=${Qt${QT_VERSION_MAJOR}Core_DIR}"
    "-DQt${QT_VERSION_MAJOR}CoreTools_DIR:PATH=${Qt${QT_VERSION_MAJOR}CoreTools_DIR}"
    "-DQt${QT_VERSION_MAJOR}Gui_DIR:PATH=${Qt${QT_VERSION_MAJOR}Gui_DIR}"
    "-DQt${QT_VERSION_MAJOR}GuiTools_DIR:PATH=${Qt${QT_VERSION_MAJOR}GuiTools_DIR}"
    "-DQt${QT_VERSION_MAJOR}OpenGL_DIR:PATH=${Qt${QT_VERSION_MAJOR}OpenGL_DIR}"
	  "-DQt${QT_VERSION_MAJOR}Network_DIR:PATH=${Qt${QT_VERSION_MAJOR}Network_DIR}"
 	  "-DQt${QT_VERSION_MAJOR}Widgets_DIR:PATH=${Qt${QT_VERSION_MAJOR}Widgets_DIR}"
	  "-DQt${QT_VERSION_MAJOR}Concurrent_DIR:PATH=${Qt${QT_VERSION_MAJOR}Concurrent_DIR}"
    "-DMACDEPLOYQT_OUTPUT_LEVEL:STRING=${MACDEPLOYQT_OUTPUT_LEVEL}"
    "-DQWT_INCLUDE:PATH=${QWT_INCLUDE}"
    "-DQWT_LIBRARY_DIR:PATH=${QWT_LIBRARY_DIR}"
    "-DQWT_LIBRARY:STRING=${QWT_LIBRARY}"
    "-DQWT_INSTALL_DIR:PATH=${QWT_INSTALL_DIR}"
  )
ENDIF()

ExternalProject_Add( SCIRun_external
  DEPENDS ${SCIRun_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${SCIRUN_SOURCE_DIR}
  BINARY_DIR ${SCIRUN_BINARY_DIR}
  CMAKE_CACHE_ARGS ${SCIRUN_CACHE_ARGS}
  INSTALL_COMMAND ""
)
