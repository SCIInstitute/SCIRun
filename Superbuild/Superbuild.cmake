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

SET(DEFAULT_QT_MIN_VERSION "5.15.2")

set(SCIRUN_QT_MIN_VERSION ${DEFAULT_QT_MIN_VERSION} CACHE STRING "Qt version")
set_property(CACHE SCIRUN_QT_MIN_VERSION PROPERTY STRINGS 5.12.8 5.15.2 6.3.1)
string(REPLACE "." ";" SCIRUN_QT_MIN_VERSION_LIST ${SCIRUN_QT_MIN_VERSION})
list(GET SCIRUN_QT_MIN_VERSION_LIST 0 QT_VERSION_MAJOR)
list(GET SCIRUN_QT_MIN_VERSION_LIST 1 QT_VERSION_MINOR)
list(GET SCIRUN_QT_MIN_VERSION_LIST 2 QT_VERSION_PATCH)

IF(NOT BUILD_HEADLESS)

  SET(Qt_PATH "" CACHE PATH "Path to directory where Qt is installed. Directory should contain lib and bin subdirectories.")

  IF(IS_DIRECTORY ${Qt_PATH})
    if (${QT_VERSION_MAJOR} STREQUAL "6")
      FIND_PACKAGE(Qt${QT_VERSION_MAJOR} ${SCIRUN_QT_MIN_VERSION} COMPONENTS DBus DBusTools Core Gui Widgets Network OpenGL Concurrent PrintSupport Svg CoreTools GuiTools WidgetsTools OpenGLWidgets REQUIRED HINTS ${Qt_PATH})
    else()
      FIND_PACKAGE(Qt${QT_VERSION_MAJOR} ${SCIRUN_QT_MIN_VERSION} COMPONENTS Core Gui Widgets Network OpenGL Concurrent PrintSupport Svg REQUIRED HINTS ${Qt_PATH})
    endif()
  ELSE()
    MESSAGE(SEND_ERROR "Set Qt_PATH to directory where Qt is installed (containing lib and bin subdirectories) or set BUILD_HEADLESS to ON.")
  ENDIF()

  IF(APPLE)
    SET(MACDEPLOYQT_OUTPUT_LEVEL 0 CACHE STRING "Set macdeployqt output level (0-3)")
    MARK_AS_ADVANCED(MACDEPLOYQT_OUTPUT_LEVEL)
  ENDIF()
ELSE()
  ADD_DEFINITIONS(-DBUILD_HEADLESS)
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
    "-DSCI_DATA_DIR:PATH=${SCI_DATA_DIR}"
    "-DGENERATE_COMPILATION_DATABASE:BOOL=${GENERATE_COMPILATION_DATABASE}"
)

IF(BUILD_WITH_PYTHON)
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

IF(WIN32)
  LIST(APPEND SCIRUN_CACHE_ARGS
    "-DSCIRUN_SHOW_CONSOLE:BOOL=${SCIRUN_SHOW_CONSOLE}"
  )
ENDIF()

IF(NOT BUILD_HEADLESS)
  LIST(APPEND SCIRUN_CACHE_ARGS
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
    "-DQWT_DIR:PATH=${QWT_DIR}"
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
