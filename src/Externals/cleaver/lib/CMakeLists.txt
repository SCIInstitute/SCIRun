#-------------------------------------------------------------------
#
# // Cleaver - A MultiMaterial Conforming Tetrahedral Meshing Library
# -- Library CMakeList
#
# Primary Author: Jonathan Bronson (bronson@sci.utah.edu)
#
#-------------------------------------------------------------------
#-------------------------------------------------------------------
#
# Copyright (c) 2011, 2012, Scientific Computing & Imaging Institute
# and the University of Utah. All Rights Reserved.
#
# Redistribution and use in source and binary forms is strictly
# prohibited without the direct consent of the author.
#
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
OPTION(BUILD_SHARED_LIBS OFF)

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
