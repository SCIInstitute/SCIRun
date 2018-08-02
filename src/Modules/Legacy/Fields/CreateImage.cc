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

#include <Modules/Legacy/Fields/CreateImage.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

MODULE_INFO_DEF(CreateImage, NewField, SCIRun)

CreateImage::CreateImage() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(SizeMatrix)
  INITIALIZE_PORT(OVMatrix);
  INITIALIZE_PORT(OutputField);
}

void CreateLatVol::setStateDefaults()
{
  setStateIntFromAlgo(Parameters::Width);
  setStateIntFromAlgo(Parameters::Height);
  setStateDoubleFromAlgo(Parameters::PadPercent);
  
  setStateStringFromAlgoOtpion(Parameters::Axis);
  
  setStateDoubleFromAlgo(Parameters::CenterX);
  setStateDoubleFromAlgo(Parameters::CenterY);
  setStateDoubleFromAlgo(Parameters::CenterZ);
  
  setStateDoubleFromAlgo(Parameters::NormalX);
  setStateDoubleFromAlgo(Parameters::NormalY);
  setStateDoubleFromAlgo(Parameters::NormalZ);
  
  setStateDoubleFromAlgo(Parameters::Position);
  
  setStateStringFromAlgoOtpion(Parameters::DataLocation);
}

void CreateImage::execute()
{
  auto inputField = getOptionalInput(InputField);
  auto sizeMatrix = getOptionalInput(SizeMatrix);
  auto oVMatrix = getOptionalInput(OVMatrix);
  
  if (needToExecute())
  {
    setAlgoIntFromState(Parameters::Width);
    setAlgoIntFromState(Parameters::Height);
    setAlgoDoubleFromState(Parameters::PadPercent);
    
    setAlgoOptionsFromState(Parameters::Axis);
    
    setAlgoDoubleFromState(Parameters::CenterX);
    setAlgoDoubleFromState(Parameters::CenterY);
    setAlgoDoubleFromState(Parameters::CenterZ);
    
    setAlgoDoubleFromState(Parameters::NormalX);
    setAlgoDoubleFromState(Parameters::NormalY);
    setAlgoDoubleFromState(Parameters::NormalZ);
    
    setAlgoDoubleFromState(Parameters::Position);
    
    setAlgoOptionsFromState(Parameters::DataLocation);
    
    auto output=algo.run(withInputData((InputField, inputField)(SizeMatrix,sizeMatrix)(OVMatrix, oVMatrix)));
    
    sendOutputFromAlgorihtm(OutputField, output);
  }
}
