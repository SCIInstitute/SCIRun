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


#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/IEPlugin/NrrdField_Plugin.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::TestUtils;

namespace
{
  boost::filesystem::path testNrrd = TestResources::rootDir() / "ToolKits" / "FwdInvToolbox" / "pot_based_FEM_forward" / "Segmentation.nrrd";
  boost::filesystem::path testNrrdHeader = TestResources::rootDir() / "Fields" / "nrrd" / "fieldOut.nhdr";
}

TEST(ReadNrrdTests, CanReadFullNrrdFile)
{
  auto field = NrrdToField_reader(nullptr, testNrrd.string().c_str());

  ASSERT_TRUE(field != nullptr);
  EXPECT_EQ(18869130, field->vmesh()->num_nodes());
  EXPECT_EQ(18620000, field->vmesh()->num_elems());
  EXPECT_EQ(1, field->vmesh()->basis_order());
}

TEST(ReadNrrdTests, CanReadNrrdHeaderFile)
{
  auto field = NrrdToField_reader(nullptr, testNrrdHeader.string().c_str());

  ASSERT_TRUE(field != nullptr);
  EXPECT_EQ(19120104, field->vmesh()->num_nodes());
  EXPECT_EQ(18869130, field->vmesh()->num_elems());
  EXPECT_EQ(1, field->vmesh()->basis_order());
}

TEST(ReadNrrdTests, CanReadNrrdFile_Nodal)
{
  auto field = Nodal_NrrdToField_reader(nullptr, testNrrd.string().c_str());

  ASSERT_TRUE(field != nullptr);
  EXPECT_EQ(18869130, field->vmesh()->num_nodes());
  EXPECT_EQ(18620000, field->vmesh()->num_elems());
  EXPECT_EQ(1, field->vmesh()->basis_order());
}

TEST(ReadNrrdTests, CanReadNrrdFile_Modal)
{
  auto field = Modal_NrrdToField_reader(nullptr, testNrrd.string().c_str());

  ASSERT_TRUE(field != nullptr);
  EXPECT_EQ(19120104, field->vmesh()->num_nodes());
  EXPECT_EQ(18869130, field->vmesh()->num_elems());
  EXPECT_EQ(1, field->vmesh()->basis_order());
}

TEST(ReadNrrdTests, CanReadNrrdFile_IPNodal)
{
  auto field = IPNodal_NrrdToField_reader(nullptr, testNrrd.string().c_str());

  ASSERT_TRUE(field != nullptr);
  EXPECT_EQ(18869130, field->vmesh()->num_nodes());
  EXPECT_EQ(18620000, field->vmesh()->num_elems());
  EXPECT_EQ(1, field->vmesh()->basis_order());
}

TEST(ReadNrrdTests, CanReadNrrdFile_IPModal)
{
  auto field = IPModal_NrrdToField_reader(nullptr, testNrrd.string().c_str());

  ASSERT_TRUE(field != nullptr);
  EXPECT_EQ(19120104, field->vmesh()->num_nodes());
  EXPECT_EQ(18869130, field->vmesh()->num_elems());
  EXPECT_EQ(1, field->vmesh()->basis_order());
}

TEST(WriteNrrdTests, CanWriteNrrdFile)
{
  auto field = NrrdToField_reader(nullptr, testNrrd.string().c_str());

  ASSERT_TRUE(field != nullptr);
  EXPECT_EQ(18869130, field->vmesh()->num_nodes());
  EXPECT_EQ(18620000, field->vmesh()->num_elems());
  EXPECT_EQ(1, field->vmesh()->basis_order());

  boost::filesystem::path out(TestResources::rootDir() / "TransientOutput" / "fieldOutUnit.nrrd");
  ASSERT_TRUE(FieldToNrrd_writer(nullptr, field, out.string().c_str()));
}

TEST(WriteNrrdTests, CanWriteNrrdFileHeader)
{
  auto field = NrrdToField_reader(nullptr, testNrrd.string().c_str());

  ASSERT_TRUE(field != nullptr);
  EXPECT_EQ(18869130, field->vmesh()->num_nodes());
  EXPECT_EQ(18620000, field->vmesh()->num_elems());
  EXPECT_EQ(1, field->vmesh()->basis_order());

  boost::filesystem::path out(TestResources::rootDir() / "TransientOutput" / "fieldOutUnitHeader.nhdr");
  ASSERT_TRUE(FieldToNrrd_writer(nullptr, field, out.string().c_str()));
}
