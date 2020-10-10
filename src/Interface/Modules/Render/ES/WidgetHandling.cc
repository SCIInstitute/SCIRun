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


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <es-log/trace-log.h>
// Needed for OpenGL include files on Travis:
#include <gl-platform/GLPlatform.hpp>
#include <Interface/Modules/Render/UndefiningX11Cruft.h>
#include <QOpenGLWidget>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <Interface/Modules/Render/ES/SRInterface.h>
#include <Interface/Modules/Render/ES/SRCamera.h>
#include <Interface/Modules/Render/ES/comp/StaticClippingPlanes.h>
#include <Core/Application/Preferences/Preferences.h>

#include <Core/Logging/Log.h>
#include <Core/Application/Application.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Interface/Modules/Render/ES/RendererCollaborators.h>

ObjectTransformCalculatorFactory::ObjectTransformCalculatorFactory(const BasicRendererObjectProvider* brop, const glm::vec2& initPos, float initW)
  : brop_(brop), initPos_(initPos), initW_(initW)
{
}

WidgetUpdateService::WidgetUpdateService(ObjectTransformer* transformer, const ScreenParams& screen) :
  transformer_(transformer), screen_(screen)
{
}

void WidgetUpdateService::doPostSelectSetup(int x, int y, float depth)
{
  auto initialW = getInitialW(depth);
  auto initialPosition = screen_.positionFromClick(x, y);

  XYZ xyz(initialPosition, initialW, transformFactory_);

  for (const auto& movement : movements_)
    currentTransformationCalculators_.emplace(movement, xyz.make(movement)(currentWidget_));
}

namespace
{
  WidgetInteraction yetAnotherEnumConversion(MouseButton btn)
  {
    switch (btn)
    {
    case MouseButton::MOUSE_LEFT:
      return WidgetInteraction::CLICK;
    case MouseButton::MOUSE_RIGHT:
      return WidgetInteraction::RIGHT_CLICK;
    default:
      return WidgetInteraction::CLICK;
    }
  }
}

void WidgetUpdateService::setCurrentWidget(Graphics::Datatypes::WidgetHandle w)
{
  currentWidget_ = w;
  movements_ = w->movementType(yetAnotherEnumConversion(buttonPushed_));
}
