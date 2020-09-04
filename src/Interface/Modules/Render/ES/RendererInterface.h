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

#ifndef INTERFACE_MODULES_RENDER_SPIRESCIRUN_RENDERERINTERFACE_H
#define INTERFACE_MODULES_RENDER_SPIRESCIRUN_RENDERERINTERFACE_H

#include <Interface/Modules/Render/ES/RendererInterfaceCollaborators.h>
#include <Interface/Modules/Render/ES/RendererInterfaceFwd.h>
#include <glm/gtc/quaternion.hpp>
#include <es-general/comp/Transform.hpp>
#include <Interface/Modules/Render/share.h>

class QOpenGLContext;
class QColor;

namespace SCIRun
{
  namespace Render
  {
    class SCISHARE ScreenParameters
    {
    public:
      virtual ~ScreenParameters() {}

      virtual size_t getScreenWidthPixels() const = 0;
      virtual size_t getScreenHeightPixels() const = 0;
      virtual void calculateScreenSpaceCoords(int x_in, int y_in, float& x_out, float& y_out) = 0;
      virtual MouseMode getMouseMode() const = 0;
    };

    class SCISHARE ObjectTransformer
    {
    public:
      virtual ~ObjectTransformer() {}

      virtual void modifyObject(const std::string& id, const gen::Transform& trans) = 0;
      //TODO: move
      virtual glm::mat4 getStaticCameraViewProjection() = 0;
    };

    class SCISHARE RendererWidgetInterface
    {
    public:
      virtual ~RendererWidgetInterface() {}

      virtual void setLockZoom(bool lock) = 0;
      virtual void setLockPanning(bool lock) = 0;
      virtual void setLockRotation(bool lock) = 0;
      virtual bool hasShaderPromise() const = 0;
      virtual void widgetMouseDown(MouseButton btn, int x, int y) = 0;
      virtual void widgetMouseMove(MouseButton btn, int x, int y) = 0;
      virtual void widgetMouseUp() = 0;
      virtual void inputMouseDown(MouseButton btn, float x, float y) = 0;
      virtual void inputMouseMove(MouseButton btn, float x, float y) = 0;
      virtual void inputMouseUp() = 0;
      virtual void inputMouseWheel(int32_t delta) = 0;
      virtual void setMouseMode(MouseMode mode) = 0;
      virtual void eventResize(size_t width, size_t height) = 0;
      virtual void doFrame(double constantDeltaTime) = 0;
    };

    class SCISHARE RendererModuleInterface
    {
    public:
      virtual ~RendererModuleInterface() {}

      virtual void setContext(QOpenGLContext* context) = 0;
      virtual void setBackgroundColor(const QColor& color) = 0;
      virtual void setCameraDistance(const float distance) = 0;
      virtual void setCameraLookAt(const glm::vec3& lookAt) = 0;
      virtual void setCameraRotation(const glm::quat& rotation) = 0;
      virtual bool hasObject(const std::string& object) = 0;
      virtual void removeAllGeomObjects() = 0;
      virtual void runGCOnNextExecution() = 0;
      virtual void setLightOn(int index, bool value) = 0;
      virtual void setLightColor(int index, float r, float g, float b) = 0;
      virtual glm::quat getCameraRotation() const = 0;
      virtual std::string toString(std::string prefix) const = 0;
      virtual void setZoomInverted(bool value) = 0;
      virtual float getCameraDistance() const = 0;
      virtual glm::vec3 getCameraLookAt() const = 0;
      virtual void gcInvalidObjects(const std::vector<std::string>& validObjects) = 0;
      virtual glm::mat4 getWidgetTransform() = 0;
      virtual void setView(const glm::vec3& view, const glm::vec3& up) = 0;
      virtual void doAutoView() = 0;
      virtual void setZoomSpeed(int zoomSpeed) = 0;
      virtual void setAutoRotateVector(const glm::vec2& axis) = 0;
      virtual void setAutoRotateSpeed(double speed) = 0;
      virtual void handleGeomObject(Graphics::Datatypes::GeometryHandle object, int port) = 0;
      virtual void doInitialWidgetUpdate(Graphics::Datatypes::WidgetHandle& widget, int x, int y) = 0;
      virtual Graphics::Datatypes::WidgetHandle select(int x, int y, Graphics::Datatypes::WidgetList& widgets) = 0;
      virtual void setClippingPlaneIndex(int index) = 0;
      virtual void setClippingPlaneFrameOn(bool value) = 0;
      virtual void reverseClippingPlaneNormal(bool value) = 0;
      virtual void setClippingPlaneX(double value) = 0;
      virtual void setClippingPlaneY(double value) = 0;
      virtual void setClippingPlaneZ(double value) = 0;
      virtual void setClippingPlaneD(double value) = 0;
      virtual void showOrientation(bool value) = 0;
      virtual void setLightAzimuth(int index, float azimuth) = 0;
      virtual void setLightInclination(int index, float inclination) = 0;
      virtual void setMaterialFactor(MatFactor factor, double value) = 0;
      virtual void setFog(FogFactor factor, double value) = 0;
      virtual const glm::mat4& getViewToProjection() const = 0;
      virtual void setOrientSize(int size) = 0;
      virtual void setOrientPosX(int pos) = 0;
      virtual void setOrientPosY(int pos) = 0;
      virtual void setFogColor(const glm::vec4 &color) = 0;
      virtual void setTransparencyRendertype(RenderState::TransparencySortType rType) = 0;
      virtual StaticClippingPlanes* getClippingPlanes() = 0;
      virtual void setClippingPlaneVisible(bool value) = 0;
      virtual Core::Geometry::BBox getSceneBox() = 0;
      virtual const glm::mat4& getWorldToView() const = 0;
      virtual glm::mat4 getWorldToProjection() const = 0;
    };

    class SCISHARE RendererInterface : public ScreenParameters, public RendererModuleInterface, public RendererWidgetInterface
    {
    };

  }
}

#endif
