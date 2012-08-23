#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

PROJECT(SCIRun5Prototype)

########################################################################
# Build shared libraries by default

OPTION(BUILD_SHARED_LIBS "Build shared libraries" ON)
MARK_AS_ADVANCED(BUILD_SHARED_LIBS)

########################################################################
# SCIRUN version number.

SET(SCIRUN_VERSION_MAJOR "5")
SET(SCIRUN_VERSION_MINOR "0")
SET(SCIRUN_VERSION_PATCH "0")

SET(SCIRUN_VERSION_STRING "${SCIRUN_VERSION_MAJOR}.${SCIRUN_VERSION_MINOR}")

########################################################################
# Standard tests

INCLUDE(CMakeBackwardCompatibilityC)
INCLUDE(CMakeBackwardCompatibilityCXX)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckCXXSourceCompiles)

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
ENDIF()


########################################################################
# Find and configure Qt

SET(QT_MIN_VERSION "4.7.0")
FIND_PACKAGE(Qt4 REQUIRED)

IF(QT_FOUND)
  # TODO: configure Qt OpenGL?
  # TODO: may want to grab Seg3D's custom FindQt4 module and UseQt4 file if
  # build issues haven't been corrected in CMake 2.8.7 and newer

  INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
ELSE()
  MESSAGE(FATAL_ERROR "QT 4.7 or greater is required to build SCIRun5Prototype")
ENDIF()

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)


########################################################################
# Find and configure Boost

SET(Boost_ADDITIONAL_VERSIONS "1.42" "1.49" "1.49.0")

#IF(CMAKE_COMPILER_IS_GNUCC AND NOT APPLE AND CMAKE_SIZEOF_VOID_P EQUAL 8)
#  # For 64-bit linux builds, static libraries need to be compiled with -fPIC
#  # (default on OS X), which doesn't seem to be the case for Boost static libs.
#  # Using shared object libraries seems to be the easier solution for now.
#  SET(Boost_USE_STATIC_LIBS OFF)
#ELSE()
#  SET(Boost_USE_STATIC_LIBS ON)
#ENDIF()

SET(Boost_USE_STATIC_LIBS ON)
SET(Boost_USE_MULTITHREADED ON)
SET(Boost_USE_STATIC_RUNTIME OFF)

IF(WIN32 AND MSVC)
  # Test directives only - remove when Boost is added to CMake build
  SET(Boost_DIR "E:/boost_1_49_0")
  SET(BOOST_LIBRARYDIR "E:/git/scirunguiprototype/src/Externals/x64/Release")
ENDIF()

# TODO: eventually replace with CMake project
FIND_PACKAGE(Boost 1.49.0
               REQUIRED
               COMPONENTS date_time filesystem regex system signals serialization thread program_options
)

IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  MESSAGE(STATUS "Boost")
  MESSAGE(STATUS ${Boost_INCLUDE_DIRS})
  SET(SCIRUN5_BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}/)
ELSE()
  MESSAGE(FATAL_ERROR "Boost 1.49 or greater is required to build SCIRun5Prototype")
ENDIF()


INCLUDE_DIRECTORIES(BEFORE ${CMAKE_CURRENT_BINARY_DIR})

INCLUDE_DIRECTORIES(
  ${SCIRun5Prototype_SOURCE_DIR}/
  ${SCIRun5Prototype_SOURCE_DIR}/Externals/gmock/gtest/include
  ${SCIRun5Prototype_SOURCE_DIR}/Externals/gmock/include
  ${SCIRun5Prototype_BINARY_DIR}/
  ${SCIRUN5_BOOST_INCLUDE_DIRS}
)

OPTION(VTK_SUPPORT "Build VTK RenderWindow" OFF)
IF(VTK_SUPPORT)
  ADD_DEFINITIONS(-DBUILD_VTK_SUPPORT)
ENDIF(VTK_SUPPORT)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

SET(SCIRUN_CORE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

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

ADD_SUBDIRECTORY(Interface)
ADD_SUBDIRECTORY(Core)
ADD_SUBDIRECTORY(Dataflow)
ADD_SUBDIRECTORY(Modules)
ADD_SUBDIRECTORY(Main)
ADD_SUBDIRECTORY(Externals)


########################################################################
# Organize projects into folders

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET_PROPERTY(TARGET Algorithms_Base         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Base_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")
SET_PROPERTY(TARGET Algorithms_DataIO       PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_DataIO_Tests PROPERTY FOLDER "Core/Algorithms/Tests")
SET_PROPERTY(TARGET Algorithms_Math         PROPERTY FOLDER "Core/Algorithms")
SET_PROPERTY(TARGET Algorithms_Math_Tests   PROPERTY FOLDER "Core/Algorithms/Tests")

SET_PROPERTY(TARGET Dataflow_Network         PROPERTY FOLDER "Dataflow")
SET_PROPERTY(TARGET Dataflow_Network_Tests   PROPERTY FOLDER "Dataflow/Tests")
SET_PROPERTY(TARGET Core_Datatypes         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Logging         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Logging_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_CommandLine         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_CommandLine_Tests   PROPERTY FOLDER "Core/Tests")
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

SET_PROPERTY(TARGET Interface_Modules_Base   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_DataIO   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Math   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Visualization   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Render   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Testing   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_String   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Modules_Factory   PROPERTY FOLDER "Interface/Modules")
SET_PROPERTY(TARGET Interface_Application   PROPERTY FOLDER "Interface/Application")

SET_PROPERTY(TARGET Modules_Basic   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Basic_Tests   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Math   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Math_Tests   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_DataIO   PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_DataIO_Tests   PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_Render   PROPERTY FOLDER "Modules/Render")
SET_PROPERTY(TARGET Modules_Render_Tests   PROPERTY FOLDER "Modules/Render")
SET_PROPERTY(TARGET Modules_String   PROPERTY FOLDER "Modules/String")
SET_PROPERTY(TARGET Modules_String_Tests   PROPERTY FOLDER "Modules/String")
SET_PROPERTY(TARGET Modules_Visualization   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Modules_Visualization_Tests   PROPERTY FOLDER "Modules/Visualization")
SET_PROPERTY(TARGET Modules_Factory   PROPERTY FOLDER "Modules")


########################################################################
# Configure test support
# TODO: eventually move to a Testing directory along with regression test support

IF(RUN_UNIT_TESTS)
  FOREACH(test ${UNIT_TEST_LIST})
    ADD_TEST("${test}" "${EXE_DIRPATH}/${test}")
  ENDFOREACH(test ${UNIT_TEST_LIST})
ENDIF()
