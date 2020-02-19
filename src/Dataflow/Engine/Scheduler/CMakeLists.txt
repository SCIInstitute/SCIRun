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


SET(Engine_Scheduler_SRCS
  BasicMultithreadedNetworkExecutor.cc
  BasicParallelExecutionStrategy.cc
  BoostGraphParallelScheduler.cc
  BoostGraphSerialScheduler.cc
  DesktopExecutionStrategyFactory.cc
  DynamicMultithreadedNetworkExecutor.cc
  DynamicParallelExecutionStrategy.cc
  ExecutionStrategy.cc
  GraphNetworkAnalyzer.cc
  LinearSerialNetworkExecutor.cc
  ParallelModuleExecutionOrder.cc
  SchedulerInterfaces.cc
  SerialModuleExecutionOrder.cc
  SerialExecutionStrategy.cc
)

SET(Engine_Scheduler_HEADERS
  BasicMultithreadedNetworkExecutor.h
  BasicParallelExecutionStrategy.h
  BoostGraphParallelScheduler.h
  BoostGraphSerialScheduler.h
  DesktopExecutionStrategyFactory.h
  DynamicMultithreadedNetworkExecutor.h
  DynamicParallelExecutionStrategy.h
  GraphNetworkAnalyzer.h
  ExecutionStrategy.h
  LinearSerialNetworkExecutor.h
  ParallelModuleExecutionOrder.h
  SchedulerInterfaces.h
  SerialModuleExecutionOrder.h
  SerialExecutionStrategy.h
  DynamicExecutor/WorkQueue.h
  DynamicExecutor/WorkUnitConsumer.h
  DynamicExecutor/WorkUnitExecutor.h
  DynamicExecutor/WorkUnitProducer.h
  DynamicExecutor/WorkUnitProducerInterface.h
  share.h
)

SCIRUN_ADD_LIBRARY(Engine_Scheduler
  ${Engine_Scheduler_HEADERS}
  ${Engine_Scheduler_SRCS}
)

TARGET_LINK_LIBRARIES(Engine_Scheduler
  Dataflow_Network
  Core_Thread
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Engine_Scheduler)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
