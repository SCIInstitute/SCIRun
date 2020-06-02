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
OPTION(WITH_TETGEN "Build Tetgen." OFF)

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
# Travis CI build needs to be as slim as possible
OPTION(TRAVIS_BUILD "Slim build for Travis CI" OFF)
MARK_AS_ADVANCED(TRAVIS_BUILD)

IF(TRAVIS_BUILD)
  SET(BUILD_TESTING OFF)
  SET(DOWNLOAD_TOOLKITS OFF)
  SET(BUILD_WITH_SCIRUN_DATA OFF)
  IF(APPLE)
    # build everything; qt flag is in travis.yml
  ELSE()
    IF(CMAKE_C_COMPILER_ID MATCHES "GNU")
      SET(BUILD_HEADLESS ON)
      SET(BUILD_WITH_PYTHON OFF)
    ELSE()
      # try building everything with clang!
    ENDIF()
  ENDIF()
ENDIF()

###########################################
# Configure Qt
IF(NOT BUILD_HEADLESS)
  IF(TRAVIS_BUILD OR UNIX)
    SET(QT_MIN_VERSION "5.4")
  ELSE()
    SET(QT_MIN_VERSION "5.13")
  ENDIF()

  SET(Qt5_PATH "" CACHE PATH "Path to directory where Qt 5 is installed. Directory should contain lib and bin subdirectories.")

  IF(IS_DIRECTORY ${Qt5_PATH})
    FIND_PACKAGE(Qt5 ${QT_MIN_VERSION} COMPONENTS Core Gui Widgets Network OpenGL Concurrent REQUIRED HINTS ${Qt5_PATH})
  ELSE()
    MESSAGE(SEND_ERROR "Set Qt5_PATH to directory where Qt 5 is installed (containing lib and bin subdirectories) or set BUILD_HEADLESS to ON.")
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
ADD_EXTERNAL( ${SUPERBUILD_DIR}/LibPNGExternal.cmake LibPNG_external )
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
    "-DSCIRUN_TEST_RESOURCE_DIR:PATH=${SCIRUN_TEST_RESOURCE_DIR}"
    "-DBUILD_WITH_PYTHON:BOOL=${BUILD_WITH_PYTHON}"
    "-DTRAVIS_BUILD:BOOL=${TRAVIS_BUILD}"
    "-DUSER_PYTHON_VERSION:STRING=${USER_PYTHON_VERSION}"
    "-DUSER_PYTHON_VERSION_MAJOR:STRING=${USER_PYTHON_VERSION_MAJOR}"
    "-DUSER_PYTHON_VERSION_MINOR:STRING=${USER_PYTHON_VERSION_MINOR}"
    "-DWITH_TETGEN:BOOL=${WITH_TETGEN}"
    "-DWITH_OSPRAY:BOOL=${WITH_OSPRAY}"
    "-DREGENERATE_MODULE_FACTORY_CODE:BOOL=${REGENERATE_MODULE_FACTORY_CODE}"
    "-DGENERATE_MODULE_FACTORY_CODE:BOOL=${GENERATE_MODULE_FACTORY_CODE}"
    "-DEigen_DIR:PATH=${Eigen_DIR}"
    "-DZlib_DIR:PATH=${Zlib_DIR}"
    "-DLibPNG_DIR:PATH=${LibPNG_DIR}"
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
    "-DQt5_PATH:PATH=${Qt5_PATH}"
    "-DQt5Core_DIR:PATH=${Qt5Core_DIR}"
    "-DQt5Gui_DIR:PATH=${Qt5Gui_DIR}"
    "-DQt5OpenGL_DIR:PATH=${Qt5OpenGL_DIR}"
	  "-DQt5Network_DIR:PATH=${Qt5Network_DIR}"
 	  "-DQt5Widgets_DIR:PATH=${Qt5Widgets_DIR}"
	  "-DQt5Concurrent_DIR:PATH=${Qt5Concurrent_DIR}"
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
