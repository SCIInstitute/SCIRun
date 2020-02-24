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
#include <Core/Algorithms/BrainStimulator/ElectrodeCoilSetupAlgorithm.h>
#include <Testing/Utils/SCIRunUnitTests.h>
#include <Testing/Utils/MatrixTestUtilities.h>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm/count.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/format.hpp>
#include <boost/assign.hpp>
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
  FieldHandle LoadMickeyScalpSurfMeshForAlgo()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/BrainStim/micky_scalp.fld");
  }
  FieldHandle LoadMickeyTMSCoilForAlgo()
  {
    return loadFieldFromFile(TestResources::rootDir() / "Fields/BrainStim/mickey_tms_coil.fld");
  }

  DenseMatrixHandle TMSCoilLocationForAlgo()
  {
   DenseMatrixHandle m(boost::make_shared<DenseMatrix>(1,3));
   (*m)(0,0) = 33.6654;
   (*m)(0,1) = 34.625;
   (*m)(0,2) = 10.6;

   return m;
  }
}


TEST(ElectrodeCoilSetupAlgorithmTests, AlgoCorrect)
{
  ElectrodeCoilSetupAlgorithm algo;
  FieldHandle coils_field,electrodes_field, final_electrode_field;
  FieldHandle scalp=LoadMickeyScalpSurfMeshForAlgo();
  Variable::List table;
  DenseMatrixHandle elc_sponge_loc_avr, locations=TMSCoilLocationForAlgo();
  std::vector<FieldHandle> elc_coil_proto;
  elc_coil_proto.push_back(LoadMickeyTMSCoilForAlgo());
  Variable var1,var2,var3,var4,var5,var6,var7,var8,var9,var10;
  var1=makeVariable("Input #", boost::str(boost::format("%s") % "USE_MODULE_INPUT_3"));
  var2=makeVariable("Type",   boost::str(boost::format("%s") % "TMS"));
  var3=makeVariable("X",     boost::str(boost::format("%.3f") % "33.6654"));
  var4=makeVariable("Y",     boost::str(boost::format("%.3f") % "34.625"));
  var5=makeVariable("Y",     boost::str(boost::format("%.3f") % "7.6"));
  var6=makeVariable("Angle",  boost::str(boost::format("%s") % "1"));
  var7=makeVariable("NX", boost::str(boost::format("%s") % "???"));
  var8=makeVariable("NY", boost::str(boost::format("%s") % "???"));
  var9=makeVariable("NZ", boost::str(boost::format("%s") % "???"));
  var10=makeVariable("thickness",boost::str(boost::format("%s") % "2"));
  Variable::List tmp{ var1, var2, var3, var4, var5, var6, var7, var8, var9, var10 };
  table.push_back(makeVariable("row" + boost::lexical_cast<std::string>(0), tmp));
  VariableHandle output(new Variable(Name("Table"), table));
  ///@Dan: I dont know how to set the table to the GUI so that the module can executed properly.

 // boost::tie(table, elc_sponge_loc_avr, electrodes_field, final_electrode_field, coils_field) = algo.run(scalp, locations, elc_coil_proto);
  /*
  EXPECT_EQ(coils_field->vmesh()->num_nodes(),18);
  EXPECT_EQ(coils_field->vmesh()->num_elems(),18);
  EXPECT_EQ(coils_field->vfield()->num_values(),18);
  //need more check routines here !!!
  */
}
