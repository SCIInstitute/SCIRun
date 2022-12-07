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


SET(Dataflow_Network_SRCS
  Connection.cc
  ConnectionId.cc
  Module.cc
  ModuleDescription.cc
  ModuleFactory.cc
  ModuleInterface.cc
  ModuleStateInterface.cc
  Network.cc
  NetworkSettings.cc
  NullModuleState.cc
  Port.cc
  PortInterface.cc
  SimpleSourceSink.cc
)

SET(Dataflow_Network_HEADERS
  Connection.h
  ConnectionId.h
  DataflowInterfaces.h
  DefaultModuleFactories.h
  ExecutableObject.h
  GeometryGeneratingModule.h
  ModuleReexecutionStrategies.h
  ModuleTemplateImpl.h
  ModuleWithAsyncDynamicPorts.h
  Module.h
  ModuleBuilder.h
  ModuleFactory.h
  ModuleDescription.h
  ModuleInterface.h
  ModuleStateInterface.h
  ModuleDisplayInterface.h
  ModuleExceptions.h
  ModuleExecutionInterfaces.h
  ModuleIdGenerator.h
  ModuleInfoProvider.h
  ModulePortDescriptionTags.h
  ModuleTraits.h
  Network.h
  NetworkFwd.h
  NetworkInterface.h
  NetworkSettings.h
  NullModuleState.h
  Port.h
  PortNames.h
  PortInterface.h
  PortManager.h
  share.h
  SimpleSourceSink.h
)

SCIRUN_ADD_LIBRARY(Dataflow_Network
  ${Dataflow_Network_HEADERS}
  ${Dataflow_Network_SRCS}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Dataflow_Network)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)

TARGET_LINK_LIBRARIES(Dataflow_Network
  Core_Datatypes
  Core_Logging
  Algorithms_Base
  Algorithms_Describe
  ${SCI_BOOST_LIBRARY}
)
