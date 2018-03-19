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

#include <Modules/Visualization/InterfaceWithOspray.h>
#include <Modules/Visualization/OsprayAlgorithm.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/range/join.hpp>

#ifdef WITH_OSPRAY
#include <ospray/ospray.h>
#endif

using namespace SCIRun;
using namespace Dataflow::Networks;
using namespace Modules::Visualization;
using namespace Core;
using namespace Core::Algorithms;
using namespace Core::Geometry;
using namespace Visualization;
using namespace Datatypes;

MODULE_INFO_DEF(InterfaceWithOspray, Visualization, SCIRun)

void InterfaceWithOspray::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::DefaultColorR, 0.5);
  state->setValue(Parameters::DefaultColorG, 0.5);
  state->setValue(Parameters::DefaultColorB, 0.5);
  state->setValue(Parameters::DefaultColorA, 1.0);
}

InterfaceWithOspray::InterfaceWithOspray() : Module(staticInfo_)
{
  INITIALIZE_PORT(Field);
  INITIALIZE_PORT(ColorMapObject);
  INITIALIZE_PORT(Streamlines);
  INITIALIZE_PORT(SceneGraph);
}

void InterfaceWithOspray::execute()
{
  #ifdef WITH_OSPRAY
  auto fields = getOptionalDynamicInputs(Field);
  auto colorMaps = getOptionalDynamicInputs(ColorMapObject);
  auto streamlines = getOptionalDynamicInputs(Streamlines);

  if (needToExecute())
  {
    OsprayDataAlgorithm ospray;
    //TODO
    ospray.set(Parameters::DefaultColorR, get_state()->getValue(Parameters::DefaultColorR).toDouble());
    ospray.set(Parameters::DefaultColorG, get_state()->getValue(Parameters::DefaultColorR).toDouble());
    ospray.set(Parameters::DefaultColorB, get_state()->getValue(Parameters::DefaultColorR).toDouble());
    ospray.set(Parameters::DefaultColorA, get_state()->getValue(Parameters::DefaultColorR).toDouble());

    if (!fields.empty())
    {
      if (colorMaps.size() < fields.size())
        colorMaps.resize(fields.size());

      for (auto&& fieldColor : zip(fields, colorMaps))
      {
        FieldHandle field;
        ColorMapHandle color;
        boost::tie(field, color) = fieldColor;

        FieldInformation info(field);

        if (!info.is_trisurfmesh())
          THROW_INVALID_ARGUMENT("Module currently only works with trisurfs.");

        ospray.addField(field, color);
      }
    }

    for (auto& streamline : streamlines)
    {
      FieldInformation info(streamline);

      if (!info.is_curvemesh())
        THROW_INVALID_ARGUMENT("Module currently only works with curvemesh streamlines.");

      ospray.addStreamline(streamline);
    }
    
    auto geom = boost::make_shared<CompositeOsprayGeometryObject>(ospray.allObjectsToRender());
    
    sendOutput(SceneGraph, geom);
  }
  #else
  error("Build SCIRun with WITH_OSPRAY set to true to enable this module.");
  #endif
}
