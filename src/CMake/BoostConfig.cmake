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
#
# code borrowed from ITK4 HDFMacros.cmake

MACRO(EXTERNAL_BOOST_LIBRARY compress_type)

  ADD_DEFINITIONS(-DBOOST_ALL_NO_LIB=1)

  SET(boost_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Externals/boost")
  SET(boost_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Externals/boost")

  SET(SCI_BOOST_INCLUDE ${boost_SOURCE_DIR} CACHE INTERNAL "Boost include directories." FORCE)
  SET(SCI_BOOST_LIBRARY boost CACHE INTERNAL "Boost library name." FORCE)

  IF(WIN32)
    # See cmake-2.8/Modules/FindBoost.cmake
    #SET(boost_LIB_PREFIX "lib")
    SET(boost_LIB_PREFIX "")
  ELSE()
    SET(boost_LIB_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
  ENDIF()

  IF(BUILD_WITH_PYTHON)
    ADD_DEFINITIONS(-DBOOST_PYTHON_STATIC_LIB=1)
  ENDIF()

  SET(boost_LIBRARY_PATH "${boost_BINARY_DIR}/${boost_LIB_PREFIX}${SCI_BOOST_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  # other configuration options are RELWITHDEBINFO and MINSIZEREL
  SET(boost_LIBRARY_PATH_RELEASE "${boost_BINARY_DIR}/Release/${boost_LIB_PREFIX}${SCI_BOOST_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")
  SET(boost_LIBRARY_PATH_DEBUG "${boost_BINARY_DIR}/Debug/${boost_LIB_PREFIX}${SCI_BOOST_LIBRARY}${CMAKE_STATIC_LIBRARY_SUFFIX}")

  IF(${compress_type} MATCHES "GIT")

    SET(boost_GIT_TAG "origin/master")

    # TODO: fix install step
    #
    # If CMake ever allows overriding the checkout command or adding flags,
    # git checkout -q will silence message about detached head (harmless).
    ExternalProject_Add(Boost_external
      PREFIX ${boost_BINARY_DIR}
      GIT_REPOSITORY "https://github.com/SCIInstitute/boost.git"
      GIT_TAG ${boost_GIT_TAG}
      SOURCE_DIR ${boost_SOURCE_DIR}
      BINARY_DIR ${boost_BINARY_DIR}
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/boost"
      PATCH_COMMAND ""
      ${DISABLED_UPDATE}
      INSTALL_DIR ""
      INSTALL_COMMAND ""
      #CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX:PATH=${boost_BINARY_DIR}"
      CMAKE_CACHE_ARGS
        -DCMAKE_VERBOSE_MAKEFILE:BOOL=${CMAKE_VERBOSE_MAKEFILE}
        -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
        -DBUILD_PYTHON:BOOL=${BUILD_WITH_PYTHON}
        -DSCI_PYTHON_INCLUDE:PATH=${SCI_PYTHON_INCLUDE}
        -DSCI_PYTHON_LIBRARY:FILEPATH=${SCI_PYTHON_LIBRARY}
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

  ADD_LIBRARY(${SCI_BOOST_LIBRARY} STATIC IMPORTED GLOBAL)

  # adding Boost as a build target and dependency
  #
  # TODO: how to make boost include dependent on Boost_external?
  ADD_DEPENDENCIES(${SCI_BOOST_LIBRARY} Boost_external)
  ADD_DEPENDENCIES(${SCI_BOOST_LIBRARY} ${SCI_BOOST_INCLUDE})

  IF(BUILD_WITH_PYTHON)
    ADD_DEPENDENCIES(Boost_external Python_external)
  ENDIF()

  IF (CMAKE_GENERATOR MATCHES "Makefiles")
    SET_TARGET_PROPERTIES(${SCI_BOOST_LIBRARY}
      PROPERTIES
        IMPORTED_LOCATION ${boost_LIBRARY_PATH}
    )
  ELSE() # IDEs: Xcode, VS, others...
    SET_TARGET_PROPERTIES(${SCI_BOOST_LIBRARY}
      PROPERTIES
        IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
        IMPORTED_LOCATION_RELEASE ${boost_LIBRARY_PATH_RELEASE}
        IMPORTED_LOCATION_DEBUG ${boost_LIBRARY_PATH_DEBUG}
    )
  ENDIF()

ENDMACRO()
