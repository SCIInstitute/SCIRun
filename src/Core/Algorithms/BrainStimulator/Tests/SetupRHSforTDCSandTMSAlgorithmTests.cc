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
#include <boost/lexical_cast.hpp>
#include <vector>
#include <iostream>

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

void makeRandomValues(int m, std::vector<Variable>& elc, std::vector<double>& compare)
{
/*  for (int i=0; i<m; i++)
  {
    int temp = rand() % 100;
    AlgorithmParameter elc_i(Name("elc" + boost::lexical_cast<std::string>(i)), temp);
    elc.push_back(elc_i);
    compare.push_back((double)temp/1000); // values of temp are converted to amps, thus temp/1000
  }*/
}


TEST(SetupRHSforTDCSandTMSAlgorithm, ComparingValuesSimple)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  int m = 10;
  std::vector<Variable> elc;
  for (int i=0; i<m; i++)
  {
    AlgorithmParameter elc_i(Name("elc" + boost::lexical_cast<std::string>(i)), i);
    elc.push_back(elc_i);
  }
  // values of i are converted to amps, thus i/1000
  double compare [17] = {0, 0, 0, 0, 0, 0, 0, 0, 0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009};
  auto o = algo.run(CreateTetMeshScalarOnElem(), elc, m);
  for (int i=0; i<o->nrows(); i++)
    EXPECT_EQ(compare[i], o->coeff(i,0));*/
}

TEST(SetupRHSforTDCSandTMSAlgorithm, ComparingValuesTriSurfScalarOnElem)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  int m = 128;
  std::vector<double> compare(6,0); // nodes are 6 zero values
  std::vector<Variable> elc;
  makeRandomValues(m, elc, compare);
  auto o = algo.run(CreateTriSurfScalarOnElem(), elc, m);
  for (int i=0; i<o->nrows(); i++)
    EXPECT_EQ(compare[i], o->coeff(i,0));*/
}


TEST(SetupRHSforTDCSandTMSAlgorithm, ComparingValuesTriSurfScalarOnNode)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  int m = 128;
  std::vector<double> compare(6,0);
  std::vector<Variable> elc;
  makeRandomValues(m, elc, compare);
  auto o = algo.run(CreateTriSurfScalarOnNode(), elc, m);
  for (int i=0; i<o->nrows(); i++)
    EXPECT_EQ(compare[i], o->coeff(i,0));*/
}


TEST(SetupRHSforTDCSandTMSAlgorithm, ComparingValuesTriSurfVectorOnElem)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  int m = 128;
  std::vector<double> compare(6,0);
  std::vector<Variable> elc;
  makeRandomValues(m, elc, compare);
  auto o = algo.run(CreateTriSurfVectorOnElem(), elc, m);
  for (int i=0; i<o->nrows(); i++)
    EXPECT_EQ(compare[i], o->coeff(i,0));*/
}

TEST(SetupRHSforTDCSandTMSAlgorithm, ComparingValuesTriSurfVectorOnNode)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  int m = 128;
  std::vector<double> compare(6,0);
  std::vector<Variable> elc;
  makeRandomValues(m, elc, compare);
  auto o = algo.run(CreateTriSurfVectorOnNode(), elc, m);
  for (int i=0; i<o->nrows(); i++)
    EXPECT_EQ(compare[i], o->coeff(i,0));*/
}

TEST(SetupRHSforTDCSandTMSAlgorithm, ComparingValuesTetMeshVectorOnNode)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  int m = 128;
  std::vector<double> compare(7,0);
  std::vector<Variable> elc;
  makeRandomValues(m, elc, compare);
  auto o = algo.run(CreateTetMeshVectorOnNode(), elc, m);
  for (int i=0; i<o->nrows(); i++)
    EXPECT_EQ(compare[i], o->coeff(i,0));*/
}

TEST(SetupRHSforTDCSandTMSAlgorithm, ComparingValuesTetMeshVectorOnElem)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  int m = 128;
  std::vector<double> compare(7,0);
  std::vector<Variable> elc;
  makeRandomValues(m, elc, compare);
  auto o = algo.run(CreateTetMeshVectorOnElem(), elc, m);
  for (int i=0; i<o->nrows(); i++)
    EXPECT_EQ(compare[i], o->coeff(i,0));*/
}

TEST(SetupRHSforTDCSandTMSAlgorithm, ComparingValuesTetMeshScalarOnElem)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  int m = 128;
  std::vector<double> compare(7,0);
  std::vector<Variable> elc;
  makeRandomValues(m, elc, compare);
  auto o = algo.run(CreateTetMeshScalarOnElem(), elc, m);
  for (int i=0; i<o->nrows(); i++)
    EXPECT_EQ(compare[i], o->coeff(i,0));*/
}

TEST(SetupRHSforTDCSandTMSAlgorithm, ComparingValuesTetMeshScalarOnNode)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  int m = 128;
  std::vector<double> compare(7,0);
  std::vector<Variable> elc;
  makeRandomValues(m, elc, compare);
  auto o = algo.run(CreateTetMeshScalarOnNode(), elc, m);
  for (int i=0; i<o->nrows(); i++)
    EXPECT_EQ(compare[i], o->coeff(i,0));*/
}

TEST(SetupRHSforTDCSandTMSAlgorithm, NegativeNumberOfElectrodesGivenTetMeshScalarOnElem)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  int m = -10;
  std::vector<Variable> elc;
  AlgorithmParameter elc_i(Name("elc" + boost::lexical_cast<std::string>(0)), 1.0);
  elc.push_back(elc_i);
  EXPECT_THROW(algo.run(CreateTetMeshScalarOnElem(), elc, m), AlgorithmInputException);*/
}

TEST(SetupRHSforTDCSandTMSAlgorithm, ElectrodeNumberExceedsWhatIsPossible)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  int m = 360;
  std::vector<Variable> elc;
  AlgorithmParameter elc_i(Name("elc" + boost::lexical_cast<std::string>(0)), 1.0);
  elc.push_back(elc_i);
  EXPECT_THROW(algo.run(CreateTetMeshScalarOnElem(), elc, m), AlgorithmInputException); */
}

TEST(SetupRHSforTDCSandTMSAlgorithm, NullFieldGivenAsInput)
{
/*  SetupRHSforTDCSandTMSAlgorithm algo;
  FieldHandle f;
  int m = 10;
  std::vector<Variable> elc;
  for (int i=0; i<m; i++)
  {
    AlgorithmParameter elc_i(Name("elc" + boost::lexical_cast<std::string>(i)), i);
    elc.push_back(elc_i);
  }
  EXPECT_THROW(algo.run(f, elc, m), AlgorithmInputException);*/
}
