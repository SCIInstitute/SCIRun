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
        class VtkScaleBarOverlay : public VtkOverlay
        {
        public:
            void initialize(vtkRenderer*) override;
            void cameraChanged(vtkCamera*) override;
            void resize(int, int) override;
            void setVisible(bool visible) override;

            void setFontSize(int size);
            void setLength(double length);
            void setHeight(double height);
            void setMultiplier(double mul);
            void setNumTicks(double num);
            void setLineWidth(double width);
            void setLineColor(double color);
            void setUnit(const std::string& unit);
            void setProjLength(double length);

           private:
            vtkRenderer* sceneRenderer_ = nullptr;
            vtkSmartPointer<vtkRenderer> overlayRenderer_;

            vtkSmartPointer<vtkActor2D> barActor_;
            vtkSmartPointer<vtkTextActor> textActor_;

            bool visible_{true};
            int width_{1};
            int height_{1};

            int sbFontSize_;
            double sbLength_;
            double sbHeight_;
            double sbMultiplier_;
            double sbNumTicks_;
            double sbLineWidth_;
            double sbLineColor_;
            std::string sbUnit_;
            double sbProjLength_;

            void updateProjectedLength();
            void updateScale();
        };
    }
}
#endif