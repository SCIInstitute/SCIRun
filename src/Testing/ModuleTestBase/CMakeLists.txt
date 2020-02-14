#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2020 Scientific Computing and Imaging Institute,
#  University of Utah.
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


SET(Testing_ModuleTestBase_HEADERS
  ModuleTestBase.h
  share.h
)

SET(Testing_ModuleTestBase_SRCS
  ModuleTestBase.cc
)

SCIRUN_ADD_LIBRARY(Testing_ModuleTestBase
  ${Testing_ModuleTestBase_HEADERS}
  ${Testing_ModuleTestBase_SRCS}
)

TARGET_COMPILE_DEFINITIONS(Testing_ModuleTestBase PRIVATE -DGTEST_LINKED_AS_SHARED_LIBRARY)

TARGET_LINK_LIBRARIES(Testing_ModuleTestBase
  Core_Datatypes
  Modules_Factory
  Algorithms_Factory
  Dataflow_State
  gtest
  gmock
  ${SCI_BOOST_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Testing_ModuleTestBase)
ENDIF(BUILD_SHARED_LIBS)
