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

FieldHandle CreateTetMeshScalarOnNode()
{
  return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
}

TEST(SetConductivitiesToTetMeshAlgorithm, spanks_test)
{
  SetConductivitiesToTetMeshAlgorithm algo;
  
  algo.set(SetConductivitiesToTetMeshAlgorithm::skin(),     16);
  algo.set(SetConductivitiesToTetMeshAlgorithm::skull(),    12);
  algo.set(SetConductivitiesToTetMeshAlgorithm::CSF(),       2);
  algo.set(SetConductivitiesToTetMeshAlgorithm::GM(),        3);
  algo.set(SetConductivitiesToTetMeshAlgorithm::WM(),        4);
  algo.set(SetConductivitiesToTetMeshAlgorithm::electrode(), 5);
  
  algo.run(CreateTetMeshScalarOnNode());
}
