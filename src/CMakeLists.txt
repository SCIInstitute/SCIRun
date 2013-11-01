#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2013 Scientific Computing and Imaging Institute,
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
#

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11 FATAL_ERROR)

PROJECT(SCIRun5Prototype)

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


########################################################################
# Build shared libraries by default

OPTION(BUILD_SHARED_LIBS "Build shared libraries" ON)
MARK_AS_ADVANCED(BUILD_SHARED_LIBS)

########################################################################
# Headless build

OPTION(BUILD_HEADLESS "Build headless version, without Qt" OFF)
MARK_AS_ADVANCED(BUILD_HEADLESS)


########################################################################
# Set default CMAKE_BUILD_TYPE
# if empty for Unix Makefile builds

IF (CMAKE_GENERATOR MATCHES "Unix Makefiles" AND NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF (CMAKE_GENERATOR MATCHES "Unix Makefiles" AND NOT CMAKE_BUILD_TYPE)


########################################################################
# SCIRUN version number.

SET(SCIRUN_VERSION_MAJOR "5")
SET(SCIRUN_VERSION_MINOR "0")
SET(SCIRUN_VERSION_PATCH "0")

SET(SCIRUN_VERSION_STRING "${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")


########################################################################
# APPLICATION Information (used by CPack)
#
# Based on Seg3D settings...

SET(APPLICATION_NAME "SCIRunGUIPrototype" CACHE INTERNAL "Application name" FORCE)
# TODO: temporary hack to allow testing
SET(TEST_APPLICATION_NAME "SCIRunGUIPrototype_test" CACHE INTERNAL "Application name" FORCE)
# add others?
SET(APPLICATION_EXTENSION ".net" CACHE INTERNAL "Network file extension" FORCE)
SET(APPLICATION_URL "www.scirun.org" CACHE INTERNAL "Project URL" FORCE)
SET(APPLICATION_VENDOR "SCI" CACHE INTERNAL "Vendor information" FORCE)
SET(APPLICATION_CONTACT "scirun-users@sci.utah.edu" CACHE INTERNAL "Contact information" FORCE)
SET(APPLICATION_WINDOWS_ICON "${CMAKE_SOURCE_DIR}/Interface/Application\\\\scirun-icon-large.ico" CACHE INTERNAL "Windows icon" FORCE)
SET(APPLICATION_OSX_ICON_FULL_PATH "${CMAKE_SOURCE_DIR}/Interface/Application/scirun-icon-large.icns" CACHE INTERNAL "OSX icon" FORCE)
SET(APPLICATION_OSX_ICON "scirun-icon-large.icns" CACHE INTERNAL "OSX icon" FORCE)

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
# Configure test support

INCLUDE( CTest )

# ON by default
IF(BUILD_TESTING)
  ENABLE_TESTING()

  # Hack BUILDNAME to differentiate from regular SCIRun builds
  # TODO: remove when transitioning to production code
  SET(BUILDNAME "${BUILDNAME_SCIRun5Prototype}" CACHE STRING "Name of build on the dashboard")
  MARK_AS_ADVANCED(BUILDNAME)

  OPTION(RUN_UNIT_TESTS "Run gtest unit tests" ON)
  OPTION(RUN_BASIC_REGRESSION_TESTS "Run basic regression tests" ON)

  SET(USER_UNIT_TEST_RESOURCE_DIR "../../../scirun/trunk_ref/SCIRunUnitTestData/SCIRun5/UnitTests" CACHE PATH "Root for unit testing files")

  IF(NOT EXISTS "${USER_UNIT_TEST_RESOURCE_DIR}")
    MESSAGE( WARNING "Unit test resource path does not exist. Please set it correctly to run all the unit tests. Try mapping this svn path: https://gforge.sci.utah.edu/gf/project/sciruntests/scmsvn/?action=browse&path=%2FSCIRunUnitTestData%2FSCIRun5%2FUnitTests%2F" )
  ENDIF()

  ADD_DEFINITIONS(-DTEST_RESOURCE_ROOT_DIR="${USER_UNIT_TEST_RESOURCE_DIR}" -DBUILD_TESTING)
ENDIF()

########################################################################
# Python

OPTION(BUILD_WITH_PYTHON "Build with python support." OFF)

IF(BUILD_WITH_PYTHON)
  MESSAGE(STATUS "Configuring Python")
  SET(PythonLibs_FIND_VERSION "3.2")
  FIND_PACKAGE(PythonLibs REQUIRED)
  IF(NOT PYTHONLIBS_FOUND)
    MESSAGE(FATAL_ERROR "Python 3.3 or 3.2 not found. Set PYTHON_INCLUDE_DIR and PYTHON_LIBRARY to the correct Python installation.")
  ENDIF()

  ADD_DEFINITIONS(-DBOOST_PYTHON_STATIC_LIB)
  ADD_DEFINITIONS(-DBUILD_WITH_PYTHON)
ENDIF(BUILD_WITH_PYTHON)  


########################################################################
# Custom compiler flags and definitions

# TODO: Better compiler check for OS X?
# Try CMAKE_<LANG>_COMPILER_ID MATCHES Clang? Or check for Intel, GNU?
IF(UNIX)
  SET(CXX_LANGUAGE_STANDARD_FLAG "-std=c++11")
  CHECK_CXX_COMPILER_FLAG(${CXX_LANGUAGE_STANDARD_FLAG} USE_CXX11_FLAG)
  IF(NOT USE_CXX11_FLAG)
    SET(CXX_LANGUAGE_STANDARD_FLAG "-std=c++0x")
    CHECK_CXX_COMPILER_FLAG(${CXX_LANGUAGE_STANDARD_FLAG} USE_CXX0x_FLAG)
    IF(NOT USE_CXX0x_FLAG)
      SET(CXX_LANGUAGE_STANDARD_FLAG "")
    ENDIF()
  ENDIF()

  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_LANGUAGE_STANDARD_FLAG} -Wall")
ENDIF()

IF(APPLE)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -ftemplate-depth=256")
  SET(CMAKE_CXX_FLAGS_DEBUG "-Wshorten-64-to-32 ${CMAKE_CXX_FLAGS_DEBUG}")

  ADD_DEFINITIONS(-DGTEST_USE_OWN_TR1_TUPLE=1)
ENDIF()

########################################################################
# Find and configure Git

FIND_PACKAGE(Git)
IF(NOT GIT_FOUND)
  MESSAGE(FATAL_ERROR "Git not found.")
ENDIF()

########################################################################
# Find and configure Awk & Sed  {(OS X only), needed to build app bundle

# should be installed by default, searching system paths only
IF(APPLE)
  FIND_PROGRAM(AWK
    NAMES awk
  )
  MARK_AS_ADVANCED(AWK)

  FIND_PROGRAM(SED
    NAMES sed
  )
  MARK_AS_ADVANCED(SED)
ENDIF()


########################################################################
# Configure documentation

OPTION(BUILD_DOXYGEN_DOCUMENTATION "Generate doxygen-based documentation." OFF)
MARK_AS_ADVANCED(BUILD_DOXYGEN_DOCUMENTATION)

IF(BUILD_DOXYGEN_DOCUMENTATION)
  FIND_PACKAGE(Doxygen)

  IF(DOXYGEN_FOUND)
    ADD_SUBDIRECTORY(Documentation/Doxygen)
  ELSE()
    SET(BUILD_DOXYGEN_DOCUMENTATION OFF CACHE BOOL "Generate doxygen-based documentation." FORCE)
  ENDIF()
ENDIF()

# TODO: uncomment when LaTeX files are available
# OPTION(BUILD_LATEX_DOCUMENTATION "Compile SCIRun LaTeX manuals and tutorials to PDF." OFF)
# MARK_AS_ADVANCED(BUILD_LATEX_DOCUMENTATION)
#
# IF(BUILD_LATEX_DOCUMENTATION)
#   FIND_PACKAGE(LATEX)
#   IF(PDFLATEX_COMPILER)
#     IF(BUILD_DOCUMENTATION)
#     ADD_SUBDIRECTORY(Documentation/Manuals)
#   ENDIF()
# ENDIF()

########################################################################
# External project utilities

OPTION(DISABLE_ExternalProject_UPDATE "Turn off ExternalProject UPDATE_COMMAND property (if not connected to the internet, for example" ON)
MARK_AS_ADVANCED(DISABLE_ExternalProject_UPDATE)
IF(DISABLE_ExternalProject_UPDATE)
  # Skips update command by returning true
  SET(DISABLED_UPDATE "UPDATE_COMMAND" "TRUE")
ENDIF()

########################################################################
# Configure Boost external project

# TODO: make macro

ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB=1)

SET(boost_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Externals/boost")
SET(boost_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Externals/boost")

SET(SCI_BOOST_INCLUDE ${boost_SOURCE_DIR} CACHE INTERNAL "Boost include directories." FORCE)
SET(SCI_BOOST_LIBRARY boost CACHE INTERNAL "Boost library name." FORCE)

IF(WIN32)
#  # See cmake-2.8/Modules/FindBoost.cmake
#  #SET(boost_LIB_PREFIX "lib")
  SET(boost_LIB_PREFIX "")
ELSE()
  SET(boost_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
ENDIF()


SET(boost_LIBRARY_PATH "${boost_BINARY_DIR}/${boost_LIB_PREFIX}${SCI_BOOST_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
# other configuration options are RELWITHDEBINFO and MINSIZEREL
SET(boost_LIBRARY_PATH_RELEASE "${boost_BINARY_DIR}/Release/${boost_LIB_PREFIX}${SCI_BOOST_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
SET(boost_LIBRARY_PATH_DEBUG "${boost_BINARY_DIR}/Debug/${boost_LIB_PREFIX}${SCI_BOOST_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")

# Fixes Mac build for now, without extra boost/python cmake magic.
SET(boost_GIT_TAG "origin/master")

# TODO: fix install step
#
# If CMake ever allows overriding the checkout command or adding flags,
# git checkout -q will silence message about detached head (harmless).
ExternalProject_Add(Boost_external
  PREFIX ${boost_BINARY_DIR}
  GIT_REPOSITORY "https://github.com/SCIInstitute/boost.git"
  GIT_TAG ${boost_GIT_TAG}
  SOURCE_DIR ${boost_SOURCE_DIR}
  BINARY_DIR ${boost_BINARY_DIR}
  DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/boost"
  PATCH_COMMAND ""
  ${DISABLED_UPDATE}
  INSTALL_DIR ""
  INSTALL_COMMAND ""
  #CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${boost_BINARY_DIR}"
  CMAKE_CACHE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DBUILD_PYTHON:BOOL=${BUILD_WITH_PYTHON}
    -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
    -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
)

ADD_LIBRARY(${SCI_BOOST_LIBRARY} STATIC IMPORTED GLOBAL)

# adding Boost as a build target and dependency
#
# TODO: how to make boost include dependent on Boost_external?
ADD_DEPENDENCIES(${SCI_BOOST_LIBRARY} Boost_external)
ADD_DEPENDENCIES(${SCI_BOOST_LIBRARY} ${SCI_BOOST_INCLUDE})

IF (CMAKE_GENERATOR MATCHES "Makefiles")
  SET_TARGET_PROPERTIES(${SCI_BOOST_LIBRARY}
    PROPERTIES
      IMPORTED_LOCATION ${boost_LIBRARY_PATH}
  )
ELSE() # IDEs: Xcode, VS, others...
  SET_TARGET_PROPERTIES(${SCI_BOOST_LIBRARY}
    PROPERTIES
      IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
      IMPORTED_LOCATION_RELEASE ${boost_LIBRARY_PATH_RELEASE}
      IMPORTED_LOCATION_DEBUG ${boost_LIBRARY_PATH_DEBUG}
  )
ENDIF()

INCLUDE_DIRECTORIES(${SCI_BOOST_INCLUDE})


########################################################################
# Configure Spire external project

# Ensure CMAKE_MODULE_PATH includes path to Spire's SpirePM.cmake.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
include(SpirePM)

set(SPIRE_OPTIONS)
IF(WIN32)
  set(SPIRE_OPTIONS ${SPIRE_OPTIONS} "USE_STD_THREADS;OFF")
ELSE()
  set(SPIRE_OPTIONS ${SPIRE_OPTIONS} "USE_STD_THREADS;ON")
  add_definitions("-DSPIRE_USE_STD_THREADS")
ENDIF()

set(SPIRE_OPTIONS ${SPIRE_OPTIONS} "SHADER_OUTPUT_DIR;${CMAKE_CURRENT_BINARY_DIR}/Shaders")
set(SPIRE_OPTIONS ${SPIRE_OPTIONS} "ASSET_OUTPUT_DIR;${CMAKE_CURRENT_BINARY_DIR}/Assets")

# Add core spire library. Either download it or use a local version.
set(SPIRE_CORE "Spire_Core_EP")
Spire_AddCore(${SPIRE_CORE}
  ${SPIRE_OPTIONS}
  GIT_TAG "v0.5.0")
Spire_AddModule(${SPIRE_CORE} SCIRun "https://github.com/iauns/spire-scirun" "e58de83865ed03927ddfb16ab73ff8ff146160b1")

# Include / library directories placed in-scope by Spire_AddCore and Spire_AddModule.
link_directories(${SPIRE_LIBRARY_DIRS})
include_directories(${SPIRE_INCLUDE_DIR})
include_directories(${SPIRE_MODULE_INCLUDE_DIRS})
include_directories(SYSTEM ${SPIRE_3RDPARTY_INCLUDE_DIRS})

# Spire related includes (required for linking on windows)
IF(UNIX)
  IF(APPLE)
    FIND_LIBRARY(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    SET(SCI_SPIRE_EXTRA_LIBRARIES ${CORE_FOUNDATION_FRAMEWORK})
  ENDIF()
ENDIF()


########################################################################
# Find and configure OpenGL

FIND_PACKAGE(OpenGL REQUIRED)

########################################################################
# Find and configure Qt

# TODO: exclude for headless
SET(QT_MIN_VERSION "4.8.0")
FIND_PACKAGE(Qt4 REQUIRED)
SET(QT_USE_QTOPENGL TRUE)

IF(QT_FOUND)
  # TODO: may want to grab Seg3D's custom FindQt4 module and UseQt4 file if
  # build issues haven't been corrected in CMake 2.8.7 and newer

  INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
  INCLUDE(${QT_USE_FILE})
  ADD_DEFINITIONS(${QT_DEFINITIONS})
ELSE()
  MESSAGE(FATAL_ERROR "QT 4.8 or greater is required to build SCIRun5Prototype")
ENDIF()

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)


########################################################################
# Includes

INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES(
  ${SCIRun5Prototype_SOURCE_DIR}/
  ${SCIRun5Prototype_SOURCE_DIR}/include
  ${SCIRun5Prototype_SOURCE_DIR}/Externals/gmock/gtest/include
  ${SCIRun5Prototype_SOURCE_DIR}/Externals/gmock/include
  ${SCI_BOOST_INCLUDE}
  ${SCIRun5Prototype_BINARY_DIR}/
  ${SCIRun5Prototype_BINARY_DIR}/include
  ${SCIRun5Prototype_SOURCE_DIR}/Externals/eigen
  # TODO: set up include variable in log4cpp project
  ${SCIRun5Prototype_SOURCE_DIR}/Externals/log4cpp/include
)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

SET(SCIRUN_CORE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
SET(QT_USE_QTOPENGL TRUE)

########################################################################
# Macros

MACRO(SCIRUN_ADD_UNIT_TEST test)
  ADD_EXECUTABLE(${test} ${ARGN})
  LIST(APPEND tmp ${test} ${UNIT_TEST_LIST})
  SET(UNIT_TEST_LIST ${tmp} CACHE INTERNAL "List of unit tests." FORCE)
ENDMACRO(SCIRUN_ADD_UNIT_TEST)


########################################################################
# Output directories.

SET(SCIRUN_CORE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SCIRUN_CORE_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SCIRUN_CORE_BINARY_DIR} CACHE INTERNAL "Single output directory for building all executables.")
MARK_AS_ADVANCED(CMAKE_LIBRARY_OUTPUT_DIRECTORY CMAKE_RUNTIME_OUTPUT_DIRECTORY)

SET(BUILD_FROM_TOP_LEVEL 1)

ADD_SUBDIRECTORY(Externals)
ADD_SUBDIRECTORY(Interface)
ADD_SUBDIRECTORY(Core)
ADD_SUBDIRECTORY(Dataflow)
ADD_SUBDIRECTORY(Modules)

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
SET(scirun_src_dir "${SCIRun5Prototype_SOURCE_DIR}" CACHE INTERNAL "Path to SCIRun's src directory")
# DV Technically, I don't have access to this variable once we fix building
# these projets separately!!!
SET(scirun_obj_dir ${SCIRun5Prototype_BINARY_DIR} CACHE INTERNAL "Path to SCIRun's binary directory")

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
SET_PROPERTY(TARGET Algorithms_Base_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
SET_PROPERTY(TARGET Algorithms_DataIO       PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_DataIO_Tests PROPERTY FOLDER "Core/Algorithms/Tests")
SET_PROPERTY(TARGET Algorithms_Math         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Math_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
SET_PROPERTY(TARGET Algorithms_Field         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Field_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
SET_PROPERTY(TARGET Algorithms_Factory   PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Core_Algorithms_Legacy_Fields         PROPERTY FOLDER "Core/Algorithms")

SET_PROPERTY(TARGET Dataflow_Network         PROPERTY FOLDER "Dataflow")
SET_PROPERTY(TARGET Dataflow_Network_Tests   PROPERTY FOLDER "Dataflow/Tests")
SET_PROPERTY(TARGET Core_Datatypes         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Datatypes_Mesh         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Mesh_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Base         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Base_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Field         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Legacy_Field_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Basis         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Basis_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Persistent         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Logging         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Logging_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_ConsoleApplication         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_CommandLine         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_CommandLine_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Command         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Command_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Thread         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Thread_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Geometry_Primitives         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Geometry_Primitives_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Application         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Application_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Utils         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Utils_Tests         PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Math         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Math_Tests         PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Exceptions_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Util_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Containers         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Containers_Tests         PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Thread_Legacy         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Serialization_Network         PROPERTY FOLDER "Dataflow/Serialization")
SET_PROPERTY(TARGET Core_Serialization_Network_Tests         PROPERTY FOLDER "Dataflow/Serialization/Tests")
SET_PROPERTY(TARGET Engine_Network         PROPERTY FOLDER "Dataflow/Engine")
SET_PROPERTY(TARGET Engine_Network_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")
SET_PROPERTY(TARGET Engine_Scheduler         PROPERTY FOLDER "Dataflow/Engine")
SET_PROPERTY(TARGET Engine_Scheduler_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")
SET_PROPERTY(TARGET Dataflow_State         PROPERTY FOLDER "Dataflow")
SET_PROPERTY(TARGET Dataflow_State_Tests   PROPERTY FOLDER "Dataflow/Tests")

SET_PROPERTY(TARGET gmock         PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET gmock_main   PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET gtest         PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET gtest_main   PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET Testing_Utils PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET Testing_ModuleTestBase PROPERTY FOLDER "Testing Support")

SET_PROPERTY(TARGET Interface_Modules_Base   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_DataIO   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Math   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Visualization   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Render   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_FiniteElements   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Testing   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_String   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Fields   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Factory   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Application   PROPERTY FOLDER "Interface/Application")

SET_PROPERTY(TARGET Modules_Basic   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Basic_Tests   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Math   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Math_Tests   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Fields   PROPERTY FOLDER "Modules/Fields")
SET_PROPERTY(TARGET Modules_Fields_Tests   PROPERTY FOLDER "Modules/Fields")
SET_PROPERTY(TARGET Modules_DataIO   PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_DataIO_Tests   PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_Render   PROPERTY FOLDER "Modules/Render")
SET_PROPERTY(TARGET Modules_Render_Tests   PROPERTY FOLDER "Modules/Render")
SET_PROPERTY(TARGET Modules_String   PROPERTY FOLDER "Modules/String")
SET_PROPERTY(TARGET Modules_String_Tests   PROPERTY FOLDER "Modules/String")
SET_PROPERTY(TARGET Modules_FiniteElements   PROPERTY FOLDER "Modules/FiniteElements")
SET_PROPERTY(TARGET Modules_FiniteElements_Tests   PROPERTY FOLDER "Modules/FiniteElements")
SET_PROPERTY(TARGET Modules_Visualization   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Modules_Visualization_Tests   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Dataflow_Modules_Legacy_Fields PROPERTY FOLDER "Modules/Fields")
SET_PROPERTY(TARGET Modules_Factory   PROPERTY FOLDER "Modules")

SET_PROPERTY(TARGET Boost_external   PROPERTY FOLDER "Externals")
SET_PROPERTY(TARGET ${SPIRE_CORE}   PROPERTY FOLDER "Externals")
SET_PROPERTY(TARGET spire_spm_SCIRun   PROPERTY FOLDER "Externals")
SET_PROPERTY(TARGET ${SCI_LOG4CPP_LIBRARY} PROPERTY FOLDER "Externals")


IF(BUILD_WITH_PYTHON)
  SET_PROPERTY(TARGET Core_Python         PROPERTY FOLDER "Core")
  SET_PROPERTY(TARGET Core_Python_Tests         PROPERTY FOLDER "Core/Tests")
  SET_PROPERTY(TARGET SCIRunPythonAPI PROPERTY FOLDER "Dataflow/Engine")
  SET_PROPERTY(TARGET Engine_Python_Tests   PROPERTY FOLDER "Dataflow/Engine/Tests")
ENDIF()

########################################################################
# Custom CMake settings for CPack

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Modules")

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

#SET(PACKAGING_ICON "${CMAKE_SOURCE_DIR}/Resources/Icons\\\\package_icon.bmp" CACHE INTERNAL "Packing icon." FORCE )

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

SET(CPACK_PACKAGE_INSTALL_DIRECTORY 
    "${APPLICATION_NAME} ${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")


# other platform settings will be built up from here
IF(APPLE)
 SET(CPACK_PACKAGE_DEFAULT_LOCATION "/Applications/")
 SET(CPACK_TOPLEVEL_TAG "")
 IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   SET(CPACK_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
   SET(CMAKE_INSTALL_PREFIX "/" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
 ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
 SET(CPACK_PACKAGING_INSTALL_PREFIX "/")
 SET(CPACK_STRIP_FILES TRUE)
 SET(CPACK_SOURCE_STRIP_FILES TRUE)
 SET(CPACK_PACKAGE_EXECUTABLES "${APPLICATION_NAME}.app" "${APPLICATION_NAME}")
ENDIF()

INCLUDE(CPack)
