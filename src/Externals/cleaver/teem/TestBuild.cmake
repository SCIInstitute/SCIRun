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

# Where the source code lives
SET (CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}")
SET (CTEST_BINARY_DIRECTORY "${CTEST_SOURCE_DIRECTORY}/build-ctest")

# Make sure we always reconfigure cmake stuff from scratch and don't
# rely on previously built libraries
SET (CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)

SET (CTEST_CMAKE_COMMAND "cmake")
# NOTE(bigler): On windows you should make sure something like cygwin is in
# your path.  Then it will find cygwin's svn.  Otherwise, your nightlies
# won't find svn and won't do the updates.  I put it in the bat script at
# the end:
#   set PATH=%PATH%;"C:\Program Files\CMake 2.4\bin";"c:/cygwin/bin"
SET (CTEST_CVS_COMMAND "svn")

# A smoke test only builds the code and doesn't run any tests, so we
# exclude all tests here
SET (CTEST_COMMAND "ctest -D Nightly")
#SET (CTEST_COMMAND "ctest -D Experimental")

SET(CTEST_INITIAL_CACHE "
  BUILD_EXPERIMENTAL_LIBS:BOOL=OFF
  BUILD_EXPERIMENTAL_APPS:BOOL=OFF
  BUILD_HEX:BOOL=OFF
  BUILD_SHARED_LIBS:BOOL=OFF
  BUILD_TESTING:BOOL=ON
  CMAKE_BUILD_TYPE:STRING=Release
  DART_TESTING_TIMEOUT:INTEGER=600
  TEEM_BZIP2:BOOL=ON
  TEEM_PNG:BOOL=ON
  TEEM_ZLIB:BOOL=ON
")

# Here are some other intersting things you can set for the initial cache
# MAKECOMMAND:STRING=/usr/bin/make -j16
# BUILDNAME:STRING=MySpecialConfig




