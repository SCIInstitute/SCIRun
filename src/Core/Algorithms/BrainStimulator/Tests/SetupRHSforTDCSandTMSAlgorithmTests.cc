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
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;

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

TEST(SetupRHSforTDCSandTMSAlgorithm, testing)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, 5.321);
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc2, 3.5);
  
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));
  (*m)(0,0) = 8;
  (*m)(1,0) = 1;
  (*m)(2,0) = 1;
  
  auto output = algo.run(CreateTetMeshScalarOnElem(), m);
  
  // DEBUG: displaying vector created
  for (int i=0; i<output->nrows(); i++)
    std::cout << i << " " << output->coeff(i,0) << std::endl;
}

//TODO: test all the way up to 128 electrodes. There seems to be 1 missing because of 0 to 126 and 1 to 128

TEST(SetupRHSforTDCSandTMSAlgorithm, ElectrodeNumberExceedsWhatIsPossible)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 360;
  EXPECT_THROW(algo.run(CreateTetMeshScalarOnElem(), m), AlgorithmInputException);
}
