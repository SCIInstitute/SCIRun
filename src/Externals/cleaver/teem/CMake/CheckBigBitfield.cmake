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

#
# Checks whether this platform can (1) or cannot(0) do bit-fields greater than
# 32 bits.  This is necessary for correct handling of IEEE floating point
# special values.
#
# VARIABLE - variable to store the result to
#

MACRO(CHECK_BIG_BITFIELD VARIABLE LOCAL_TEST_DIR)
 IF("HAVE_${VARIABLE}" MATCHES "^HAVE_${VARIABLE}$")
  MESSAGE(STATUS "Checking to see if this platform supports large bit-fields (>32 bits)")
  TRY_RUN(DUMMY ${VARIABLE}
    ${CMAKE_BINARY_DIR}
    ${LOCAL_TEST_DIR}/CheckBigBitfield.c
    OUTPUT_VARIABLE OUTPUT)
  IF(${VARIABLE})
    SET(HAVE_${VARIABLE} TRUE CACHE INTERNAL " ")
    MESSAGE(STATUS "Checking to see if this platform supports large bit-fields (>32 bits) - yes")
    FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log
      "Checking to see if this platform supports large bit-fields (>32 bits) passed with "
      "the following output:\n${OUTPUT}\n\n")
  ELSE(${VARIABLE})
    MESSAGE(STATUS "Checking to see if this platform supports large bit-fields (>32 bits) - no")
    FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log
      "Checking to see if this platform supports large bit-fields (>32 bits) failed with "
      "the following output:\n${OUTPUT}\n\n")
  ENDIF(${VARIABLE})
  ENDIF("HAVE_${VARIABLE}" MATCHES "^HAVE_${VARIABLE}$")
ENDMACRO(CHECK_BIG_BITFIELD)
