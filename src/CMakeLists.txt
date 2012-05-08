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

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

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

SET(BUILD_FROM_TOP_LEVEL 1)

ADD_SUBDIRECTORY(Interface)
ADD_SUBDIRECTORY(Core)
ADD_SUBDIRECTORY(Algorithms)
ADD_SUBDIRECTORY(Engine)
ADD_SUBDIRECTORY(Modules)
ADD_SUBDIRECTORY(Externals)
