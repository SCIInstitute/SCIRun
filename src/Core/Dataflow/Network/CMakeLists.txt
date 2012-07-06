#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

SET(Core_Dataflow_Network_SRCS
  Connection.cc
  ConnectionId.cc
  Module.cc
  ModuleDescription.cc
  ModuleFactory.cc
  ModuleInterface.cc
  ModuleStateInterface.cc
  Network.cc
  Port.cc
  PortInterface.cc
)

SET(Core_Dataflow_Network_HEADERS
  Connection.h
  ConnectionId.h
  DataflowInterfaces.h
  Module.h
  ModuleFactory.h
  ModuleDescription.h
  ModuleInterface.h
  ModuleStateInterface.h
  Network.h
  NetworkFwd.h
  NetworkInterface.h
  Port.h
  PortInterface.h
  PortManager.h
  Share.h
)

ADD_LIBRARY(Core_Dataflow_Network 
${Core_Dataflow_Network_HEADERS}
${Core_Dataflow_Network_SRCS})

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Dataflow_Network)
ENDIF(BUILD_SHARED_LIBS)

ADD_SUBDIRECTORY(Tests)

TARGET_LINK_LIBRARIES(Core_Dataflow_Network
  Core_Datatypes
)