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
# Check if the system is big endian or little endian
#
# VARIABLE - variable to store the result to
#

MACRO(TESTNO_ICC_IDYNAMIC_NEEDED VARIABLE LOCAL_TEST_DIR)
  IF("HAVE_${VARIABLE}" MATCHES "^HAVE_${VARIABLE}$")
    TRY_RUN(${VARIABLE} HAVE_${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${LOCAL_TEST_DIR}/TestNO_ICC_IDYNAMIC_NEEDED.cxx
      OUTPUT_VARIABLE OUTPUT)
    MESSAGE(STATUS "Check if using the Intel icc compiler, and if -i_dynamic is needed... COMPILE_RESULT...${HAVE_${VARIABLE}} RUN_RESULT...${VARIABLE}\n")
    IF(HAVE_${VARIABLE}) #Test compiled, either working intel w/o -i_dynamic, or another compiler
      IF(${VARIABLE})   #Intel icc compiler, -i_dynamic not needed
        FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log
                       "-i_dynamic not needed, (Not Intel icc, or this version of Intel icc does not conflict with OS glibc.")
        MESSAGE(STATUS "-i_dynamic not needed, (Not Intel icc, or this version of Intel icc does not conflict with OS glibc.")
      ELSE(${VARIABLE}) #The compiler is not Intel icc
        FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log 
                       "The compiler ERROR--This should never happen")
        MESSAGE(STATUS "The compiler ERROR--This should never happen")
      ENDIF(${VARIABLE})
    ELSE(HAVE_${VARIABLE})  #Test did not compile, either badly broken compiler, or intel -i_dynamic needed
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log
            "\tThe -i_dynamic compiler flag is needed for the Intel icc compiler on this platform.\n")
      MESSAGE("The -i_dynamic compiler flag is needed for the Intel icc compiler on this platform.")
    ENDIF(HAVE_${VARIABLE})
    FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log "TestNO_ICC_IDYNAMIC_NEEDED produced following output:\n${OUTPUT}\n\n")
  ENDIF("HAVE_${VARIABLE}" MATCHES "^HAVE_${VARIABLE}$")
ENDMACRO(TESTNO_ICC_IDYNAMIC_NEEDED)
