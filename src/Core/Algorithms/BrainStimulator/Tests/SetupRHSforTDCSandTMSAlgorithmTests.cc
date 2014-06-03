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
  FieldHandle CreateTetMeshScalarOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_elem/scalar/tet_scalar_on_elem.fld");
  }
  FieldHandle CreateTetMeshScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
}

TEST(SetupRHSforTDCSandTMSAlgorithm, MatrixWithMoreThanOneElementTetMeshScalarOnElem)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, 5.321);
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc2, 0.3564);
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));
  (*m)(0,0) = 8;
  (*m)(1,0) = 1;
  (*m)(2,0) = 1;
  auto o = algo.run(CreateTetMeshScalarOnElem(), m);
  double compare [15] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.005321, -0.001, 0.0003564, 0.0, 0.0, 0.0, 0.0, 0.0,};
  for (int i=0; i<o->nrows(); i++)
    EXPECT_NEAR(compare[i], o->coeff(i,0), 0.000001);
}

TEST(SetupRHSforTDCSandTMSAlgorithm, MaximumElectrodesTetMeshScalarOnElem)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, 1.7415); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc1, 0.25526); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc2, -1.3065); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc3, 0.42186); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc4, -0.4539); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc5, -1.2746); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc6, -1.5449); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc7, -1.7244); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc8, 1.2369); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc9, 0.96567); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc10, -1.5915); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc11, -0.37345); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc12, -0.01536); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc13, -0.45124); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc14, -1.2777); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc15, -0.12756); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc16, 1.1917); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc17, 0.18443); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc18, 0.28836); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc19, -1.8642); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc20, -0.48022); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc21, -0.78156); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc22, -0.88066); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc23, 0.61242); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc24, -0.067962); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc25, -0.22678); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc26, 1.374); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc27, -0.24822); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc28, -0.77921); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc29, 0.29418); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc30, -1.3451); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc31, -0.92555); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc32, 0.55988); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc33, 0.18315); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc34, 0.45444); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc35, -1.1022); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc36, -0.93708); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc37, 0.27889); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc38, -0.77501); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc39, -1.7809); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc40, -1.1369); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc41, -0.82229); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc42, 1.5627); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc43, 0.44028); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc44, -0.36362); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc45, 0.16248); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc46, 1.7986); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc47, 1.0852); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc48, -1.3368); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc49, 1.9374); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc50, -1.4548); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc51, 1.6877); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc52, -0.67617); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc53, -0.0008453); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc54, 1.128); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc55, -0.28646); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc56, 1.5112); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc57, -0.68466); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc58, -0.39834); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc59, 1.7761); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc60, 1.7746); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc61, 1.39); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc62, 1.9215); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc63, 0.2593); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc64, -1.3289); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc65, -0.46956); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc66, -0.080518); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc67, -1.4555); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc68, 1.6092); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc69, -1.6322); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc70, 1.5679); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc71, -0.067987); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc72, 1.4274); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc73, -0.23748); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc74, 1.9817); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc75, -1.0667); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc76, -1.3948); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc77, -1.6673); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc78, -1.9479); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc79, -0.020532); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc80, 0.91431); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc81, 1.597); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc82, -0.031395); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc83, -0.34244); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc84, -1.0515); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc85, 1.6681); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc86, -0.64638); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc87, -0.067916); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc88, 1.3158); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc89, -1.7542); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc90, -0.36193); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc91, 0.23746); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc92, -1.7677); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc93, -0.62366); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc94, 0.19222); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc95, -1.3588); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc96, -0.13049); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc97, -0.21555); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc98, -0.72026); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc99, 0.53124); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc100, 1.0428); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc101, -0.31569); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc102, -1.4675); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc103, 0.37289); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc104, 1.5495); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc105, 0.22462); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc106, 0.79926); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc107, 0.39445); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc108, -1.3335); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc109, 0.38549); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc110, 0.4393); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc111, 0.5582); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc112, 1.439); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc113, 0.95948); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc114, 1.6527); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc115, 0.28241); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc116, 0.97087); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc117, 0.80978); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc118, 0.30057); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc119, 1.5232); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc120, 0.019732); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc121, -0.82563); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc122, 1.0257); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc123, -1.1403); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc124, 1.7036); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc125, 0.42447); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc126, 1.9864); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc127, 1.1173);  
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 128;
  auto o = algo.run(CreateTetMeshScalarOnElem(), m);
  double compare [135] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0017415, 0.00025526, -0.0013065, 0.00042186, -0.0004539, -0.0012746, -0.0015449, -0.0017244, 0.0012369, 0.00096567, -0.0015915, -0.00037345, -1.536e-05, -0.00045124, -0.0012777, -0.00012756, 0.0011917, 0.00018443, 0.00028836, -0.0018642, -0.00048022, -0.00078156, -0.00088066, 0.00061242, -6.7962e-05, -0.00022678, 0.001374, -0.00024822, -0.00077921, 0.00029418, -0.0013451, -0.00092555, 0.00055988, 0.00018315, 0.00045444, -0.0011022, -0.00093708, 0.00027889, -0.00077501, -0.0017809, -0.0011369, -0.00082229, 0.0015627, 0.00044028, -0.00036362, 0.00016248, 0.0017986, 0.0010852, -0.0013368, 0.0019374, -0.0014548, 0.0016877, -0.00067617, -8.453e-07, 0.001128, -0.00028646, 0.0015112, -0.00068466, -0.00039834, 0.0017761, 0.0017746, 0.00139, 0.0019215, 0.0002593, -0.0013289, -0.00046956, -8.0518e-05, -0.0014555, 0.0016092, -0.0016322, 0.0015679, -6.7987e-05, 0.0014274, -0.00023748, 0.0019817, -0.0010667, -0.0013948, -0.0016673, -0.0019479, -2.0532e-05, 0.00091431, 0.001597, -3.1395e-05, -0.00034244, -0.0010515, 0.0016681, -0.00064638, -6.7916e-05, 0.0013158, -0.0017542, -0.00036193, 0.00023746, -0.0017677, -0.00062366, 0.00019222, -0.0013588, -0.00013049, -0.00021555, -0.00072026, 0.00053124, 0.0010428, -0.00031569, -0.0014675, 0.00037289, 0.0015495, 0.00022462, 0.00079926, 0.00039445, -0.0013335, 0.00038549, 0.0004393, 0.0005582, 0.001439, 0.00095948, 0.0016527, 0.00028241, 0.00097087, 0.00080978, 0.00030057, 0.0015232, 1.9732e-05, -0.00082563, 0.0010257, -0.0011403, 0.0017036, 0.00042447, 0.0019864, 0.0011173};
  for (int i=0; i<o->nrows(); i++)
    EXPECT_NEAR(compare[i], o->coeff(i,0), 0.000001);
}


TEST(SetupRHSforTDCSandTMSAlgorithm, NegativeNumberOfElectrodesGivenTetMeshScalarOnElem)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = -5;
  EXPECT_THROW(algo.run(CreateTetMeshScalarOnElem(), m), AlgorithmInputException);
}
TEST(SetupRHSforTDCSandTMSAlgorithm, NegligbleElectrodeCurrentIntensitiesTetMeshScalarOnElem)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, 0.0001);
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc1, -0.0001);
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 8;
  EXPECT_THROW(algo.run(CreateTetMeshScalarOnElem(), m), AlgorithmInputException);
}
TEST(SetupRHSforTDCSandTMSAlgorithm, ElectrodeNumberExceedsWhatIsPossible)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 360;
  EXPECT_THROW(algo.run(CreateTetMeshScalarOnElem(), m), AlgorithmInputException);
}
TEST(SetupRHSforTDCSandTMSAlgorithm, NullFieldGivenAsInput)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 120;
  FieldHandle f;
  EXPECT_THROW(algo.run(f, m), AlgorithmInputException);
}
