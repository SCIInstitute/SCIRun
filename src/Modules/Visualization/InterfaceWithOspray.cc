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
  state->setValue(Parameters::ImageHeight, 768);
  state->setValue(Parameters::ImageWidth, 1024);
  state->setValue(Parameters::CameraPositionX, 5.0);
  state->setValue(Parameters::CameraPositionY, 5.0);
  state->setValue(Parameters::CameraPositionZ, 5.0);
  state->setValue(Parameters::CameraUpX, 0.0);
  state->setValue(Parameters::CameraUpY, 0.0);
  state->setValue(Parameters::CameraUpZ, 1.0);
  state->setValue(Parameters::CameraViewX, 0.0);
  state->setValue(Parameters::CameraViewY, 0.0);
  state->setValue(Parameters::CameraViewZ, 0.0);
  state->setValue(Parameters::DefaultColorR, 0.5);
  state->setValue(Parameters::DefaultColorG, 0.5);
  state->setValue(Parameters::DefaultColorB, 0.5);
  state->setValue(Parameters::DefaultColorA, 1.0);
  state->setValue(Parameters::BackgroundColorR, 0.0);
  state->setValue(Parameters::BackgroundColorG, 0.0);
  state->setValue(Parameters::BackgroundColorB, 0.0);
  state->setValue(Parameters::FrameCount, 10);
  state->setValue(Parameters::ShowImageInWindow, true);
  state->setValue(Parameters::LightColorR, 1.0);
  state->setValue(Parameters::LightColorG, 1.0);
  state->setValue(Parameters::LightColorB, 1.0);
  state->setValue(Parameters::LightIntensity, 1.0);
  state->setValue(Parameters::LightVisible, false);
  state->setValue(Parameters::LightType, std::string("ambient"));
  state->setValue(Parameters::AutoCameraView, true);
  state->setValue(Parameters::StreamlineRadius, 0.1);
  state->setValue(Variables::Filename, std::string(""));
}

InterfaceWithOspray::InterfaceWithOspray() : GeometryGeneratingModule(staticInfo_)//, impl_(new OsprayImpl(get_state()))
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
    OsprayAlgorithm ospray(get_state());
    ospray.setup();

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
