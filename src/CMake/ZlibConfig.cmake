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

MACRO(EXTERNAL_ZLIB_LIBRARY compress_type)

  SET(zlib_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Externals/zlib")
  SET(zlib_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Externals/zlib")

  SET(SCI_ZLIB_INCLUDE "${zlib_SOURCE_DIR};${zlib_BINARY_DIR}" CACHE INTERNAL "Zlib include directories." FORCE)
  SET(SCI_ZLIB_LIBRARY z CACHE INTERNAL "Zlib library name." FORCE)

  SET(zlib_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
  IF(WIN32)
    SET(DEBUG_POSTFIX "d")
  ENDIF()

  SET(zlib_LIBRARY_PATH "${zlib_BINARY_DIR}/${zlib_LIB_PREFIX}${SCI_ZLIB_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE INTERNAL "Zlib library paths")
  # other configuration options are RELWITHDEBINFO and MINSIZEREL
  SET(zlib_LIBRARY_PATH_RELEASE "${zlib_BINARY_DIR}/Release/${zlib_LIB_PREFIX}${SCI_ZLIB_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE INTERNAL "Zlib library paths")
  SET(zlib_LIBRARY_PATH_DEBUG "${zlib_BINARY_DIR}/Debug/${zlib_LIB_PREFIX}${SCI_ZLIB_LIBRARY}${DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}" CACHE INTERNAL "Zlib library paths")

  IF(${compress_type} MATCHES "GIT")

    SET(zlib_GIT_TAG "origin/master")

    # TODO: fix install step
    #
    # If CMake ever allows overriding the checkout command or adding flags,
    # git checkout -q will silence message about detached head (harmless).
    ExternalProject_Add(Zlib_external
      PREFIX ${zlib_BINARY_DIR}
      GIT_REPOSITORY "https://github.com/CIBC-Internal/zlib.git"
      GIT_TAG ${zlib_GIT_TAG}
      SOURCE_DIR ${zlib_SOURCE_DIR}
      BINARY_DIR ${zlib_BINARY_DIR}
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/zlib"
      PATCH_COMMAND ""
      ${DISABLED_UPDATE}
      INSTALL_DIR ""
      INSTALL_COMMAND ""
      #CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${zlib_BINARY_DIR}"
      CMAKE_CACHE_ARGS
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
        -DSCI_ZLIB_LIBRARY:FILEPATH=${SCI_ZLIB_LIBRARY}
        -DSCI_ZLIB_INCLUDE:PATH=${SCI_ZLIB_INCLUDE}
    )
  ELSEIF(${compress_type} MATCHES "ZIP")
  ENDIF()

  ADD_LIBRARY(${SCI_ZLIB_LIBRARY} STATIC IMPORTED GLOBAL)

  # adding Zlib as a build target and dependency
  ADD_DEPENDENCIES(${SCI_ZLIB_LIBRARY} Zlib_external)
  ADD_DEPENDENCIES(${SCI_ZLIB_LIBRARY} ${SCI_ZLIB_INCLUDE})

  IF(CMAKE_GENERATOR MATCHES "Makefiles")
    SET_TARGET_PROPERTIES(${SCI_ZLIB_LIBRARY}
      PROPERTIES
        IMPORTED_LOCATION ${zlib_LIBRARY_PATH}
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
