# Copyright (C) 1998-2003 Daniel Veillard.  All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is fur-
# nished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
# NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# DANIEL VEILLARD BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CON-
# NECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Except as contained in this notice, the name of Daniel Veillard shall not
# be used in advertising or otherwise to promote the sale, use or other deal-
# ings in this Software without prior written authorization from him.


# CMake build scripts provided by Scientific Computing and Imaging Institute
# Copyright (c) 2009 Scientific Computing and Imaging Institute, University of Utah.

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(VERSION 2.6)

  IF(CMAKE_VERSION VERSION_GREATER 3.0)
    # TODO: when support for versions older than 2.8.12 is dropped,
    # enable new policy and get rid of version check
    #
    # see https://cmake.org/cmake/help/v3.0/policy/CMP0042.html
    CMAKE_POLICY(SET CMP0042 OLD)
  ENDIF()
ENDIF()

PROJECT(libxml2)

IF(BUILD_STATIC_THIRDPARTY)
  IF(BUILD_SHARED_LIBS AND UNIX AND NOT APPLE AND NOT CYGWIN)
    SET(ENABLE_PIC ON)
  ENDIF(BUILD_SHARED_LIBS AND UNIX AND NOT APPLE AND NOT CYGWIN)
  SET(BUILD_SHARED_LIBS OFF)
ENDIF(BUILD_STATIC_THIRDPARTY)

#-----------------------------------------------------------------------------
# Test for some required system information.
INCLUDE(CMakeBackwardCompatibilityC)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckFunctionExists)

# from libxml2 configure script
SET(LIBXML_MAJOR_VERSION 2 CACHE INTERNAL "Libxml2 major version." FORCE)
SET(LIBXML_MINOR_VERSION 6 CACHE INTERNAL "Libxml2 minor version." FORCE)
SET(LIBXML_MICRO_VERSION 22 CACHE INTERNAL "Libxml2 micro version." FORCE)
SET(LIBXML_MICRO_VERSION_SUFFIX "" CACHE INTERNAL "Libxml2 micro suffix." FORCE)

IF(CMAKE_USE_PTHREADS)

  SET(HAVE_LIBPTHREAD ${CMAKE_USE_PTHREADS})
  ADD_DEFINITIONS(-D_REENTRANT)
  SET(WITH_THREADS 1)

ELSE(CMAKE_USE_PTHREADS)

  IF(CMAKE_USE_WIN32_THREADS)
    #SET(HAVE_WIN32_THREADS ${CMAKE_USE_WIN32_THREADS})
    ADD_DEFINITIONS(-D_REENTRANT -DHAVE_WIN32_THREADS)
    SET(WITH_THREADS 1)
  ELSE(CMAKE_USE_WIN32_THREADS)
    SET(WITH_THREADS 0)
  ENDIF(CMAKE_USE_WIN32_THREADS)

ENDIF(CMAKE_USE_PTHREADS)

IF(M_LIBRARY)
  SET(HAVE_LIBM TRUE)
ENDIF(M_LIBRARY)

IF(CMAKE_HAVE_PTHREAD_H)
  SET(HAVE_PTHREAD_H ${CMAKE_HAVE_PTHREAD_H})
ENDIF(CMAKE_HAVE_PTHREAD_H)

IF(CMAKE_HAVE_UNISTD_H)
  SET(HAVE_UNISTD_H ${CMAKE_HAVE_UNISTD_H})
ENDIF(CMAKE_HAVE_UNISTD_H)

IF(CMAKE_HAVE_LIMITS_H)
  SET(HAVE_LIMITS_H ${CMAKE_HAVE_LIMITS_H})
ENDIF(CMAKE_HAVE_LIMITS_H)

IF(CMAKE_HAVE_SYS_PRCTL_H)
  SET(HAVE_SYS_TYPES_H ${CMAKE_HAVE_SYS_PRCTL_H})
ENDIF(CMAKE_HAVE_SYS_PRCTL_H)

CHECK_INCLUDE_FILE("ansidecl.h" HAVE_ANSIDECL_H)
CHECK_INCLUDE_FILE("ctype.h" HAVE_CTYPE_H)
CHECK_INCLUDE_FILE("dirent.h" HAVE_DIRENT_H)
CHECK_INCLUDE_FILE("fcntl.h" HAVE_FCNTL_H)
CHECK_INCLUDE_FILE("errno.h" HAVE_ERRNO_H)
CHECK_INCLUDE_FILE("malloc.h" HAVE_MALLOC_H)
CHECK_INCLUDE_FILE("stdarg.h" HAVE_STDARG_H)
CHECK_INCLUDE_FILE("stdint.h" HAVE_STDINT_H)
CHECK_INCLUDE_FILE("stdlib.h" HAVE_STDLIB_H)
CHECK_INCLUDE_FILE("inttypes.h" HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("inttypes.h.h" HAVE_INTTYPES_H_H)
CHECK_INCLUDE_FILE("time.h" HAVE_TIME_H)
CHECK_INCLUDE_FILE("ieeefp.h" HAVE_IEEEFP_H)
CHECK_INCLUDE_FILE("nan.h" HAVE_NAN_H)
CHECK_INCLUDE_FILE("math.h" HAVE_MATH_H)
CHECK_INCLUDE_FILE("float.h" HAVE_FLOAT_H)
CHECK_INCLUDE_FILE("fp_class.h" HAVE_FP_CLASS_H)
CHECK_INCLUDE_FILE("netdb.h" HAVE_NETDB_H)
CHECK_INCLUDE_FILE("signal.h" HAVE_SIGNAL_H)
CHECK_INCLUDE_FILE("resolv.h" HAVE_RESOLV_H)
CHECK_INCLUDE_FILE("dlfcn.h" HAVE_DLFCN_H)
CHECK_INCLUDE_FILE("dl.h" HAVE_DL_H)
CHECK_INCLUDE_FILE("string.h" HAVE_STRING_H)
CHECK_INCLUDE_FILE("strings.h" HAVE_STRINGS_H)
CHECK_INCLUDE_FILE("memory.h" HAVE_MEMORY_H)

CHECK_INCLUDE_FILE("arpa/inet.h" HAVE_ARPA_INET_H)
CHECK_INCLUDE_FILE("arpa/nameser.h" HAVE_ARPA_NAMESER_H)
CHECK_INCLUDE_FILE("netinet/in.h" HAVE_NETINET_IN_H)
CHECK_INCLUDE_FILE("sys/socket.h" HAVE_SYS_SOCKET_H)
CHECK_INCLUDE_FILE("sys/stat.h" HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/time.h" HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILE("sys/select.h" HAVE_SYS_SELECT_H)
CHECK_INCLUDE_FILE("sys/stat.h" HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/mman.h" HAVE_SYS_MMAN_H)
CHECK_INCLUDE_FILE("sys/timeb.h" HAVE_SYS_TIMEB_H)

# same functionality as configure script
CHECK_FUNCTION_EXISTS("isnan" HAVE_ISNAN)
CHECK_FUNCTION_EXISTS("isnand" HAVE_ISNAND)
CHECK_FUNCTION_EXISTS("isinf" HAVE_ISINF)
CHECK_FUNCTION_EXISTS("finite" HAVE_FINITE)
CHECK_FUNCTION_EXISTS("fp_class" HAVE_FP_CLASS)
CHECK_FUNCTION_EXISTS("class" HAVE_CLASS)
CHECK_FUNCTION_EXISTS("fpclass" HAVE_FPCLASS)
CHECK_FUNCTION_EXISTS("gettimeofday" HAVE_GETTIMEOFDAY)
CHECK_FUNCTION_EXISTS("strdup" HAVE_STRDUP)
CHECK_FUNCTION_EXISTS("strndup" HAVE_STRNDUP)
CHECK_FUNCTION_EXISTS("strerror" HAVE_STRERROR)
CHECK_FUNCTION_EXISTS("strftime" HAVE_STRFTIME)
CHECK_FUNCTION_EXISTS("localtime" HAVE_STRFTIME)
CHECK_FUNCTION_EXISTS("stat" HAVE_STAT)
CHECK_FUNCTION_EXISTS("_stat" HAVE__STAT)
CHECK_FUNCTION_EXISTS("signal" HAVE_SIGNAL)

CHECK_FUNCTION_EXISTS("printf" HAVE_PRINTF)
CHECK_FUNCTION_EXISTS("sprintf" HAVE_SPRINTF)
CHECK_FUNCTION_EXISTS("fprintf" HAVE_FPRINTF)
CHECK_FUNCTION_EXISTS("snprintf" HAVE_SNPRINTF)
CHECK_FUNCTION_EXISTS("vfprintf" HAVE_VFPRINTF)
CHECK_FUNCTION_EXISTS("vsprintf" HAVE_VSPRINTF)
CHECK_FUNCTION_EXISTS("vsnprintf" HAVE_PRINTF)
CHECK_FUNCTION_EXISTS("sscanf" HAVE_SSCANF)

IF(NOT HAVE_PRINTF AND
   NOT HAVE_SPRINTF AND
   NOT HAVE_FPRINTF AND
   NOT HAVE_SNPRINTF AND
   NOT HAVE_VFPRINTF AND
   NOT HAVE_VSPRINTF AND
   NOT HAVE_PRINTF AND
   NOT HAVE_SSCANF)
  SET(WITH_TRIO 1)
ELSE(NOT HAVE_PRINTF AND
     NOT HAVE_SPRINTF AND
     NOT HAVE_FPRINTF AND
     NOT HAVE_SNPRINTF AND
     NOT HAVE_VFPRINTF AND
     NOT HAVE_VSPRINTF AND
     NOT HAVE_PRINTF AND
     NOT HAVE_SSCANF)
  SET(WITH_TRIO 0)
ENDIF(NOT HAVE_PRINTF AND
      NOT HAVE_SPRINTF AND
      NOT HAVE_FPRINTF AND
      NOT HAVE_SNPRINTF AND
      NOT HAVE_VFPRINTF AND
      NOT HAVE_VSPRINTF AND
      NOT HAVE_PRINTF AND
      NOT HAVE_SSCANF)

# Note: not building libxml2 with compression support (zlib package) and not
#       supporting ICONV, which are the same options used in the old SCIRun
#       Thirdparty configuration

SET(WITH_ZLIB 0)
SET(WITH_ICONV 0)

# Enable DOM like tree manipulation APIs
SET(WITH_TREE 1)

# Enable serialization/saving support
SET(WITH_OUTPUT 1)

# Enable default read/write support
SET(WITH_PUSH 1)
SET(WITH_READER 1)
SET(WITH_PATTERN 1)
SET(WITH_WRITER 0)
SET(WITH_SAX1 1)
SET(WITH_FTP 0)
SET(WITH_HTTP 0)
SET(WITH_VALID 1)
SET(WITH_HTML 1)
SET(WITH_LEGACY 1)
SET(WITH_C14N 1)
SET(WITH_CATALOG 1)
SET(WITH_DOCB 0)
SET(WITH_XPATH 1)
SET(WITH_XPTR 1)
SET(WITH_XINCLUDE 1)
SET(WITH_ISO8859X 1)
SET(WITH_DEBUG 1)
SET(WITH_MEM_DEBUG 0)
SET(WITH_RUN_DEBUG 0)
SET(WITH_REGEXPS 1)
SET(WITH_SCHEMAS 1)
SET(WITH_SCHEMATRON 0)

SET(WITH_MODULES 0)
# Using shared library extension for now
# If building static libraries, change to static library extension
# or disable modules?
SET(MODULE_EXTENSION ${CMAKE_SHARED_LIBRARY_SUFFIX})

CHECK_FUNCTION_EXISTS("getaddrinfo" HAVE_GETADDRINFO)
# check some well-known libraries
SET(INET_LIBRARIES "bsd socket inet")
IF(NOT HAVE_GETADDRINFO)
  CHECK_LIBRARY_EXISTS(${INET_LIBRARIES} "getaddrinfo" "" HAVE_GETADDRINFO)
ENDIF(NOT HAVE_GETADDRINFO)

IF(UNIX)
  SET(CMAKE_REQUIRED_INCLUDES /usr/include)
ENDIF(UNIX)

# Note: need win32 std C name mangling workaround?

# needed for config.h.in
SET(PACKAGE "libxml2")
SET(VERSION ${LIBXML_MAJOR_VERSION}.${LIBXML_MINOR_VERSION}.${LIBXML_MICRO_VERSION})
SET(PACKAGE_STRING "")
SET(PACKAGE_NAME "")
SET(PACKAGE_TARNAME "")
SET(PACKAGE_VERSION "")
SET(PACKAGE_BUGREPORT "")
SET(PROTOTYPES 1)

# needed for xmlversion.h.in
MATH(EXPR LIBXML_VERSION_NUMBER
       "${LIBXML_MAJOR_VERSION}*10000 + ${LIBXML_MINOR_VERSION}*100 + ${LIBXML_MICRO_VERSION}")

SET(LIBXML2_SRCS
     DOCBparser.c
     HTMLparser.c
     HTMLtree.c
     SAX.c
     SAX2.c
     c14n.c
     catalog.c
     chvalid.c
     debugXML.c
     dict.c
     encoding.c
     entities.c
     error.c
     globals.c
     hash.c
     legacy.c
     list.c
     #nanoftp.c
     #nanohttp.c
     parser.c
     parserInternals.c
     pattern.c
     relaxng.c
     schematron.c
     testdso.c
     threads.c
     tree.c
#     trio.c
#     trionan.c
#     triostr.c
     uri.c
     valid.c
     xinclude.c
     xlink.c
     xmlcatalog.c
     xmlIO.c
#     xmllint.c
     xmlmemory.c
     xmlmodule.c
     xmlreader.c
     xmlregexp.c
     xmlsave.c
     xmlschemas.c
     xmlschemastypes.c
     xmlstring.c
     xmlunicode.c
     xmlwriter.c
     xpath.c
     xpointer.c
   )

## make sure config.h.in defines are correct
IF(WIN32)
  CONFIGURE_FILE(
                  ${CMAKE_CURRENT_SOURCE_DIR}/include/win32config.h
                  ${CMAKE_CURRENT_BINARY_DIR}/include/config.h
                  COPYONLY
                )
ELSE(WIN32)
  CONFIGURE_FILE(
                  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
                  ${CMAKE_CURRENT_BINARY_DIR}/include/config.h
                )
ENDIF(WIN32)

CONFIGURE_FILE(
                ${CMAKE_CURRENT_SOURCE_DIR}/include/libxml/xmlversion.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/include/libxml/xmlversion.h
              )

ADD_DEFINITIONS(-DHAVE_CONFIG_H)

IF(WIN32 AND MSVC)
  # Bind application to the current version of the VC libraries
  ADD_DEFINITIONS(-D_BIND_TO_CURRENT_VCLIBS_VERSION)

  # MSVC level 3 warnings are a little ridiculous
  SET(CMAKE_C_WARNING_LEVEL 1)
  IF(CMAKE_C_FLAGS MATCHES "/W[0-4]")
    STRING(REGEX REPLACE "/W[0-4]" "/W1"
            CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  ELSE(CMAKE_C_FLAGS MATCHES "/W[0-4]")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W1")
  ENDIF(CMAKE_C_FLAGS MATCHES "/W[0-4]")
ENDIF(WIN32 AND MSVC)

SET(SCI_LIBXML2_LIBRARY xml2 CACHE INTERNAL "Libxml2 library name." FORCE)

SET(SCI_LIBXML2_INCLUDE
     ${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/include
     ${CMAKE_CURRENT_BINARY_DIR}/include
	 ${CMAKE_CURRENT_BINARY_DIR}/include/libxml
    CACHE INTERNAL
      "Public libxml2 include directories."
    FORCE
   )

INCLUDE_DIRECTORIES( ${SCI_LIBXML2_INCLUDE} )
SCIRUN_ADD_LIBRARY(${SCI_LIBXML2_LIBRARY} ${LIBXML2_SRCS})

IF(UNIX)
  TARGET_LINK_LIBRARIES(
                         ${SCI_LIBXML2_LIBRARY}
                         ${DL_LIBRARY}
                       )
ENDIF(UNIX)

IF(ENABLE_PIC)
  SET_TARGET_PROPERTIES(${SCI_LIBXML2_LIBRARY}
                        PROPERTIES
                          COMPILE_FLAGS "-fPIC"
                       )
ENDIF(ENABLE_PIC)

IF(WIN32 AND MSVC)
  # Turn off Visual Studio warnings about strcpy et.al. being unsafe
  # for release builds (only supporting CMake defaults).
  SET_TARGET_PROPERTIES(${SCI_LIBXML2_LIBRARY}
                        PROPERTIES
                          COMPILE_DEFINITIONS_RELEASE "_CRT_SECURE_NO_WARNINGS=1"
                          COMPILE_DEFINITIONS_MINSIZEREL "_CRT_SECURE_NO_WARNINGS=1"
                       )
ENDIF(WIN32 AND MSVC)
