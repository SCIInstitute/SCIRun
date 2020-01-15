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
  FieldHandle LoadColin27ScalpSurfMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/setuptdcs/colin27_skin_surf.fld");
  }
  FieldHandle LoadColin27ElecSurfMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/setuptdcs/colin27_elc_surf.fld");
  }
  FieldHandle LoadColin27TetMesh()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/setuptdcs/elc_tet.fld");
  }

  DenseMatrixHandle ElectrodeSpongeLocationAndThickness()
  {
   DenseMatrixHandle m(boost::make_shared<DenseMatrix>(2,4));
   (*m)(0,0) = 59.5343;
   (*m)(0,1) = 15.5240;
   (*m)(0,2) = 84.5013;
   (*m)(0,3) = 4.0;
   (*m)(1,0) = 122.798;
   (*m)(1,1) = 32.2019;
   (*m)(1,2) = 128.798;
   (*m)(1,3) = 4.0;

   return m;
  }
}

TEST(SetupRHSforTDCSandTMSAlgorithm, Correct)
{
  SetupTDCSAlgorithm algo;
  int m=2; //2 patch electrodes for colin27
  int ref_node = 1000;
  double impedance = 5000;
  DenseMatrixHandle contact_impedance_times_elc_surface(boost::make_shared<DenseMatrix>(2,1));
  (*contact_impedance_times_elc_surface)(1,0) = 16.408535;
  (*contact_impedance_times_elc_surface)(0,0) = 15.326115;
  std::vector<Variable> elc;

  elc.push_back(AlgorithmParameter(Name("elc" + boost::lexical_cast<std::string>(0)), -1.0));
  elc.push_back(AlgorithmParameter(Name("elc" + boost::lexical_cast<std::string>(1)),  1.0));

  std::vector<Variable> impelc;
  for (int i=0; i<m; i++)
  {
     AlgorithmParameter imp_elc_i(Name("imp_elc" + boost::lexical_cast<std::string>(i)), impedance);
     impelc.push_back(imp_elc_i);
  }
  FieldHandle mesh(LoadColin27TetMesh());
  FieldHandle scalp_tri_surf(LoadColin27ScalpSurfMesh());
  FieldHandle elc_tri_surf(LoadColin27ElecSurfMesh());
  DenseMatrixHandle elc_sponge_location(ElectrodeSpongeLocationAndThickness());

  algo.set(Parameters::refnode, ref_node);
  auto result = algo.run(mesh,elc,impelc,m,scalp_tri_surf,elc_tri_surf,elc_sponge_location);
  DenseMatrixHandle lhs_knowns = result.get<0>();
  DenseMatrixHandle elc_element= result.get<1>();
  DenseMatrixHandle elc_element_typ = result.get<2>();
  DenseMatrixHandle elc_element_def = result.get<3>();
  DenseMatrixHandle elc_contact_imp= result.get<4>();
  DenseMatrixHandle rhs = result.get<5>();
  FieldHandle elec_sponge_surf = result.get<6>();
  DenseMatrixHandle selectmatrixind = result.get<7>();

  EXPECT_EQ(lhs_knowns->nrows(),84350);
  EXPECT_EQ(lhs_knowns->ncols(),1);

  EXPECT_EQ(elc_element->nrows(),27743);
  EXPECT_EQ(elc_element->ncols(),1);

  EXPECT_EQ(elc_element_typ->nrows(),27743);
  EXPECT_EQ(elc_element_typ->ncols(),1);

  EXPECT_EQ(elc_element_def->nrows(),27743);
  EXPECT_EQ(elc_element_def->ncols(),4);

  EXPECT_EQ(elc_contact_imp->nrows(),27743);
  EXPECT_EQ(elc_contact_imp->ncols(),1);

  for (int i=0; i<lhs_knowns->nrows(); i++)
  {
   if (i!=ref_node)
    EXPECT_TRUE(IsNan((*lhs_knowns)(i,0)));
     else
        EXPECT_EQ((*lhs_knowns)(i), 0.0);
  }

  for (int i=0; i<elc_element_typ->nrows(); i++)
      EXPECT_EQ((*elc_element_typ)(i,0), 2.0);

  for (int i=0; i<selectmatrixind->nrows(); i++)
      EXPECT_EQ((*selectmatrixind)(i,0), i);

  VMesh* elec_sponge_surf_vmesh = elec_sponge_surf->vmesh();

  for (int i=0; i<elc_element->nrows(); i++)
  {
    for (int j=0; j<m; j++)
    {
     if ((*elc_element)(i,0)==j)
       EXPECT_NEAR((*elc_contact_imp)(i,0), (*contact_impedance_times_elc_surface)(j,0),0.0001);
    }

    if ((*elc_element)(i,0)>=m)
       EXPECT_EQ(1,0);   //intended throw
  }

  EXPECT_EQ(elec_sponge_surf_vmesh->num_nodes(),14343);
  EXPECT_EQ(elec_sponge_surf_vmesh->num_elems(),27743);
}

TEST(SetupRHSforTDCSandTMSAlgorithm, Correct_scalpIsElectrodeContact)
{
  SetupTDCSAlgorithm algo;
  algo.set(Parameters::GetContactSurface, true);  //determine the surface that joins scalp and electrode spoge
  int m=2; //2 patch electrodes for colin27
  int ref_node = 1050;
  double impedance = 10000; // 100000 [Ohm] !!!
  DenseMatrixHandle contact_impedance_times_elc_surface(boost::make_shared<DenseMatrix>(2,1));
  (*contact_impedance_times_elc_surface)(0,0) = 28.882450711537267; /// BrainStimulator expects Ohm * m^2
  (*contact_impedance_times_elc_surface)(1,0) = 30.353635349846389;
  std::vector<Variable> elc;

  elc.push_back(AlgorithmParameter(Name("elc" + boost::lexical_cast<std::string>(0)), -1.0));
  elc.push_back(AlgorithmParameter(Name("elc" + boost::lexical_cast<std::string>(1)),  1.0));

  std::vector<Variable> impelc;
  for (int i=0; i<m; i++)
  {
     AlgorithmParameter imp_elc_i(Name("imp_elc" + boost::lexical_cast<std::string>(i)), impedance);
     impelc.push_back(imp_elc_i);
  }
  FieldHandle mesh(LoadColin27TetMesh());
  FieldHandle scalp_tri_surf(LoadColin27ScalpSurfMesh());
  FieldHandle elc_tri_surf(LoadColin27ElecSurfMesh());
  DenseMatrixHandle elc_sponge_location(ElectrodeSpongeLocationAndThickness());

  algo.set(Parameters::refnode, ref_node);
  auto result = algo.run(mesh,elc,impelc,m,scalp_tri_surf,elc_tri_surf,elc_sponge_location);
  DenseMatrixHandle lhs_knowns = result.get<0>();
  DenseMatrixHandle elc_element= result.get<1>();
  DenseMatrixHandle elc_element_typ = result.get<2>();
  DenseMatrixHandle elc_element_def = result.get<3>();
  DenseMatrixHandle elc_contact_imp= result.get<4>();
  DenseMatrixHandle rhs = result.get<5>();
  FieldHandle elec_sponge_surf = result.get<6>();
  DenseMatrixHandle selectmatrixind = result.get<7>();

  EXPECT_EQ(lhs_knowns->nrows(),84350);
  EXPECT_EQ(lhs_knowns->ncols(),1);

  EXPECT_EQ(elc_element->nrows(),25865);
  EXPECT_EQ(elc_element->ncols(),1);

  EXPECT_EQ(elc_element_typ->nrows(),25865);
  EXPECT_EQ(elc_element_typ->ncols(),1);

  EXPECT_EQ(elc_element_def->nrows(),25865);
  EXPECT_EQ(elc_element_def->ncols(),4);

  EXPECT_EQ(elc_contact_imp->nrows(),25865);
  EXPECT_EQ(elc_contact_imp->ncols(),1);

  for (int i=0; i<lhs_knowns->nrows(); i++)
  {
   if (i!=ref_node)
    EXPECT_TRUE(IsNan((*lhs_knowns)(i,0)));
     else
        EXPECT_EQ((*lhs_knowns)(i), 0.0);
  }

  for (int i=0; i<elc_element_typ->nrows(); i++)
      EXPECT_EQ((*elc_element_typ)(i,0), 2.0);

  for (int i=0; i<selectmatrixind->nrows(); i++)
      EXPECT_EQ((*selectmatrixind)(i,0), i);

  VMesh* elec_sponge_surf_vmesh = elec_sponge_surf->vmesh();

  for (int i=0; i<elc_element->nrows(); i++)
  {
    for (int j=0; j<m; j++)
    {
     if ((*elc_element)(i,0)==j)
       EXPECT_NEAR((*elc_contact_imp)(i,0), (*contact_impedance_times_elc_surface)(j,0),0.001);
    }

    if ((*elc_element)(i,0)>=m)
       EXPECT_EQ(1,0);   //intended throw
  }

  EXPECT_EQ(elec_sponge_surf_vmesh->num_nodes(),77595);
  EXPECT_EQ(elec_sponge_surf_vmesh->num_elems(),25865);
}
