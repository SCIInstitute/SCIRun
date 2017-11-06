/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
#include <Modules/BrainStimulator/SolveBiotSavartContour.h>
#include <iostream>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Scalar.h>
#include <Core/Algorithms/BrainStimulator/BiotSavartSolverAlgorithm.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <vector>

using namespace SCIRun::Modules::BrainStimulator;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Dataflow::Networks;

MODULE_INFO_DEF(SolveBiotSavartContour, BrainStimulator, SCIRun)

SolveBiotSavartContour::SolveBiotSavartContour() : Module(ModuleLookupInfo("SolveBiotSavartContour", "BrainStimulator", "SCIRun"),false)
{
 INITIALIZE_PORT(Mesh);
 INITIALIZE_PORT(Coil);
 INITIALIZE_PORT(VectorBField);
 INITIALIZE_PORT(VectorAField);
}

void SolveBiotSavartContour::setStateDefaults()
{
  auto state = get_state();
  setStateIntFromAlgo(Parameters::OutType); 
}

void SolveBiotSavartContour::execute()
{
  auto mesh = getRequiredInput(Mesh);
  auto coil = getRequiredInput(Coil);
  AlgorithmOutput output;

  if (oport_connected(VectorBField) || oport_connected(VectorAField))
  {
      setAlgoIntFromState(Parameters::OutType);
      if (oport_connected(VectorBField) && oport_connected(VectorAField))
      {
       error(" This module only compute the first or the second module output depending if connected to another module. It does not compute both at the same time.");
       return;
      }
      
      if(oport_connected(VectorBField) && oport_connected(VectorAField))
      {
	 algo().set(Parameters::OutType, 3);
      } else
      {
       if (oport_connected(VectorBField))
	 algo().set(Parameters::OutType, 1);
       
       if (oport_connected(VectorAField)) 
         algo().set(Parameters::OutType, 2);

      }
   }
   
 if (needToExecute())  //newStatePresent
 {  
   auto input = make_input((Mesh, mesh)(Coil, coil));
   
   if ((oport_connected(VectorBField) || oport_connected(VectorAField)))
      output = algo().run(input);  
   
   if(oport_connected(VectorBField))
        sendOutputFromAlgorithm(VectorBField, output);
   
   if(oport_connected(VectorAField))
        sendOutputFromAlgorithm(VectorAField, output);
 }

}
