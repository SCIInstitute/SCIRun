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


# Sources of Core/Parser classes

SET(Core_Parser_HEADERS
  ArrayMathEngine.h
  LinAlgEngine.h
  Parser.h
  ArrayMathFunctionCatalog.h
  LinAlgFunctionCatalog.h
  share.h
  ArrayMathInterpreter.h
  LinAlgInterpreter.h
)

SET(Core_Parser_SRCS
  ArrayMathFunctionElement.cc
  ArrayMathFunctionTensor.cc
  ArrayMathFunctionVector.cc
  ArrayMathFunctionScalar.cc
  ArrayMathFunctionBasic.cc
  ArrayMathFunctionCatalog.cc
  ArrayMathFunctionSourceSink.cc
  ArrayMathInterpreter.cc
  ArrayMathEngine.cc
  LinAlgFunctionSourceSink.cc
  LinAlgFunctionScalar.cc
  LinAlgFunctionBasic.cc
  LinAlgFunctionCatalog.cc
  LinAlgInterpreter.cc
  LinAlgEngine.cc
  Parser.cc
)

SCIRUN_ADD_LIBRARY(Core_Parser
  ${Core_Parser_HEADERS}
  ${Core_Parser_SRCS}
)

TARGET_LINK_LIBRARIES(Core_Parser
	Core_Datatypes_Legacy_Field
  #Core_Algorithms_Math
  Core_Datatypes)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Parser)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
