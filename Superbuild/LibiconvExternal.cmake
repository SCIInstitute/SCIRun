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

SET_PROPERTY(DIRECTORY PROPERTY EP_BASE ${ep_base})

find_package(Iconv)

if(Iconv_FOUND)

  message(STATUS "Using system Iconv")

  set(LIBICONV_INCLUDE_DIR
      ${Iconv_INCLUDE_DIR}
      CACHE PATH "Iconv include dir")

  set(LIBICONV_LIBRARY
      ${Iconv_LIBRARY}
      CACHE FILEPATH "Iconv library")

  set(LIBICONV_DIR
      "SYSTEM"
      CACHE PATH "Iconv source")

else()

  message(STATUS "Iconv not found, building external libiconv")

  ExternalProject_Add(
    Libiconv_external

    GIT_REPOSITORY ${LIBICONV_GIT_URL}
    GIT_TAG        ${LIBICONV_GIT_TAG}

    UPDATE_COMMAND ""
    PATCH_COMMAND ""

    # TODO:
    # configure/build commands for your chosen iconv source

    INSTALL_DIR    ${ep_install_dir}

    CMAKE_CACHE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
  )

  ExternalProject_Get_Property(Libiconv_external BINARY_DIR)

  set(LIBICONV_DIR
      ${BINARY_DIR}
      CACHE PATH "Iconv build dir")

  if(WIN32)
    set(LIBICONV_LIBRARY
        ${BINARY_DIR}/lib/iconv.lib
        CACHE FILEPATH "Iconv library")
  endif()

endif()

message(STATUS "LIBICONV_INCLUDE_DIR: ${LIBICONV_INCLUDE_DIR}")
message(STATUS "LIBICONV_LIBRARY: ${LIBICONV_LIBRARY}")