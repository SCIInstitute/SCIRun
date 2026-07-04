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

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoNodes.h>
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
  // f(x,y,z) = x, stored as linear (node) data. Trilinear interpolation
  // reproduces a linear function exactly, so expected outputs below are exact.
  FieldHandle makeLinearScalarLatVol()
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
      vfield->set_value(p.x(), idx);
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

  FieldHandle makeConstantElemScalarLatVol()
  {
    FieldInformation lfi("LatVolMesh", 0, "double");
    Point minb(0.0, 0.0, 0.0), maxb(2.0, 2.0, 2.0);
    MeshHandle mesh = CreateMesh(lfi, 3, 3, 3, minb, maxb);
    FieldHandle field = CreateField(lfi, mesh);

    VMesh* vmesh = field->vmesh();
    VField* vfield = field->vfield();
    vfield->resize_values();
    for (VMesh::Elem::index_type idx = 0; idx < vmesh->num_elems(); idx++)
    {
      Point p;
      vmesh->get_center(p, idx);
      vfield->set_value(p.x(), idx);
    }
    return field;
  }

  FieldHandle makeNoDataLatVol()
  {
    FieldInformation lfi("LatVolMesh", -1, "double");
    Point minb(0.0, 0.0, 0.0), maxb(2.0, 2.0, 2.0);
    MeshHandle mesh = CreateMesh(lfi, 3, 3, 3, minb, maxb);
    return CreateField(lfi, mesh);
  }
}

TEST(MapFieldDataOntoNodesAlgoTest, IdentityMappingCopiesSourceValuesAtCoincidentNodes)
{
  MapFieldDataOntoNodesAlgo algo;
  FieldHandle source = makeLinearScalarLatVol();
  FieldHandle destination = source;

  FieldHandle output;
  ASSERT_TRUE(algo.runImpl(source, destination, output));
  ASSERT_TRUE(output != nullptr);

  VMesh* omesh = output->vmesh();
  VField* ofield = output->vfield();
  ASSERT_EQ(omesh->num_nodes(), ofield->num_values());

  for (VMesh::Node::index_type idx = 0; idx < omesh->num_nodes(); idx++)
  {
    Point p;
    omesh->get_center(p, idx);
    double val = 0.0;
    ofield->get_value(val, idx);
    EXPECT_NEAR(p.x(), val, 1e-10);
  }
}

TEST(MapFieldDataOntoNodesAlgoTest, GradientQuantityProducesConstantUnitVector)
{
  MapFieldDataOntoNodesAlgo algo;
  algo.setOption(Parameters::Quantity, "gradient");
  FieldHandle source = makeLinearScalarLatVol();
  FieldHandle destination = source;

  FieldHandle output;
  ASSERT_TRUE(algo.runImpl(source, destination, output));

  VField* ofield = output->vfield();
  for (VMesh::Node::index_type idx = 0; idx < output->vmesh()->num_nodes(); idx++)
  {
    Vector v;
    ofield->get_value(v, idx);
    EXPECT_NEAR(1.0, v.x(), 1e-8);
    EXPECT_NEAR(0.0, v.y(), 1e-8);
    EXPECT_NEAR(0.0, v.z(), 1e-8);
  }
}

TEST(MapFieldDataOntoNodesAlgoTest, GradientNormQuantityProducesOne)
{
  MapFieldDataOntoNodesAlgo algo;
  algo.setOption(Parameters::Quantity, "gradientnorm");
  FieldHandle source = makeLinearScalarLatVol();
  FieldHandle destination = source;

  FieldHandle output;
  ASSERT_TRUE(algo.runImpl(source, destination, output));

  VField* ofield = output->vfield();
  for (VMesh::Node::index_type idx = 0; idx < output->vmesh()->num_nodes(); idx++)
  {
    double val = 0.0;
    ofield->get_value(val, idx);
    EXPECT_NEAR(1.0, val, 1e-8);
  }
}

TEST(MapFieldDataOntoNodesAlgoTest, WeightsScaleOutputValues)
{
  MapFieldDataOntoNodesAlgo algo;
  FieldHandle source = makeLinearScalarLatVol();
  FieldHandle weights = makeConstantScalarLatVol(3.0);
  FieldHandle destination = source;

  FieldHandle output;
  ASSERT_TRUE(algo.runImpl(source, weights, destination, output));

  VMesh* omesh = output->vmesh();
  VField* ofield = output->vfield();
  for (VMesh::Node::index_type idx = 0; idx < omesh->num_nodes(); idx++)
  {
    Point p;
    omesh->get_center(p, idx);
    double val = 0.0;
    ofield->get_value(val, idx);
    EXPECT_NEAR(3.0 * p.x(), val, 1e-9);
  }
}

TEST(MapFieldDataOntoNodesAlgoTest, MissingSourceFieldFails)
{
  MapFieldDataOntoNodesAlgo algo;
  FieldHandle output;
  EXPECT_FALSE(algo.runImpl(FieldHandle(), makeLinearScalarLatVol(), output));
}

TEST(MapFieldDataOntoNodesAlgoTest, MissingDestinationFieldFails)
{
  MapFieldDataOntoNodesAlgo algo;
  FieldHandle output;
  EXPECT_FALSE(algo.runImpl(makeLinearScalarLatVol(), FieldHandle(), output));
}

TEST(MapFieldDataOntoNodesAlgoTest, NoDataInSourceFieldFails)
{
  MapFieldDataOntoNodesAlgo algo;
  FieldHandle output;
  EXPECT_FALSE(algo.runImpl(makeNoDataLatVol(), makeLinearScalarLatVol(), output));
}

TEST(MapFieldDataOntoNodesAlgoTest, ClosestNodeDataRejectsElementalSource)
{
  MapFieldDataOntoNodesAlgo algo;
  algo.setOption(Parameters::InterpolationModel, "closestnodedata");
  FieldHandle output;
  EXPECT_FALSE(algo.runImpl(makeConstantElemScalarLatVol(), makeLinearScalarLatVol(), output));
}

TEST(MapFieldDataOntoNodesAlgoTest, FluxQuantityRequiresSurfaceDestination)
{
  MapFieldDataOntoNodesAlgo algo;
  algo.setOption(Parameters::Quantity, "flux");
  FieldHandle output;
  EXPECT_FALSE(algo.runImpl(makeLinearScalarLatVol(), makeLinearScalarLatVol(), output));
}
