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


SET(Algorithms_Base_SRCS
  AlgorithmBase.cc
  AlgorithmVariableNames.cc
)

SET(Algorithms_Base_HEADERS
  AlgorithmBase.h
  AlgorithmData.h
  AlgorithmFactory.h
  AlgorithmFwd.h
  AlgorithmInputBuilder.h
  AlgorithmInterfaces.h
  AlgorithmLogger.h
  AlgorithmMacros.h
  AlgorithmParameterHelper.h
  AlgorithmParameterList.h
  AlgorithmPreconditions.h
  AlgorithmStatusReporter.h
  AlgorithmVariableNames.h
  Name.h
  Option.h
  Variable.h
  share.h
)

SCIRUN_ADD_LIBRARY(Algorithms_Base
  ${Algorithms_Base_HEADERS}
  ${Algorithms_Base_SRCS}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Algorithms_Base)
ENDIF(BUILD_SHARED_LIBS)

TARGET_LINK_LIBRARIES(Algorithms_Base
  Core_Logging
  Core_Utils
  Core_Thread
)

SCIRUN_ADD_TEST_DIR(Tests)
