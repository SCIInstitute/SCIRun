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
#include "VtkOverlay.h"
#include <Core/Algorithms/Visualization/VtkIncludes.h>

namespace SCIRun {
    namespace Render {
        class VtkOrientationOverlay : public VtkOverlay
        {
        public:
            void initialize(vtkRenderer* renderer) override;

            void cameraChanged(vtkCamera* camera) override;

            void resize(int width, int height) override;

            void setVisible(bool visible) override;

            void setSize(int value);

            void setPosX(int value);

            void setPosY(int value);

            void setPosition(
                int x,
                int y);

        private:
            void updateViewport();

        private:
            bool visible_{ true };

            int size_{ 100 };

            int posX_{ 20 };
            int posY_{ 20 };

            int width_{ 1 };
            int height_{ 1 };

            vtkSmartPointer<vtkRenderer>
                overlayRenderer_;

            vtkSmartPointer<vtkAxesActor>
                axesActor_;
        };
    }
}
#endif