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
#include <Core/Algorithms/Visualization/OsprayRenderAlgorithm.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/String.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

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
  setStateIntFromAlgo(Parameters::ImageHeight);
  setStateIntFromAlgo(Parameters::ImageWidth);
  setStateDoubleFromAlgo(Parameters::CameraPositionX);
  setStateDoubleFromAlgo(Parameters::CameraPositionY);
  setStateDoubleFromAlgo(Parameters::CameraPositionZ);
  setStateDoubleFromAlgo(Parameters::CameraUpX);
  setStateDoubleFromAlgo(Parameters::CameraUpY);
  setStateDoubleFromAlgo(Parameters::CameraUpZ);
  setStateDoubleFromAlgo(Parameters::CameraViewX);
  setStateDoubleFromAlgo(Parameters::CameraViewY);
  setStateDoubleFromAlgo(Parameters::CameraViewZ);
  setStateDoubleFromAlgo(Parameters::BackgroundColorR);
  setStateDoubleFromAlgo(Parameters::BackgroundColorG);
  setStateDoubleFromAlgo(Parameters::BackgroundColorB);
  setStateIntFromAlgo(Parameters::FrameCount);
  setStateBoolFromAlgo(Parameters::ShowImageInWindow);
  setStateDoubleFromAlgo(Parameters::LightColorR);
  setStateDoubleFromAlgo(Parameters::LightColorG);
  setStateDoubleFromAlgo(Parameters::LightColorB);
  setStateDoubleFromAlgo(Parameters::LightIntensity);
  setStateBoolFromAlgo(Parameters::LightVisible);
  setStateStringFromAlgo(Parameters::LightType);
  setStateBoolFromAlgo(Parameters::AutoCameraView);
  setStateDoubleFromAlgo(Parameters::StreamlineRadius);
  setStateStringFromAlgo(Variables::Filename);
}

ViewOsprayScene::ViewOsprayScene() : Module(staticInfo_)
{
  INITIALIZE_PORT(OspraySceneGraph);
}

void ViewOsprayScene::execute()
{
  auto geoms = getOptionalDynamicInputs(OspraySceneGraph);

  if (needToExecute())
  {
    setAlgoIntFromState(Parameters::ImageHeight);
    setAlgoIntFromState(Parameters::ImageWidth);
    setAlgoDoubleFromState(Parameters::CameraPositionX);
    setAlgoDoubleFromState(Parameters::CameraPositionY);
    setAlgoDoubleFromState(Parameters::CameraPositionZ);
    setAlgoDoubleFromState(Parameters::CameraUpX);
    setAlgoDoubleFromState(Parameters::CameraUpY);
    setAlgoDoubleFromState(Parameters::CameraUpZ);
    setAlgoDoubleFromState(Parameters::CameraViewX);
    setAlgoDoubleFromState(Parameters::CameraViewY);
    setAlgoDoubleFromState(Parameters::CameraViewZ);
    setAlgoDoubleFromState(Parameters::BackgroundColorR);
    setAlgoDoubleFromState(Parameters::BackgroundColorG);
    setAlgoDoubleFromState(Parameters::BackgroundColorB);
    setAlgoIntFromState(Parameters::FrameCount);
    setAlgoBoolFromState(Parameters::ShowImageInWindow);
    setAlgoDoubleFromState(Parameters::LightColorR);
    setAlgoDoubleFromState(Parameters::LightColorG);
    setAlgoDoubleFromState(Parameters::LightColorB);
    setAlgoDoubleFromState(Parameters::LightIntensity);
    setAlgoBoolFromState(Parameters::LightVisible);
    setAlgoStringFromState(Parameters::LightType);
    setAlgoBoolFromState(Parameters::AutoCameraView);
    setAlgoDoubleFromState(Parameters::StreamlineRadius);
    setAlgoStringFromState(Variables::Filename);

    auto output = algo().run(withInputData((OspraySceneGraph, geoms)));
    get_state()->setTransientValue(Variables::Filename, output.get<String>(Variables::Filename)->value());

    // algo adjusts camera based on rendered objects
    setStateDoubleFromAlgo(Parameters::CameraViewX);
    setStateDoubleFromAlgo(Parameters::CameraViewY);
    setStateDoubleFromAlgo(Parameters::CameraViewZ);
    setStateDoubleFromAlgo(Parameters::CameraUpX);
    setStateDoubleFromAlgo(Parameters::CameraUpY);
    setStateDoubleFromAlgo(Parameters::CameraUpZ);
  }
}
