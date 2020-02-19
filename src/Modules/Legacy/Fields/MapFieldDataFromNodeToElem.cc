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


#include <Modules/Legacy/Fields/MapFieldDataFromNodeToElem.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromNodeToElem.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

/// @brief This module contains several filters for converting data that is
/// stored in the nodes to data that is stored in the elements.

MapFieldDataFromNodeToElem::MapFieldDataFromNodeToElem()
  : Module(ModuleLookupInfo("MapFieldDataFromNodeToElem", "ChangeFieldData", "SCIRun"), true)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
}

void MapFieldDataFromNodeToElem::setStateDefaults()
{
  setStateStringFromAlgoOption(MapFieldDataFromNodeToElemAlgo::Method);
}

void MapFieldDataFromNodeToElem::execute()
{
  auto input = getRequiredInput(InputField);

  if (needToExecute())
  {
    setAlgoOptionFromState(MapFieldDataFromNodeToElemAlgo::Method);

    auto output = algo().run(withInputData((InputField, input)));

    sendOutputFromAlgorithm(OutputField, output);
  }
}
