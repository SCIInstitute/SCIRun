#
# Teem: Tools to process and visualize scientific data and images              
# Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
# Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# (LGPL) as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# The terms of redistributing and/or modifying this software also
# include exceptions to the LGPL that facilitate static linking.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#

IF(NOT Teem_FOUND)
  MESSAGE(FATAL_ERROR "Something went wrong. You are including TeemUse.cmake but Teem was not found")
ENDIF(NOT Teem_FOUND)

# Make Teem easier to use
INCLUDE_DIRECTORIES(${Teem_INCLUDE_DIRS})
LINK_DIRECTORIES(${Teem_LIBRARY_DIRS})

# Load the compiler settings used for Teem.
IF(Teem_BUILD_SETTINGS_FILE)
  INCLUDE(CMakeImportBuildSettings)
  CMAKE_IMPORT_BUILD_SETTINGS(${Teem_BUILD_SETTINGS_FILE})
ENDIF(Teem_BUILD_SETTINGS_FILE)

# Support static builds
IF(NOT Teem_BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DTEEM_STATIC=1)
ENDIF(NOT Teem_BUILD_SHARED_LIBS)

# Add compiler flags needed to use Teem.
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${Teem_REQUIRED_C_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${Teem_REQUIRED_EXE_LINKER_FLAGS}")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${Teem_REQUIRED_SHARED_LINKER_FLAGS}")
SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${Teem_REQUIRED_MODULE_LINKER_FLAGS}")

