#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2014 Scientific Computing and Imaging Institute,
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

MACRO(EXTERNAL_ZLIB_LIBRARY compress_type cmake_cxx_flags disabled_update)

  SET(zlib_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Externals/zlib")
  SET(zlib_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Externals/zlib")

  SET(SCI_ZLIB_INCLUDE ${zlib_SOURCE_DIR} CACHE INTERNAL "Zlib include directories." FORCE)
  SET(SCI_ZLIB_LIBRARY zlib CACHE INTERNAL "Zlib library name." FORCE)

  # TODO: prefix for windows build?

  SET(zlib_LIBRARY_PATH "${zlib_BINARY_DIR}/${SCI_ZLIB_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  # other configuration options are RELWITHDEBINFO and MINSIZEREL
  SET(zlib_LIBRARY_PATH_RELEASE "${zlib_BINARY_DIR}/Release/${SCI_ZLIB_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  SET(zlib_LIBRARY_PATH_DEBUG "${zlib_BINARY_DIR}/Debug/${SCI_ZLIB_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")

  IF(${compress_type} MATCHES "GIT")

    SET(zlib_GIT_TAG "origin/external_build")

    # TODO: fix install step
    #
    # If CMake ever allows overriding the checkout command or adding flags,
    # git checkout -q will silence message about detached head (harmless).
    ExternalProject_Add(Zlib_external
      PREFIX ${zlib_BINARY_DIR}
      GIT_REPOSITORY "https://github.com/SCIInstitute/zlib.git"
      GIT_TAG ${zlib_GIT_TAG}
      SOURCE_DIR ${zlib_SOURCE_DIR}
      BINARY_DIR ${zlib_BINARY_DIR}
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/zlib"
      PATCH_COMMAND ""
      ${disabled_update}
      INSTALL_DIR ""
      INSTALL_COMMAND ""
      #CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${zlib_BINARY_DIR}"
      CMAKE_CACHE_ARGS
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
        -DCMAKE_CXX_FLAGS:STRING=${cmake_cxx_flags}
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    )
  ELSEIF(${compress_type} MATCHES "ZIP")
  ENDIF()

  ADD_LIBRARY(${SCI_ZLIB_LIBRARY} STATIC IMPORTED GLOBAL)

  # adding Zlib as a build target and dependency
  ADD_DEPENDENCIES(${SCI_ZLIB_LIBRARY} Zlib_external)
  ADD_DEPENDENCIES(${SCI_ZLIB_LIBRARY} ${SCI_ZLIB_INCLUDE})

  IF (CMAKE_GENERATOR MATCHES "Makefiles")
    SET_TARGET_PROPERTIES(${SCI_ZLIB_LIBRARY}
      PROPERTIES
        IMPORTED_LOCATION ${boost_LIBRARY_PATH}
    )
  ELSE() # IDEs: Xcode, VS, others...
    SET_TARGET_PROPERTIES(${SCI_ZLIB_LIBRARY}
      PROPERTIES
        IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
        IMPORTED_LOCATION_RELEASE ${zlib_LIBRARY_PATH_RELEASE}
        IMPORTED_LOCATION_DEBUG ${zlib_LIBRARY_PATH_DEBUG}
    )
  ENDIF()
ENDMACRO()
