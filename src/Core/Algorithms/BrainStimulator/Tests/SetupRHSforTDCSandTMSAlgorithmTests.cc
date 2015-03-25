/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;

namespace
{
  FieldHandle LoadMickeyMouseCleaverMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/mickey_mouse/cleaver_mesh.fld");
  }
  FieldHandle LoadMickeyMouseScalpMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/mickey_mouse/scalp_surf_mesh.fld");
  }
  FieldHandle LoadMickeyMouseElectrodeSpongeMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/mickey_mouse/elc_surf_mesh.fld");
  }
  
  DenseMatrixHandle ElectrodeSpongeLocationAndThickness()
  { 
   DenseMatrixHandle m(boost::make_shared<DenseMatrix>(6,4));
   (*m)(0,0) = 33.5;
   (*m)(0,1) = 33.5;
   (*m)(0,2) = 18.246;
   (*m)(0,3) = 3.0;   
   (*m)(1,0) = 33.5;
   (*m)(1,1) = 18.246;
   (*m)(1,2) = 33.5;
   (*m)(1,3) = 3.0;   
   (*m)(2,0) = 18.246;
   (*m)(2,1) = 33.5;
   (*m)(2,2) = 33.5;
   (*m)(2,3) = 3.0;  
   (*m)(3,0) = 48.754;
   (*m)(3,1) = 33.5;
   (*m)(3,2) = 33.5;
   (*m)(3,3) = 3.0;  
   (*m)(4,0) = 33.5;
   (*m)(4,1) = 48.754;
   (*m)(4,2) = 33.5;
   (*m)(4,3) = 3.0;
   (*m)(5,0) = 33.5;
   (*m)(5,1) = 33.5;
   (*m)(5,2) = 48.754;
   (*m)(5,3) = 3.0;

   return m;
  }
}

TEST(SetupRHSforTDCSandTMSAlgorithm, Correct)
{
  SetupTDCSAlgorithm algo;
  int m=6;
  std::vector<Variable> elc;
  
  elc.push_back(AlgorithmParameter(Name("elc" + boost::lexical_cast<std::string>(0)), -1.0));
  elc.push_back(AlgorithmParameter(Name("elc" + boost::lexical_cast<std::string>(1)),  1.0));
  
  for (int i=2; i<m; i++)
  {
    AlgorithmParameter elc_i(Name("elc" + boost::lexical_cast<std::string>(i)), 0.0);   
    elc.push_back(elc_i);
  }
  
  std::vector<Variable> impelc;
  for (int i=0; i<m; i++)
  {
     AlgorithmParameter imp_elc_i(Name("imp_elc" + boost::lexical_cast<std::string>(i)), 1.0);   
     impelc.push_back(imp_elc_i);
  }
  FieldHandle mesh(LoadMickeyMouseCleaverMesh());
  FieldHandle scalp_tri_surf(LoadMickeyMouseScalpMesh());
  FieldHandle elc_tri_surf(LoadMickeyMouseElectrodeSpongeMesh());
  DenseMatrixHandle elc_sponge_location(ElectrodeSpongeLocationAndThickness());
  auto result = algo.run(mesh,elc,impelc,m,scalp_tri_surf,elc_tri_surf,elc_sponge_location);
  DenseMatrixHandle lhs_knowns = result.get<0>();
  DenseMatrixHandle elc_element= result.get<1>();
  DenseMatrixHandle elc_element_typ = result.get<2>();
  DenseMatrixHandle elc_element_def = result.get<3>();
  DenseMatrixHandle elc_contact_imp= result.get<4>();
  DenseMatrixHandle rhs = result.get<5>();
  FieldHandle elec_sponge_surf = result.get<6>();
  DenseMatrixHandle selectmatrixind = result.get<7>();
   
  EXPECT_EQ(lhs_knowns->nrows(),33707);
  EXPECT_EQ(lhs_knowns->ncols(),1);
  
  EXPECT_EQ(elc_element->nrows(),3024);
  EXPECT_EQ(elc_element->ncols(),1);
  
  EXPECT_EQ(elc_element_typ->nrows(),3024);
  EXPECT_EQ(elc_element_typ->ncols(),1);
  
  EXPECT_EQ(elc_element_def->nrows(),3024);
  EXPECT_EQ(elc_element_def->ncols(),4);
  
  EXPECT_EQ(elc_contact_imp->nrows(),3024);
  EXPECT_EQ(elc_contact_imp->ncols(),1);
     
  for (int i=1; i<lhs_knowns->nrows(); i++)
   EXPECT_TRUE(IsNan((*lhs_knowns)(i,0)));
   
  EXPECT_EQ((*lhs_knowns)(0,0), 0.0); 
  
  for (int i=0; i<elc_element_typ->nrows(); i++)
      EXPECT_EQ((*elc_element_typ)(i,0), 2.0);
  
  for (int i=0; i<elc_element_typ->nrows(); i++)
      EXPECT_EQ((*elc_contact_imp)(i,0), 1.0);    
   
  for (int i=0; i<selectmatrixind->nrows(); i++) 
      EXPECT_EQ((*selectmatrixind)(i,0), i);

  VMesh* elec_sponge_surf_vmesh = elec_sponge_surf->vmesh();
  
  EXPECT_EQ(elec_sponge_surf_vmesh->num_nodes(),1614);
  EXPECT_EQ(elec_sponge_surf_vmesh->num_elems(),3024);
}

