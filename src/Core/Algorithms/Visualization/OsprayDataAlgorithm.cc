/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
University of Utah.

License for the specific language governing rights and limitations under
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

#include <Core/Algorithms/Visualization/OsprayDataAlgorithm.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Visualization;
using namespace Core::Datatypes;

ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorR);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorG);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorB);
ALGORITHM_PARAMETER_DEF(Visualization, DefaultColorA);

OsprayDataAlgorithm::OsprayDataAlgorithm()
{
  addParameter(Parameters::DefaultColorR, 0.5);
  addParameter(Parameters::DefaultColorG, 0.5);
  addParameter(Parameters::DefaultColorB, 0.5);
  addParameter(Parameters::DefaultColorA, 1.0);
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::addStreamline(FieldHandle field) const
{
  auto obj = fillDataBuffers(field, nullptr);

  obj->isStreamline = true;
  auto& fieldData = obj->data;
  const auto& vertex = fieldData.vertex;
  const auto& color = fieldData.color;

  auto& index = fieldData.index;
  {
    auto facade(field->mesh()->getFacade());
    for (const auto& edge : facade->edges())
    {
      auto nodesFromEdge = edge.nodeIndices();
      index.push_back(nodesFromEdge[0]);
    }
  }
  return obj;
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::fillDataBuffers(FieldHandle field, ColorMapHandle colorMap) const
{
  auto facade(field->mesh()->getFacade());

  auto obj = makeObject(field);
  auto& fieldData = obj->data;
  auto& vertex = fieldData.vertex;
  auto& color = fieldData.color;

  auto vfield = field->vfield();

  {
    double value;
    ColorRGB nodeColor(get(Parameters::DefaultColorR).toDouble(),
      get(Parameters::DefaultColorG).toDouble(),
      get(Parameters::DefaultColorB).toDouble());
    auto alpha = static_cast<float>(get(Parameters::DefaultColorA).toDouble());

    for (const auto& node : facade->nodes())
    {
      auto point = node.point();
      vertex.push_back(static_cast<float>(point.x()));
      vertex.push_back(static_cast<float>(point.y()));
      vertex.push_back(static_cast<float>(point.z()));
      vertex.push_back(0);

      vfield->get_value(value, node.index());
      if (colorMap)
      {
        nodeColor = colorMap->valueToColor(value);
      }
      color.push_back(static_cast<float>(nodeColor.r()));
      color.push_back(static_cast<float>(nodeColor.g()));
      color.push_back(static_cast<float>(nodeColor.b()));
      color.push_back(alpha);
    }
  }

  auto& index = fieldData.index;
  {
    for (const auto& face : facade->faces())
    {
      auto nodes = face.nodeIndices();
      index.push_back(static_cast<int32_t>(nodes[0]));
      index.push_back(static_cast<int32_t>(nodes[1]));
      index.push_back(static_cast<int32_t>(nodes[2]));
    }
  }
  return obj;
}

OsprayGeometryObjectHandle OsprayDataAlgorithm::makeObject(FieldHandle field) const
{
  OsprayGeometryObjectHandle obj(new OsprayGeometryObject);
  auto vmesh = field->vmesh();
  auto bbox = vmesh->get_bounding_box();
  obj->box = bbox;
  return obj;
}

AlgorithmOutput OsprayDataAlgorithm::run(const AlgorithmInput& input) const
{ 
  auto fields = input.getList<Field>(Name("Field"));
  auto colorMaps = input.getList<ColorMap>(Name("ColorMapObject"));
  auto streamlines = input.getList<Field>(Name("Streamlines"));

  std::vector<OsprayGeometryObjectHandle> renderables;

  for (auto&& fieldColor : zip(fields, colorMaps))
  {
    FieldHandle field;
    ColorMapHandle colorMap;
    boost::tie(field, colorMap) = fieldColor;

    FieldInformation info(field);

    if (!info.is_trisurfmesh())
    {
      THROW_ALGORITHM_INPUT_ERROR("Ospray rendering currently only works with trisurfs.");
    }

    renderables.push_back(fillDataBuffers(field, colorMap));
  }

  for (auto& streamline : streamlines)
  {
    FieldInformation info(streamline);

    if (!info.is_curvemesh())
      THROW_ALGORITHM_INPUT_ERROR("Ospray rendering currently only works with curvemesh streamlines.");

    renderables.push_back(addStreamline(streamline));
  }

  auto geom = boost::make_shared<CompositeOsprayGeometryObject>(renderables);
  AlgorithmOutput output;
  output[Name("SceneGraph")] = geom;
  return output;
}