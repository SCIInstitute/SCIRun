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


SET(Algorithm_Layer_Tests_SRCS
  AlgorithmLayerTests.cc
)

SCIRUN_ADD_UNIT_TEST(Algorithm_Layer_Test
  ${Algorithm_Layer_Tests_SRCS}
)

TARGET_LINK_LIBRARIES(Algorithm_Layer_Test
  Algorithms_Factory
  Algorithms_Base
  Algorithms_BrainStimulator
  Core_Algorithms_Legacy_Fields
  Algorithms_Math
  Algorithms_DataIO
  Algorithms_Field
  Core_Algorithms_Legacy_FiniteElements
  Core_Algorithms_Legacy_Converter
  Dataflow_Network
  Engine_Network
  gtest_main
  gtest
  ${SCI_BOOST_LIBRARY}
)
