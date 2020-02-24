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


SET(Engine_Scheduler_Tests_SRCS
  BoostGraphExampleTests.cc
  SchedulerBehavioralTests.cc
  SchedulingWithBoostGraph.cc
  BoostStateChartExampleTests.cc
)

#SET(Engine_Network_Tests_HEADERS
#  NetworkEditorControllerTests_.h
#)

SCIRUN_ADD_UNIT_TEST(Engine_Scheduler_Tests
  #${Engine_Network_Tests_HEADERS}
  ${Engine_Scheduler_Tests_SRCS}
)

TARGET_LINK_LIBRARIES(Engine_Scheduler_Tests
  Dataflow_Network
  Engine_Network
  Modules_Math
  Modules_Factory
  Algorithms_Math
  Core_Datatypes
  Dataflow_Network
  Dataflow_State
  Engine_Scheduler
  Algorithms_Factory
  gtest_main
  gtest
  gmock
  ${SCI_BOOST_LIBRARY}
)
