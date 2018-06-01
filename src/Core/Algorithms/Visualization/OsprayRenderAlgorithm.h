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

#ifndef CORE_ALGORITHMS_VISUALIZATION_OSPRAYRENDERALGORITHM_H
#define CORE_ALGORITHMS_VISUALIZATION_OSPRAYRENDERALGORITHM_H

#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Algorithms/Visualization/share.h>

namespace detail
{
  class OsprayImpl;
}

namespace SCIRun
{
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
        ALGORITHM_PARAMETER_DECL(StreamlineRadius);

        class SCISHARE OsprayRenderAlgorithm : public AlgorithmBase
        {
        public:
          OsprayRenderAlgorithm();
          virtual AlgorithmOutput run(const AlgorithmInput& input) const override;
        private:
          void render(const AlgorithmInput& input) const;
          std::string writeImage() const;
          mutable SharedPointer<detail::OsprayImpl> impl_;
        };
      }
    }
  }
}

#endif
