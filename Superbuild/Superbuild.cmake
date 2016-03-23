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
OPTION(BUILD_TESTING "Build with tests." ON)

###########################################
# Configure python
OPTION(BUILD_WITH_PYTHON "Build with python support." ON)

###########################################
# Configure python
OPTION(WITH_TETGEN "Build Tetgen." ON)
MARK_AS_ADVANCED(WITH_TETGEN)

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
IF(NOT BUILD_HEADLESS)
  SET(QT_MIN_VERSION "4.8.1")
  INCLUDE(FindQt4)
  FIND_PACKAGE(Qt4 COMPONENTS QtMain QtCore QtGui QtNetwork REQUIRED)
  SET(QT_USE_QTOPENGL TRUE)

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

ADD_EXTERNAL( ${SUPERBUILD_DIR}/ZlibExternal.cmake Zlib_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/SQLiteExternal.cmake SQLite_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/LibPNGExternal.cmake LibPNG_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/TeemExternal.cmake Teem_external )
ADD_EXTERNAL( ${SUPERBUILD_DIR}/FreetypeExternal.cmake Freetype_external )

IF(BUILD_WITH_PYTHON)
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/PythonExternal.cmake Python_external )
ENDIF()

IF(WITH_TETGEN)
  MESSAGE(STATUS "Configuring Tetgen library under GPL. The SCIRun InterfaceWithTetGen module can be disabled by setting the CMake build variable WITH_TETGEN to OFF.")
  ADD_EXTERNAL( ${SUPERBUILD_DIR}/TetgenExternal.cmake Tetgen_external )
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
    "-DWITH_TETGEN:BOOL=${WITH_TETGEN}"
    "-DREGENERATE_MODULE_FACTORY_CODE:BOOL=${REGENERATE_MODULE_FACTORY_CODE}"
    "-DGENERATE_MODULE_FACTORY_CODE:BOOL=${GENERATE_MODULE_FACTORY_CODE}"
    "-DZlib_DIR:PATH=${Zlib_DIR}"
    "-DLibPNG_DIR:PATH=${LibPNG_DIR}"
    "-DSQLite_DIR:PATH=${SQLite_DIR}"
    "-DBoost_DIR:PATH=${Boost_DIR}"
    "-DTeem_DIR:PATH=${Teem_DIR}"
    "-DTetgen_DIR:PATH=${Tetgen_DIR}"
    "-DFreetype_DIR:PATH=${Freetype_DIR}"
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

IF(WIN32)
  LIST(APPEND SCIRUN_CACHE_ARGS
    "-DSCIRUN_SHOW_CONSOLE:BOOL=${SCIRUN_SHOW_CONSOLE}"
  )
ENDIF()

IF(NOT BUILD_HEADLESS)
  LIST(APPEND SCIRUN_CACHE_ARGS
    "-DQT_QMAKE_EXECUTABLE:FILEPATH=${QT_QMAKE_EXECUTABLE}"
    "-DQT_USE_QTOPENGL:BOOL=${QT_USE_QTOPENGL}"
    "-DQT_MIN_VERSION:STRING=${QT_MIN_VERSION}"
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
