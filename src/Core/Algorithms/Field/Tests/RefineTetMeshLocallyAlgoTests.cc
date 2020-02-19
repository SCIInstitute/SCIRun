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
#include <Core/Algorithms/Field/RefineTetMeshLocallyAlgorithm.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <Core/IEPlugin/MatlabFiles_Plugin.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshNodes.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::TestUtils;

FieldList LoadAllCasesInputField()
{
  FieldList result;
  for(int i=1; i<64; i++)
  {
   if (!(i==31 || i==47 || i==55 || i==59 || i==61 || i==62))
   {
    auto file = (TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_input/").string() + std::to_string(i) + ".mat";
    FieldHandle field = MatlabField_reader(0, file.c_str());
    result.push_back(field);
   }
  }

  return result;
}

FieldList LoadAllCasesResultField()
{
  FieldList result;
  for(int i=1; i<64; i++)
  {
    auto file = TestResources::rootDir() / "Fields/refinetetmeshlocally/all57outof64case_result/";
    auto str = file.string();
    str.append(std::to_string(i));
    str.append(".mat");
    result.push_back(MatlabField_reader(0, str.c_str()));
  }

  return result;
}


SparseRowMatrixHandle GetTheoreticalTetCases(int cases, int nr_nodes)
 {
  SparseRowMatrixFromMap::Values result;

  if (cases==31)
  {
   result[1][2]=1;
   result[0][2]=1;
   result[0][3]=1;
   result[1][3]=1;
   result[2][3]=1;
  } else
  if (cases==47)
  {
   result[0][1]=1;
   result[0][2]=1;
   result[0][3]=1;
   result[1][3]=1;
   result[2][3]=1;
  } else
  if (cases==55)
  {
   result[0][1]=1;
   result[1][2]=1;
   result[0][3]=1;
   result[1][3]=1;
   result[2][3]=1;
  } else
  if (cases==59)
  {
   result[0][1]=1;
   result[1][2]=1;
   result[0][2]=1;
   result[1][3]=1;
   result[2][3]=1;
  } else
  if (cases==61)
  {
   result[0][1]=1;
   result[1][2]=1;
   result[0][2]=1;
   result[0][3]=1;
   result[2][3]=1;
  } else
  if (cases==62)
  {
   result[0][1]=1;
   result[1][2]=1;
   result[0][2]=1;
   result[0][3]=1;
   result[1][3]=1;
  }

  SparseRowMatrixHandle case_ = SparseRowMatrixFromMap::make(nr_nodes, nr_nodes, result);
  return case_;
}


TEST(RefineTetMeshLocallyAlgoTests, Test59basicCutCases)
{
  FieldList input_files = LoadAllCasesInputField();
  FieldList result_files = LoadAllCasesResultField();

  double epsilon=1e-8;

  RefineTetMeshLocallyAlgorithm algo;
  algo.set(Parameters::RefineTetMeshLocallyIsoValue, 1.0);
  algo.set(Parameters::RefineTetMeshLocallyEdgeLength, 0.0);
  algo.set(Parameters::RefineTetMeshLocallyVolume, 0.0);
  algo.set(Parameters::RefineTetMeshLocallyDihedralAngleSmaller, 180.0);
  algo.set(Parameters::RefineTetMeshLocallyDihedralAngleBigger, 0.0);
  algo.set(Parameters::RefineTetMeshLocallyRadioButtons, 0);
  algo.set(Parameters::RefineTetMeshLocallyDoNoSplitSurfaceTets, false);
  algo.set(Parameters::RefineTetMeshLocallyCounterClockWiseOrdering, false);
  algo.set(Parameters::RefineTetMeshLocallyUseModuleInputField, false);
  algo.set(Parameters::RefineTetMeshLocallyMaxNumberRefinementIterations, 1);
  GetMeshNodesAlgo getfieldnodes_algo;
  DenseMatrixHandle output_nodes,exp_result_nodes;
  VMesh::Node::array_type onodes1(4),onodes2(4);
  VMesh *output_vmesh, *exp_result_vmesh;
  FieldHandle input,output, exp_result;

  int count=0;
  for(int i=0; i<63; i++)
  {
   if (!(i==31-1 || i==47-1 || i==55-1 || i==59-1 || i==61-1 || i==62-1))
   {
    if(!input_files[count] || !result_files[i])
    {
      FAIL() << " ERROR: could not load data files. Please check path set in SCIRUN_TEST_RESOURCE_DIR variable (cmake). " << std::endl;
    }
    input=input_files[count++];
    exp_result=result_files[i];

    try
    {
     algo.runImpl(input, output);
    } catch (...)
    {
      FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work. " << std::endl;
    }
    exp_result_vmesh=exp_result->vmesh();
    output_vmesh=output->vmesh();
    try
    {
     getfieldnodes_algo.run(output,output_nodes);
     getfieldnodes_algo.run(exp_result,exp_result_nodes);
    } catch (...)
    {
     FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work (could not get field nodes from input files). " << std::endl;
    }

    if( output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
      FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work (number of nodes is different than expected). " << std::endl;

    for (long idx=0;idx<exp_result_nodes->nrows();idx++)
    {
      EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
      EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
      EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
    }

    output_vmesh->synchronize(Mesh::NODES_E);
    exp_result_vmesh->synchronize(Mesh::NODES_E);

    if( output_vmesh->num_elems()!=exp_result_vmesh->num_elems() || output_vmesh->num_elems()!=exp_result_vmesh->num_elems() )
      FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work (number of elements is different than expected). " << std::endl;

    for(VMesh::Elem::index_type idx=0; idx<output_vmesh->num_elems(); idx++)
    {
     output_vmesh->get_nodes(onodes1, idx);
     exp_result_vmesh->get_nodes(onodes2, idx);
     for (int j=0;j<4;j++)
      if (onodes1[j]!=onodes2[j])
       FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 1 does not work (definition of resulting and expected tet definition differs). " << std::endl;
    }

   }

  }

}

TEST(RefineTetMeshLocallyAlgoTests, Test5MoreTheoreticalCutCases)
{
 double epsilon=1e-8;
 RefineTetMeshLocallyAlgorithm algo;
 VMesh::Node::array_type onodes1(4),onodes2(4);
 DenseMatrixHandle output_nodes,exp_result_nodes;
 FieldList input_list=LoadAllCasesInputField();
 FieldList result_list=LoadAllCasesResultField();
 FieldHandle input=input_list[56];
 int nr_nodes = input->vmesh()->num_nodes();
 SparseRowMatrixHandle case_;
 GetMeshNodesAlgo getfieldnodes_algo;
 int cases[] = {31, 47, 55, 59, 61, 62};
 for(int i=0; i<sizeof(cases)/4; i++)
 {
  SparseRowMatrixHandle case_=GetTheoreticalTetCases(cases[i], nr_nodes);
  FieldHandle Casefld=algo.RefineMesh(input, case_);
  FieldHandle case_exp_result=result_list[cases[i]-1];
  VMesh *case_vmesh=Casefld->vmesh();
  VMesh *case_exp_vmesh=case_exp_result->vmesh();

  try
  {
   getfieldnodes_algo.run(Casefld,output_nodes);
   getfieldnodes_algo.run(case_exp_result,exp_result_nodes);
  } catch (...)
  {
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 31 does not work (could not get field nodes from input files). " << std::endl;
  }

  if(output_nodes->ncols()!=exp_result_nodes->ncols() || output_nodes->nrows()!=exp_result_nodes->nrows() )
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of nodes is different than expected). " << std::endl;

  for (long idx=0;idx<exp_result_nodes->nrows();idx++)
  {
    EXPECT_NEAR( (*exp_result_nodes)(idx,0),(*output_nodes)(idx,0), epsilon);
    EXPECT_NEAR( (*exp_result_nodes)(idx,1),(*output_nodes)(idx,1), epsilon);
    EXPECT_NEAR( (*exp_result_nodes)(idx,2),(*output_nodes)(idx,2), epsilon);
  }

 case_vmesh->synchronize(Mesh::NODES_E);
 case_exp_vmesh->synchronize(Mesh::NODES_E);

 if( case_vmesh->num_elems()!=case_exp_vmesh->num_elems() || case_vmesh->num_elems()!=case_exp_vmesh->num_elems() )
   FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (number of elements is different than expected). " << std::endl;

 for(VMesh::Elem::index_type idx=0; idx<case_vmesh->num_elems(); idx++)
 {
  case_vmesh->get_nodes(onodes1, idx);
  case_exp_vmesh->get_nodes(onodes2, idx);
  for (int j=0;j<4;j++)
   if (onodes1[j]!=onodes2[j])
    FAIL() << " ERROR: RefineTetMeshLocallyAlgorithm: Case 63 does not work (definition of resulting and expected tet definition differs). " << std::endl;
 }

 }

}
