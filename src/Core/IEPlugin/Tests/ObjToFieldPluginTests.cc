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
#include <Core/IEPlugin/ObjToField_Plugin.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/IEPlugin/PointCloudField_Plugin.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToPointCloudMeshAlgo.h>

/// TODO: intern
using namespace SCIRun::TestUtils;

TEST(ObjToFieldPluginTests, DISABLED_CanRead)
{
  FAIL() << "todo";
}

TEST(ObjToFieldPluginTests, DISABLED_CanWrite)
{
  FAIL() << "todo";
}

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;

TEST(PointCloudFieldTests, PrecisionOfNodePositions)
{
  FieldInformation lfi(LATVOLMESH_E, LINEARDATA_E, DOUBLE_E);
  Point minb(-1.0, -1.0, -1.0);
  Point maxb(1.0, 1.0, 1.0);
  MeshHandle mesh = CreateMesh(lfi, 3, 3, 3, minb, maxb);
  FieldHandle ofh = CreateField(lfi, mesh);


  ConvertMeshToPointCloudMeshAlgo algo;
  FieldHandle pc;
  algo.runImpl(ofh, pc);

  ASSERT_TRUE(pc->vmesh()->is_pointcloudmesh());

  boost::filesystem::path out(TestResources::rootDir() / "TransientOutput" / "v5pc.pts");
  ASSERT_TRUE(PointCloudFieldToText_writer(nullptr, pc, out.string().c_str()));
}
