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
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_elem/vector/tet_vector_on_elem.fld");
  }
  FieldHandle CreateTetMeshScalarSevenElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/tet_mesh_7elem.fld");
  }
  FieldHandle CreateTetMeshScalarOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_elem/scalar/tet_scalar_on_elem.fld");
  }
  FieldHandle CreateTetMeshScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
}

TEST(SetConductivitiesToTetMeshAlgorithmTest, TetMeshScalars)
{
  SetConductivitiesToMeshAlgorithm algo;

  double conductivities[] = {9.25, 25.1988, 18.99999, 3.5, 5.1988, 5.22, 22.2013};

  algo.set(Parameters::Skin,  conductivities[0]);
  algo.set(Parameters::SoftBone, conductivities[1]);
  algo.set(Parameters::HardBone, conductivities[2]);
  algo.set(Parameters::CSF,   conductivities[3]);
  algo.set(Parameters::GM,    conductivities[4]);
  algo.set(Parameters::WM,    conductivities[5]);
  algo.set(Parameters::Electrode, conductivities[6]);

  auto input  = CreateTetMeshScalarSevenElem();
  auto output = algo.run(input);

  VField* ivfield = input->vfield();
  VField* ovfield = output->vfield();
  int ival = 0;
  double oval = 0;
;
  for (VMesh::Elem::index_type i=0; i < ivfield->vmesh()->num_elems(); i++)
  {
    ivfield->get_value(ival, i);
    ovfield->get_value(oval, i);
    EXPECT_EQ(oval, conductivities[ival-1]);
  }
}

TEST(SetConductivitiesToTetMeshAlgorithmTest, TetMeshScalarSevenElem)
{
  SetConductivitiesToMeshAlgorithm algo;

  double conductivities[] = {9.25, 25.1988, 18.99999, 3.5, 5.1988, 5.22, 22.2013};

  algo.set(Parameters::Skin,  conductivities[0]);
  algo.set(Parameters::SoftBone, conductivities[1]);
  algo.set(Parameters::HardBone, conductivities[2]);
  algo.set(Parameters::CSF,   conductivities[3]);
  algo.set(Parameters::GM,    conductivities[4]);
  algo.set(Parameters::WM,    conductivities[5]);
  algo.set(Parameters::Electrode, conductivities[6]);

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
  SetConductivitiesToMeshAlgorithm algo;
  double conductivities[] = {9.25, 18.999, 123,456, 25.1988};

  algo.set(Parameters::Skin,  conductivities[0]);
  algo.set(Parameters::SoftBone, conductivities[1]);
  algo.set(Parameters::HardBone, conductivities[2]);
  algo.set(Parameters::CSF,   conductivities[4]);

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
  SetConductivitiesToMeshAlgorithm algo;
  EXPECT_THROW(algo.run(CreateTetMeshVectorOnElem()), AlgorithmInputException);
}

TEST(SetConductivitiesToTetMeshAlgorithmTest, ThrowsForDataOnNode)
{
  SetConductivitiesToMeshAlgorithm algo;
  EXPECT_THROW(algo.run(CreateTetMeshScalarOnNode()), AlgorithmInputException);
}

TEST(SetConductivitiesToTetMeshAlgorithmTest, ThrowsForNullInput)
{
  SetConductivitiesToMeshAlgorithm algo;
  FieldHandle nullField;
  EXPECT_THROW(algo.run(nullField), AlgorithmInputException);
}
