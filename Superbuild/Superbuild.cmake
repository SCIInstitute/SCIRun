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

# Compute -G arg for configuring external projects with the same CMake generator:
#if(CMAKE_EXTRA_GENERATOR)
#  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
#else()
#  set(gen "${CMAKE_GENERATOR}" )
#endif()

###########################################
# Configure test support
OPTION(BUILD_TESTING "Build with tests." ON)

###########################################
# Configure python
OPTION(BUILD_WITH_PYTHON "Build with python support." ON)

###########################################
# Configure Qt
IF(NOT BUILD_HEADLESS)
  SET(QT_MIN_VERSION "4.6.0")
  INCLUDE(FindQt4)

  IF(QT4_FOUND)
    MESSAGE(STATUS "QTVERSION=${QTVERSION}")
    MESSAGE(STATUS "Found use file: ${QT_USE_FILE}")
    IF(APPLE AND ${QTVERSION} VERSION_EQUAL 4.8 AND ${QTVERSION} VERSION_LESS 4.8.5)
      MESSAGE(WARNING "Qt 4.8 versions earlier than 4.8.3 contain a bug that disables menu items under some circumstances. Upgrade to a more recent version.")
    ENDIF()
  ELSE()
    MESSAGE(FATAL_ERROR "QT ${QT_MIN_VERSION} or later is required for building the SCIRun GUI")
  ENDIF()
ENDIF()


###########################################
# Configure Doxygen documentation
OPTION(BUILD_DOCUMENTATION "Build documentation" OFF)
MARK_AS_ADVANCED(BUILD_DOCUMENTATION)

###########################################
# Configure externals

SET( SCIRun_DEPENDENCIES )

SET(SUPERBUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)
SET(SCIRUN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../src CACHE INTERNAL "" FORCE)
SET(SCIRUN_BINARY_DIR ${CMAKE_BINARY_DIR}/SCIRun CACHE INTERNAL "" FORCE)

IF(BUILD_TESTING)
  INCLUDE( ${SUPERBUILD_DIR}/TestDataConfig.cmake )
  LIST(APPEND SCIRun_DEPENDENCIES SCIRunTestData_external)
ENDIF()

INCLUDE( ${SUPERBUILD_DIR}/ZlibExternal.cmake )
LIST(APPEND SCIRun_DEPENDENCIES Zlib_external)

INCLUDE( ${SUPERBUILD_DIR}/SQLiteExternal.cmake )
LIST(APPEND SCIRun_DEPENDENCIES SQLite_external)

INCLUDE( ${SUPERBUILD_DIR}/LibPNGExternal.cmake )
LIST(APPEND SCIRun_DEPENDENCIES LibPNG_external)

IF(BUILD_WITH_PYTHON)
  INCLUDE( ${SUPERBUILD_DIR}/PythonExternal.cmake )
  LIST(APPEND SCIRun_DEPENDENCIES Python_external)
ENDIF()

INCLUDE( ${SUPERBUILD_DIR}/BoostExternal.cmake )
LIST(APPEND SCIRun_DEPENDENCIES Boost_external)

INCLUDE( ${SUPERBUILD_DIR}/TeemExternal.cmake )
LIST(APPEND SCIRun_DEPENDENCIES Teem_external)

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
    "-DBUILD_LARGE_VOLUME_TOOLS:BOOL=${BUILD_LARGE_VOLUME_TOOLS}"
    "-DBUILD_MOSAIC_TOOLS:BOOL=${BUILD_MOSAIC_TOOLS}"
    "-DSCIRUN_BITS:STRING=${SCIRUN_BITS}"
    "-DBUILD_TESTING:BOOL=${BUILD_TESTING}"
    "-DBUILD_DOCUMENTATION:BOOL=${BUILD_DOCUMENTATION}"
    "-DSCIRUN_BUILD_INTERFACE:BOOL=${SCIRUN_BUILD_INTERFACE}"
    "-DSCIRUN_SHOW_CONSOLE:BOOL=${SCIRUN_SHOW_CONSOLE}"
    "-DSCIRUN_TEST_RESOURCE_DIR:PATH=${SCIRUN_TEST_RESOURCE_DIR}"
    "-DBUILD_WITH_PYTHON:BOOL=${BUILD_WITH_PYTHON}"
    "-DZlib_DIR:PATH=${Zlib_DIR}"
    "-DLibPNG_DIR:PATH=${LibPNG_DIR}"
    "-DSQLite_DIR:PATH=${SQLite_DIR}"
    "-DBoost_DIR:PATH=${Boost_DIR}"
    "-DTeem_DIR:PATH=${Teem_DIR}"
)

IF(BUILD_WITH_PYTHON)
  LIST(APPEND SCIRUN_CACHE_ARGS
    "-DPython_DIR:PATH=${Python_DIR}"
    "-DPYTHON_EXECUTABLE:FILEPATH=${SCI_PYTHON_EXE}"
  )
ENDIF()

IF(SCIRUN_BUILD_INTERFACE)
  LIST(APPEND SCIRUN_CACHE_ARGS
    "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
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
