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


/*
 *  ConvertNrrdToField.cc:  Convert a Nrrd to a Field
 *
 *  Written by:
 *   David Weinstein
 *   School of Computing
 *   University of Utah
 *   February 2001
 *
 */

#include <Modules/Legacy/Teem/Converters/ConvertNrrdToField.h>
#include <Core/Algorithms/Legacy/Converter/ConverterAlgo.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Legacy/Field/Field.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <Core/Datatypes/Legacy/Nrrd/NrrdData.h>

using namespace SCIRun;
using namespace Modules::Teem;
using namespace Dataflow::Networks;
using namespace Core::Algorithms;
using namespace Teem;

ALGORITHM_PARAMETER_DEF(Teem, DataLocation);
ALGORITHM_PARAMETER_DEF(Teem, FieldType);
ALGORITHM_PARAMETER_DEF(Teem, ConvertParity);

MODULE_INFO_DEF(ConvertNrrdToField, Converters, Teem)

ConvertNrrdToField::ConvertNrrdToField() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputNrrd);
  INITIALIZE_PORT(OutputField);
}

void ConvertNrrdToField::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::DataLocation, std::string("Node"));
  state->setValue(Parameters::FieldType, std::string("Auto"));
  state->setValue(Parameters::ConvertParity, std::string("Do Not Correct"));
}

void ConvertNrrdToField::execute()
{
  auto nrrd = getRequiredInput(InputNrrd);

  if (needToExecute())
  {
    // TODO: convert to algo factory call
    ConverterAlgo algo(getLogger());
    auto state = get_state();

    std::string datalocation = state->getValue(Parameters::DataLocation).toString();
    std::string fieldtype = state->getValue(Parameters::FieldType).toString();
    std::string convertparity = state->getValue(Parameters::ConvertParity).toString();
    FieldHandle ofield;
    if (!algo.nrrdToField(nrrd,ofield,datalocation,fieldtype,convertparity))
    {
      error("False returned on algorithm direct call.");
      return;
    }
    sendOutput(OutputField, ofield);
  }
}
