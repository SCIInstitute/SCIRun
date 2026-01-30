#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
#  
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software. 
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.


# ZlibExternal.cmake (modernized + consistent)

# ZlibExternal.cmake  —  installs zlib and exports an installed prefix for consumers

# ZlibExternal.cmake — install zlib and export a clean install prefix for consumers

set_property(DIRECTORY PROPERTY EP_BASE "${ep_base}")

set(zlib_GIT_TAG "v1.2.8")

# No output redirection; let install populate a normal prefix
set(_cmake_args
  -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  # (optional) -DBUILD_SHARED_LIBS=OFF
)

if(NOT CMAKE_CONFIGURATION_TYPES AND CMAKE_BUILD_TYPE)
  list(APPEND _cmake_args -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})
endif()

set(_zlib_src  "${CMAKE_BINARY_DIR}/Externals/Source/Zlib_external")
set(_zlib_bin  "${CMAKE_BINARY_DIR}/Externals/Build/Zlib_external")
set(_zlib_inst "${CMAKE_BINARY_DIR}/Externals/Install/Zlib_external")

ExternalProject_Add(Zlib_external
  # ...
  CMAKE_ARGS
    -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
    # Optional – prefer static libraries everywhere:
    # -DBUILD_SHARED_LIBS=OFF

  INSTALL_COMMAND "${CMAKE_COMMAND}" --build . --target install

  # Let the generator know which files must exist before dependents configure
  BUILD_BYPRODUCTS
    "${_zlib_inst}/include/zlib.h"
    "${_zlib_inst}/include/zconf.h"
    # We don't rely on exact names later, but byproducts help ordering.
    "${_zlib_inst}/lib/z.lib"
    "${_zlib_inst}/lib/libz.a"
    "${_zlib_inst}/lib/libz.so"
    "${_zlib_inst}/lib/libz.dylib"
)

# Export just the prefix (cached so other files/functions can see it)
ExternalProject_Get_Property(Zlib_external INSTALL_DIR)
set(ZLIB_INSTALL_DIR "${INSTALL_DIR}" CACHE PATH "zlib install prefix" FORCE)

# ---------------- Export for other externals (FreeType, etc.) ----------------
# Always export the install prefix and canonical include/lib dirs to the cache.
# IMPORTANT: do NOT test for file existence here — these files are built later.

# 1) Install prefix
set(ZLIB_INSTALL_DIR "${_zlib_inst}" CACHE PATH "zlib install prefix" FORCE)

# 2) Include directory that contains zlib.h (this is what FindZLIB expects)
set(ZLIB_INCLUDE_DIR "${ZLIB_INSTALL_DIR}/include" CACHE PATH "zlib include dir" FORCE)

# 3) Library directory (consumers may choose the exact filename later)
set(ZLIB_LIBRARY_DIR "${ZLIB_INSTALL_DIR}/lib" CACHE PATH "zlib library dir" FORCE)

# (Optional) Provide a best-guess library filename per platform *without* EXISTS checks.
# Consumers that need an exact file can adjust per their build (static/shared).
if(WIN32)
  set(_ZLIB_LIB_GUESS "${ZLIB_LIBRARY_DIR}/z.lib")
elseif(APPLE)
  # prefer shared name; many toolchains will still produce libz.a if shared is off
  set(_ZLIB_LIB_GUESS "${ZLIB_LIBRARY_DIR}/libz.dylib")
else()
  set(_ZLIB_LIB_GUESS "${ZLIB_LIBRARY_DIR}/libz.so")
endif()
set(ZLIB_LIBRARY "${_ZLIB_LIB_GUESS}" CACHE FILEPATH "zlib library (best guess)" FORCE)

message(STATUS "[Zlib_external] INSTALL_DIR=${ZLIB_INSTALL_DIR}")
message(STATUS "[Zlib_external] INCLUDE_DIR=${ZLIB_INCLUDE_DIR}")
message(STATUS "[Zlib_external] LIB_DIR=${ZLIB_LIBRARY_DIR}")
message(STATUS "[Zlib_external] LIB_GUESS=${ZLIB_LIBRARY}")