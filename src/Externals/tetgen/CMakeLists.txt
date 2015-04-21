#TetGen License
# --------------
#
# The software (TetGen) is licensed under the terms of the  MIT  license
# with the following exceptions:
#
# Distribution of  modified  versions  of this code is permissible UNDER
# THE CONDITION THAT  THIS CODE AND ANY MODIFICATIONS  MADE TO IT IN THE
# SAME SOURCE FILES  tetgen.h AND tetgen.cxx  REMAIN UNDER  COPYRIGHT OF
# THE  ORIGINAL AUTHOR,  BOTH  SOURCE AND OBJECT  CODE  ARE MADE  FREELY
# AVAILABLE  WITHOUT   CHARGE,   AND  CLEAR   NOTICE  IS  GIVEN  OF  THE 
# MODIFICATIONS.
#
# Distribution of this code for  any  commercial purpose  is permissible
# ONLY BY DIRECT ARRANGEMENT WITH THE COPYRIGHT OWNER.
#
# The full  license text is reproduced below.
#
# This means that TetGen is no free software, but for private, research,
# and  educational purposes it  can be  used at  absolutely no  cost and
# without further arrangements.
#
#
# For details, see http://tetgen.berlios.de
#
# ==============================================================================
#
# TetGen
# A Quality Tetrahedral Mesh Generator and 3D Delaunay Triangulator
# Version 1.4 (Released on January 14, 2006).
#
# Copyright 2002, 2004, 2005, 2006 
# Hang Si
# Rathausstr. 9, 10178 Berlin, Germany
# si@wias-berlin.de
#
# Permission is hereby granted, free  of charge, to any person obtaining
# a  copy  of this  software  and  associated  documentation files  (the
# "Software"), to  deal in  the Software without  restriction, including
# without limitation  the rights to  use, copy, modify,  merge, publish,
# distribute,  sublicense and/or  sell copies  of the  Software,  and to
# permit persons to whom the Software  is furnished to do so, subject to
# the following conditions:
#
# Distribution of  modified  versions  of this code is permissible UNDER
# THE CONDITION THAT  THIS CODE AND ANY MODIFICATIONS  MADE TO IT IN THE
# SAME SOURCE FILES  tetgen.h AND tetgen.cxx  REMAIN UNDER  COPYRIGHT OF
# THE  ORIGINAL AUTHOR,  BOTH  SOURCE AND OBJECT  CODE  ARE MADE  FREELY
# AVAILABLE  WITHOUT   CHARGE,   AND  CLEAR   NOTICE  IS  GIVEN  OF  THE 
# MODIFICATIONS.
#
# Distribution of this code for  any  commercial purpose  is permissible
# ONLY BY DIRECT ARRANGEMENT WITH THE COPYRIGHT OWNER.
#
# The  above  copyright  notice  and  this permission  notice  shall  be
# included in all copies or substantial portions of the Software.
#
# THE  SOFTWARE IS  PROVIDED  "AS  IS", WITHOUT  WARRANTY  OF ANY  KIND,
# EXPRESS OR  IMPLIED, INCLUDING  BUT NOT LIMITED  TO THE  WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT  SHALL THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY
# CLAIM, DAMAGES OR  OTHER LIABILITY, WHETHER IN AN  ACTION OF CONTRACT,
# TORT  OR OTHERWISE, ARISING  FROM, OUT  OF OR  IN CONNECTION  WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# ==============================================================================


# CMake build scripts modifications by Scientific Computing and Imaging Institute
# Copyright (c) 2015 Scientific Computing and Imaging Institute, University of Utah.


CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(VERSION 2.6)
ENDIF(COMMAND CMAKE_POLICY)

PROJECT(tetgen)

SET(TETGEN_SRCS
        tetgen.cxx
        predicates.cxx
      )

ADD_DEFINITIONS(-DSELF_CHECK)

IF(WIN32 AND MSVC)
  # Bind application to the current version of the VC libraries
  ADD_DEFINITIONS(-D_BIND_TO_CURRENT_VCLIBS_VERSION) 

  # MSVC level 3 warnings are a little ridiculous
  SET(CMAKE_CXX_WARNING_LEVEL 1)
  IF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    STRING(REGEX REPLACE "/W[0-4]" "/W1"
              CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  ELSE(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W1")
  ENDIF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")

  SET_SOURCE_FILES_PROPERTIES(predicates.cxx
                              PROPERTIES
                                COMPILE_FLAGS "/Od"
                             )
ELSE(WIN32 AND MSVC)
  # Usually we support GCC and MSVC, so we're not likely to run
  # into trouble with this.
  SET_SOURCE_FILES_PROPERTIES(predicates.cxx
                              PROPERTIES
                                COMPILE_FLAGS "-O0"
                             )
ENDIF(WIN32 AND MSVC)


SET(SCI_TETGEN_LIBRARY tetlib CACHE INTERNAL "Tetgen library" FORCE)
SET(SCI_TETGEN_EXECUTABLE tetgen CACHE INTERNAL "Tetgen executable" FORCE)
SET(SCI_TETGEN_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE INTERNAL
      "Tetgen include directories" FORCE)

ADD_EXECUTABLE(${SCI_TETGEN_EXECUTABLE} ${TETGEN_SRCS})

# Tetgen is meant to be built as a static library
ADD_LIBRARY(${SCI_TETGEN_LIBRARY} STATIC ${TETGEN_SRCS})
SET_TARGET_PROPERTIES(${SCI_TETGEN_LIBRARY}
                      PROPERTIES
                        COMPILE_DEFINITIONS "TETLIBRARY"
                     )
IF(BUILD_SHARED_LIBS AND UNIX AND NOT CYGWIN AND NOT APPLE)
  SET_TARGET_PROPERTIES(${SCI_TETGEN_LIBRARY}
                        PROPERTIES
                          COMPILE_FLAGS "-fPIC"
                       )
ENDIF(BUILD_SHARED_LIBS AND UNIX AND NOT CYGWIN AND NOT APPLE)

