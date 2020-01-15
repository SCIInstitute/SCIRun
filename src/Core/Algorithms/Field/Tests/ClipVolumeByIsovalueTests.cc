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

   Author:             Moritz Dannhauer
   Last Modification:   April 21 2015
*/


#include <gtest/gtest.h>

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Legacy/Fields/ClipMesh/ClipMeshByIsovalue.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;

FieldHandle LoadTriangles()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tri.fld");
}

FieldHandle LoadTetrahedrals()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tet.fld");
}

FieldHandle LoadLatVol()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_latvol.fld");
}

TEST(ClipVolumeByIsovalueAlgoTest, ClipVolumeByIsovalue_Triangles_DataOnNodes)
{
  ClipMeshByIsovalueAlgo algo;
  FieldHandle input_tiangle, output;
  input_tiangle=LoadTriangles();
  algo.set(ClipMeshByIsovalueAlgo::ScalarIsoValue, 1.5);
  algo.set(ClipMeshByIsovalueAlgo::LessThanIsoValue, true);
  algo.run(input_tiangle,output);
  EXPECT_EQ(output->vmesh()->num_nodes(),65);
  EXPECT_EQ(output->vmesh()->num_elems(),116);
  EXPECT_EQ(output->vfield()->num_values(),65);
  algo.set(ClipMeshByIsovalueAlgo::LessThanIsoValue, false);
  algo.run(input_tiangle,output);
  EXPECT_EQ(output->vmesh()->num_nodes(),15);
  EXPECT_EQ(output->vmesh()->num_elems(),16);
  EXPECT_EQ(output->vfield()->num_values(),15);
}


TEST(ClipVolumeByIsovalueAlgoTest, ClipVolumeByIsovalue_Tetrahedrals_DataOnNodes)
{
  ClipMeshByIsovalueAlgo algo;
  FieldHandle input_tets, output;
  input_tets=LoadTetrahedrals();
  algo.set(ClipMeshByIsovalueAlgo::ScalarIsoValue, 0.0);
  algo.set(ClipMeshByIsovalueAlgo::LessThanIsoValue, false);
  algo.run(input_tets,output);
  EXPECT_EQ(output->vmesh()->num_nodes(),104);
  EXPECT_EQ(output->vmesh()->num_elems(),247);
  EXPECT_EQ(output->vfield()->num_values(),104);
  algo.set(ClipMeshByIsovalueAlgo::LessThanIsoValue, true);
  algo.run(input_tets,output);
  EXPECT_EQ(output->vmesh()->num_nodes(),236);
  EXPECT_EQ(output->vmesh()->num_elems(),610);
  EXPECT_EQ(output->vfield()->num_values(),236);
}


TEST(ClipVolumeByIsovalueAlgoTest, DISABLED_ClipVolumeByIsovalue_LatVol_DataOnNodes)
{
  ClipMeshByIsovalueAlgo algo;
  FieldHandle input_latvol, output;
  input_latvol=LoadLatVol();
  algo.set(ClipMeshByIsovalueAlgo::ScalarIsoValue, 0.0);
  algo.set(ClipMeshByIsovalueAlgo::LessThanIsoValue, false);
  //TODO: this run call triggers an invalid bounding box assertion. Let's fix the code
  algo.run(input_latvol,output);
  EXPECT_EQ(output->vmesh()->num_nodes(),0);
  EXPECT_EQ(output->vmesh()->num_elems(),0);
  EXPECT_EQ(output->vfield()->num_values(),0);
  algo.set(ClipMeshByIsovalueAlgo::LessThanIsoValue, true);
  algo.run(input_latvol,output);
  EXPECT_EQ(output->vmesh()->num_nodes(),8);
  EXPECT_EQ(output->vmesh()->num_elems(),1);
  EXPECT_EQ(output->vfield()->num_values(),8);
}
