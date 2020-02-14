#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2020 Scientific Computing and Imaging Institute,
#  University of Utah.
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


#-------------------------------------------------------------------
#
# // Cleaver - A MultiMaterial Conforming Tetrahedral Meshing Library
# -- Library CMakeList
#
# Primary Author: Jonathan Bronson (bronson@sci.utah.edu)
#
#-------------------------------------------------------------------
#-------------------------------------------------------------------

PROJECT(cleaverlib)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# project version
SET(${PROJECT_NAME}_MAJOR_VERSION 0)
SET(${PROJECT_NAME}_MINOR_VERSION 1)
SET(${PROJECT_NAME}_PATCH_LEVEL 0)

# project options
#OPTION(BUILD_SHARED_LIBS OFF)
SET(BUILD_SHARED_LIBS OFF)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Add Teem library includes
INCLUDE_DIRECTORIES(${Teem_BINARY_DIR}/include)

# source files
FILE(GLOB Cleaver_SOURCE_FILES *.c*)
FILE(GLOB Cleaver_HEADER_FILES *.h)
SET(Cleaver_API_HEADER_FILES
    Cleaver.h
    ScalarField.h
    FloatField.h
    InverseField.h
    AbstractVolume.h
    Volume.h
    PaddedVolume.h
    TetMesh.h
    Vertex.h
    BoundingBox.h
    vec3.h
    )



# copy header files over
SET(Cleaver_INSTALLED_HEADER_FILES)
FOREACH(header_file ${Cleaver_API_HEADER_FILES})
  # When debugging, uncomment this line
  #MESSAGE(STATUS "Copy header file: ${header_file}")
  GET_FILENAME_COMPONENT(file_no_path "${header_file}" NAME)
  CONFIGURE_FILE("${header_file}"
    "${CMAKE_BINARY_DIR}/include/Cleaver/${file_no_path}" COPYONLY IMMEDIATE)
  SET(Cleaver_INSTALLED_HEADER_FILES
    ${Cleaver_INSTALLED_HEADER_FILES}
    "${CMAKE_BINARY_DIR}/include/Cleaver/${file_no_path}")
ENDFOREACH(header_file)

# output library
ADD_LIBRARY(cleaver ${Cleaver_HEADER_FILES} ${Cleaver_SOURCE_FILES})
