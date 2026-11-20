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

#pragma once

#ifdef WITH_VTK

#include <glm/glm.hpp>

#include <Core/Datatypes/Feedback.h>

class vtkRenderer;
class vtkCamera;

namespace SCIRun {
    namespace Render {

class VtkCameraController
      {
       public:
        void setCamera(vtkCamera* camera);

        void mousePress(float x, float y, MouseButton button);

        void mouseMove(float x, float y);

        void mouseRelease();

        void mouseWheel(int32_t delta);

        void resetView();

        void setRotationSpeed(float speed) { rotationSpeed_ = speed; }
        void setPanSpeed(float speed) { panSpeed_ = speed; }
        void setZoomSpeed(float speed) { zoomSpeed_ = speed; }

        float rotationSpeed() const { return rotationSpeed_; }
        float panSpeed() const { return panSpeed_; }
        float zoomSpeed() const { return zoomSpeed_; }

       private:
        void rotate(float dx, float dy);

        void pan(float dx, float dy);

        void zoom(float amount);

       private:
        vtkCamera* camera_{nullptr};

        bool dragging_{false};

        MouseButton activeButton_;

        glm::vec2 lastMousePos_{0.f, 0.f};

        float rotationSpeed_{0.5f};
        float panSpeed_{0.01f};
        float zoomSpeed_{0.15f};
      };

    }
} // namespace SCIRun::Render

#endif