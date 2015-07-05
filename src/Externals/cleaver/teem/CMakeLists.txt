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

# This is the root Teem CMakeLists file
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)


# Remove PNG support, as teem uses deprecated libpng calls
SET(Teem_PNG OFF CACHE BOOL "Build Teem with support for PNG images.")

# As of CMake 2.6 Policies were introduced in order to provide a mechanism for
# adding backwards compatibility one feature at a time.

# Run cmake --help-policy CMP<num> to see documentation.
IF(COMMAND cmake_policy)
  # Library paths (/path/to/libmy.so not translated to -L/path/to -lmy)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

PROJECT(Teem)

#-----------------------------------------------------------------------------
# Teem version number.  Must reflect the values in teem/src/air/air.h
SET(Teem_VERSION_MAJOR "1")
SET(Teem_VERSION_MINOR "10")
SET(Teem_VERSION_PATCH "0")

# Version string should not include patch level.  The major.minor is
# enough to distinguish available features of the toolkit.
SET(Teem_VERSION_STRING "${Teem_VERSION_MAJOR}.${Teem_VERSION_MINOR}.${Teem_VERSION_PATCH}")

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# We need ansi c-flags, especially on HP
SET(CMAKE_C_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_C_FLAGS}")
SET(CMAKE_REQUIRED_FLAGS ${CMAKE_ANSI_CFLAGS})

#-----------------------------------------------------------------------------
# Test for some required system information.
INCLUDE (CMakeBackwardCompatibilityC)

#-----------------------------------------------------------------------------
# Output directories.
SET (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
SET (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")
#output directory for installing all include files.")
MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)
SET(Teem_LIBRARY_PATH "${LIBRARY_OUTPUT_PATH}")
SET(Teem_EXECUTABLE_PATH "${EXECUTABLE_OUTPUT_PATH}")
SET(C_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})


FILE(MAKE_DIRECTORY ${Teem_BINARY_DIR}/include)
FILE(MAKE_DIRECTORY ${Teem_BINARY_DIR}/include/teem)

#-----------------------------------------------------------------------------
# Find platform-specific differences in the handling of IEEE floating point 
# special values.

INCLUDE(${Teem_SOURCE_DIR}/CMake/TestQnanhibit.cmake)
TEST_QNANHIBIT(QNANHIBIT_VALUE ${Teem_SOURCE_DIR}/CMake)
IF(QNANHIBIT_VALUE)
  SET(QNANHIBIT 1 CACHE INTERNAL "The 22nd bit of 32-bit floating-point quiet NaN.")
ELSE(QNANHIBIT_VALUE)
  SET(QNANHIBIT 0 CACHE INTERNAL "The 22nd bit of 32-bit floating-point quiet NaN.")
ENDIF(QNANHIBIT_VALUE)


#-----------------------------------------------------------------------------
# Teem build configuration options.
OPTION(BUILD_SHARED_LIBS "Build Teem with shared libraries." OFF)
SET(Teem_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
IF(NOT Teem_BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DTEEM_STATIC)
ENDIF(NOT Teem_BUILD_SHARED_LIBS)

OPTION(Teem_ZLIB "Build Teem with support for gzip compression." ON)
OPTION(Teem_PNG "Build Teem with support for PNG images." ON)
OPTION(Teem_VTK_MANGLE "Build Teem using the mangled libraries from VTK for ZLIB and PNG." OFF)
SET(Teem_VTK_ZLIB_MANGLE_IPATH "" CACHE PATH "Location of vtk_zlib_mangle.h")
OPTION(Teem_VTK_TOOLKITS_IPATH "Location of VTK Build directory for ZLIB and PNG includes." OFF)

# Can't build PNG without ZLIB, so force it on.
IF(Teem_PNG AND NOT Teem_ZLIB)
  SET(Teem_ZLIB ON)
ENDIF(Teem_PNG AND NOT Teem_ZLIB)

IF(Teem_VTK_MANGLE)
  ADD_DEFINITIONS(-DTEEM_VTK_MANGLE=1)
  INCLUDE_DIRECTORIES( ${Teem_VTK_TOOLKITS_IPATH} )
  INCLUDE_DIRECTORIES( ${Teem_VTK_ZLIB_MANGLE_IPATH} )
ENDIF(Teem_VTK_MANGLE)


SET(Teem_ZLIB_LIB "")
SET(Teem_PNG_LIB "")
IF(Teem_ZLIB)
  # Find ZLIB
  FIND_PACKAGE(ZLIB)
  IF(ZLIB_FOUND)
    ADD_DEFINITIONS(-DTEEM_ZLIB)
    SET(Teem_ZLIB_LIB ${ZLIB_LIBRARIES})
    SET(Teem_ZLIB_IPATH ${ZLIB_INCLUDE_DIR})
  ELSE(ZLIB_FOUND)
    # We need to set this as a cache variable, so that it will show up as
    # being turned off in the cache.
    MESSAGE(SEND_ERROR "Turning off Teem_ZLIB, because it wasn't found.")
    SET(Teem_ZLIB OFF CACHE BOOL "Build Teem with support for gzip compression." FORCE)
  ENDIF(ZLIB_FOUND)

  IF(Teem_PNG)
    FIND_PACKAGE(PNG)
    IF(PNG_FOUND)
      ADD_DEFINITIONS(-DTEEM_PNG ${PNG_DEFINITIONS})
      SET(Teem_PNG_LIB ${PNG_LIBRARIES})
      SET(Teem_PNG_IPATH ${PNG_INCLUDE_DIR})
    ELSE(PNG_FOUND)
      # We need to set this as a cache variable, so that it will show up as
      # being turned off in the cache.
      MESSAGE(SEND_ERROR "Turning off Teem_PNG, because it wasn't found.")
      SET(Teem_PNG OFF CACHE BOOL "Build Teem with support for PNG images." FORCE)
    ENDIF(PNG_FOUND)
  ENDIF(Teem_PNG)
ENDIF(Teem_ZLIB)

# Try and locate BZIP2 stuff
OPTION(Teem_BZIP2 "Build Teem with support for bzip compression." ON)
SET(Teem_BZIP2_LIB "")

IF(Teem_BZIP2)
  FIND_PACKAGE(BZ2)

  IF(BZ2_FOUND)
    ADD_DEFINITIONS(-DTEEM_BZIP2)
    SET(Teem_BZIP2_LIB ${BZ2_LIBRARIES})
    SET(Teem_BZIP2_IPATH ${BZ2_INCLUDE_DIR})
  ELSE(BZ2_FOUND)
    # We need to set this as a cache variable, so that it will show up as
    # being turned off in the cache.
    MESSAGE(SEND_ERROR "Turning off Teem_BZIP2, because it wasn't found.")
    SET(Teem_BZIP2 OFF CACHE BOOL "Build Teem with support for bzip compression." FORCE)
  ENDIF(BZ2_FOUND)
ENDIF(Teem_BZIP2)

# Look for threading libraries
OPTION(Teem_PTHREAD "Build Teem with pthread library support." ON)
IF(Teem_PTHREAD)
  INCLUDE(FindThreads)
  IF(CMAKE_USE_PTHREADS_INIT)
    ADD_DEFINITIONS(-DTEEM_PTHREAD)
  ELSE(CMAKE_USE_PTHREADS_INIT)
    # We need to set this as a cache variable, so that it will show up as
    # being turned off in the cache.
    MESSAGE(SEND_ERROR "Turning off Teem_PTHREAD, because it wasn't found.")
    SET(Teem_PTHREAD OFF CACHE BOOL "Build Teem with pthread library support." FORCE)
    SET(Teem_PTHREAD OFF)
  ENDIF(CMAKE_USE_PTHREADS_INIT)
ENDIF(Teem_PTHREAD)

# Look for "levmar" library <http://www.ics.forth.gr/~lourakis/levmar/>
OPTION(Teem_LEVMAR "Build Teem with levmar library support." OFF)
SET(Teem_LEVMAR_LIB "")
IF(Teem_LEVMAR)
  FIND_PACKAGE(LEVMAR)

  IF(LEVMAR_FOUND)
    ADD_DEFINITIONS(-DTEEM_LEVMAR)
    SET(Teem_LEVMAR_LIB ${LEVMAR_LIBRARIES})
    SET(Teem_LEVMAR_IPATH ${LEVMAR_INCLUDE_DIR})
  ELSE(LEVMAR_FOUND)
    # We need to set this as a cache variable, so that it will show up as
    # being turned off in the cache.
    MESSAGE(SEND_ERROR "Turning off Teem_LEVMAR, because it wasn't found.")
    SET(Teem_LEVMAR OFF CACHE BOOL "Build Teem with levmar library support." FORCE)
  ENDIF(LEVMAR_FOUND)
ENDIF(Teem_LEVMAR)

#-----------------------------------------------------------------------------

IF(${CMAKE_GENERATOR} MATCHES "Visual Studio")
  # For Visual Studio we don't care about warnings about deprecated sprintf and
  # the like.
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
ENDIF(${CMAKE_GENERATOR} MATCHES "Visual Studio")

#Include Teem Defines
# Set compiler flags for endian-ness.
IF(CMAKE_WORDS_BIGENDIAN)
  ADD_DEFINITIONS(-DTEEM_ENDIAN=4321)
ELSE(CMAKE_WORDS_BIGENDIAN)
  ADD_DEFINITIONS(-DTEEM_ENDIAN=1234)
ENDIF(CMAKE_WORDS_BIGENDIAN)

# Set compiler flags for 32 or 64 bit architecture (based on the size
# of a void pointer).
IF(CMAKE_SIZEOF_VOID_P MATCHES  8)
  ADD_DEFINITIONS(-DTEEM_32BIT=0)
ELSE(CMAKE_SIZEOF_VOID_P MATCHES 8)
  ADD_DEFINITIONS(-DTEEM_32BIT=1)
ENDIF(CMAKE_SIZEOF_VOID_P MATCHES 8)

#The QNANHIBIT variable is configured by the root level CMakeLists.txt
IF(QNANHIBIT)
  ADD_DEFINITIONS(-DTEEM_QNANHIBIT=1)
ELSE(QNANHIBIT)
  ADD_DEFINITIONS(-DTEEM_QNANHIBIT=0)
ENDIF(QNANHIBIT)

#DirectIO is the fast way to do multi-gigabyte I/O and currently only available
#for SGI platforms.  Use of DirectIO is enabled manually for now.
#OPTION(USE_DIRECTIO "Use DirectIO for Nrrd file IO.  Only valid on SGI systems." 0)
#MARK_AS_ADVANCED(USE_DIRECTIO)
#IF(USE_DIRECTIO)
#  ADD_DEFINITIONS(-DTEEM_DIO=1)
#ELSE(USE_DIRECTIO)
ADD_DEFINITIONS(-DTEEM_DIO=0)
#ENDIF(USE_DIRECTIO)

SET(BUILD_EXPERIMENTAL_LIBS OFF CACHE BOOL "Build Teem's experimental/unstable libraries")
SET(BUILD_EXPERIMENTAL_APPS OFF CACHE BOOL "Build Teem's non-essential command-line tools")

SET(Teem_LIBRARIES air hest biff nrrd ell unrrdu gage limn seek hoover echo ten dye mite)
IF(BUILD_EXPERIMENTAL_LIBS)
  SET(Teem_LIBRARIES ${Teem_LIBRARIES} bane moss coil alan pull)
ENDIF(BUILD_EXPERIMENTAL_LIBS)

SET(Teem_HEADER_FILES
  air/air.h
  hest/hest.h
  biff/biff.h
  nrrd/nrrd.h
  nrrd/nrrdDefines.h
  nrrd/nrrdMacros.h
  nrrd/nrrdEnums.h
  ell/ell.h
  ell/ellMacros.h
  unrrdu/unrrdu.h
  gage/gage.h
  limn/limn.h
  seek/seek.h
  hoover/hoover.h
  echo/echo.h
  ten/ten.h
  ten/tenMacros.h
  dye/dye.h
  mite/mite.h
  )
IF(BUILD_EXPERIMENTAL_LIBS)
  SET(Teem_HEADER_FILES ${Teem_HEADER_FILES}
    bane/bane.h
    moss/moss.h
    coil/coil.h
    alan/alan.h
    pull/pull.h
    )
ENDIF(BUILD_EXPERIMENTAL_LIBS)
SET(Teem_INSTALLED_HEADER_FILES)
FOREACH(header_file ${Teem_HEADER_FILES})
  # When debugging, uncomment this line
  #MESSAGE(STATUS "Copy header file: ${header_file}")
  GET_FILENAME_COMPONENT(file_no_path "${header_file}" NAME)
  CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/${header_file}"
    "${CMAKE_BINARY_DIR}/include/teem/${file_no_path}" COPYONLY IMMEDIATE)
  SET(Teem_INSTALLED_HEADER_FILES
    ${Teem_INSTALLED_HEADER_FILES}
    "${CMAKE_BINARY_DIR}/include/teem/${file_no_path}")
ENDFOREACH(header_file)

#---
# Include directory 
INCLUDE_DIRECTORIES(
  "${CMAKE_BINARY_DIR}/include/"
  "${Teem_SOURCE_DIR}/include/"
  )


## Add external library path includes
IF(Teem_ZLIB)
  INCLUDE_DIRECTORIES(${Teem_ZLIB_IPATH})
  INCLUDE_DIRECTORIES(${Teem_ZLIB_DLLCONF_IPATH})
  IF(Teem_PNG)
    INCLUDE_DIRECTORIES(${Teem_PNG_IPATH})
    INCLUDE_DIRECTORIES(${Teem_PNG_DLLCONF_IPATH})
  ENDIF(Teem_PNG)
ENDIF(Teem_ZLIB)

IF(Teem_BZIP2)
  INCLUDE_DIRECTORIES(${Teem_BZIP2_IPATH})
ENDIF(Teem_BZIP2)

IF(Teem_LEVMAR)
  INCLUDE_DIRECTORIES(${Teem_LEVMAR_IPATH})
ENDIF(Teem_LEVMAR)

# All the source files
SET(Teem_SOURCES)

#-----------------------------------------------------------------------------
# Macro for adding the current set of source files to the master list
MACRO(ADD_Teem_LIBRARY dir)
  SET(srcs)
  FOREACH(src ${ARGN})
    SET(srcs ${srcs} "src/${dir}/${src}")
  ENDFOREACH(src ${ARGN})
  SET(Teem_SOURCES ${Teem_SOURCES} ${srcs})
  # This will group all the source files in the VS project by directory
  SOURCE_GROUP( ${dir} FILES ${srcs} )
ENDMACRO(ADD_Teem_LIBRARY name)


#-----------------------------------------------------------------------------
# Dispatch the build into the proper subdirectories.
FOREACH(dir ${Teem_LIBRARIES})
  INCLUDE(src/${dir}/sources.cmake)
ENDFOREACH(dir ${Teem_LIBRARIES})

#-----------------------------------------------------------------------------
# Build the Mega library
#
# This needs to occur after the parsing of the subdirectories.

# Create the library target
ADD_LIBRARY(teem ${Teem_SOURCES})

# Set up some library paths for installation.  Windows will ingore the
# RPATH stuff, and mac will ingore the INSTALL_RPATH.  Make sure for
# macs to set BUILD_WITH_INSTALL_RPATH OFF and set INSTALL_NAME_DIR.
# I belive INSTALL_NAME_DIR will be ignored on linux.
SET_TARGET_PROPERTIES(teem PROPERTIES
  BUILD_WITH_INSTALL_RPATH OFF
  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
  INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib
  SOVERSION 1
  VERSION ${Teem_VERSION_STRING}
  )

IF(Teem_BZIP2_LIB)
  TARGET_LINK_LIBRARIES(teem ${Teem_BZIP2_LIB})
ENDIF(Teem_BZIP2_LIB)
IF(Teem_ZLIB_LIB)
  TARGET_LINK_LIBRARIES(teem ${Teem_ZLIB_LIB})
  IF(Teem_PNG_LIB)
    TARGET_LINK_LIBRARIES(teem ${Teem_PNG_LIB})
  ENDIF(Teem_PNG_LIB)
ENDIF(Teem_ZLIB_LIB)

IF(Teem_LEVMAR_LIB)
  TARGET_LINK_LIBRARIES(teem ${Teem_LEVMAR_LIB})
ENDIF(Teem_LEVMAR_LIB)

IF(Teem_PTHREAD)
  TARGET_LINK_LIBRARIES(teem ${CMAKE_THREAD_LIBS_INIT})
ENDIF(Teem_PTHREAD)

IF(UNIX)
  TARGET_LINK_LIBRARIES(teem -lm)
ENDIF(UNIX)

OPTION(Teem_USE_LIB_INSTALL_SUBDIR "Add a Teem-X.Y.Z directory layer to the installation tree for libraries and archives." OFF)
IF(Teem_USE_LIB_INSTALL_SUBDIR)
  SET(EXTRA_INSTALL_PATH /Teem-${Teem_VERSION_STRING})
ELSE(Teem_USE_LIB_INSTALL_SUBDIR)
  SET(EXTRA_INSTALL_PATH "")
ENDIF(Teem_USE_LIB_INSTALL_SUBDIR)

INSTALL(TARGETS teem
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib${EXTRA_INSTALL_PATH}
  ARCHIVE DESTINATION lib${EXTRA_INSTALL_PATH}
  ) 

# Now compile the binaries
ADD_SUBDIRECTORY(src/bin)

# Stand-alone programs to process hex encoding of data
OPTION(BUILD_HEX "Build dehex and enhex" OFF)
IF(BUILD_HEX)
  ADD_SUBDIRECTORY(src/hex)
ENDIF(BUILD_HEX)

#-----------------------------------------------------------------------------
# For testing
OPTION(BUILD_TESTING "Enable this to perform testing of Teem" OFF)
IF(BUILD_TESTING)
  SET(DART_ROOT "" CACHE INTERNAL "" FORCE)
  INCLUDE(CTest)
  ENABLE_TESTING()
  MARK_AS_ADVANCED(TCL_TCLSH)
ENDIF(BUILD_TESTING)

#-----------------------------------------------------------------------------
# Help outside projects build Teem projects.
INCLUDE(CMakeExportBuildSettings)
EXPORT_LIBRARY_DEPENDENCIES(${Teem_BINARY_DIR}/TeemLibraryDepends.cmake)
CMAKE_EXPORT_BUILD_SETTINGS(${Teem_BINARY_DIR}/TeemBuildSettings.cmake)

SET(CFLAGS "${CMAKE_C_FLAGS}")
SET(CC "${CMAKE_C_COMPILER}")

# For build tree usage
SET(Teem_LIBRARY_DEPENDS_FILE ${Teem_BINARY_DIR}/TeemLibraryDepends.cmake)
SET(Teem_EXECUTABLE_DIRS ${EXECUTABLE_OUTPUT_PATH})
SET(Teem_LIBRARY_DIRS ${LIBRARY_OUTPUT_PATH})
SET(Teem_USE_FILE ${Teem_SOURCE_DIR}/CMake/TeemUse.cmake)
SET(Teem_INCLUDE_DIRS "${Teem_BINARY_DIR}/include")
SET(Teem_BUILD_SETTINGS_FILE ${Teem_BINARY_DIR}/TeemBuildSettings.cmake)

SET(prefix "${Teem_BINARY_DIR}")
SET(exec_prefix "${prefix}")
SET(exec_prefix_set "no")
SET(bindir "${prefix}/bin")
SET(includedir "${prefix}")
SET(libdir "${prefix}/bin")
SET(srcdir "${Teem_SOURCE_DIR}")

CONFIGURE_FILE("${Teem_SOURCE_DIR}/CMake/TeemConfig.cmake.in"
  "${Teem_BINARY_DIR}/TeemConfig.cmake" @ONLY IMMEDIATE)

# For installed tree usage
SET(Teem_LIBRARY_DEPENDS_FILE ${CMAKE_INSTALL_PREFIX}/lib/Teem-${Teem_VERSION_STRING}/TeemLibraryDepends.cmake)
SET(Teem_EXECUTABLE_DIRS "${CMAKE_INSTALL_PREFIX}/bin")
SET(Teem_LIBRARY_DIRS "${CMAKE_INSTALL_PREFIX}/lib${EXTRA_INSTALL_PATH}")
SET(Teem_USE_FILE "${CMAKE_INSTALL_PREFIX}/lib/Teem-${Teem_VERSION_STRING}/TeemUse.cmake")
SET(Teem_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
SET(Teem_BUILD_SETTINGS_FILE ${CMAKE_INSTALL_PREFIX}/lib/Teem-${Teem_VERSION_STRING}/TeemBuildSettings.cmake)
SET(Teem_BUILT_LIBRARIES teem) # The libraries built by teem.  Currently we only build the mega library.
SET(prefix "${CMAKE_INSTALL_PREFIX}")
SET(exec_prefix "${prefix}")
SET(exec_prefix_set "no")
SET(bindir "${prefix}/bin")
SET(includedir "${prefix}")
SET(libdir "${prefix}/lib${EXTRA_INSTALL_PATH}")
SET(srcdir ".")

CONFIGURE_FILE("${Teem_SOURCE_DIR}/CMake/TeemConfig.cmake.in"
  "${Teem_BINARY_DIR}/CMake/TeemConfig.cmake" @ONLY IMMEDIATE)

INSTALL(FILES ${Teem_INSTALLED_HEADER_FILES}
  DESTINATION include/teem
  )
INSTALL(FILES
  "${Teem_SOURCE_DIR}/CMake/TeemUse.cmake"
  "${Teem_BINARY_DIR}/TeemBuildSettings.cmake"
  "${Teem_BINARY_DIR}/TeemLibraryDepends.cmake"
  "${Teem_BINARY_DIR}/CMake/TeemConfig.cmake"
  DESTINATION lib/Teem-${Teem_VERSION_STRING}
  )

