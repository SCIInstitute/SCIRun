#-----------------------------------------------------------------------------
#
# Cleaver - A MultiMaterial Tetrahedral Mesher
# -- CMakeList
#
# Primary Author:        Jonathan Bronson (bronson@sci.utah.edu)
# Contributing Authors:  Joshua A. Levine (levine@sci.utah.edu)
#
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
#
#  Copyright (C) 2011, 2012, 2013, Jonathan Bronson
#  Scientific Computing  &  Imaging Institute
#  University of Utah
#
#
#  Permission is hereby  granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to  use, copy, modify,  merge, publish, distribute, sublicense,
#  and/or  sell  copies of  the Software, and  to permit persons to whom the
#  Software is  furnished to  do so,  subject  to  the following conditions:
#
#  The above copyright notice  and  this permission notice shall be included
#  in all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING  BUT NOT  LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A  PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN  NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER  IN  AN  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT  OF  OR IN  CONNECTION  WITH THE  SOFTWARE  OR  THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#-----------------------------------------------------------------------------

PROJECT(CLEAVER)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_BUILD_TYPE DEBUG)
#SET(CMAKE_BUILD_TYPE RelWithDebInfo)
#SET(CMAKE_CXX_FLAGS "-Wall -ansi -O3" )
#SET(CMAKE_DEBUG_FLAGS "-Wall -ansi -O3" )

OPTION(BUILD_CLEAVER_APP "Build Cleaver Command Line App (Requires TEEM)" ON)

# Build Cleaver Library (no dependencies)
ADD_SUBDIRECTORY(lib)

# Build Cleaver Command App (requires teem library)
IF(BUILD_CLEAVER_APP)
    ADD_SUBDIRECTORY(teem)
    ADD_SUBDIRECTORY(cmd)
ENDIF(BUILD_CLEAVER_APP)

