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


#include <Modules/Legacy/Fields/ApplyMappingMatrix.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/ApplyMappingMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

/// @brief Apply a mapping matrix to project the data from one field onto the mesh of another field.

ApplyMappingMatrix::ApplyMappingMatrix()
  : Module(ModuleLookupInfo("ApplyMappingMatrix", "ChangeFieldData", "SCIRun"), false)
{
  INITIALIZE_PORT(Source);
  INITIALIZE_PORT(Destination);
  INITIALIZE_PORT(Mapping);
  INITIALIZE_PORT(Output);
}

void ApplyMappingMatrix::execute()
{
  auto src = getRequiredInput(Source);
  auto dest = getRequiredInput(Destination);
  auto mapp = getRequiredInput(Mapping);

  if (needToExecute())
  {
   auto out = algo().run(withInputData((Source, src)(Destination, dest)(Mapping, mapp)));

   sendOutputFromAlgorithm(Output, out);
  }
}
