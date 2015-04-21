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

MACRO(EXTERNAL_TEEM_LIBRARY compress_type)

  SET(teem_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Externals/teem")
  SET(teem_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Externals/teem")

  SET(SCI_TEEM_INCLUDE ${teem_BINARY_DIR}/include CACHE INTERNAL "Teem include directories." FORCE)
  SET(SCI_TEEM_LIBRARY teem CACHE INTERNAL "Teem library name." FORCE)

  SET(teem_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})

  SET(teem_LIBRARY_PATH
    "${teem_BINARY_DIR}/bin/${teem_LIB_PREFIX}${SCI_TEEM_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}"
      CACHE INTERNAL "Teem library paths")
  # other configuration options are RELWITHDEBINFO and MINSIZEREL
  SET(teem_LIBRARY_PATH_RELEASE
    "${teem_BINARY_DIR}/bin/Release/${teem_LIB_PREFIX}${SCI_TEEM_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}"
	  CACHE INTERNAL "Teem library paths")
  SET(teem_LIBRARY_PATH_DEBUG
    "${teem_BINARY_DIR}/bin/Debug/${teem_LIB_PREFIX}${SCI_TEEM_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}"
	  CACHE INTERNAL "Teem library paths")

  ADD_DEFINITIONS(-DTEEM_STATIC)
 
  IF(${compress_type} MATCHES "GIT")

    SET(teem_GIT_TAG "origin/master")

    # TODO: fix install step
    #
    # If CMake ever allows overriding the checkout command or adding flags,
    # git checkout -q will silence message about detached head (harmless).
    ExternalProject_Add(Teem_external
      PREFIX ${teem_BINARY_DIR}
      GIT_REPOSITORY "https://github.com/CIBC-Internal/teem.git"
      GIT_TAG ${teem_GIT_TAG}
      SOURCE_DIR ${teem_SOURCE_DIR}
      BINARY_DIR ${teem_BINARY_DIR}
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/teem"
      PATCH_COMMAND ""
      ${DISABLED_UPDATE}
      INSTALL_DIR ""
      INSTALL_COMMAND ""
      #CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${teem_BINARY_DIR}"
      CMAKE_CACHE_ARGS
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
		-DSCI_TEEM_LIBRARY:FILEPATH=${SCI_TEEM_LIBRARY}
        -DZLIB_INCLUDE_DIR:PATH=${SCI_ZLIB_INCLUDE}
        -DZLIB_LIBRARY:FILEPATH=${zlib_LIBRARY_PATH}
        -DZLIB_LIBRARY_RELEASE:FILEPATH=${zlib_LIBRARY_PATH_RELEASE}
        -DZLIB_LIBRARY_DEBUG:FILEPATH=${zlib_LIBRARY_PATH_DEBUG}
    )

  ELSEIF(${compress_type} MATCHES "ZIP")
  ENDIF()

  ADD_LIBRARY(${SCI_TEEM_LIBRARY} STATIC IMPORTED GLOBAL)

  # adding Teem as a build target and dependency
  ADD_DEPENDENCIES(${SCI_TEEM_LIBRARY} Teem_external)
  ADD_DEPENDENCIES(Teem_external Zlib_external)
  ADD_DEPENDENCIES(${SCI_TEEM_LIBRARY} ${SCI_TEEM_INCLUDE})

  IF (CMAKE_GENERATOR MATCHES "Makefiles")
    SET_TARGET_PROPERTIES(${SCI_TEEM_LIBRARY}
      PROPERTIES
        IMPORTED_LOCATION ${teem_LIBRARY_PATH}
		IMPORTED_LINK_INTERFACE_LIBRARIES ${zlib_LIBRARY_PATH}
    )
  ELSE() # IDEs: Xcode, VS, others...
    SET_TARGET_PROPERTIES(${SCI_TEEM_LIBRARY}
      PROPERTIES
        IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
        IMPORTED_LOCATION_RELEASE ${teem_LIBRARY_PATH_RELEASE}
        IMPORTED_LOCATION_DEBUG ${teem_LIBRARY_PATH_DEBUG}
		IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE ${zlib_LIBRARY_PATH_RELEASE}
		IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG ${zlib_LIBRARY_PATH_DEBUG}
    )
  ENDIF()
ENDMACRO()
