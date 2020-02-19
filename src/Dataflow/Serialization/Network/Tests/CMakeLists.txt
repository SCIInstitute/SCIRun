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


SET(Core_Serialization_Network_Tests_SRCS
  ModuleSerializationTests.cc
  NetworkSerializationTests.cc
  StateSerializationTests.cc
  LegacyNetworkFileImporterTests.cc
)

SET(Core_Serialization_Network_Tests_HEADERS
)

SCIRUN_ADD_UNIT_TEST(Core_Serialization_Network_Tests
  ${Core_Serialization_Network_Tests_HEADERS}
  ${Core_Serialization_Network_Tests_SRCS}
)

TARGET_LINK_LIBRARIES(Core_Serialization_Network_Tests
  Dataflow_Network
  Core_Serialization_Network
  Core_Serialization_Network_Importer
  Core_Datatypes
  Modules_Factory
  Dataflow_State
  Engine_Network
  Testing_Utils
  Core_ConsoleApplication
  Core_Application
  gtest_main
  gtest
  gmock
)
