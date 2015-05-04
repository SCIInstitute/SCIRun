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

MACRO(EXTERNAL_PYTHON_LIBRARY compress_type)

  SET(python_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Externals/python")
  SET(python_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Externals/python")

  # Python build directory included for pyconfig.h for *nix platforms. Windows builds use hardcoded config header.
  SET(SCI_PYTHON_INCLUDE ${python_SOURCE_DIR}/Include ${python_BINARY_DIR} CACHE INTERNAL "Python include directories." FORCE)
  SET(SCI_PYTHON_LIBRARY python CACHE INTERNAL "Python library name." FORCE)

  # TODO: create variable for python module library
  #SET(PYTHON_MODULE_SEARCH_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/pythonlib.zip" CACHE INTERNAL "Python modules." FORCE)
  SET(PYTHON_MODULE_SEARCH_PATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" CACHE INTERNAL "Python modules." FORCE)

  IF(WIN32)
    SET(python_LIB_PREFIX "")
  ELSE()
    SET(python_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
  ENDIF()

  SET(python_LIBRARY_PATH "${python_BINARY_DIR}/${python_LIB_PREFIX}${SCI_PYTHON_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  # other configuration options are RELWITHDEBINFO and MINSIZEREL
  SET(python_LIBRARY_PATH_RELEASE "${python_BINARY_DIR}/Release/${python_LIB_PREFIX}${SCI_PYTHON_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  SET(python_LIBRARY_PATH_DEBUG "${python_BINARY_DIR}/Debug/${python_LIB_PREFIX}${SCI_PYTHON_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")

  IF(${compress_type} MATCHES "GIT")

    SET(python_GIT_TAG "origin/master")

    # TODO: fix install step
    #
    # If CMake ever allows overriding the checkout command or adding flags,
    # git checkout -q will silence message about detached head (harmless).
    ExternalProject_Add(Python_external
      PREFIX ${python_BINARY_DIR}
      GIT_REPOSITORY "https://github.com/CIBC-Internal/python.git"
      GIT_TAG ${boost_GIT_TAG}
      SOURCE_DIR ${python_SOURCE_DIR}
      BINARY_DIR ${python_BINARY_DIR}
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/python"
      PATCH_COMMAND ""
      ${DISABLED_UPDATE}
      INSTALL_DIR ""
      INSTALL_COMMAND ""
      CMAKE_CACHE_ARGS
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
        -DPYTHONLIB_DST_PATH:STRING=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
        -DPYTHON_MODULE_SEARCH_PATH:INTERNAL=${PYTHON_MODULE_SEARCH_PATH}
        -DSCI_ZLIB_MANGLE:BOOL=${SCI_ZLIB_MANGLE}
        -DSCI_ZLIB_INCLUDE:PATH=${SCI_ZLIB_INCLUDE}
        -DSCI_ZLIB_LIBRARY:FILEPATH=${SCI_ZLIB_LIBRARY}
    )

  ELSEIF(${compress_type} MATCHES "ZIP")
    MESSAGE(STATUS "ZIP compress_type NOT implemented")
#    EXTERNALPROJECT_ADD (JPEG
#        URL ${JPEG_URL}
#        URL_MD5 ""
#        INSTALL_COMMAND ""
#        CMAKE_ARGS
#            -DBUILD_SHARED_LIBS:BOOL=OFF
#            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
#            -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#            -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
#            -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
#            -DCMAKE_ANSI_CFLAGS:STRING="${jpeg_pic}"
#    ) 
  ENDIF()

  #  EXTERNALPROJECT_GET_PROPERTY (JPEG BINARY_DIR SOURCE_DIR) 

  #  IF (${BLDTYPE} MATCHES "Debug")
  #    IF (WIN32 AND NOT MINGW)
  #      SET (DBG_EXT "_D")
  #    ELSE (WIN32 AND NOT MINGW)
  #      SET (DBG_EXT "_debug")
  #    ENDIF (WIN32 AND NOT MINGW)
  #  ELSE (${BLDTYPE} MATCHES "Debug")
  #    SET (DBG_EXT "")
  #  ENDIF (${BLDTYPE} MATCHES "Debug")
  #  SET (EXTERNAL_LIBRARY_LIST ${EXTERNAL_LIBRARY_LIST} ${JPEG_LIBRARY})

  #  IF (WIN32 AND NOT MINGW)
  #    SET (JPEG_LIBRARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${BLDTYPE}/libjpeg${DBG_EXT}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  #  ELSE (WIN32 AND NOT MINGW)
  #    SET (JPEG_LIBRARY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libjpeg${DBG_EXT}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  #  ENDIF (WIN32 AND NOT MINGW)
  #  SET (JPEG_INCLUDE_DIR_GEN "${BINARY_DIR}")
  #  SET (JPEG_INCLUDE_DIR "${SOURCE_DIR}/src")
  #  SET (JPEG_FOUND 1)
  #  SET (JPEG_LIBRARIES ${JPEG_LIBRARY})
  #  SET (JPEG_INCLUDE_DIRS ${JPEG_INCLUDE_DIR_GEN} ${JPEG_INCLUDE_DIR})

  ADD_LIBRARY(${SCI_PYTHON_LIBRARY} STATIC IMPORTED GLOBAL)

  ADD_DEPENDENCIES(${SCI_PYTHON_LIBRARY} Python_external)
  ADD_DEPENDENCIES(${SCI_PYTHON_LIBRARY} ${SCI_PYTHON_INCLUDE})

  ADD_DEPENDENCIES(Python_external Zlib_external)

  IF (CMAKE_GENERATOR MATCHES "Makefiles")
    SET_TARGET_PROPERTIES(${SCI_PYTHON_LIBRARY}
      PROPERTIES
        IMPORTED_LOCATION ${python_LIBRARY_PATH}
    )
  ELSE() # IDEs: Xcode, VS, others...
    SET_TARGET_PROPERTIES(${SCI_PYTHON_LIBRARY}
      PROPERTIES
        IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
        IMPORTED_LOCATION_RELEASE ${python_LIBRARY_PATH_RELEASE}
        IMPORTED_LOCATION_DEBUG ${python_LIBRARY_PATH_DEBUG}
    )
  ENDIF()

  INCLUDE_DIRECTORIES(${SCI_PYTHON_INCLUDE})
  ADD_DEFINITIONS(-DBUILD_WITH_PYTHON -DPy_NO_ENABLE_SHARED=1)

ENDMACRO()
