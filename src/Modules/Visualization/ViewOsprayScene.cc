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

#include <Modules/Visualization/ViewOsprayScene.h>
#include <Modules/Visualization/InterfaceWithOspray.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Mesh/VirtualMeshFacade.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

#include <boost/date_time/posix_time/posix_time.hpp>

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

MODULE_INFO_DEF(ViewOsprayScene, Visualization, SCIRun)

void ViewOsprayScene::setStateDefaults()
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

ViewOsprayScene::ViewOsprayScene() : Module(staticInfo_)
{
  INITIALIZE_PORT(OspraySceneGraph);
}

void ViewOsprayScene::execute()
{
  #ifdef WITH_OSPRAY
  auto geoms = getOptionalDynamicInputs(OspraySceneGraph);

  if (needToExecute())
  {
    OsprayAlgorithm ospray(get_state());
    ospray.setup();
    for (auto& geom : geoms)
    {
      auto g = boost::dynamic_pointer_cast<CompositeOsprayGeometryObject>(geom);
      if (g)
        ospray.render(*g);
    }

    auto isoString = boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::universal_time());
    auto filename = "scirunOsprayOutput_" + isoString + ".ppm";
    auto filePath = get_state()->getValue(Variables::Filename).toString() / boost::filesystem::path(filename);
    ospray.writeImage(filePath.string());
    remark("Saving output to " + filePath.string());

    get_state()->setTransientValue(Variables::Filename, filePath.string());
  }
  #else
  error("Build SCIRun with WITH_OSPRAY set to true to enable this module.");
  #endif
}
