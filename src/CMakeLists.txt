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
FIND_PACKAGE(Qt4 REQUIRED)

set(Boost_ADDITIONAL_VERSIONS "1.42" "1.49" "1.49.0")
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
set(Boost_DIR "C:/Dev/git/boost_1_49_0")
set(BOOST_LIBRARYDIR "C:/dev/git/scirunguiprototype/src/Externals/x64")

find_package( Boost 1.49.0 REQUIRED COMPONENTS thread date_time)
  if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    MESSAGE(STATUS "Boost")
    MESSAGE(STATUS ${Boost_INCLUDE_DIRS})
    set(SCIRUN5_BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}/)
  endif()


INCLUDE_DIRECTORIES(
  ${SCIRun5Prototype_SOURCE_DIR}/
  ${SCIRun5Prototype_SOURCE_DIR}/Externals/gmock/gtest/include
  ${SCIRun5Prototype_SOURCE_DIR}/Externals/gmock/include
  ${SCIRun5Prototype_BINARY_DIR}/
  ${SCIRUN5_BOOST_INCLUDE_DIRS}
)

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

SET(SCIRUN_CORE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

########################################################################
# Output directories.

SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SCIRUN_CORE_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SCIRUN_CORE_BINARY_DIR} CACHE INTERNAL "Single output directory for building all executables.")
MARK_AS_ADVANCED(CMAKE_LIBRARY_OUTPUT_DIRECTORY CMAKE_RUNTIME_OUTPUT_DIRECTORY)

MACRO(SCIRUN_ADD_UNIT_TEST test)
  ADD_EXECUTABLE(${test} ${ARGN})
  LIST(APPEND tmp ${test} ${UNIT_TEST_LIST})
  SET(UNIT_TEST_LIST ${tmp} CACHE INTERNAL "List of unit tests." FORCE)
ENDMACRO(SCIRUN_ADD_UNIT_TEST)

SET(BUILD_FROM_TOP_LEVEL 1)

ADD_SUBDIRECTORY(Interface)
ADD_SUBDIRECTORY(Core)
ADD_SUBDIRECTORY(Algorithms)
ADD_SUBDIRECTORY(Engine)
ADD_SUBDIRECTORY(Modules)
ADD_SUBDIRECTORY(Externals)


#
# Organize projects into folders
#
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET_PROPERTY(TARGET Algorithms_Base         PROPERTY FOLDER "Algorithms")
SET_PROPERTY(TARGET Algorithms_Base_Tests   PROPERTY FOLDER "Algorithms/Tests")
SET_PROPERTY(TARGET Algorithms_DataIO       PROPERTY FOLDER "Algorithms")
SET_PROPERTY(TARGET Algorithms_DataIO_Tests PROPERTY FOLDER "Algorithms/Tests")
SET_PROPERTY(TARGET Algorithms_Math         PROPERTY FOLDER "Algorithms")
SET_PROPERTY(TARGET Algorithms_Math_Tests   PROPERTY FOLDER "Algorithms/Tests")

SET_PROPERTY(TARGET Core_Dataflow_Network         PROPERTY FOLDER "Core/Dataflow")
SET_PROPERTY(TARGET Core_Dataflow_Network_Tests   PROPERTY FOLDER "Core/Dataflow/Tests")
SET_PROPERTY(TARGET Core_Datatypes         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Datatypes_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Core_Logging         PROPERTY FOLDER "Core")
SET_PROPERTY(TARGET Core_Logging_Tests   PROPERTY FOLDER "Core/Tests")
SET_PROPERTY(TARGET Engine_Network         PROPERTY FOLDER "Core/Engine")
SET_PROPERTY(TARGET Engine_Network_Tests   PROPERTY FOLDER "Core/Engine/Tests")
SET_PROPERTY(TARGET Engine_Scheduler         PROPERTY FOLDER "Core/Engine")
SET_PROPERTY(TARGET Engine_Scheduler_Tests   PROPERTY FOLDER "Core/Engine/Tests")
SET_PROPERTY(TARGET Engine_State         PROPERTY FOLDER "Core/Engine")
SET_PROPERTY(TARGET Engine_State_Tests   PROPERTY FOLDER "Core/Engine/Tests")

SET_PROPERTY(TARGET gmock         PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET gmock_main   PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET gtest         PROPERTY FOLDER "Testing Support")
SET_PROPERTY(TARGET gtest_main   PROPERTY FOLDER "Testing Support")

SET_PROPERTY(TARGET Interface_Modules   PROPERTY FOLDER "Interface/Modules")

SET_PROPERTY(TARGET Modules_Basic   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Basic_Tests   PROPERTY FOLDER "Modules/Basic")
SET_PROPERTY(TARGET Modules_Math   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_Math_Tests   PROPERTY FOLDER "Modules/Math")
SET_PROPERTY(TARGET Modules_DataIO   PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_DataIO_Tests   PROPERTY FOLDER "Modules/DataIO")
SET_PROPERTY(TARGET Modules_Factory   PROPERTY FOLDER "Modules")

OPTION(RUN_UNIT_TESTS "Run gtest unit tests" ON)

ENABLE_TESTING()

IF(RUN_UNIT_TESTS)
  FOREACH(test ${UNIT_TEST_LIST})
    ADD_TEST("${test}" "${EXE_DIRPATH}/${test}")
  ENDFOREACH(test ${UNIT_TEST_LIST})
ENDIF(RUN_UNIT_TESTS)
