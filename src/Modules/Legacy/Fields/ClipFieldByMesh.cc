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


#include <Modules/Legacy/Fields/ClipFieldByMesh.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>

#include <Core/Algorithms/Legacy/Fields/ClipMesh/ClipMeshBySelection.h>
#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateIsInsideField.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(ClipFieldByMesh, NewField, SCIRun)

/// @class ClipFieldByMesh
/// @brief Clip a mesh to another mesh.

ClipFieldByMesh::ClipFieldByMesh() : Module(staticInfo_, false)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(ObjectField);
  INITIALIZE_PORT(OutputField);
  INITIALIZE_PORT(Mapping);
}

void ClipFieldByMesh::execute()
{
  auto input = getRequiredInput(InputField);
  FieldHandle output;
  auto object = getRequiredInput(ObjectField);
  FieldHandle selection;
  MatrixHandle interpolant;

  if (needToExecute())
  {
    //TODO: two-algo module

    CalculateIsInsideFieldAlgo insideAlgo;
    //insideAlgo.setLogger(getLogger());

    insideAlgo.setOption(Parameters::FieldOutputType, "char");
    insideAlgo.setOption(Parameters::SamplingScheme, "regular2");

    if (!insideAlgo.runImpl(input,object,selection))
    {
      THROW_ALGORITHM_PROCESSING_ERROR("False returned from inside algo");
    }

    ClipMeshBySelectionAlgo clipAlgo;
    if (!selection || !selection->vfield() || !input->vmesh())
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Selection output field is null");
    }

    if (input->vmesh()->num_elems() == selection->vfield()->num_values())
      clipAlgo.setOption(Parameters::ClipMethod, "Element Center");

    if (!clipAlgo.runImpl(input,selection,output,interpolant))
    {
      THROW_ALGORITHM_PROCESSING_ERROR("False returned from clip algo");
    }

    sendOutput(OutputField, output);
    sendOutput(Mapping, interpolant);
  }
}
