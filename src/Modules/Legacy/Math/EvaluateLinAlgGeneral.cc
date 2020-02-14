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


#include <Modules/Legacy/Math/EvaluateLinAlgGeneral.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Parser/LinAlgEngine.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;

/// @class EvaluateLinAlgGeneral
/// @brief This module performs a user defined linear algebra operation on up to five input matrices.

MODULE_INFO_DEF(EvaluateLinearAlgebraGeneral, Math, SCIRun)

EvaluateLinearAlgebraGeneral::EvaluateLinearAlgebraGeneral() : Module(staticInfo_)
{
  INITIALIZE_PORT(i1);
  INITIALIZE_PORT(i2);
  INITIALIZE_PORT(i3);
  INITIALIZE_PORT(i4);
  INITIALIZE_PORT(i5);
  INITIALIZE_PORT(o1);
  INITIALIZE_PORT(o2);
  INITIALIZE_PORT(o3);
  INITIALIZE_PORT(o4);
  INITIALIZE_PORT(o5);
}

void EvaluateLinearAlgebraGeneral::setStateDefaults()
{
  get_state()->setValue(Variables::FunctionString, std::string("o1 = i1 * i2;"));
}

namespace
{
  std::string inputName(int i)
  {
    std::ostringstream o;
    o << "i" << (i + 1);
    return o.str();
  }
  std::string outputName(int i)
  {
    std::ostringstream o;
    o << "o" << (i + 1);
    return o.str();
  }
}

void
EvaluateLinearAlgebraGeneral::execute()
{
  const int NUM_PORTS = 5;
  std::vector<boost::optional<MatrixHandle>> imatrix;
  std::vector<MatrixHandle> omatrix(NUM_PORTS);

  imatrix.push_back(getOptionalInput(i1));
  imatrix.push_back(getOptionalInput(i2));
  imatrix.push_back(getOptionalInput(i3));
  imatrix.push_back(getOptionalInput(i4));
  imatrix.push_back(getOptionalInput(i5));

  if (needToExecute())
  {
    NewLinAlgEngine engine;
    engine.setLogger(getLogger());

    for (size_t p = 0; p < NUM_PORTS; p++)
    {
      std::string matrixname = outputName(p);
      if (outputPorts()[p]->nconnections() > 0)
      {
        if (!engine.add_output_matrix(matrixname))
          return;
      }

      if (imatrix[p] && *imatrix[p])
      {
        if (!engine.add_input_matrix(inputName(p), *imatrix[p]))
          return;
      }
    }

    const std::string function = get_state()->getValue(Variables::FunctionString).toString();

    if(!engine.add_expressions(function))
      return;

    // Actual engine call, which does the dynamic compilation, the creation of the
    // code for all the objects, as well as inserting the function and looping
    // over every data point

    if (!engine.run())
    {
      ///-----------------------
      // Backwards compatibility with version 3.0.2
      std::ostringstream ostr;
      ostr << "\n"
        << ("This module does not fully support backwards compatibility: ")
        << ("C++/C functions are not supported in by this module anymore. ")
        << ("Please review documentation to explore available functionality and grammar of this module. ")
        << ("We are sorry for this inconvenience, but we do not longer support dynamically compiling in SCIRun.");
      error(ostr.str());
      ///-----------------------

      return;
    }

    for (size_t p = 0; p < NUM_PORTS; p++)
    {
      engine.get_matrix(outputName(p), omatrix[p]);
    }

    sendOutput(o1, omatrix[0]);
    sendOutput(o2, omatrix[1]);
    sendOutput(o3, omatrix[2]);
    sendOutput(o4, omatrix[3]);
    sendOutput(o5, omatrix[4]);
  }
}
