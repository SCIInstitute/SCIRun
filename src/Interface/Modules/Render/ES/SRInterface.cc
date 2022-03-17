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
#include <glm/gtx/vec_swizzle.hpp>

#include <Interface/Modules/Render/ES/SRInterface.h>
#include <Interface/Modules/Render/ES/SRCamera.h>
#include <Interface/Modules/Render/ES/comp/StaticClippingPlanes.h>
#include <Core/Application/Preferences/Preferences.h>

#include <Core/Logging/Log.h>
#include <Core/Application/Application.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Interface/Modules/Render/ES/RendererCollaborators.h>

#include <es-general/comp/StaticScreenDims.hpp>
#include <es-general/comp/StaticCamera.hpp>
#include <es-general/comp/StaticOrthoCamera.hpp>
#include <es-general/comp/StaticObjRefID.hpp>
#include <es-render/comp/StaticIBOMan.hpp>
#include <es-render/comp/StaticVBOMan.hpp>
#include <es-render/comp/StaticShaderMan.hpp>
#include <es-render/comp/StaticFBOMan.hpp>
#include <es-render/comp/Texture.hpp>
#include <es-render/util/Uniform.hpp>
#include <es-render/comp/VBO.hpp>
#include <es-render/comp/IBO.hpp>
#include <es-render/comp/Shader.hpp>
#include <es-fs/fscomp/StaticFS.hpp>
#include <es-fs/FilesystemSync.hpp>

#include "CoreBootstrap.h"
#include "comp/RenderBasicGeom.h"
#include "comp/SRRenderState.h"
#include "comp/RenderList.h"
#include "comp/StaticWorldLight.h"
#include "comp/LightingUniforms.h"
#include "comp/ClippingPlaneUniforms.h"

using namespace SCIRun;
using namespace Core;
using namespace Datatypes;
using namespace Graphics::Datatypes;
using namespace Geometry;

using namespace std::placeholders;
using namespace Render;

namespace fs = spire;

namespace
{
  static glm::vec4 inverseGammaCorrect(const glm::vec4& in)
  {
    return glm::vec4(pow(glm::vec3(in), glm::vec3(2.2f)), in.a);
  }

  static glm::vec3 inverseGammaCorrect(const glm::vec3& in)
  {
    return pow(in, glm::vec3(2.2f));
  }

  const std::string widgetSelectFboName = "Selection:FBO:0";
}

SRInterface::SRInterface(int frameInitLimit) :
  widgetUpdater_(this, screen_),
  frameInitLimit_(frameInitLimit)
{
  mCamera.reset(new SRCamera(this));
  widgetUpdater_.setCamera(mCamera.get());
  // Construct ESCore. We will need to bootstrap the core. We should also
  // probably add utility static classes.
  setupCore();
  setupLights();
}

SRInterface::~SRInterface()
{
  glDeleteTextures(1, &mFontTexture);
}

bool SRInterface::hasShaderPromise() const
{
  return mCore.hasShaderPromise();
}

void SRInterface::runGCOnNextExecution()
{
  mCore.runGCOnNextExecution();
}

    //----------------------------------------------------------------------------------------------
    void SRInterface::setupCore()
    {
      mCore.addUserSystem(getSystemName_CoreBootstrap());

      // Add screen height / width static component.
      {
        gen::StaticScreenDims dims;
        dims.width = static_cast<uint32_t>(screen_.width);
        dims.height = static_cast<uint32_t>(screen_.height);
        mCore.addStaticComponent(dims);
      }

      // Be exceptionally careful with non-serializable components. They must be
      // created outside of the normal bootstrap. They cannot depend on anything
      // being serialized correctly. In this circumstance, the filesystem component
      // is system dependent and cannot be reliably serialized, so we add it and
      // mark it as non-serializable.
      {
        // Generate synchronous filesystem, manually add its static component,
        // then mark it as non-serializable.
        std::string filesystemRoot = Application::Instance().executablePath().string();
        filesystemRoot += boost::filesystem::path::preferred_separator;
        fs::StaticFS fileSystem((std::make_shared<fs::FilesystemSync>(filesystemRoot)));
        mCore.addStaticComponent(fileSystem);
        mCore.disableComponentSerialization<fs::StaticFS>();
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::setupLights()
    {
      mLightDirectionPolar.push_back(glm::vec2(0.0, 0.0));
      mLightDirectionView.push_back(glm::vec3(0.0, 0.0, -1.0));
      mLightsOn.push_back(true);
      for (int i = 1; i < LIGHT_NUM; ++i)
      {
        mLightDirectionPolar.push_back(glm::vec2(0.0, 0.0));
        mLightDirectionView.push_back(glm::vec3(0.0, 0.0, -1.0));
        mLightsOn.push_back(false);
      }
    }

    //----------------------------------------------------------------------------------------------
    std::string SRInterface::toString(std::string prefix) const
    {
      std::string output = prefix + "SR_INTERFACE:\n";
      prefix += "  ";

      output += prefix + "SRObjects: " + std::to_string(mSRObjects.size()) + "\n";
      for(auto& srobj : mSRObjects)
      {
        output += prefix + "  Name: \"" + srobj.mName
          + "\"  Port: " + std::to_string(srobj.mPort)
          + "  Passes: " + std::to_string(srobj.mPasses.size()) + "\n";

        for(auto& srpass: srobj.mPasses)
          output += prefix + "    PassName: \"" + srpass.passName + "\"  RenderType: " + "\n";
      }
      output += "\n";

      output += mCore.toString(prefix);

      return output;
    }

    //----------------------------------------------------------------------------------------------
    //---------------- Input -----------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    void SRInterface::widgetMouseMove(int x, int y)
    {
      widgetUpdater_.updateWidget(x, y);
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::widgetMouseUp()
    {
      widgetUpdater_.reset();
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::inputMouseDown(float x, float y)
    {
      autoRotateVector_ = glm::vec2(0.0, 0.0);
      tryAutoRotate_ = false;
      mCamera->mouseDownEvent(glm::vec2{x,y});
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::inputMouseMove(MouseButton btn, float x, float y)
    {
      mCamera->mouseMoveEvent(btn, glm::vec2{x,y});
      updateCamera();
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::inputMouseUp()
    {
      tryAutoRotate_ = Preferences::Instance().autoRotateViewerOnMouseRelease;
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::inputMouseWheel(int32_t delta)
    {
      if (!widgetUpdater_.currentWidget())
      {
        mCamera->mouseWheelEvent(delta, mZoomSpeed);
        updateCamera();
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::calculateScreenSpaceCoords(int x_in, int y_in, float& x_out, float& y_out)
    {
      x_out =  2.0f * static_cast<float>(x_in) / static_cast<float>(getScreenWidthPixels())  - 1.0f;
      y_out = -2.0f * static_cast<float>(y_in) / static_cast<float>(getScreenHeightPixels()) + 1.0f;
    }

    //----------------------------------------------------------------------------------------------
    //---------------- Camera ----------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------

    void SRInterface::cleanupSelect()
    {
      if (widgetSelectFboId_)
      {
        std::weak_ptr<ren::FBOMan> fm = mCore.getStaticComponent<ren::StaticFBOMan>()->instance_;
        std::shared_ptr<ren::FBOMan> fboMan = fm.lock();
        if (fboMan)
        {
          fboMan->removeInMemoryFBO(*widgetSelectFboId_);
          widgetSelectFboId_ = std::nullopt;
        }
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::eventResize(size_t width, size_t height)
    {
      screen_.width = width;
      screen_.height = height;

      GL(glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));

      // Obtain StaticScreenDims component and populate.
      gen::StaticScreenDims* dims = mCore.getStaticComponent<gen::StaticScreenDims>();
      if (dims)
      {
        dims->width = static_cast<uint32_t>(width);
        dims->height = static_cast<uint32_t>(height);
      }

      gen::StaticCamera* cam = mCore.getStaticComponent<gen::StaticCamera>();
      gen::StaticOrthoCamera* orthoCam = mCore.getStaticComponent<gen::StaticOrthoCamera>();
      if (cam == nullptr || orthoCam == nullptr) return;

      //Setup default orthognal camera projection.
      float aspect = static_cast<float>(width) / static_cast<float>(height);
      float orthoZNear = -1000.0f;
      float orthoZFar = 1000.0f;
      glm::mat4 orthoProj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, orthoZNear, orthoZFar);
      orthoCam->data.setOrthoProjection(orthoProj, aspect, 2.0f, 2.0f, orthoZNear, orthoZFar);
      orthoCam->data.winWidth = static_cast<float>(width);

      cam->data.winWidth = static_cast<float>(width);
      mCamera->setAsPerspective();
      updateCamera();
    }

    void SRInterface::setBackgroundColor(const QColor& color)
    {
      mCore.setBackgroundColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::doAutoView()
    {
      mCamera->doAutoView();
      updateCamera();
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::setView(const glm::vec3& view, const glm::vec3& up)
    {
      mCamera->setView(view, up);
      updateCamera();
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::updateCamera()
    {
      // Update the static camera with the appropriate world to view transform.
      glm::mat4 view = mCamera->getWorldToView();
      glm::mat4 projection = mCamera->getViewToProjection();

      gen::StaticCamera* camera = mCore.getStaticComponent<gen::StaticCamera>();
      if (camera)
      {
        camera->data.winWidth = static_cast<float>(screen_.width);
        camera->data.setView(view);
        camera->data.setProjection(projection, mCamera->getFOVY(), mCamera->getAspect(), mCamera->getZNear(), mCamera->getZFar());
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::applyAutoRotation()
    {
      if (length(autoRotateVector_) > 0.1)
        mCamera->rotate(autoRotateVector_ * autoRotateSpeed_);
      if (tryAutoRotate_)
        mCamera->tryAutoRotate();
    }

    glm::vec2 SRInterface::autoRotateVector() const
    {
      return autoRotateVector_;
    }

    void SRInterface::setAutoRotateVector(const glm::vec2& axis)
    {
      tryAutoRotate_ = false;
      // if (autoRotateVector_.x == axis.x && autoRotateVector_.y == axis.y)
      // {
      //   autoRotateVector_ = glm::vec2(0.0, 0.0);
      // }
      // else
      // {
        autoRotateVector_ = axis;
      // }
    }

    //Getters/Setters-------------------------------------------------------------------------------
    void SRInterface::setCameraDistance(const float distance) {mCamera->setDistance(distance);}
    float SRInterface::getCameraDistance() const {return mCamera->getDistance();}
    void SRInterface::setCameraLookAt(const glm::vec3& lookAt) {mCamera->setLookAt(lookAt);}
    glm::vec3 SRInterface::getCameraLookAt() const {return mCamera->getLookAt();}
    void SRInterface::setCameraRotation(const glm::quat& roation) {mCamera->setRotation(roation);}
    glm::quat SRInterface::getCameraRotation() const {return mCamera->getRotation();}
    void SRInterface::setAutoRotateSpeed(double speed) { autoRotateSpeed_ = speed; }
    void SRInterface::setZoomInverted(bool value) {mCamera->setZoomInverted(value);}
    void SRInterface::setLockZoom(bool lock)      {mCamera->setLockZoom(lock);}
    void SRInterface::setLockPanning(bool lock)   {mCamera->setLockPanning(lock);}
    void SRInterface::setLockRotation(bool lock)  {mCamera->setLockRotation(lock);}
    glm::mat4 SRInterface::getWorldToView() const       {return mCamera->getWorldToView(); }
    glm::mat4 SRInterface::getViewToProjection() const  {return mCamera->getViewToProjection(); }

    //----------------------------------------------------------------------------------------------
    //---------------- Widgets ---------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------

    void WidgetUpdateService::reset()
    {
      currentWidget_.reset();
      event_.reset();
    }

    class ScopedLambdaExecutor
    {
    public:
      explicit ScopedLambdaExecutor(std::function<void()> f) : func_(f) {}
      ~ScopedLambdaExecutor() { func_(); }
    private:
      std::function<void()> func_;
    };

    void SRInterface::doInitialWidgetUpdate(WidgetHandle widget, int x, int y)
    {
      widgetUpdater_.reset();
      widgetUpdater_.setCurrentWidget(widget);
      widgetUpdater_.doInitialUpdate(x, y, selectionDepth_);
    }

    void SRInterface::setWidgetInteractionMode(MouseButton btn)
    {
      widgetUpdater_.setButtonPushed(btn);
    }

    WidgetHandle SRInterface::select(int x, int y, const WidgetList& widgets)
    {
      if (!mContext || !mContext->isValid())
        return nullptr;
      // Ensure our rendering context is current on our thread.
      mContext->makeCurrent(mContext->surface());

      widgetUpdater_.reset();

      //get vbo ibo man
      std::weak_ptr<ren::VBOMan> vm = mCore.getStaticComponent<ren::StaticVBOMan>()->instance_;
      std::weak_ptr<ren::IBOMan> im = mCore.getStaticComponent<ren::StaticIBOMan>()->instance_;
      std::shared_ptr<ren::VBOMan> vboMan = vm.lock();
      std::shared_ptr<ren::IBOMan> iboMan = im.lock();
      if (!vboMan || !iboMan)
        return nullptr;

      //retrieve and bind fbo for selection
      std::weak_ptr<ren::FBOMan> fm = mCore.getStaticComponent<ren::StaticFBOMan>()->instance_;
      std::shared_ptr<ren::FBOMan> fboMan = fm.lock();
      if (!fboMan)
        return nullptr;
      widgetSelectFboId_ = fboMan->getOrCreateFBO(mCore, GL_TEXTURE_2D, screen_.width, screen_.height, 1, widgetSelectFboName);
      fboMan->bindFBO(*widgetSelectFboId_);

      //a map from selection id to name
      std::map<uint32_t, std::string> selMap;
      std::vector<uint64_t> entityList;

      //modify and add each object to draw
      for (auto& widget : widgets)
      {
        addSelectVertexBufferObjects(widget, vboMan);
        addSelectIndexBufferObjects(widget, iboMan);

        auto passInfo = addSelectPasses(widget);
        selMap.insert({ std::get<0>(passInfo), std::get<1>(passInfo) });
        auto newEntities = std::get<2>(passInfo);
        entityList.insert(entityList.end(), newEntities.begin(), newEntities.end());
      }

      updateCamera();
      updateWorldLight();

      mCore.executeWithoutAdvancingClock();

      {
        ScopedLambdaExecutor removeEntities([this, &entityList]() { for (auto& it : entityList) mCore.removeEntity(it); });
        {
          ScopedLambdaExecutor unbindFBOs([&fboMan]() { fboMan->unbindFBO(); });
          GLuint value;
          if (fboMan->readFBO(mCore, widgetSelectFboName, x, y, 1, 1, (GLvoid*)&value, (GLvoid*)&selectionDepth_))
          {
            auto it = selMap.find(value);
            if (it != selMap.end())
            {
              auto widgetId = it->second;

              for (auto& widget : widgets)
              {
                if (widget->uniqueID() == widgetId)
                {
                  widgetUpdater_.setCurrentWidget(widget);
                  break;
                }
              }
            }
          }
        }

        if (widgetUpdater_.currentWidget())
        {
          widgetUpdater_.doInitialUpdate(x, y, selectionDepth_);
        }
      }

      return widgetUpdater_.currentWidget();
    }

    std::tuple<uint32_t, std::string, std::vector<uint64_t>> SRInterface::addSelectPasses(
        WidgetHandle widget)
    {
      std::weak_ptr<ren::ShaderMan> sm = mCore.getStaticComponent<ren::StaticShaderMan>()->instance_;
      auto shaderMan = sm.lock();
      if (!shaderMan)
        return {};

      std::string objectName = widget->uniqueID();
      uint32_t selid = getSelectIDForName(objectName);
      glm::vec4 selCol = getVectorForID(selid);
      std::vector<uint64_t> entityIds;

      for (auto& pass : widget->passes())
      {
        uint64_t entityID = getEntityIDForName(pass.passName, 0);

        if (pass.renderType == RenderType::RENDER_VBO_IBO)
        {
          addVBOToEntity(entityID, pass.vboName);
          addIBOToEntity(entityID, pass.iboName);
        }

        const char* selectionShaderName = "Shaders/Selection";
        if (shaderMan->getIDForAsset(selectionShaderName) == 0)
        {
          const char* vs =
            "uniform mat4 uModelViewProjection;\n"
            "uniform vec4 uColor;\n"
            "uniform bool hack;\n"
            "attribute vec3 aPos;\n"
            "varying vec4 fColor;\n"
            "void main()\n"
            "{\n"
            "  gl_Position = uModelViewProjection * vec4(aPos, 1.0);\n"
            "  if(hack) gl_Position.xy = ((gl_Position.xy/gl_Position.w) * vec2(0.5) - vec2(0.5)) * gl_Position.w;\n"
            "  fColor = uColor;\n"
            "}\n";
          const char* fs =
            "#ifdef OPENGL_ES\n"
            "  #ifdef GL_FRAGMENT_PRECISION_HIGH\n"
            "    precision highp float;\n"
            "  #else\n"
            "    precision mediump float;\n"
            "  #endif\n"
            "#endif\n"
            "varying vec4 fColor;\n"
            "void main()\n"
            "{\n"
            "  gl_FragColor = fColor;\n"
            "}\n";

          shaderMan->addInMemoryVSFS(vs, fs, selectionShaderName);
        }
        addShaderToEntity(entityID, selectionShaderName);

        // Add transformation
        gen::Transform trafo;
        mCore.addComponent(entityID, trafo);

        // Add SCIRun render state.
        SRRenderState state;
        state.state = pass.renderState;
        mCore.addComponent(entityID, state);
        RenderBasicGeom geom;
        mCore.addComponent(entityID, geom);
        ren::CommonUniforms commonUniforms;
        mCore.addComponent(entityID, commonUniforms);

        applyUniform(entityID, SpireSubPass::Uniform("uColor", selCol));
        applyUniform(entityID, SpireSubPass::Uniform("hack", Preferences::Instance().widgetSelectionCorrection));

        // Add components associated with entity. We just need a base class which
        // we can pass in an entity ID, then a derived class which bundles
        // all associated components (including types) together. We can use
        // a variadic template for this. This will allow us to place any components
        // we want on the objects in question in show field. This could lead to
        // much simpler customization.

        pass.renderState.mSortType = mRenderSortType;
        pass.renderState.set(RenderState::ActionFlags::USE_BLEND, false);
        mCore.addComponent(entityID, pass);
        entityIds.push_back(entityID);
      }
      return std::make_tuple(selid, objectName, entityIds);
    }

    void SRInterface::addSelectVertexBufferObjects(WidgetHandle widget, std::shared_ptr<ren::VBOMan> vboMan)
    {
      for (const auto& vbo : widget->vbos())
      {
        if (vbo.onGPU)
        {
          // Generate vector of attributes to pass into the entity system.
          std::vector<std::tuple<std::string, size_t, bool>> attributeData;
          for (const auto& attribData : vbo.attributes)
          {
            attributeData.push_back(std::make_tuple(attribData.name, attribData.sizeInBytes, attribData.normalize));
          }

          vboMan->addInMemoryVBO(vbo.data->getBuffer(), vbo.data->getBufferSize(), attributeData, vbo.name);
        }
      }
    }

    void SRInterface::addSelectIndexBufferObjects(WidgetHandle widget, std::shared_ptr<ren::IBOMan> iboMan)
    {
      for (const auto& ibo : widget->ibos())
      {
        auto primType = computePrimitiveType(ibo.indexSize);
        auto primitive = computePrimitive(ibo);

        int numPrimitives = ibo.data->getBufferSize() / ibo.indexSize;
        iboMan->addInMemoryIBO(ibo.data->getBuffer(), ibo.data->getBufferSize(), primitive, primType, numPrimitives, ibo.name);
      }
    }

    GLenum SRInterface::computePrimitiveType(size_t indexSize)
    {
      auto primType = GL_UNSIGNED_SHORT;
      switch (indexSize)
      {
      case 1: // 8-bit
        primType = GL_UNSIGNED_BYTE;
        break;

      case 2: // 16-bit
        primType = GL_UNSIGNED_SHORT;
        break;

      case 4: // 32-bit
        primType = GL_UNSIGNED_INT;
        break;

      default:
        throw std::invalid_argument("Unable to determine index buffer depth.");
      }
      return primType;
    }

    GLenum SRInterface::computePrimitive(const SpireIBO& ibo)
    {
      auto primitive = GL_TRIANGLES;
      switch (ibo.prim)
      {
      case SpireIBO::PRIMITIVE::POINTS:
        primitive = GL_POINTS;
        break;

      case SpireIBO::PRIMITIVE::LINES:
        primitive = GL_LINES;
        break;

      case SpireIBO::PRIMITIVE::TRIANGLES:
        primitive = GL_TRIANGLES;
        break;

      case SpireIBO::PRIMITIVE::QUADS:
        primitive = GL_QUADS;
        break;
      }
      return primitive;
    }

    glm::mat4 SRInterface::getStaticCameraViewProjection()
    {
      // TODO: move this matrix to SRCamera?
      auto cam = mCore.getStaticComponent<gen::StaticCamera>();
      return cam->data.viewProjection;
    }

    glm::mat4 SRInterface::getWorldToProjection() const
    {
      return mCamera->getWorldToProjection();
    }

//----------------------------------------------------------------------------------------------
uint32_t SRInterface::getSelectIDForName(const std::string& name)
{
  return static_cast<uint32_t>(std::hash<std::string>()(name));
}

glm::vec4 SRInterface::getVectorForID(const uint32_t id)
{
  float a = ((id >> 24) & 0xff) / 255.0f;
  float b = ((id >> 16) & 0xff) / 255.0f;
  float g = ((id >> 8)  & 0xff) / 255.0f;
  float r = ((id)       & 0xff) / 255.0f;
  return glm::vec4(r, g, b, a);
}

uint32_t SRInterface::getIDForVector(const glm::vec4& vec)
{
  uint32_t r = (uint32_t)(vec.r*255.0) & 0xff;
  uint32_t g = (uint32_t)(vec.g*255.0) & 0xff;
  uint32_t b = (uint32_t)(vec.b*255.0) & 0xff;
  uint32_t a = (uint32_t)(vec.a*255.0) & 0xff;
  return (a << 24) | (b << 16) | (g << 8) | (r);
}


void SRInterface::modifyObject(const std::string& id, const gen::Transform& trans)
{
  auto contTrans = mCore.getOrCreateComponentContainer<gen::Transform>();

  auto component = contTrans->getComponent(mEntityIdMap[id]);
  if (component.first != nullptr)
    contTrans->modifyIndex(trans, component.second, 0);
}

glm::vec2 ScreenParams::positionFromClick(int x, int y) const
{
  return glm::vec2(float(x) / float(width) * 2.0 - 1.0,
               -(float(y) / float(height) * 2.0 - 1.0));
}

//---------------- Clipping Planes -------------------------------------------------------------

    StaticClippingPlanes* SRInterface::getClippingPlanes()
    {
      return static_cast<StaticClippingPlanes*>(mCore.getStaticComponent<StaticClippingPlanes>());
    }

    double SRInterface::getMaxProjLength(const glm::vec3 &n)
    {
      static const glm::vec3 a1(-1.0, 1.0, -1.0);
      static const glm::vec3 a2(-1.0, 1.0, 1.0);
      static const glm::vec3 a3(1.0, 1.0, -1.0);
      static const glm::vec3 a4(1.0, 1.0, 1.0);
      return std::max(
        std::max(
        std::abs(dot(n, a1)),
        std::abs(dot(n, a2))),
        std::max(
        std::abs(dot(n, a3)),
        std::abs(dot(n, a4))));
    }

    bool SRInterface::updateClippingPlanes()
    {
      auto* clippingPlanes = mCore.getStaticComponent<StaticClippingPlanes>();
      if (!clippingPlanes || !sceneBBox_.valid())
        return false;

      clippingPlanes->clippingPlanes.clear();
      clippingPlanes->clippingPlaneCtrls.clear();
      //boundbox transformation
      glm::mat4 trans_bb = glm::mat4(1.0f);
      glm::vec3 scale_bb(sceneBBox_.x_length() / 2.0, sceneBBox_.y_length() / 2.0, sceneBBox_.z_length() / 2.0);
      glm::vec3 center_bb(sceneBBox_.center().x(), sceneBBox_.center().y(), sceneBBox_.center().z());
      glm::mat4 temp = scale(glm::mat4(1.0f), scale_bb);
      trans_bb = temp * trans_bb;
      temp = translate(glm::mat4(1.0f), center_bb);
      trans_bb = temp * trans_bb;
      int index = 0;
      for (const auto& plane : clippingPlaneManager_->allPlanes())
      {
        glm::vec3 n3(plane.x, plane.y, plane.z);
        float d = plane.d;
        glm::vec4 n(0.0);
        if (length(n3) > 0.0)
        {
          n3 = normalize(n3);
          n = glm::vec4(n3, 0.0);
          d *= getMaxProjLength(n3);
        }
        auto o = glm::vec4(n.x, n.y, n.z, 1.0) * d;
        o.w = 1;
        o = trans_bb * o;
        n = inverseTranspose(trans_bb) * n;
        o.w = 0;
        n.w = 0;
        n = normalize(n);
        n.w = -dot(o, n);
        clippingPlanes->clippingPlanes.push_back(n);
        glm::vec4 control(plane.visible ? 1.0 : 0.0,
            plane.showFrame ? 1.0 : 0.0,
            plane.reverseNormal ? 1.0 : 0.0, 0.0);
        clippingPlanes->clippingPlaneCtrls.push_back(control);
        index++;
      }
      return true;
    }



    //----------------------------------------------------------------------------------------------
    //---------------- Data Handling --------------------------------------------------------------
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    void SRInterface::handleGeomObject(GeometryHandle obj, int port)
    {
      //logRendererInfo("Handling geom object on port {}", port);
      RENDERER_LOG_FUNCTION_SCOPE;
      RENDERER_LOG("Ensure our rendering context is current on our thread.");
      //DEBUG_LOG_LINE_INFO

      std::string objectName = obj->uniqueID();
      BBox bbox; // Bounding box containing all vertex buffer objects.

      if(!mContext || !mContext->isValid()) return;
      mContext->makeCurrent(mContext->surface());

      RENDERER_LOG("Check to see if the object already exists in our list. "
        "If so, then remove the object. We will re-add it.");
      auto foundObject = std::find_if(
        mSRObjects.begin(), mSRObjects.end(),
        [&objectName](const SRObject& sro)
      {
        return (sro.mName == objectName);
      });

      //DEBUG_LOG_LINE_INFO

      auto vmc = mCore.getStaticComponent<ren::StaticVBOMan>();
      auto imc = mCore.getStaticComponent<ren::StaticIBOMan>();
      if (!vmc || !imc) return;

      if (std::shared_ptr<ren::VBOMan> vboMan = vmc->instance_)
      {
        //DEBUG_LOG_LINE_INFO;
        if (std::shared_ptr<ren::IBOMan> iboMan = imc->instance_)
        {
          //DEBUG_LOG_LINE_INFO
          if (foundObject != mSRObjects.end())
          {
            //DEBUG_LOG_LINE_INFO

            RENDERER_LOG("Iterate through each of the passes and remove their associated entity ID.");
            for (const auto& pass : foundObject->mPasses)
            {
              uint64_t entityID = getEntityIDForName(pass.passName, port);
              mEntityIdMap.erase(objectName);
              mCore.removeEntity(entityID);
            }

            RENDERER_LOG("We need to renormalize the core after removing entities. We don't need "
              "to run a new pass however. Renormalization is enough to remove "
              "old entities from the system.");
            mCore.renormalize(true);

            RENDERER_LOG("Run a garbage collection cycle for the VBOs and IBOs. We will likely"
              " be using similar VBO and IBO names.");
            vboMan->runGCCycle(mCore);
            iboMan->runGCCycle(mCore);

            RENDERER_LOG("Remove the object from the entity system.");
            mSRObjects.erase(foundObject);
          }

          //DEBUG_LOG_LINE_INFO
          RENDERER_LOG("Add vertex buffer objects.");
          std::vector<char*> vbo_buffer;
          std::vector<size_t> stride_vbo;

          int nameIndex = 0;
          for (auto it = obj->vbos().cbegin(); it != obj->vbos().cend(); ++it, ++nameIndex)
          {
            const auto& vbo = *it;

            if (vbo.onGPU)
            {
              RENDERER_LOG("Generate vector of attributes to pass into the entity system: {}, {}", nameIndex, vbo.name);
              std::vector<std::tuple<std::string, size_t, bool >> attributeData;
              for (const auto& attribData : vbo.attributes)
              {
                attributeData.push_back(std::make_tuple(attribData.name, attribData.sizeInBytes, attribData.normalize));
              }

              vboMan->addInMemoryVBO(vbo.data->getBuffer(), vbo.data->getBufferSize(), attributeData, vbo.name);
            }

            vbo_buffer.push_back(reinterpret_cast<char*>(vbo.data->getBuffer()));
            size_t stride = 0;
            for (auto a : vbo.attributes)
              stride += a.sizeInBytes;
            stride_vbo.push_back(stride);

            bbox.extend(vbo.boundingBox);
          }

          //DEBUG_LOG_LINE_INFO
          RENDERER_LOG("Add index buffer objects.");
          nameIndex = 0;
          for (auto it = obj->ibos().cbegin(); it != obj->ibos().cend(); ++it, ++nameIndex)
          {
            const auto& ibo = *it;
            GLenum primType = GL_UNSIGNED_SHORT;
            switch (ibo.indexSize)
            {
            case 1: // 8-bit
              primType = GL_UNSIGNED_BYTE;
              break;

            case 2: // 16-bit
              primType = GL_UNSIGNED_SHORT;
              break;

            case 4: // 32-bit
              primType = GL_UNSIGNED_INT;
              break;

            default:
              primType = GL_UNSIGNED_INT;
              logRendererError("Unable to determine index buffer depth.");
              throw std::invalid_argument("Unable to determine index buffer depth.");
            }

            GLenum primitive = GL_TRIANGLES;
            switch (ibo.prim)
            {
              case SpireIBO::PRIMITIVE::POINTS:
                primitive = GL_POINTS;
                break;

              case SpireIBO::PRIMITIVE::LINES:
                primitive = GL_LINES;
                break;

              case SpireIBO::PRIMITIVE::TRIANGLES:
                primitive = GL_TRIANGLES;
                break;

              case SpireIBO::PRIMITIVE::QUADS:
                primitive = GL_QUADS;
                break;
            }

            if (mRenderSortType == RenderState::TransparencySortType::LISTS_SORT)
            {
              RENDERER_LOG("Create sorted lists of Buffers for transparency in each direction of the axis.");
              uint32_t* ibo_buffer = reinterpret_cast<uint32_t*>(ibo.data->getBuffer());
              size_t num_triangles = ibo.data->getBufferSize() / (sizeof(uint32_t) * 3);
              Vector dir(0.0, 0.0, 0.0);

              std::vector<DepthIndex> rel_depth(num_triangles);
              for (int i = 0; i <= 6; ++i)
              {
                std::string name = ibo.name;
                if (i == 0)
                {
                  int numPrimitives = ibo.data->getBufferSize() / ibo.indexSize;
                  iboMan->addInMemoryIBO(ibo.data->getBuffer(),
                    ibo.data->getBufferSize(), primitive, primType,
                    numPrimitives, ibo.name);
                }
                if (i == 1)
                {
                  dir = Vector(1.0, 0.0, 0.0);
                  name += "X";
                }
                if (i == 2)
                {
                  dir = Vector(0.0, 1.0, 0.0);
                  name += "Y";
                }
                if (i == 3)
                {
                  dir = Vector(0.0, 0.0, 1.0);
                  name += "Z";
                }
                if (i == 4)
                {
                  dir = Vector(-1.0, 0.0, 0.0);
                  name += "NegX";
                }
                if (i == 5)
                {
                  dir = Vector(0.0, -1.0, 0.0);
                  name += "NegY";
                }
                if (i == 6)
                {
                  dir = Vector(0.0, 0.0, -1.0);
                  name += "NegZ";
                }
                if (i > 0)
                {
                  for (size_t j = 0; j < num_triangles; j++)
                  {
                    float* vertex1 = reinterpret_cast<float*>(vbo_buffer[nameIndex] + stride_vbo[nameIndex] * (ibo_buffer[j * 3]));
                    Point node1(vertex1[0], vertex1[1], vertex1[2]);

                    float* vertex2 = reinterpret_cast<float*>(vbo_buffer[nameIndex] + stride_vbo[nameIndex] * (ibo_buffer[j * 3 + 1]));
                    Point node2(vertex2[0], vertex2[1], vertex2[2]);

                    float* vertex3 = reinterpret_cast<float*>(vbo_buffer[nameIndex] + stride_vbo[nameIndex] * (ibo_buffer[j * 3 + 2]));
                    Point node3(vertex3[0], vertex3[1], vertex3[2]);

                    rel_depth[j].mDepth = Dot(dir, node1) + Dot(dir, node2) + Dot(dir, node3);
                    rel_depth[j].mIndex = j;
                  }

                  std::sort(rel_depth.begin(), rel_depth.end());

                  int numPrimitives = ibo.data->getBufferSize() / ibo.indexSize;

                  std::vector<char> sorted_buffer(ibo.data->getBufferSize());
                  char* ibuffer = reinterpret_cast<char*>(ibo.data->getBuffer());
                  char* sbuffer = !sorted_buffer.empty() ? reinterpret_cast<char*>(&sorted_buffer[0]) : nullptr;

                  if (sbuffer && num_triangles > 0)
                  {
                    size_t tri_size = ibo.data->getBufferSize() / num_triangles;
                    for (size_t j = 0; j < num_triangles; j++)
                    {
                      memcpy(sbuffer + j * tri_size, ibuffer + rel_depth[j].mIndex * tri_size, tri_size);
                    }
                    iboMan->addInMemoryIBO(sbuffer, ibo.data->getBufferSize(), primitive, primType, numPrimitives, name);
                  }
                }
              }
            }
            else
            {
              int numPrimitives = ibo.data->getBufferSize() / ibo.indexSize;
              iboMan->addInMemoryIBO(ibo.data->getBuffer(), ibo.data->getBufferSize(), primitive, primType, numPrimitives, ibo.name);
            }
          }

          RENDERER_LOG("Add default identity transform to the object globally (instead of per-pass)");
          mSRObjects.push_back(SRObject(objectName, bbox, obj->colorMap(), port));
          SRObject& elem = mSRObjects.back();

          std::weak_ptr<ren::ShaderMan> sm = mCore.getStaticComponent<ren::StaticShaderMan>()->instance_;
          if (auto shaderMan = sm.lock())
          {
            RENDERER_LOG("Recalculate scene bounding box. Should only be done when an object is added.");
            sceneBBox_.reset();
            for (auto it = mSRObjects.begin(); it != mSRObjects.end(); ++it)
            {
              if (it->mBBox.valid())
              {
                sceneBBox_.extend(it->mBBox);
              }
            }

            RENDERER_LOG("Add passes");
            for (auto& pass : obj->passes())
            {
              uint64_t entityID = getEntityIDForName(pass.passName, port);

              if (pass.renderType == RenderType::RENDER_VBO_IBO)
              {
                addVBOToEntity(entityID, pass.vboName);
                if (mRenderSortType == RenderState::TransparencySortType::LISTS_SORT)
                {
                  for (int i = 0; i <= 6; ++i)
                  {
                    std::string name = pass.iboName;
                    if (i == 1)
                      name += "X";
                    if (i == 2)
                      name += "Y";
                    if (i == 3)
                      name += "Z";
                    if (i == 4)
                      name += "NegX";
                    if (i == 5)
                      name += "NegY";
                    if (i == 6)
                      name += "NegZ";

                    addIBOToEntity(entityID, name);
                  }
                }
                else
                {
                  addIBOToEntity(entityID, pass.iboName);
                }
                RENDERER_LOG("add texture");
                addTextToEntity(entityID, pass.text);
                addTextureToEntity(entityID, pass.texture);
              }

              RENDERER_LOG("Load vertex and fragment shader will use an already loaded program.");
              shaderMan->loadVertexAndFragmentShader(mCore, entityID, pass.programName);

              RENDERER_LOG("Add transformation");
              gen::Transform trafo;

              if (widgetUpdater_.currentWidget() && objectName == widgetUpdater_.currentWidget()->uniqueID())
              {
                mSelectedID = entityID;
              }
              mEntityIdMap.insert(std::make_pair(objectName, entityID));

              mCore.addComponent(entityID, trafo);

              RENDERER_LOG("Add lighting uniform checks");
              LightingUniforms lightUniforms;
              mCore.addComponent(entityID, lightUniforms);
              RENDERER_LOG("plane uniforms");
              ClippingPlaneUniforms clipplingPlaneUniforms;
              mCore.addComponent(entityID, clipplingPlaneUniforms);

              RENDERER_LOG("Add SCIRun render state.");
              SRRenderState state;
              state.state = pass.renderState;
              mCore.addComponent(entityID, state);
              RenderBasicGeom geom;
              mCore.addComponent(entityID, geom);
              RENDERER_LOG("Ensure common uniforms are covered.");
              ren::CommonUniforms commonUniforms;
              mCore.addComponent(entityID, commonUniforms);

              for (auto& uniform : pass.mUniforms)
              {
                applyMatFactors(uniform);
                applyUniform(entityID, uniform);
              }

              {
                SpireSubPass::Uniform uniform;
                uniform.name = "uFogSettings";
                applyFog(uniform);
                applyUniform(entityID, uniform);
                uniform.name = "uFogColor";
                applyFog(uniform);
                applyUniform(entityID, uniform);
              }

              // Add components associated with entity. We just need a base class which
              // we can pass in an entity ID, then a derived class which bundles
              // all associated components (including types) together. We can use
              // a variadic template for this. This will allow us to place any components
              // we want on the objects in question in show field. This could lead to
              // much simpler customization.

              RENDERER_LOG("Add a pass to our local object.");
              elem.mPasses.emplace_back(pass.passName, pass.renderType);
              pass.renderState.mSortType = mRenderSortType;
              mCore.addComponent(entityID, pass);
            }
          }
          mCamera->setSceneBoundingBox(sceneBBox_);
        }
      }
      //DEBUG_LOG_LINE_INFO
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::removeAllGeomObjects()
    {
      for (auto it = mSRObjects.begin(); it != mSRObjects.end(); ++it)
      {
        // Iterate through each of the passes and remove their associated
        // entity ID.
        for (const auto& pass : it->mPasses)
        {
          mCore.removeEntity(getEntityIDForName(pass.passName, it->mPort));
        }
      }
      mEntityIdMap.clear();

      mCore.renormalize(true);
      mSRObjects.clear();
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::gcInvalidObjects(const std::vector<std::string>& validObjects)
    {
      mEntityIdMap.clear();
      for (auto it = mSRObjects.begin(); it != mSRObjects.end();)
      {
        if (std::find(validObjects.begin(), validObjects.end(), it->mName) == validObjects.end())
        {
          for (const auto& pass : it->mPasses)
          {
            mCore.removeEntity(getEntityIDForName(pass.passName, it->mPort));
          }
          it = mSRObjects.erase(it);
        }
        else
        {
          for (const auto& pass : it->mPasses)
          {
            uint64_t entityID = getEntityIDForName(pass.passName, it->mPort);
            mEntityIdMap.insert(std::make_pair(it->mName, entityID));
          }
          ++it;
        }
      }

      mCore.renormalize(true);
    }

    //----------------------------------------------------------------------------------------------
    bool SRInterface::hasObject(const std::string& object)
    {
      for (const auto& mSRObject : mSRObjects)
        if (mSRObject.mName == object)
          return true;

      return false;
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::addVBOToEntity(uint64_t entityID, const std::string& vboName)
    {
      std::weak_ptr<ren::VBOMan> vm = mCore.getStaticComponent<ren::StaticVBOMan>()->instance_;
      if (std::shared_ptr<ren::VBOMan> vboMan = vm.lock()) {
        ren::VBO vbo;
        vbo.glid = vboMan->hasVBO(vboName);
        mCore.addComponent(entityID, vbo);
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::addIBOToEntity(uint64_t entityID, const std::string& iboName)
    {
      const std::weak_ptr<ren::IBOMan> im = mCore.getStaticComponent<ren::StaticIBOMan>()->instance_;
      if (const auto iboMan = im.lock()) {
        ren::IBO ibo;
        const auto iboData = iboMan->getIBOData(iboName);
        ibo.glid = iboMan->hasIBO(iboName);
        ibo.primType = iboData.primType;
        ibo.primMode = iboData.primMode;
        ibo.numPrims = iboData.numPrims;

        mCore.addComponent(entityID, ibo);
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::addTextToEntity(uint64_t entityID, const SpireText& text)
    {
      if (text.name.empty()) return;
      std::weak_ptr<ren::TextureMan> tm = mCore.getStaticComponent<ren::StaticTextureMan>()->instance_;
      std::shared_ptr<ren::TextureMan> textureMan = tm.lock();
      if (!textureMan) return;

      std::stringstream ss;
      ss << "FontTexture:" << entityID << text.name << text.width << text.height;
      const std::string assetName = ss.str();

      ren::Texture texture;

      spire::CerealHeap<ren::Texture>* contTex = mCore.getOrCreateComponentContainer<ren::Texture>();
      std::pair<const ren::Texture*, size_t> component = contTex->getComponent(entityID);

      if (!component.first)
        texture = textureMan->createTexture(assetName, text.width, text.height, text.bitmap);
      else
        texture = *component.first;

      texture.textureUnit = 0;
      texture.setUniformName("uTX0");
      mCore.addComponent(entityID, texture);
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::addTextureToEntity(uint64_t entityID, const SpireTexture2D& texture)
    {
      if (texture.name.empty()) return;
      std::weak_ptr<ren::TextureMan> tm = mCore.getStaticComponent<ren::StaticTextureMan>()->instance_;
      std::shared_ptr<ren::TextureMan> textureMan = tm.lock();
      if (!textureMan) return;

      std::stringstream ss;
      ss << "Texture:" << entityID << texture.name << texture.width << texture.height;
      const std::string assetName = ss.str();

      ren::Texture renTexture;
      const auto contTex = mCore.getOrCreateComponentContainer<ren::Texture>();
      const auto component = contTex->getComponent(entityID);

      if (!component.first)
        renTexture = textureMan->createTexture(assetName, GL_RGBA, texture.width, texture.height,
          GL_RGBA,  GL_UNSIGNED_BYTE, texture.bitmap);
      else
        renTexture = *component.first;

      renTexture.textureUnit = 0;
      renTexture.setUniformName("uTX0");
      mCore.addComponent(entityID, renTexture);
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::addShaderToEntity(uint64_t entityID, const std::string& shaderName)
    {
      std::weak_ptr<ren::ShaderMan> sm = mCore.getStaticComponent<ren::StaticShaderMan>()->instance_;
      if (std::shared_ptr<ren::ShaderMan> shaderMan = sm.lock())
      {
        const ren::Shader shader{ shaderMan->getIDForAsset(shaderName.c_str()) };
        mCore.addComponent(entityID, shader);
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::generateTextures()
    {
      //font texture
      //read in the font data
      bool success = true;
      const auto fontPath = Application::Instance().executablePath() / "Assets" / "times_new_roman.font";
      std::ifstream in(fontPath.string(), std::ifstream::binary);
      if (in.fail())
      {
        //try the MAC App location if the UNIX/Windows location didn't work.
        in.open("SCIRun.app/Contents/MacOS/Assets/times_new_roman.font");
        if (in.fail())
        {
          std::cerr << "Cannot find font \"Assets/times_new_roman.font\"" << std::endl;
          success = false;
          in.close();
        }
      }
      if (success)
      {
        size_t w, h;
        in >> w >> h;
        char temp;
        in.read(reinterpret_cast<char*>(&temp), sizeof(char));
        std::vector<uint16_t> fontData(w*h);
        in.read(reinterpret_cast<char*>(&fontData[0]), sizeof(uint16_t)*w*h);
        in.close();
        std::vector<char> font(w * h * 4);
        for (size_t i = 0; i < w*h; i++)
        {
          uint16_t pixel = fontData[i];
          font[i * 4] = (pixel & 0x00ff);
          font[i * 4 + 1] = (pixel & 0x00ff);
          font[i * 4 + 2] = (pixel & 0x00ff);
          font[i * 4 + 3] = (pixel >> 8);
        }

        // Build font texture
        GL(glActiveTexture(GL_TEXTURE0));
        GL(glGenTextures(1, &mFontTexture));
        GL(glBindTexture(GL_TEXTURE_2D, mFontTexture));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GL(glPixelStorei(GL_UNPACK_ROW_LENGTH, w));
        GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        GL(glTexImage2D(GL_TEXTURE_2D, 0,
          GL_RGBA,
          GLsizei(w), GLsizei(h), 0,
          GL_RGBA,
          GL_UNSIGNED_BYTE, (GLvoid*)&font[0]));

      }
    }

    uint64_t SRInterface::getEntityIDForName(const std::string& name, int port)
    {
      return (static_cast<uint64_t>(std::hash<std::string>()(name)) >> 8) + (static_cast<uint64_t>(port) << 56);
    }

    //----------------------------------------------------------------------------------------------
    //---------------- Rendering -------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------

    void SRInterface::doFrame(double constantDeltaTime)
    {
      // todo Only render a frame if something has changed (new or deleted
      // objects, or the view point has changed).
      applyAutoRotation();
      updateCamera();
      updateWorldLight();

      mCore.execute(constantDeltaTime);

      // Do not even attempt to render if the framebuffer is not complete. This
      // can happen when the rendering window is hidden (in SCIRun5 for example);
      if (showOrientation_ && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
        renderCoordinateAxes();
    }

    void SRInterface::renderCoordinateAxes()
    {
      // Only execute if static rendering resources are available. All of these
      // resource checks wouldn't be necessary if we were operating in the perview
      // of the entity system.
      if (mCore.getStaticComponent<ren::StaticVBOMan>() == nullptr) return;

      // This rendering algorithm is fairly inefficient. Use the entity component
      // system to optimize the rendering of a large amount of objects.
      std::weak_ptr<ren::VBOMan> vm = mCore.getStaticComponent<ren::StaticVBOMan>()->instance_;
      std::weak_ptr<ren::IBOMan> im = mCore.getStaticComponent<ren::StaticIBOMan>()->instance_;
      std::weak_ptr<ren::ShaderMan> sm = mCore.getStaticComponent<ren::StaticShaderMan>()->instance_;
      if (std::shared_ptr<ren::VBOMan> vboMan = vm.lock())
      {
        if (std::shared_ptr<ren::IBOMan> iboMan = im.lock())
        {
          if (std::shared_ptr<ren::ShaderMan> shaderMan = sm.lock())
          {
            GLuint arrowVBO = vboMan->hasVBO("Assets/arrow.geom");
            GLuint arrowIBO = iboMan->hasIBO("Assets/arrow.geom");
            GLuint shader = shaderMan->getIDForAsset("Shaders/OrientationGlyph");

            // Bail if assets have not been loaded yet (asynchronous loading may take a
            // few frames).
            if (arrowVBO == 0 || arrowIBO == 0 || shader == 0)
            {
              axesFailCount_++;
              if (axesFailCount_ > frameInitLimit_)
                throw FatalRendererError("Failed to initialize axes after many attempts. ViewScene is unusable. Halting renderer loop.");
              return;
            }

            const ren::IBOMan::IBOData* iboData;
            try
            {
              iboData = &iboMan->getIBOData("Assets/arrow.geom");
            }
            catch (...)
            {
              // Return if IBO data not available.
              return;
            }

            // Ensure shader attributes are setup appropriately.
            mArrowAttribs.setup(arrowVBO, shader, *vboMan);

            GL(glUseProgram(shader));

            GL(glBindBuffer(GL_ARRAY_BUFFER, arrowVBO));
            GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, arrowIBO));

            bool depthMask = glIsEnabled(GL_DEPTH_WRITEMASK);
            bool cullFace = glIsEnabled(GL_CULL_FACE);
            bool blend = glIsEnabled(GL_BLEND);

            GL(glDepthMask(GL_TRUE));
            GL(glDisable(GL_CULL_FACE));
            GL(glDisable(GL_BLEND));
            glClear(GL_DEPTH_BUFFER_BIT);

            // Note that we can pull aspect ratio from the screen dimensions static
            // variable.
            gen::StaticScreenDims* dims = mCore.getStaticComponent<gen::StaticScreenDims>();
            float aspect = static_cast<float>(dims->width) / static_cast<float>(dims->height);

            // Project onto a orthographic plane with respect to aspect ratio
            glm::mat4 projection = glm::ortho(-aspect/2, aspect/2, -0.5f, 0.5f, 0.0f, 2.0f);

            // Build world transform for all axes. Rotates about uninverted camera's
            // view, then translates to a specified corner on the screen.
            glm::mat4 axesRot = mCamera->getWorldToView();
            axesRot[3][0] = 0.0f;
            axesRot[3][1] = 0.0f;
            axesRot[3][2] = 0.0f;

            // Remap x and y position(-0.5 to 0.5) so the edge doesn't pass the margin, regardless of size
            float margin = orientSize / 10.0f;
            float xLow2 = -aspect/2 + margin;
            float xHigh2 = aspect/2 - margin;
            float yLow2 = -0.5 + margin;
            float yHigh2 = 0.5 - margin;
            float xPos, yPos;

            // If the scale is larger than the width, the scale centers at 0
            if(xLow2 > 0 && xHigh2 < 0)
            {
                xPos = 0;
            }
            else
            {
                xPos = xLow2 + (orientPosX + 0.5f) * (xHigh2 - xLow2);
            }
            yPos = yLow2 + (orientPosY + 0.5f) * (yHigh2 - yLow2);

            glm::mat4 invCamTrans = translate(glm::mat4(1.0f), glm::vec3(xPos, yPos, -1.5f));
            glm::mat4 axesScale = scale(glm::mat4(1.0f), glm::vec3(orientSize));
            glm::mat4 axesTransform = axesScale * axesRot;

            GLint locCamViewVec = glGetUniformLocation(shader, "uCamViewVec");
            GLint locLightDirecionView = glGetUniformLocation(shader, "uLightDirectionView");
            GLint locDiffuseColor = glGetUniformLocation(shader, "uColor");
            GLint locProjIVObject = glGetUniformLocation(shader, "uModelViewProjection");
            GLint locObject = glGetUniformLocation(shader, "uModel");

            GL(glUniform3f(locCamViewVec, 0.0f, 0.0f, -1.0f));
            GL(glUniform3f(locLightDirecionView, 0.0f, 0.0f, -1.0f));

            // Build projection for the axes to use on the screen. The arrors will not
            // use the camera, but will use the camera's transformation matrix.

            mArrowAttribs.bind();

            // X Axis (dark)
            {
              glm::mat4 xform = rotate(glm::mat4(1.0f), glm::pi<float>() / 2.0f, glm::vec3(0.0, 1.0, 0.0));
              glm::mat4 finalTrafo = axesTransform * xform;

              GL(glUniform4f(locDiffuseColor, 0.25f, 0.0f, 0.0f, 1.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, nullptr));
            }

            // X Axis
            {
              glm::mat4 xform = rotate(glm::mat4(1.0f), -glm::pi<float>() / 2.0f, glm::vec3(0.0, 1.0, 0.0));
              glm::mat4 finalTrafo = axesTransform * xform;

              GL(glUniform4f(locDiffuseColor, 1.0f, 0.0f, 0.0f, 1.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, nullptr));
            }

            // Y Axis (dark)
            {
              glm::mat4 xform = rotate(glm::mat4(1.0f), -glm::pi<float>() / 2.0f, glm::vec3(1.0, 0.0, 0.0));
              glm::mat4 finalTrafo = axesTransform * xform;

              GL(glUniform4f(locDiffuseColor, 0.0f, 0.25f, 0.0f, 1.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, nullptr));
            }

            // Y Axis
            {
              glm::mat4 xform = rotate(glm::mat4(1.0f), glm::pi<float>() / 2.0f, glm::vec3(1.0, 0.0, 0.0));
              glm::mat4 finalTrafo = axesTransform * xform;

              GL(glUniform4f(locDiffuseColor, 0.0f, 1.0f, 0.0f, 1.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, nullptr));
            }

            // Z Axis (dark)
            {
              // No rotation at all
              glm::mat4 finalTrafo = axesTransform;

              GL(glUniform4f(locDiffuseColor, 0.0f, 0.0f, 0.25f, 1.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, nullptr));
            }

            // Z Axis
            {
              // No rotation at all
              glm::mat4 xform = rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(1.0, 0.0, 0.0));
              glm::mat4 finalTrafo = axesTransform * xform;

              GL(glUniform4f(locDiffuseColor, 0.0f, 0.0f, 1.0f, 1.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, nullptr));
            }

            mArrowAttribs.unbind();

            if (!depthMask)
            {
              GL(glDepthMask(GL_FALSE));
            }
            if (cullFace)
            {
              GL(glEnable(GL_CULL_FACE));
            }
            if (blend)
            {
              GL(glEnable(GL_BLEND));
            }
          }
        }
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::updateWorldLight()
    {
      auto light = mCore.getStaticComponent<StaticWorldLight>();

      if (light)
        for (int i = 0; i < LIGHT_NUM; ++i)
          light->lightDir[i] = mLightsOn[i] ? mLightDirectionView[i] : glm::vec3(0.0, 0.0, 0.0);
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::applyUniform(uint64_t entityID, const SpireSubPass::Uniform& uniform)
    {
      switch (uniform.type)
      {
      case SpireSubPass::Uniform::UniformType::UNIFORM_SCALAR:
        ren::addGLUniform(mCore, entityID, uniform.name.c_str(), static_cast<float>(uniform.data.x));
        break;

      case SpireSubPass::Uniform::UniformType::UNIFORM_VEC4:
        ren::addGLUniform(mCore, entityID, uniform.name.c_str(), uniform.data);
        break;
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::applyMatFactors(SpireSubPass::Uniform& uniform)
    {
      if (uniform.name == "uAmbientColor")
        uniform.data = inverseGammaCorrect(glm::vec4(mMatAmbient));
      else if (uniform.name == "uSpecularColor")
        uniform.data = inverseGammaCorrect(glm::vec4(mMatSpecular));
      else if (uniform.name == "uSpecularPower")
        uniform.data = glm::vec4(mMatShine);
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::applyFog(SpireSubPass::Uniform& uniform)
    {
      if (uniform.name == "uFogSettings")
      {
        float radius = sceneBBox_.diagonal().length() * 2.0;
        float start = radius * mFogStart;
        float end = radius * mFogEnd;
        uniform.data = glm::vec4(mFogIntensity, start, end, 0.0);
      }
      else if (uniform.name == "uFogColor")
      {
        uniform.data = inverseGammaCorrect(mFogColor);
      }

      uniform.type = SpireSubPass::Uniform::UniformType::UNIFORM_VEC4;
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::setLightColor(int index, float r, float g, float b)
    {
      if (index >= LIGHT_NUM)
        return;
      StaticWorldLight* light = mCore.getStaticComponent<StaticWorldLight>();
      if (light)
      {
        light->lightColor[index] = inverseGammaCorrect(glm::vec3(r, g, b));
      }
    }

    void SRInterface::setLightAzimuth(int index, float azimuth)
    {
      mLightDirectionPolar[index].x = azimuth;
      updateLightDirection(index);
    }

    void SRInterface::setLightInclination(int index, float inclination)
    {
      mLightDirectionPolar[index].y = inclination;
      updateLightDirection(index);
    }

    void SRInterface::updateLightDirection(int index)
    {
      float azimuth = mLightDirectionPolar[index].x;
      float inclination = mLightDirectionPolar[index].y;
      glm::vec3 viewVector;
      viewVector.z = cos(inclination) * cos(azimuth);
      viewVector.x = cos(inclination) * sin(azimuth);
      viewVector.y = sin(inclination);
      mLightDirectionView[index] = normalize(viewVector);
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::setLightOn(int index, bool value)
    {
      if (!mLightsOn.empty() && index < LIGHT_NUM)
        mLightsOn[index] = value;
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::setMaterialFactor(MatFactor factor, double value)
    {
      switch (factor)
      {
      case MatFactor::MAT_AMBIENT:
        mMatAmbient = value;
        break;
      case MatFactor::MAT_DIFFUSE:
        mMatDiffuse = value;
        break;
      case MatFactor::MAT_SPECULAR:
        mMatSpecular = value;
        break;
      case MatFactor::MAT_SHINE:
        mMatShine = value;
        break;
      }
    }

    //----------------------------------------------------------------------------------------------
    void SRInterface::setFog(FogFactor factor, double value)
    {
      switch (factor)
      {
      case FogFactor::FOG_INTENSITY:
        mFogIntensity = value;
        break;
      case FogFactor::FOG_START:
        mFogStart = value;
        break;
      case FogFactor::FOG_END:
        mFogEnd = value;
        break;
      }
    }

std::ostream& Render::operator<<(std::ostream& o, const glm::mat4& m)
{
  o << "{" << m[0].x << ",\t" << m[0].y << ",\t" << m[0].z << ",\t" << m[0].w << "}\n"
    << "{" << m[1].x << ",\t" << m[1].y << ",\t" << m[1].z << ",\t" << m[1].w << "}\n"
    << "{" << m[2].x << ",\t" << m[2].y << ",\t" << m[2].z << ",\t" << m[2].w << "}\n"
    << "{" << m[3].x << ",\t" << m[3].y << ",\t" << m[3].z << ",\t" << m[3].w << "}\n";
  return o;
}
