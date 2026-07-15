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
            VtkCameraController() = default;
            virtual ~VtkCameraController() = default;

            //----------------------------------------
            // Interaction
            //----------------------------------------

            void mousePress(float x, float y, MouseButton button);
            void mouseMove(float x, float y, vtkRenderer* renderer);
            void mouseRelease();

            void mouseWheel(int32_t delta, vtkRenderer* renderer);

            //----------------------------------------
            // Camera utilities
            //----------------------------------------

            void resetView(vtkRenderer* renderer);

            //----------------------------------------
            // Settings
            //----------------------------------------

            void setRotationSpeed(float speed);
            void setPanSpeed(float speed);
            void setZoomSpeed(float speed);

            float rotationSpeed() const;
            float panSpeed() const;
            float zoomSpeed() const;

        private:

            //----------------------------------------
            // Helpers
            //----------------------------------------

            void rotate(vtkCamera* camera,
                vtkRenderer* renderer,
                float dx,
                float dy);

            void pan(vtkCamera* camera,
                vtkRenderer* renderer,
                float dx,
                float dy);

            void zoom(vtkCamera* camera,
                vtkRenderer* renderer,
                float amount);

        private:

            bool dragging_{ false };

            MouseButton activeButton_;

            glm::vec2 lastMousePos_{ 0.f, 0.f };

            float rotationSpeed_{ 0.5f };
            float panSpeed_{ 0.01f };
            float zoomSpeed_{ 0.15f };
        };

    }
} // namespace SCIRun::Render

#endif