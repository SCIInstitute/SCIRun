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
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/BrainStimulator/SetConductivitiesToTetMeshAlgorithm.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::TestUtils;
using namespace SCIRun::Core::Algorithms;

namespace
{
  FieldHandle CreateTetMeshVectorOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_elem/vector/tet_vector_on_elem.fld");
  }
  FieldHandle CreateTetMeshScalarSevenElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh_7elem.fld");
  }
  FieldHandle CreateTetMeshScalarOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_elem/scalar/tet_scalar_on_elem.fld");
  }
  FieldHandle CreateTetMeshScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
}

TEST(SetConductivitiesToTetMeshAlgorithmTest, TetMeshScalarSevenElem)
{
  SetConductivitiesToTetMeshAlgorithm algo;
  
  double conductivities[] = {9.25, 25.1988, 3.5, 5.1988, 5.22, 22.2013};
  
  algo.set(SetConductivitiesToTetMeshAlgorithm::Skin,  conductivities[0]);
  algo.set(SetConductivitiesToTetMeshAlgorithm::Skull, conductivities[1]);
  algo.set(SetConductivitiesToTetMeshAlgorithm::CSF,   conductivities[2]);
  algo.set(SetConductivitiesToTetMeshAlgorithm::GM,    conductivities[3]);
  algo.set(SetConductivitiesToTetMeshAlgorithm::WM,    conductivities[4]);
  algo.set(SetConductivitiesToTetMeshAlgorithm::Electrode, conductivities[5]);
  
  FieldHandle input  = CreateTetMeshScalarSevenElem();
  FieldHandle output = algo.run(CreateTetMeshScalarSevenElem());
  
  VField* ivfield = input->vfield();
  VField* ovfield = output->vfield();
  int ival = 0;
  double oval = 0;
  for (VMesh::Elem::index_type i=0; i < ivfield->vmesh()->num_elems(); i++)
  {
    ivfield->get_value(ival, i);
    if (ival != 1 && ival != 2 && ival != 3 && ival != 4 && ival !=5 && ival !=6)
      FAIL() << "field value outside of range obtained " << std::endl;
    ovfield->get_value(oval, i);
    EXPECT_EQ(oval, conductivities[ival-1]);
  }
}

TEST(SetConductivitiesToTetMeshAlgorithmTest, TetMeshScalarThreeElem)
{
  SetConductivitiesToTetMeshAlgorithm algo;
  double conductivities[] = {9.25, 25.1988, 3.5};
  
  algo.set(SetConductivitiesToTetMeshAlgorithm::Skin,  conductivities[0]);
  algo.set(SetConductivitiesToTetMeshAlgorithm::Skull, conductivities[1]);
  algo.set(SetConductivitiesToTetMeshAlgorithm::CSF,   conductivities[2]);
  
  FieldHandle input  = CreateTetMeshScalarOnElem();
  FieldHandle output = algo.run(CreateTetMeshScalarOnElem());
  
  VField* ivfield = input->vfield();
  VField* ovfield = output->vfield();
  int ival = 0;
  double oval = 0;
  for (VMesh::Elem::index_type i=0; i < ivfield->vmesh()->num_elems(); i++)
  {
    ivfield->get_value(ival, i);
    if (ival != 1 && ival != 2 && ival != 3)
      FAIL() << "field value outside of range obtained " << std::endl;
    ovfield->get_value(oval, i);
    EXPECT_EQ(oval, conductivities[ival-1]);
  }
}

TEST(SetConductivitiesToTetMeshAlgorithmTest, TetMeshVector)
{
  SetConductivitiesToTetMeshAlgorithm algo;
  EXPECT_THROW(algo.run(CreateTetMeshVectorOnElem()), AlgorithmInputException);
}

TEST(SetConductivitiesToTetMeshAlgorithmTest, ThrowsForDataOnNode)
{
  SetConductivitiesToTetMeshAlgorithm algo;
  EXPECT_THROW(algo.run(CreateTetMeshScalarOnNode()), AlgorithmInputException);
}

TEST(SetConductivitiesToTetMeshAlgorithmTest, ThrowsForNullInput)
{
  SetConductivitiesToTetMeshAlgorithm algo;
  FieldHandle nullField;
  EXPECT_THROW(algo.run(nullField), AlgorithmInputException);
}
