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

#include <cmath>
#include <gtest/gtest.h>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoElems.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoNodes.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MappingDataSource.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

namespace
{
  // Builds a LatVol field with linear (node) data equal to f(x,y,z) = scale * x.
  // Because f is linear, trilinear interpolation reproduces it exactly anywhere
  // inside the mesh, which makes the expected values in these tests exact rather
  // than approximate.
  FieldHandle makeLinearScalarLatVol(double scale = 1.0)
  {
    FieldInformation lfi("LatVolMesh", 1, "double");
    Point minb(0.0, 0.0, 0.0), maxb(2.0, 2.0, 2.0);
    MeshHandle mesh = CreateMesh(lfi, 3, 3, 3, minb, maxb);
    FieldHandle field = CreateField(lfi, mesh);

    VMesh* vmesh = field->vmesh();
    VField* vfield = field->vfield();
    vfield->resize_values();

    for (VMesh::Node::index_type idx = 0; idx < vmesh->num_nodes(); idx++)
    {
      Point p;
      vmesh->get_center(p, idx);
      vfield->set_value(scale * p.x(), idx);
    }
    return field;
  }

  FieldHandle makeConstantScalarLatVol(double value)
  {
    FieldInformation lfi("LatVolMesh", 1, "double");
    Point minb(0.0, 0.0, 0.0), maxb(2.0, 2.0, 2.0);
    MeshHandle mesh = CreateMesh(lfi, 3, 3, 3, minb, maxb);
    FieldHandle field = CreateField(lfi, mesh);

    VField* vfield = field->vfield();
    vfield->resize_values();
    for (VMesh::Node::index_type idx = 0; idx < field->vmesh()->num_nodes(); idx++)
      vfield->set_value(value, idx);
    return field;
  }
}

TEST(MappingDataSource, InterpolatedValueInsideDomain)
{
  MapFieldDataOntoNodesAlgo algo;
  FieldHandle source = makeLinearScalarLatVol();

  auto datasource = CreateDataSource(source, FieldHandle(), &algo);
  ASSERT_TRUE(datasource != nullptr);
  ASSERT_TRUE(datasource->is_scalar());

  double val = 0.0;
  datasource->get_data(val, Point(0.5, 1.3, 1.8));
  EXPECT_NEAR(0.5, val, 1e-12);
}

TEST(MappingDataSource, InterpolatedValueOutsideDomainUsesOutsideValue)
{
  MapFieldDataOntoNodesAlgo algo;
  algo.set(Parameters::OutsideValue, 42.0);
  FieldHandle source = makeLinearScalarLatVol();

  auto datasource = CreateDataSource(source, FieldHandle(), &algo);
  ASSERT_TRUE(datasource != nullptr);

  double val = 0.0;
  datasource->get_data(val, Point(100.0, 100.0, 100.0));
  EXPECT_NEAR(42.0, val, 1e-12);
}

TEST(MappingDataSource, InterpolatedDataOnlyReturnsNanOutsideDomain)
{
  // "interpolateddataonly" is only offered by MapFieldDataOntoElemsAlgo, but the
  // option is read straight out of the shared Parameters::InterpolationModel key,
  // so any algorithm exposing that key can drive CreateDataSource.
  MapFieldDataOntoElemsAlgo algo;
  algo.setOption(Parameters::InterpolationModel, "interpolateddataonly");
  FieldHandle source = makeLinearScalarLatVol();

  auto datasource = CreateDataSource(source, FieldHandle(), &algo);
  ASSERT_TRUE(datasource != nullptr);

  double val = 0.0;
  datasource->get_data(val, Point(100.0, 100.0, 100.0));
  EXPECT_TRUE(std::isnan(val));
}

TEST(MappingDataSource, ClosestNodeDataReturnsExactNodeValue)
{
  MapFieldDataOntoNodesAlgo algo;
  algo.setOption(Parameters::InterpolationModel, "closestnodedata");
  FieldHandle source = makeLinearScalarLatVol();

  auto datasource = CreateDataSource(source, FieldHandle(), &algo);
  ASSERT_TRUE(datasource != nullptr);

  // Node at (1,1,1) holds the exact value 1.0; querying near it (but not
  // exactly on it) should snap to that node's value rather than interpolate.
  double val = 0.0;
  datasource->get_data(val, Point(1.1, 0.9, 1.05));
  EXPECT_NEAR(1.0, val, 1e-12);
}

TEST(MappingDataSource, ClosestInterpolatedDataClampsAtMeshBoundary)
{
  MapFieldDataOntoNodesAlgo algo;
  algo.setOption(Parameters::InterpolationModel, "closestinterpolateddata");
  algo.set(Parameters::MaxDistance, 5.0);
  FieldHandle source = makeLinearScalarLatVol();

  auto datasource = CreateDataSource(source, FieldHandle(), &algo);
  ASSERT_TRUE(datasource != nullptr);

  // (2.2,1,1) is just outside the mesh (which spans x in [0,2]); the closest
  // point on the mesh is (2,1,1), where f(x)=x evaluates to 2.0.
  double val = 0.0;
  datasource->get_data(val, Point(2.2, 1.0, 1.0));
  EXPECT_NEAR(2.0, val, 1e-6);
}

TEST(MappingDataSource, WeightedInterpolationScalesValue)
{
  MapFieldDataOntoNodesAlgo algo;
  FieldHandle source = makeLinearScalarLatVol();
  FieldHandle weights = makeConstantScalarLatVol(2.0);

  auto datasource = CreateDataSource(source, weights, &algo);
  ASSERT_TRUE(datasource != nullptr);

  double val = 0.0;
  datasource->get_data(val, Point(0.5, 1.0, 1.0));
  EXPECT_NEAR(1.0, val, 1e-12);
}

TEST(MappingDataSource, GradientOfLinearFieldIsConstantUnitVector)
{
  MapFieldDataOntoNodesAlgo algo;
  algo.setOption(Parameters::Quantity, "gradient");
  FieldHandle source = makeLinearScalarLatVol();

  auto datasource = CreateDataSource(source, FieldHandle(), &algo);
  ASSERT_TRUE(datasource != nullptr);
  ASSERT_TRUE(datasource->is_vector());

  Vector grad;
  datasource->get_data(grad, Point(1.0, 1.0, 1.0));
  EXPECT_NEAR(1.0, grad.x(), 1e-10);
  EXPECT_NEAR(0.0, grad.y(), 1e-10);
  EXPECT_NEAR(0.0, grad.z(), 1e-10);
}

TEST(MappingDataSource, GradientNormOfLinearFieldIsOne)
{
  MapFieldDataOntoNodesAlgo algo;
  algo.setOption(Parameters::Quantity, "gradientnorm");
  FieldHandle source = makeLinearScalarLatVol();

  auto datasource = CreateDataSource(source, FieldHandle(), &algo);
  ASSERT_TRUE(datasource != nullptr);
  ASSERT_TRUE(datasource->is_double());

  double norm = 0.0;
  datasource->get_data(norm, Point(1.0, 1.0, 1.0));
  EXPECT_NEAR(1.0, norm, 1e-10);
}
