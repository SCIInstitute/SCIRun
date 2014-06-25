#-------------------------------------------------------------------
#
# // Cleaver - A MultiMaterial Conforming Tetrahedral Meshing Library
# -- Commandline CMakeList
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

PROJECT(volumemesh)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Add Teem library includes
#INCLUDE_DIRECTORIES(${Teem_BINARY_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)

FILE(GLOB srcs *.c*)
FILE(GLOB hdrs *.h)


ADD_EXECUTABLE(volumemesh ${hdrs} ${srcs})
TARGET_LINK_LIBRARIES(volumemesh cleaver teem)

