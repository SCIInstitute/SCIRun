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


///@brief This module makes a mesh that looks like a wire

#include <Modules/Legacy/Fields/GenerateElectrodeFromPoints.h>
#include <Core/Algorithms/Legacy/Fields/GenerateElectrodeFromPointsAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/VariableHelper.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Logging/Log.h>
#include <Graphics/Glyphs/GlyphGeom.h>


using namespace SCIRun;
using namespace Core;
using namespace Logging;
using namespace Modules::Fields;
using namespace Algorithms;
using namespace Algorithms::Fields;
using namespace Dataflow::Networks;
using namespace Datatypes;

MODULE_INFO_DEF(GenerateElectrodeFromPoints, NewField, SCIRun)


GenerateElectrodeFromPoints::GenerateElectrodeFromPoints() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(ElectrodeMesh);
}

void GenerateElectrodeFromPoints::setStateDefaults()
{
  setStateDoubleFromAlgo(Parameters::ElectrodeLength);
  setStateDoubleFromAlgo(Parameters::ElectrodeThickness);
  setStateDoubleFromAlgo(Parameters::ElectrodeWidth);
  setStateIntFromAlgo(Parameters::NumberOfControlPoints);
  setStateIntFromAlgo(Parameters::ElectrodeResolution);
  setStateBoolFromAlgo(Parameters::UseFieldNodes);
  setStateStringFromAlgo(Parameters::ElectrodeType);
  setStateStringFromAlgo(Parameters::ElectrodeProjection);
}

void GenerateElectrodeFromPoints::execute()
{
  FieldHandle ofield;

  auto source = getRequiredInput(InputField);
    
  setAlgoDoubleFromState(Parameters::ElectrodeLength);
  setAlgoDoubleFromState(Parameters::ElectrodeThickness);
  setAlgoDoubleFromState(Parameters::ElectrodeWidth);
  setAlgoIntFromState(Parameters::NumberOfControlPoints);
  setAlgoIntFromState(Parameters::ElectrodeResolution);
  setAlgoBoolFromState(Parameters::UseFieldNodes);
  setAlgoOptionFromState(Parameters::ElectrodeType);
  setAlgoOptionFromState(Parameters::ElectrodeProjection);

    
  auto output = algo().run(withInputData((InputField, source)));

  sendOutputFromAlgorithm(ElectrodeMesh, output);
}


