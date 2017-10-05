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

#ifndef MODULES_VISUALIZATION_INTERFACEWITHOSPRAY_H
#define MODULES_VISUALIZATION_INTERFACEWITHOSPRAY_H

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Core/Thread/Interruptible.h>
#include <Modules/Visualization/share.h>

namespace detail
{
  class OsprayImpl;
}

namespace SCIRun {

  namespace Core
  {
    namespace Algorithms
    {
      namespace Visualization
      {
        ALGORITHM_PARAMETER_DECL(ImageHeight);
        ALGORITHM_PARAMETER_DECL(ImageWidth);
        ALGORITHM_PARAMETER_DECL(CameraPositionX);
        ALGORITHM_PARAMETER_DECL(CameraPositionY);
        ALGORITHM_PARAMETER_DECL(CameraPositionZ);
        ALGORITHM_PARAMETER_DECL(CameraUpX);
        ALGORITHM_PARAMETER_DECL(CameraUpY);
        ALGORITHM_PARAMETER_DECL(CameraUpZ);
        ALGORITHM_PARAMETER_DECL(CameraViewX);
        ALGORITHM_PARAMETER_DECL(CameraViewY);
        ALGORITHM_PARAMETER_DECL(CameraViewZ);
        ALGORITHM_PARAMETER_DECL(DefaultColorR);
        ALGORITHM_PARAMETER_DECL(DefaultColorG);
        ALGORITHM_PARAMETER_DECL(DefaultColorB);
        ALGORITHM_PARAMETER_DECL(BackgroundColorR);
        ALGORITHM_PARAMETER_DECL(BackgroundColorG);
        ALGORITHM_PARAMETER_DECL(BackgroundColorB);
        ALGORITHM_PARAMETER_DECL(FrameCount);
        ALGORITHM_PARAMETER_DECL(ShowImageInWindow);
        ALGORITHM_PARAMETER_DECL(LightVisible);
        ALGORITHM_PARAMETER_DECL(LightColorR);
        ALGORITHM_PARAMETER_DECL(LightColorG);
        ALGORITHM_PARAMETER_DECL(LightColorB);
        ALGORITHM_PARAMETER_DECL(LightIntensity);
        ALGORITHM_PARAMETER_DECL(LightType);
        ALGORITHM_PARAMETER_DECL(AutoCameraView);
      }
    }
  }

  namespace Modules {
    namespace Visualization {

      class SCISHARE InterfaceWithOspray : public Dataflow::Networks::GeometryGeneratingModule,
        public Has2InputPorts<DynamicPortTag<FieldPortTag>, DynamicPortTag<ColorMapPortTag>>,
        public Has1OutputPort<GeometryPortTag>
      {
      public:
        InterfaceWithOspray();
        virtual void execute() override;

        INPUT_PORT_DYNAMIC(0, Field, Field);
        INPUT_PORT_DYNAMIC(1, ColorMapObject, ColorMap);
        OUTPUT_PORT(0, SceneGraph, GeometryObject);

        MODULE_TRAITS_AND_INFO(ModuleHasUI)

        virtual void setStateDefaults() override;

        HAS_DYNAMIC_PORTS
      };

    }
  }
}

#endif
