
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

SET(Interface_Modules_Factory_HEADERS
  ModuleDialogFactory.h
  share.h
)

SET(Interface_Modules_Factory_SOURCES 
  ModuleDialogFactory.cc
  ModuleDialogFactoryAdditional.cc
)

SCIRUN_ADD_LIBRARY(Interface_Modules_Factory 
  ${Interface_Modules_Factory_HEADERS}
  ${Interface_Modules_Factory_SOURCES}
)

TARGET_LINK_LIBRARIES(Interface_Modules_Factory 
  Interface_Modules_Base
  Interface_Modules_DataIO
  Interface_Modules_Math
  Interface_Modules_Fields
  Interface_Modules_Bundle
  Interface_Modules_FiniteElements
  Interface_Modules_BrainStimulator
  Interface_Modules_Forward
  Interface_Modules_Inverse
  Interface_Modules_Render
  Interface_Modules_String
  Interface_Modules_Matlab
  Interface_Modules_Teem
  Interface_Modules_Testing
  Interface_Modules_Visualization
  ${SCI_BOOST_LIBRARY}
  ${QT_LIBRARIES}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_Factory)
ENDIF(BUILD_SHARED_LIBS)
  
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
