#
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
#

# CMakeLists.txt for Dataflow/Modules/String

SET(Modules_Legacy_String_SRCS
  #GetNetworkFileName.cc
  #ChooseString.cc
  #CreateString.cc
  #ReportStringInfo.cc
  #JoinStrings.cc
  PrintMatrixIntoString.cc
  PrintStringIntoString.cc
  #GetFileName.cc
  SplitFileName.cc
)

SET(Modules_Legacy_String_HEADERS
  PrintMatrixIntoString.h
  PrintStringIntoString.h
  SplitFileName.h
)

SCIRUN_ADD_LIBRARY(Modules_Legacy_String ${Modules_Legacy_String_SRCS} ${Modules_Legacy_String_HEADERS})

TARGET_LINK_LIBRARIES(Modules_Legacy_String
  Core_Datatypes
  #Core_Exceptions
  #Core_Geom
  #Core_Geometry
  #Core_GeomInterface
  #Dataflow_GuiInterface
  #Core_Persistent
  #Core_Thread
  #Dataflow_TkExtensions
  #Core_Util
  Dataflow_Network
  #${TK_LIBRARY}
  #${GL_LIBRARY}
  #${M_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Modules_Legacy_String)
ENDIF(BUILD_SHARED_LIBS)
