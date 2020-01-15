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


# Sources of Core/Exceptions classes

SET(Core_Exceptions_Legacy_SRCS
  ArrayIndexOutOfBounds.cc
  AssertionFailed.cc
  DimensionMismatch.cc
  ErrnoException.cc
  FileNotFound.cc
  InvalidState.cc
  InternalError.cc
)

SET(Core_Exceptions_Legacy_HEADERS
  ArrayIndexOutOfBounds.h
  AssertionFailed.h
  DimensionMismatch.h
  ErrnoException.h
  Exception.h
  FileNotFound.h
  GuiException.h
  IllegalValue.h
  InternalError.h
  InvalidState.h
  share.h
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Exceptions_Legacy)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_LIBRARY(Core_Exceptions_Legacy
  ${Core_Exceptions_Legacy_SRCS}
  ${Core_Exceptions_Legacy_HEADERS}
)

TARGET_LINK_LIBRARIES(Core_Exceptions_Legacy
  ${DL_LIBRARY}
)
