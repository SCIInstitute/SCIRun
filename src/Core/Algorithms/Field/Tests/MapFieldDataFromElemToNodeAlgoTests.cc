/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
#include <Core/Algorithms/Legacy/Fields/FieldData/MapFieldDataFromElemToNode.h>
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

  
  FieldHandle TetMesh()
  {     
   return loadFieldFromFile(TestResources::rootDir() / "test_mapfielddatafromelemtonode.fld");
  }

TEST(MapFieldDataFromElemToNodeTest, TetMeshTest)
{
 
 MapFieldDataFromElemToNodeAlgo algo;

 FieldHandle  result = algo.run(TetMesh());
  
 ASSERT_TRUE(result->vfield()->num_values() == 8);
  
 DenseMatrixHandle output(new DenseMatrix(8, 1));
 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 { 
   result->vfield()->get_value((*output)(idx, 0),idx);
 }
   DenseMatrix expected_result = MAKE_DENSE_MATRIX(
    (3.5,0)
    (3.5,0)
    (3.33333333333333333333333333333333333333333333333333333333,0)
    (2.0,0)
    (3.5,0)
    (2.5,0)
    (3.8,0)
    (4.33333333333333333333333333333333333333333333333333333333,0)
    );
 for (VMesh::Elem::index_type idx = 0; idx < result->vfield()->num_values(); idx++)
 {
   ASSERT_TRUE( expected_result(idx,0)-(*output)(idx, 0) < 1e-16);
 }
  
 /*std::cout << "Number of mesh elements: " <<  info->vmesh()->num_elems() << std::endl;
 std::cout << "Number of mesh nodes: " <<  info->vmesh()->num_nodes() << std::endl;
 std::cout << "Number of mesh values: " <<  info->vfield()->num_values() << std::endl;

 ASSERT_TRUE(info->vmesh()->num_elems() != 98650);
 ASSERT_TRUE(info->vmesh()->num_nodes() != 18367);
 ASSERT_TRUE(info->vfield()->num_values() != 98650);*/
 
}
