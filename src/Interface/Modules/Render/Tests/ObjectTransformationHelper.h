/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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

#ifndef INTERFACE_MODULES_RENDER_TESTS_HELPER_H
#define INTERFACE_MODULES_RENDER_TESTS_HELPER_H

#include <Interface/Modules/Render/ES/RendererCollaborators.h>
#include <Interface/Modules/Render/ES/RendererInterface.h>
#include <Interface/Modules/Render/ES/SRCamera.h>

namespace SCIRun
{
  namespace RenderTesting
  {
    class ScreenParametersTest : public SCIRun::Render::ScreenParameters
    {
    public:
      size_t getScreenWidthPixels() const override;
      size_t getScreenHeightPixels() const override;
      void calculateScreenSpaceCoords(int x_in, int y_in, float& x_out, float& y_out) override;
      SCIRun::Render::MouseMode getMouseMode() const override;
    };

    class BasicRendererObjectProviderStub : public SCIRun::Render::BasicRendererObjectProvider
    {
    public:
      SCIRun::Render::SRCamera& camera() const override;
      const SCIRun::Render::ScreenParams& screen() const override;
    };

  bool operator==(const glm::mat4& lhs, const glm::mat4& rhs);
  }
}

#endif
