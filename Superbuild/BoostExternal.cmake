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


set(_boost_git_url "https://github.com/CIBC-Internal/boost.git")
set(_boost_git_tag "v1.90.0")


ExternalProject_Add(Boost_external
  GIT_REPOSITORY            ${_boost_git_url}
  GIT_TAG                   ${_boost_git_tag}
  GIT_SHALLOW               FALSE
  GIT_PROGRESS              TRUE

  UPDATE_COMMAND            ${CMAKE_COMMAND} -E chdir <SOURCE_DIR> git submodule update --init --recursive

  BUILD_IN_SOURCE           OFF

  CONFIGURE_COMMAND
    ${CMAKE_COMMAND}
      -S <SOURCE_DIR>
      -B <BINARY_DIR>
      -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
      -DSCI_BOOST_CXX_FLAGS=${boost_CXX_Flags}



BUILD_COMMAND
    ${CMAKE_COMMAND} --build <BINARY_DIR> --config ${CMAKE_CFG_INTDIR}

INSTALL_COMMAND
    ${CMAKE_COMMAND} --build <BINARY_DIR> --target install --config ${CMAKE_CFG_INTDIR}

)

ExternalProject_Get_Property(Boost_external INSTALL_DIR)
ExternalProject_Get_Property(Boost_external SOURCE_DIR)

# Export the *installed* paths (not the source tree)
set(SCI_BOOST_INCLUDE     ${INSTALL_DIR}/include)
set(SCI_BOOST_LIBRARY_DIR ${INSTALL_DIR}/lib)
set(SCI_BOOST_USE_FILE    ${INSTALL_DIR}/UseBoost.cmake)

set(BOOST_PREFIX "boost_")
set(THREAD_POSTFIX "-mt")

set(SCI_BOOST_LIBRARY)
foreach(lib ${boost_Libraries})
  set(LIB_NAME "${BOOST_PREFIX}${lib}${THREAD_POSTFIX}")
  list(APPEND SCI_BOOST_LIBRARY ${LIB_NAME})
endforeach()

# If you still need your custom config/use files for SCIRun, keep these lines;
# otherwise consider relying on Boost's installed BoostConfig.cmake.
configure_file(${SUPERBUILD_DIR}/BoostConfig.cmake.in ${INSTALL_DIR}/BoostConfig.cmake @ONLY)
configure_file(${SUPERBUILD_DIR}/UseBoost.cmake        ${SCI_BOOST_USE_FILE}         COPYONLY)

set(Boost_DIR ${INSTALL_DIR} CACHE PATH "")
message(STATUS "Boost_DIR: ${Boost_DIR}")
