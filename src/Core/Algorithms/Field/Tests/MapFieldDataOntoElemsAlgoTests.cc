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

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoElems.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/Point.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

namespace
{
  // f(x,y,z) = x, stored as linear (node) data over a 2x2x2 grid of unit cells
  // spanning [0,2]^3. Because f is linear, sampling it anywhere inside a cell
  // and averaging reproduces the value at the cell center exactly.
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

TEST(MapFieldDataOntoElemsAlgoTest, DefaultAverageMatchesCellCenterValue)
{
  MapFieldDataOntoElemsAlgo algo; // defaults: SamplePoints=regular1, SampleMethod=average
  FieldHandle source = makeLinearScalarLatVol();
  FieldHandle destination = source;

  FieldHandle output;
  ASSERT_TRUE(algo.runImpl(source, destination, output));

  VMesh* omesh = output->vmesh();
  VField* ofield = output->vfield();
  ASSERT_EQ(omesh->num_elems(), ofield->num_values());

  for (VMesh::Elem::index_type idx = 0; idx < omesh->num_elems(); idx++)
  {
    Point center;
    omesh->get_center(center, idx);
    double val = 0.0;
    ofield->get_value(val, idx);
    EXPECT_NEAR(center.x(), val, 1e-10);
  }
}

TEST(MapFieldDataOntoElemsAlgoTest, MinSampleMethodPicksSmallestSampledXCoordinate)
{
  MapFieldDataOntoElemsAlgo algo;
  algo.setOption(Parameters::SamplePoints, "regular2");
  algo.setOption(Parameters::SampleMethod, "min");
  FieldHandle source = makeLinearScalarLatVol();
  FieldHandle destination = source;

  FieldHandle output;
  ASSERT_TRUE(algo.runImpl(source, destination, output));

  VMesh* omesh = output->vmesh();
  VField* ofield = output->vfield();

  // regular2 samples each unit cell at local x-coordinates {1/3, 2/3}, so the
  // minimum sampled x is 1/6 below the cell center.
  for (VMesh::Elem::index_type idx = 0; idx < omesh->num_elems(); idx++)
  {
    Point center;
    omesh->get_center(center, idx);
    double val = 0.0;
    ofield->get_value(val, idx);
    EXPECT_NEAR(center.x() - 1.0 / 6.0, val, 1e-6);
  }
}

TEST(MapFieldDataOntoElemsAlgoTest, MaxSampleMethodPicksLargestSampledXCoordinate)
{
  MapFieldDataOntoElemsAlgo algo;
  algo.setOption(Parameters::SamplePoints, "regular2");
  algo.setOption(Parameters::SampleMethod, "max");
  FieldHandle source = makeLinearScalarLatVol();
  FieldHandle destination = source;

  FieldHandle output;
  ASSERT_TRUE(algo.runImpl(source, destination, output));

  VMesh* omesh = output->vmesh();
  VField* ofield = output->vfield();

  for (VMesh::Elem::index_type idx = 0; idx < omesh->num_elems(); idx++)
  {
    Point center;
    omesh->get_center(center, idx);
    double val = 0.0;
    ofield->get_value(val, idx);
    EXPECT_NEAR(center.x() + 1.0 / 6.0, val, 1e-6);
  }
}

TEST(MapFieldDataOntoElemsAlgoTest, WeightsScaleOutputValues)
{
  MapFieldDataOntoElemsAlgo algo;
  FieldHandle source = makeLinearScalarLatVol();
  FieldHandle weights = makeConstantScalarLatVol(2.0);
  FieldHandle destination = source;

  FieldHandle output;
  ASSERT_TRUE(algo.runImpl(source, weights, destination, output));

  VMesh* omesh = output->vmesh();
  VField* ofield = output->vfield();
  for (VMesh::Elem::index_type idx = 0; idx < omesh->num_elems(); idx++)
  {
    Point center;
    omesh->get_center(center, idx);
    double val = 0.0;
    ofield->get_value(val, idx);
    EXPECT_NEAR(2.0 * center.x(), val, 1e-9);
  }
}

TEST(MapFieldDataOntoElemsAlgoTest, MissingSourceFieldFails)
{
  MapFieldDataOntoElemsAlgo algo;
  FieldHandle output;
  EXPECT_FALSE(algo.runImpl(FieldHandle(), makeLinearScalarLatVol(), output));
}

TEST(MapFieldDataOntoElemsAlgoTest, MissingDestinationFieldFails)
{
  MapFieldDataOntoElemsAlgo algo;
  FieldHandle output;
  EXPECT_FALSE(algo.runImpl(makeLinearScalarLatVol(), FieldHandle(), output));
}

TEST(MapFieldDataOntoElemsAlgoTest, NoDataInSourceFieldFails)
{
  MapFieldDataOntoElemsAlgo algo;
  FieldHandle output;
  EXPECT_FALSE(algo.runImpl(makeNoDataLatVol(), makeLinearScalarLatVol(), output));
}

TEST(MapFieldDataOntoElemsAlgoTest, ClosestNodeDataRejectsElementalSource)
{
  MapFieldDataOntoElemsAlgo algo;
  algo.setOption(Parameters::InterpolationModel, "closestnodedata");
  FieldHandle output;
  EXPECT_FALSE(algo.runImpl(makeConstantElemScalarLatVol(), makeLinearScalarLatVol(), output));
}

TEST(MapFieldDataOntoElemsAlgoTest, PointCloudDestinationThrows)
{
  // Only the 4-argument (weights-supporting) overload checks for a point
  // cloud destination; exercise that one explicitly.
  MapFieldDataOntoElemsAlgo algo;
  FieldInformation pfi("PointCloudMesh", 0, "double");
  FieldHandle destination = CreateField(pfi);

  FieldHandle output;
  EXPECT_THROW(algo.runImpl(makeLinearScalarLatVol(), FieldHandle(), destination, output),
    SCIRun::Core::Algorithms::AlgorithmInputException);
}
