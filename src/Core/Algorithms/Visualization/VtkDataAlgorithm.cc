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

#include <Core/Algorithms/Visualization/VtkDataAlgorithm.h>
#include <Core/Algorithms/Visualization/VtkGeometryBuilder.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include <Core/Logging/Log.h>
#include <spdlog/fmt/ostr.h>

using namespace SCIRun;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Visualization;
using namespace VtkVisualization;
using namespace Core::Datatypes;

ALGORITHM_PARAMETER_DEF(VtkVisualization, DefaultColorR);
ALGORITHM_PARAMETER_DEF(VtkVisualization, DefaultColorG);
ALGORITHM_PARAMETER_DEF(VtkVisualization, DefaultColorB);
ALGORITHM_PARAMETER_DEF(VtkVisualization, DefaultColorA);
ALGORITHM_PARAMETER_DEF(VtkVisualization, Radius);
ALGORITHM_PARAMETER_DEF(VtkVisualization, UseNormals);
ALGORITHM_PARAMETER_DEF(VtkVisualization, ShowEdges);
ALGORITHM_PARAMETER_DEF(VtkVisualization, ModuleID);

static uint32_t getNewVersionNumber()
{
  static uint32_t versionNumber = 0;
  return ++versionNumber;
}

VtkDataAlgorithm::VtkDataAlgorithm()
{
  addParameter(Parameters::DefaultColorR, 0.5);
  addParameter(Parameters::DefaultColorG, 0.5);
  addParameter(Parameters::DefaultColorB, 0.5);
  addParameter(Parameters::DefaultColorA, 1.0);
  addParameter(Parameters::Radius, 0.1);
  addParameter(Parameters::UseNormals, true);
  addParameter(Parameters::ShowEdges, false);
  addParameter(Parameters::ModuleID, 0);
}

AlgorithmOutput VtkDataAlgorithm::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Name("Field"));

  auto colorMap = input.get<ColorMap>(Name("ColorMapObject"));

  VtkGeometryBuilder builder(*this);

  auto renderable = builder.buildGeometryObject(field, colorMap);

  if (!renderable)
  {
    THROW_ALGORITHM_INPUT_ERROR("field type not supported.");
  }

  renderable->version = getNewVersionNumber();

  renderable->id = get(Parameters::ModuleID).toInt();

  AlgorithmOutput output;
  output[Name("SceneGraph")] = renderable;

  return output;
}