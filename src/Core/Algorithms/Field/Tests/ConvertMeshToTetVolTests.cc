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
   Last Modification:   April 8 2015
*/


#include <gtest/gtest.h>
#include <Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/SetFieldData.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToTetVolMesh.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Utils/StringUtil.h>
#include <Core/Algorithms/Field/Tests/LoadFieldsForAlgoCoreTests.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromNodeToElem.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

class  ConvertMeshToTetVolTest : public ::testing::Test
{
protected:

  FieldHandle CreateEmptyLatVol(size_type sizex = 3, size_type sizey = 4, size_type sizez = 5, data_info_type type=DOUBLE_E)
  {
    FieldInformation lfi(LATVOLMESH_E, LINEARDATA_E, type);
    Point minb(-1.0, -1.0, -1.0);
    Point maxb(1.0, 1.0, 1.0);
    MeshHandle mesh = CreateMesh(lfi, sizex, sizey, sizez, minb, maxb);
    FieldHandle ofh = CreateField(lfi,mesh);
    return ofh;
  }
};


TEST_F(ConvertMeshToTetVolTest, ConvertLatVolToTetVol_ScalarDataDefinedOnNodes)
{
  auto size=10;
  FieldHandle latVol = CreateEmptyLatVol(size, size, size, INT_E);

  ConvertMeshToTetVolMeshAlgo algo;

  FieldHandle output;
  algo.run(latVol,output);

  EXPECT_EQ(output->vmesh()->num_nodes(),1000);
  EXPECT_EQ(output->vmesh()->num_elems(),3645);
  EXPECT_EQ(output->vfield()->num_values(),1000);
}

TEST_F(ConvertMeshToTetVolTest, ConvertLatVolToTetVol_ScalarDataDefinedOnElements)
{

  auto size=10;
  FieldHandle latVol = CreateEmptyLatVol(size, size, size, INT_E);

  MapFieldDataFromNodeToElemAlgo algo1;

  algo1.setOption(MapFieldDataFromNodeToElemAlgo::Method, "Min");

  FieldHandle latVol_data_on_ele = algo1.runImpl(latVol);

  ConvertMeshToTetVolMeshAlgo algo2;

  FieldHandle output;
  algo2.run(latVol_data_on_ele,output);

  EXPECT_EQ(output->vmesh()->num_nodes(),1000);
  EXPECT_EQ(output->vmesh()->num_elems(),3645);
  EXPECT_EQ(output->vfield()->num_values(),3645);
}
