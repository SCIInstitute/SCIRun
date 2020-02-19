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

   Author:              Moritz Dannhauer
   Last Modification:   April 21 2015
*/


#include <gtest/gtest.h>

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Legacy/Fields/MeshDerivatives/ExtractSimpleIsosurfaceAlgo.h>
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

FieldHandle LoadInTriangles()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tri.fld");
}

FieldHandle LoadInTetrahedrals()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_tet.fld");
}

FieldHandle LoadInLatVol()
{
   return loadFieldFromFile(TestResources::rootDir() / "Fields/extractsimpleisosurface/test_isosimsuf_latvol.fld");
}

TEST(ExtractSimpleIsoSurfaceAlgoTest, ExtractSimpleIsoSurf_Triangles_DataOnNodes)
{
  ExtractSimpleIsosurfaceAlgo algo;
  FieldHandle input_tiangle, output;
  input_tiangle=LoadInTriangles();
  std::vector<double> isovalues;
  isovalues.push_back(1.5);
  algo.run(input_tiangle,isovalues,output);
  EXPECT_EQ(output->vmesh()->num_nodes(),12);
  EXPECT_EQ(output->vmesh()->num_elems(),12);
  EXPECT_EQ(output->vfield()->num_values(),12);
}


TEST(ExtractSimpleIsoSurfaceAlgoTest, ExtractSimpleIsoSurf_Tetrahedrals_DataOnNodes)
{
  ExtractSimpleIsosurfaceAlgo algo;
  FieldHandle input_tiangle, output;
  input_tiangle=LoadInTetrahedrals();
  std::vector<double> isovalues;
  isovalues.push_back(0.0);
  algo.run(input_tiangle,isovalues,output);
  EXPECT_EQ(output->vmesh()->num_nodes(),60);
  EXPECT_EQ(output->vmesh()->num_elems(),116);
  EXPECT_EQ(output->vfield()->num_values(),60);
}

TEST(ExtractSimpleIsoSurfaceAlgoTest, ExtractSimpleIsoSurf_LatVol_DataOnNodes)
{
  ExtractSimpleIsosurfaceAlgo algo;
  FieldHandle input_tiangle, output;
  input_tiangle=LoadInLatVol();
  std::vector<double> isovalues;
  isovalues.push_back(0.3);
  algo.run(input_tiangle,isovalues,output);
  EXPECT_EQ(output->vmesh()->num_nodes(),5);
  EXPECT_EQ(output->vmesh()->num_elems(),3);
  EXPECT_EQ(output->vfield()->num_values(),5);
}
