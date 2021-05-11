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

#ifndef INTERFACE_MODULES_RENDER_SPIRESCIRUN_RENDERERINTERFACECOLLABORATORS_H
#define INTERFACE_MODULES_RENDER_SPIRESCIRUN_RENDERERINTERFACECOLLABORATORS_H

#include <Core/Datatypes/Feedback.h>
#include <Graphics/Widgets/Widget.h>
#include <Dataflow/Network/NetworkFwd.h>
#include <Interface/Modules/Render/share.h>


namespace SCIRun
{
  namespace Render
  {
    enum class MouseMode
    {
      MOUSE_OLDSCIRUN,
      MOUSE_NEWSCIRUN
    };

    enum class MatFactor
    {
      MAT_AMBIENT,
      MAT_DIFFUSE,
      MAT_SPECULAR,
      MAT_SHINE
    };

    enum class FogFactor
    {
      FOG_INTENSITY,
      FOG_START,
      FOG_END
    };

    class SCISHARE ClippingPlaneManager
    {
    public:
      explicit ClippingPlaneManager(Dataflow::Networks::ModuleStateHandle state);
      const std::vector<Core::Datatypes::ClippingPlane>& allPlanes() const { return clippingPlanes_; }
      const Core::Datatypes::ClippingPlane& active() const
      {
        return clippingPlanes_[activeIndex_];
      }
      void setActive(int index);
      void setActiveVisibility(bool visible);
      void setActiveNormalReversed(bool normalReversed);
      void setActiveFrameOn(bool frameOn);
      void setActiveX(int index);
      void setActiveY(int index);
      void setActiveZ(int index);
      void setActiveD(int index);
      void loadFromState();
    private:
      Dataflow::Networks::ModuleStateHandle state_;
      std::vector<Core::Datatypes::ClippingPlane> clippingPlanes_;
      int activeIndex_{ 0 };
      Core::Algorithms::VariableList sliceWith(std::function<Core::Algorithms::Variable::Value(const Core::Datatypes::ClippingPlane&)> func);
    };

    using ClippingPlaneManagerPtr = SharedPointer<ClippingPlaneManager>;

    class SCISHARE PreviousWidgetSelectionInfo
    {
    public:
      PreviousWidgetSelectionInfo() = default;
      unsigned long timeSince(const std::chrono::system_clock::time_point& time) const;
      unsigned long timeSince(unsigned long time) const;
      unsigned long timeSinceWidgetColorRestored() const;
      unsigned long timeSinceLastSelectionAttempt() const;
      bool hasSameMousePosition(int x, int y) const;
      bool hasSameCameraTansform(const glm::mat4& mat) const;
      bool hasSameWidget(Graphics::Datatypes::WidgetHandle widget) const;
      void widgetColorRestored();
      void selectionAttempt();
      void setCameraTransform(glm::mat4 mat);
      void setMousePosition(int x, int y);
      void setFrameIsFinished(bool finished);
      bool getFrameIsFinished() const;
      void setPreviousWidget(Graphics::Datatypes::WidgetHandle widget);
      Graphics::Datatypes::WidgetHandle getPreviousWidget() const;
      bool hasPreviousWidget() const;
      void deletePreviousWidget();
      int getPreviousMouseX() const;
      int getPreviousMouseY() const;
    private:
      unsigned long timeSinceEpoch(const std::chrono::system_clock::time_point& time) const;
      std::chrono::system_clock::time_point timeWidgetColorRestored_{};
      std::chrono::system_clock::time_point timeOfLastSelectionAttempt_{};
      Graphics::Datatypes::WidgetHandle     previousSelectedWidget_;
      glm::mat4                             previousCameraTransform_{ 0.0 };
      int                                   lastMousePressEventX_{ 0 };
      int                                   lastMousePressEventY_{ 0 };
      bool                                  frameIsFinished_{ false };
    };
  }
}

#endif
