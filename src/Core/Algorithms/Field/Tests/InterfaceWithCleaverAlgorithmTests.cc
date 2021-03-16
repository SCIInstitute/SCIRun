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

#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Field/InterfaceWithCleaver2Algorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;


  FieldHandle BoxSignedDistanceField(bool negative)
  {

   FieldInformation f("LatVolMesh", 1, "float");
   size_type size = 3;
   Point minb(-1.0, -1.0, -1.0);
   Point maxb(1.0, 1.0, 1.0);
   MeshHandle omesh = CreateMesh(f, size, size, size, minb, maxb);
   FieldHandle ofield = CreateField(f,omesh);
   std::vector<double> values(27);

   values[0]=-1.73205077648162841796875;
   values[1]=-1.41421353816986083984375;
   values[2]=-1.73205077648162841796875;
   values[3]=-1.41421353816986083984375;
   values[4]=-1;
   values[5]=-1.41421353816986083984375;
   values[6]=-1.73205077648162841796875;
   values[7]=-1.41421353816986083984375;
   values[8]=-1.73205077648162841796875;
   values[9]=-1.41421353816986083984375;
   values[10]=-1;
   values[11]=-1.41421353816986083984375;
   values[12]=-1;
   values[13]=0;
   values[14]=-1;
   values[15]=-1.41421353816986083984375;
   values[16]=-1;
   values[17]=-1.41421353816986083984375;
   values[18]=-1.73205077648162841796875;
   values[19]=-1.41421353816986083984375;
   values[20]=-1.73205077648162841796875;
   values[21]=-1.41421353816986083984375;
   values[22]=-1;
   values[23]=-1.41421353816986083984375;
   values[24]=-1.73205077648162841796875;
   values[25]=-1.41421353816986083984375;
   values[26]=-1.73205077648162841796875;

   if (negative)
   {
     for (int i=0;i<27;i++)
     {
       values[i]=-values[i];
     }
   }

   ofield->vfield()->resize_values();
   ofield->vfield()->set_values(values);
   return ofield;
  }

TEST(CleaverInterfaceTest, SphereSignedDistanceFieldMatrix1)
{
  InterfaceWithCleaver2Algorithm algo;

  std::vector<FieldHandle> inputs;
  inputs.push_back(BoxSignedDistanceField(true));
  inputs.push_back(BoxSignedDistanceField(false));

  auto output = boost::dynamic_pointer_cast<Field>(algo.runImpl(inputs)[Variables::OutputField]);

  std::cout << "Number of mesh elements: " <<  output->vmesh()->num_elems() << std::endl;
  std::cout << "Number of mesh nodes: " <<  output->vmesh()->num_nodes() << std::endl;
  std::cout << "Number of mesh values: " <<  output->vfield()->num_values() << std::endl;

  EXPECT_EQ(output->vmesh()->num_elems(), 12);
  EXPECT_EQ(output->vmesh()->num_nodes(), 9);
  EXPECT_EQ(output->vfield()->num_values(), 12);

}
