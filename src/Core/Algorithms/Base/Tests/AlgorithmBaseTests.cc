/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


#include <gtest/gtest.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Application/Preferences/Preferences.h>

using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;

TEST(FilenameVariableTests, CanReplaceDataDirectoryWindows)
{
  Preferences::Instance().setDataDirectory("E:\\scirun\\trunk_ref\\SCIRunData", false);
  const std::string path = Preferences::Instance().dataDirectoryPlaceholder() + "\\aneurysm\\aneurysm-mra.lvs.fld";
  AlgorithmParameter fileParameter(Name("Filename"), path);
  EXPECT_EQ(path, fileParameter.toString());
  auto pathFromState = fileParameter.toFilename().string();
#ifdef WIN32
  EXPECT_EQ("E:\\scirun\\trunk_ref\\SCIRunData\\aneurysm\\aneurysm-mra.lvs.fld", pathFromState);
#endif
}

TEST(FilenameVariableTests, CanReplaceDataDirectoryMac)
{
  Preferences::Instance().setDataDirectory("/Users/scirun/trunk_ref/SCIRunData", false);
  const std::string path = Preferences::Instance().dataDirectoryPlaceholder() + "/aneurysm/aneurysm-mra.lvs.fld";
  AlgorithmParameter fileParameter(Name("Filename"), path);
  EXPECT_EQ(path, fileParameter.toString());
  auto pathFromState = fileParameter.toFilename().string();
  EXPECT_EQ("/Users/scirun/trunk_ref/SCIRunData/aneurysm/aneurysm-mra.lvs.fld", pathFromState);
}

TEST(FilenameVariableTests, CanReplaceDataDirectoryWindowsSlashAtEnd)
{
  Preferences::Instance().setDataDirectory("E:\\scirun\\trunk_ref\\SCIRunData\\", false);
  const std::string path = Preferences::Instance().dataDirectoryPlaceholder() + "\\aneurysm\\aneurysm-mra.lvs.fld";
  AlgorithmParameter fileParameter(Name("Filename"), path);
  EXPECT_EQ(path, fileParameter.toString());
  auto pathFromState = fileParameter.toFilename().string();
#ifdef WIN32
  EXPECT_EQ("E:\\scirun\\trunk_ref\\SCIRunData\\aneurysm\\aneurysm-mra.lvs.fld", pathFromState);
#endif
}

TEST(FilenameVariableTests, CanReplaceDataDirectoryMacSlashAtEnd)
{
  Preferences::Instance().setDataDirectory("/Users/scirun/trunk_ref/SCIRunData/", false);
  const std::string path = Preferences::Instance().dataDirectoryPlaceholder() + "/aneurysm/aneurysm-mra.lvs.fld";
  AlgorithmParameter fileParameter(Name("Filename"), path);
  EXPECT_EQ(path, fileParameter.toString());
  auto pathFromState = fileParameter.toFilename().string();
#ifndef WIN32
  EXPECT_EQ("/Users/scirun/trunk_ref/SCIRunData/aneurysm/aneurysm-mra.lvs.fld", pathFromState);
#else
  // windows won't remove forward slash, doesn't matter as long as above test passes.
  EXPECT_EQ("/Users/scirun/trunk_ref/SCIRunData//aneurysm/aneurysm-mra.lvs.fld", pathFromState);
#endif
}

TEST(FilenameVariableTests, PreservesPathIfNoPlaceholder)
{
  Preferences::Instance().setDataDirectory("E:\\scirun\\trunk_ref\\SCIRunData", false);
  const std::string path = "E:\\scirun\\aneurysm\\aneurysm-mra.lvs.fld";
  AlgorithmParameter fileParameter(Name("Filename"), path);
  EXPECT_EQ(path, fileParameter.toString());
  auto pathFromState = fileParameter.toFilename().string();
  EXPECT_EQ("E:\\scirun\\aneurysm\\aneurysm-mra.lvs.fld", pathFromState);
}

TEST(NaNVaribleTests, CanHandleNaNValues)
{
  AlgorithmParameter nan1(Name("nan"), std::numeric_limits<double>::quiet_NaN());
  double value = nan1.toDouble();
  EXPECT_NE(value, value);
  std::string str = nan1.toString();
  EXPECT_EQ("NaN", str);

  nan1.setValue(1.0);
  value = nan1.toDouble();
  EXPECT_EQ(1.0, value);
  str = nan1.toString();
  EXPECT_EQ("", str);

  nan1.setValue(std::numeric_limits<double>::quiet_NaN());
  value = nan1.toDouble();
  EXPECT_NE(value, value);
  str = nan1.toString();
  EXPECT_EQ("NaN", str);
}
