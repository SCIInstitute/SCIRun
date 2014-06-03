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
  FieldHandle CreateTetMeshVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_node/vector/tet_vector_on_node.fld");
  }
  FieldHandle CreateTetMeshScalarOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_elem/scalar/tet_scalar_on_elem.fld");
  }
  FieldHandle CreateTetMeshScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tet_mesh/data_defined_on_node/scalar/tet_scalar_on_node.fld");
  }
  
  FieldHandle CreateTriSurfVectorOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tri_surf/data_defined_on_elem/vector/tri_vector_on_elem.fld");
  }
  FieldHandle CreateTriSurfVectorOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tri_surf/data_defined_on_node/vector/tri_vector_on_node.fld");
  }
  FieldHandle CreateTriSurfScalarOnElem()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tri_surf/data_defined_on_elem/scalar/tri_scalar_on_elem.fld");
  }
  FieldHandle CreateTriSurfScalarOnNode()
  {
    return loadFieldFromFile(TestResources::rootDir() / "_etfielddata/tri_surf/data_defined_on_node/scalar/tri_scalar_on_node.fld");
  }
}

TEST(SetupRHSforTDCSandTMSAlgorithm, TriSurfScalarOnElem)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, 5.871);
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc2, 0.3564);
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  auto o = algo.run(CreateTriSurfScalarOnElem(), m);
  double compare [15] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.005871, -0.001, 0.0003564, 0.0};
  for (int i=0; i<o->nrows(); i++)
    EXPECT_NEAR(compare[i], o->coeff(i,0), 0.000001);
}
TEST(SetupRHSforTDCSandTMSAlgorithm, TriSurfScalarOnNode)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, 5.4444);
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc2, 0.3564);
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  auto o = algo.run(CreateTriSurfScalarOnNode(), m);
  double compare [15] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0054444, -0.001, 0.0003564, 0.0};
  for (int i=0; i<o->nrows(); i++)
    EXPECT_NEAR(compare[i], o->coeff(i,0), 0.000001);
}

TEST(SetupRHSforTDCSandTMSAlgorithm, MaximunElectrodesTriSurfVectorOnElem)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, 1.0728); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc1, 0.41484); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc2, -0.82031); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc3, -0.23424); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc4, -1.0265); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc5, -1.9819); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc6, -1.8497); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc7, -0.14027); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc8, -1.8555); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc9, 1.5375); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc10, 1.7942); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc11, 0.7826); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc12, -0.32077); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc13, -0.12386); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc14, -1.6048); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc15, -0.1622); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc16, 0.27208); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc17, -0.17067); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc18, -0.84966); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc19, 1.9333); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc20, -1.2037); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc21, 1.43); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc22, 0.086102); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc23, 0.97266); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc24, 0.52363); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc25, -0.64706); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc26, 1.3216); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc27, 0.88486); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc28, 1.2071); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc29, 1.2197); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc30, 0.69264); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc31, -1.5214); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc32, 0.11559); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc33, 0.38412); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc34, 1.2831); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc35, -1.8757); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc36, 0.37018); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc37, -1.3779); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc38, -0.4613); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc39, 0.49356); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc40, -1.5087); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc41, -1.1394); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc42, 0.14018); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc43, -1.2559); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc44, -1.5938); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc45, 0.28305); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc46, 0.66268); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc47, -0.38659); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc48, -1.608); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc49, -0.80827); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc50, 0.49018); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc51, -0.93982); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc52, -1.8164); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc53, -0.17125); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc54, -0.16042); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc55, 0.75556); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc56, 1.7151); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc57, 1.2721); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc58, 1.628); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc59, 0.14604); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc60, 1.9627); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc61, -1.6601); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc62, -0.57097); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc63, 1.9943); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc64, 1.8785); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc65, 1.1661); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc66, 0.18014); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc67, 1.4909); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc68, 1.9654); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc69, -0.90832); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc70, 0.58353); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc71, -1.1218); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc72, 0.25337); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc73, 0.25378); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc74, 1.8031); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc75, 1.8015); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc76, 1.6356); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc77, -0.37615); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc78, 1.0357); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc79, -1.3655); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc80, -1.4289); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc81, -1.8544); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc82, 0.044401); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc83, 1.1188); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc84, 1.0952); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc85, -0.14715); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc86, -1.0484); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc87, 0.60973); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc88, 0.15507); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc89, -0.5573); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc90, -1.6693); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc91, 1.3537); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc92, -0.86254); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc93, -0.31096); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc94, 0.2668); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc95, -1.537); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc96, 0.42779); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc97, 1.2841); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc98, -0.53333); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc99, -0.496); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc100, 0.68823); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc101, -1.2119); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc102, -1.9979); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc103, -1.9239); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc104, 1.4919); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc105, 1.071); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc106, 1.9055); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc107, -0.42973); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc108, 1.5568); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc109, 0.37016); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc110, -1.5363); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc111, -0.19253); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc112, 0.52399); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc113, 1.1666); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc114, 0.23623); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc115, -1.8248); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc116, 1.5039); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc117, 0.11695); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc118, -1.4276); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc119, 1.8264); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc120, -0.7665); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc121, -1.9159); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc122, 0.86693); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc123, 1.4649); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc124, -0.74112); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc125, -1.6378); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc126, -0.44348); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc127, -1.5999);
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 128;
  auto o = algo.run(CreateTriSurfVectorOnElem(), m);
  double compare [134] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0010728, 0.00041484, -0.00082031, -0.00023424, -0.0010265, -0.0019819, -0.0018497, -0.00014027, -0.0018555, 0.0015375, 0.0017942, 0.0007826, -0.00032077, -0.00012386, -0.0016048, -0.0001622, 0.00027208, -0.00017067, -0.00084966, 0.0019333, -0.0012037, 0.00143, 8.6102e-05, 0.00097266, 0.00052363, -0.00064706, 0.0013216, 0.00088486, 0.0012071, 0.0012197, 0.00069264, -0.0015214, 0.00011559, 0.00038412, 0.0012831, -0.0018757, 0.00037018, -0.0013779, -0.0004613, 0.00049356, -0.0015087, -0.0011394, 0.00014018, -0.0012559, -0.0015938, 0.00028305, 0.00066268, -0.00038659, -0.001608, -0.00080827, 0.00049018, -0.00093982, -0.0018164, -0.00017125, -0.00016042, 0.00075556, 0.0017151, 0.0012721, 0.001628, 0.00014604, 0.0019627, -0.0016601, -0.00057097, 0.0019943, 0.0018785, 0.0011661, 0.00018014, 0.0014909, 0.0019654, -0.00090832, 0.00058353, -0.0011218, 0.00025337, 0.00025378, 0.0018031, 0.0018015, 0.0016356, -0.00037615, 0.0010357, -0.0013655, -0.0014289, -0.0018544, 4.4401e-05, 0.0011188, 0.0010952, -0.00014715, -0.0010484, 0.00060973, 0.00015507, -0.0005573, -0.0016693, 0.0013537, -0.00086254, -0.00031096, 0.0002668, -0.001537, 0.00042779, 0.0012841, -0.00053333, -0.000496, 0.00068823, -0.0012119, -0.0019979, -0.0019239, 0.0014919, 0.001071, 0.0019055, -0.00042973, 0.0015568, 0.00037016, -0.0015363, -0.00019253, 0.00052399, 0.0011666, 0.00023623, -0.0018248, 0.0015039, 0.00011695, -0.0014276, 0.0018264, -0.0007665, -0.0019159, 0.00086693, 0.0014649, -0.00074112, -0.0016378, -0.00044348, -0.0015999};
  for (int i=0; i<o->nrows(); i++)
    EXPECT_NEAR(compare[i], o->coeff(i,0), 0.000001);
}
TEST(SetupRHSforTDCSandTMSAlgorithm, MaximunElectrodesTriSurfVectorOnNode)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, -1.1589); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc1, 1.0585); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc2, 0.20792); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc3, -0.27743); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc4, 1.7544); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc5, 0.014844); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc6, -0.56926); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc7, 1.1149); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc8, -1.3482); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc9, -1.8843); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc10, -1.3855); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc11, -0.023998); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc12, 0.8845); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc13, -0.98647); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc14, 1.0523); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc15, -1.8294); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc16, -0.48104); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc17, -0.40105); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc18, 1.3096); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc19, 1.6386); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc20, 0.97895); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc21, -1.4343); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc22, -1.6443); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc23, -0.79854); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc24, -0.90073); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc25, 1.0805); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc26, -0.30421); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc27, -1.2425); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc28, 0.38463); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc29, -1.9538); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc30, 1.64); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc31, 0.71624); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc32, -0.045636); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc33, 1.7576); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc34, -0.90275); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc35, -0.22622); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc36, -0.11744); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc37, -1.3199); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc38, -1.435); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc39, -1.1561); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc40, 0.72867); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc41, 0.19117); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc42, -1.0089); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc43, 1.5606); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc44, 1.561); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc45, 0.92047); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc46, -0.098549); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc47, -1.8906); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc48, -0.84163); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc49, 0.75256); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc50, 0.83417); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc51, -1.4014); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc52, -1.6466); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc53, -0.5571); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc54, 0.97852); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc55, 1.6453); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc56, -1.353); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc57, -0.33887); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc58, -1.7924); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc59, 1.7559); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc60, -0.33857); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc61, 0.85957); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc62, -1.3109); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc63, 1.2361); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc64, 0.22988); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc65, 0.42635); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc66, -1.3063); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc67, -0.70748); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc68, 1.1696); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc69, 0.72758); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc70, 1.4648); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc71, -0.68585); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc72, -0.28396); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc73, 1.3209); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc74, 1.4094); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc75, 0.095681); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc76, -1.6324); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc77, -0.2087); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc78, 1.8682); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc79, 1.7845); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc80, -1.2203); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc81, 0.1945); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc82, 0.46942); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc83, -1.1586); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc84, 0.54285); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc85, -0.1294); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc86, -0.84663); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc87, -1.4859); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc88, 0.68524); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc89, -0.60047); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc90, -1.8993); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc91, 1.6961); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc92, -0.34808); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc93, 0.34445); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc94, 0.76345); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc95, 0.94466); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc96, -1.0351); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc97, -1.9809); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc98, 1.2537); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc99, -1.1246); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc100, 1.2168); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc101, -1.9694); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc102, -1.209); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc103, 0.30309); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc104, -0.91546); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc105, 0.0065857); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc106, -1.236); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc107, 0.57396); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc108, 1.707); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc109, -0.36397); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc110, -1.6408); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc111, 1.2249); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc112, 0.27053); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc113, -0.99664); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc114, 1.8433); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc115, -1.7853); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc116, -1.0547); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc117, -0.23528); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc118, 1.2646); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc119, 0.0082047); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc120, -0.071382); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc121, -1.977); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc122, -1.4194); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc123, -1.8496); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc124, -0.71576); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc125, 0.38599); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc126, -1.7399); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc127, 0.082062); 
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 128;
  auto o = algo.run(CreateTriSurfVectorOnNode(), m);
  double compare [134] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.0011589, 0.0010585, 0.00020792, -0.00027743, 0.0017544, 1.4844e-05, -0.00056926, 0.0011149, -0.0013482, -0.0018843, -0.0013855, -2.3998e-05, 0.0008845, -0.00098647, 0.0010523, -0.0018294, -0.00048104, -0.00040105, 0.0013096, 0.0016386, 0.00097895, -0.0014343, -0.0016443, -0.00079854, -0.00090073, 0.0010805, -0.00030421, -0.0012425, 0.00038463, -0.0019538, 0.00164, 0.00071624, -4.5636e-05, 0.0017576, -0.00090275, -0.00022622, -0.00011744, -0.0013199, -0.001435, -0.0011561, 0.00072867, 0.00019117, -0.0010089, 0.0015606, 0.001561, 0.00092047, -9.8549e-05, -0.0018906, -0.00084163, 0.00075256, 0.00083417, -0.0014014, -0.0016466, -0.0005571, 0.00097852, 0.0016453, -0.001353, -0.00033887, -0.0017924, 0.0017559, -0.00033857, 0.00085957, -0.0013109, 0.0012361, 0.00022988, 0.00042635, -0.0013063, -0.00070748, 0.0011696, 0.00072758, 0.0014648, -0.00068585, -0.00028396, 0.0013209, 0.0014094, 9.5681e-05, -0.0016324, -0.0002087, 0.0018682, 0.0017845, -0.0012203, 0.0001945, 0.00046942, -0.0011586, 0.00054285, -0.0001294, -0.00084663, -0.0014859, 0.00068524, -0.00060047, -0.0018993, 0.0016961, -0.00034808, 0.00034445, 0.00076345, 0.00094466, -0.0010351, -0.0019809, 0.0012537, -0.0011246, 0.0012168, -0.0019694, -0.001209, 0.00030309, -0.00091546, 6.5857e-06, -0.001236, 0.00057396, 0.001707, -0.00036397, -0.0016408, 0.0012249, 0.00027053, -0.00099664, 0.0018433, -0.0017853, -0.0010547, -0.00023528, 0.0012646, 8.2047e-06, -7.1382e-05, -0.001977, -0.0014194, -0.0018496, -0.00071576, 0.00038599, -0.0017399, 8.2062e-05};
  for (int i=0; i<o->nrows(); i++)
    EXPECT_NEAR(compare[i], o->coeff(i,0), 0.000001);
}

TEST(SetupRHSforTDCSandTMSAlgorithm, TetMeshVectorOnNode)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, 5.321);
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc2, 0.3564);
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  auto o = algo.run(CreateTetMeshVectorOnNode(), m);
  double compare [15] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.005321, -0.001, 0.0003564, 0.0};
  for (int i=0; i<o->nrows(); i++)
    EXPECT_NEAR(compare[i], o->coeff(i,0), 0.000001);
}

TEST(SetupRHSforTDCSandTMSAlgorithm, TetMeshVectorOnElem)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, 5.321);
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc2, 0.3564);
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 4;
  auto o = algo.run(CreateTetMeshVectorOnElem(), m);
  double compare [15] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.005321, -0.001, 0.0003564, 0.0};
  for (int i=0; i<o->nrows(); i++)
    EXPECT_NEAR(compare[i], o->coeff(i,0), 0.000001);
}

TEST(SetupRHSforTDCSandTMSAlgorithm, MaximumElectrodesTetMeshScalarOnNode)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, 1.9948); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc1, 1.2433); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc2, 1.4301); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc3, 0.9277); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc4, 1.3004); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc5, 1.4454); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc6, -0.39554); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc7, -1.6042); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc8, -1.7575); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc9, 1.1153); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc10, 0.069314); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc11, 0.49596); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc12, -0.09512); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc13, 0.94051); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc14, 1.7266); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc15, 0.25469); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc16, 1.3046); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc17, 1.8956); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc18, -1.8187); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc19, 0.27761); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc20, -1.8462); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc21, -1.0497); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc22, 1.9706); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc23, -0.72015); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc24, -0.8238); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc25, -0.58052); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc26, -0.20924); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc27, 1.1276); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc28, -1.0895); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc29, 1.0879); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc30, 0.51654); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc31, -1.5637); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc32, -1.4255); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc33, 0.39027); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc34, 0.72792); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc35, -0.43454); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc36, -1.6408); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc37, -1.6364); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc38, -0.36638); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc39, 0.66971); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc40, -1.4123); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc41, 0.23041); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc42, -1.6174); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc43, 1.8673); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc44, -0.12971); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc45, -0.86599); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc46, 1.2828); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc47, 0.65387); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc48, 1.2491); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc49, 0.71229); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc50, 0.38457); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc51, -0.19427); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc52, 1.805); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc53, -0.21093); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc54, 0.90075); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc55, 1.034); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc56, 1.0274); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc57, 1.3834); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc58, -1.8257); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc59, -1.7426); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc60, -1.2749); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc61, -0.91305); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc62, 1.2968); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc63, 0.55852); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc64, 1.2448); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc65, 1.9952); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc66, 0.73432); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc67, -0.79847); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc68, -0.50102); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc69, -0.17225); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc70, 0.24385); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc71, 0.85029); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc72, -0.0066364); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc73, -1.0462); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc74, -1.0496); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc75, -0.30422); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc76, -0.99065); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc77, -0.58214); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc78, 1.5071); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc79, -0.017591); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc80, 0.61095); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc81, 1.6314); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc82, 1.4086); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc83, 1.2073); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc84, -0.68908); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc85, 0.27396); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc86, -0.77762); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc87, 0.97286); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc88, 1.961); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc89, -0.12913); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc90, 0.88243); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc91, -1.7849); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc92, -1.6258); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc93, 0.42926); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc94, 1.9006); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc95, -0.68575); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc96, -1.3487); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc97, -1.886); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc98, 1.7723); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc99, 0.1987); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc100, -0.32988); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc101, -0.74655); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc102, -0.87773); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc103, -0.60016); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc104, -0.90766); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc105, 0.50461); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc106, -0.32633); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc107, 1.5355); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc108, 1.7694); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc109, -1.9191); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc110, 0.86071); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc111, -0.3799); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc112, -1.8486); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc113, 1.2569); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc114, 1.2278); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc115, 0.63342); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc116, -1.7316); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc117, 0.43733); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc118, 0.90713); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc119, 1.3922); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc120, 0.41156); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc121, 0.50111); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc122, 1.4755); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc123, 0.25984); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc124, 1.6339); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc125, -0.45851); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc126, 1.9561); algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc127, -0.29304); 
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,1));
  (*m)(0,0) = 128;
  auto o = algo.run(CreateTetMeshScalarOnNode(), m);
  double compare [135] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0019948, 0.0012433, 0.0014301, 0.0009277, 0.0013004, 0.0014454, -0.00039554, -0.0016042, -0.0017575, 0.0011153, 0.000069314, 0.00049596, -0.00009512, 0.00094051, 0.0017266, 0.00025469, 0.0013046, 0.0018956, -0.0018187, 0.00027761, -0.0018462, -0.0010497, 0.0019706, -0.00072015, -0.0008238, -0.00058052, -0.00020924, 0.0011276, -0.0010895, 0.0010879, 0.00051654, -0.0015637, -0.0014255, 0.00039027, 0.00072792, -0.00043454, -0.0016408, -0.0016364, -0.00036638, 0.00066971, -0.0014123, 0.00023041, -0.0016174, 0.0018673, -0.00012971, -0.00086599, 0.0012828, 0.00065387, 0.0012491, 0.00071229, 0.00038457, -0.00019427, 0.001805, -0.00021093, 0.00090075, 0.001034, 0.0010274, 0.0013834, -0.0018257, -0.0017426, -0.0012749, -0.00091305, 0.0012968, 0.00055852, 0.0012448, 0.0019952, 0.00073432, -0.00079847, -0.00050102, -0.00017225, 0.00024385, 0.00085029, -6.6364e-06, -0.0010462, -0.0010496, -0.00030422, -0.00099065, -0.00058214, 0.0015071, -1.7591e-05, 0.00061095, 0.0016314, 0.0014086, 0.0012073, -0.00068908, 0.00027396, -0.00077762, 0.00097286, 0.001961, -0.00012913, 0.00088243, -0.0017849, -0.0016258, 0.00042926, 0.0019006, -0.00068575, -0.0013487, -0.001886, 0.0017723, 0.0001987, -0.00032988, -0.00074655, -0.00087773, -0.00060016, -0.00090766, 0.00050461, -0.00032633, 0.0015355, 0.0017694, -0.0019191, 0.00086071, -0.0003799, -0.0018486, 0.0012569, 0.0012278, 0.00063342, -0.0017316, 0.00043733, 0.00090713, 0.0013922, 0.00041156, 0.00050111, 0.0014755, 0.00025984, 0.0016339, -0.00045851, 0.0019561, -0.00029304};
  
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

TEST(SetupRHSforTDCSandTMSAlgorithm, MatrixWithMoreThanOneElementLessElectrodes)
{
  SetupRHSforTDCSandTMSAlgorithm algo;
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc0, 5.321);
  algo.set(SetupRHSforTDCSandTMSAlgorithm::Elc2, 0.3564);
  DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));
  (*m)(0,0) = 8;
  (*m)(1,0) = 1;
  (*m)(2,0) = 1;
  auto o = algo.run(CreateTetMeshScalarOnElem(), m);
  double compare [15] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.005321, -0.001, 0.0003564, 0.0, 0.0, 0.0, 0.0, 0.0};
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
