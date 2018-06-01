/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <es-log/trace-log.h>
// Needed for OpenGL include files on Travis:
#include <gl-platform/GLPlatform.hpp>
#include <Interface/Modules/Render/UndefiningX11Cruft.h>
#include <QtOpenGL/QGLWidget>

#include <Interface/Modules/Render/ES/SRInterface.h>
#include <Interface/Modules/Render/ES/SRCamera.h>

#include <Core/Logging/Log.h>
#include <Core/Application/Application.h>
#include <Graphics/Glyphs/GlyphGeom.h>

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
#include "comp/StaticSRInterface.h"
#include "comp/RenderBasicGeom.h"
#include "comp/SRRenderState.h"
#include "comp/RenderList.h"
#include "comp/StaticWorldLight.h"
#include "comp/LightingUniforms.h"
#include "comp/ClippingPlaneUniforms.h"

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

using namespace std::placeholders;

namespace fs = spire;

namespace SCIRun {
  namespace Render {

    //------------------------------------------------------------------------------
    SRInterface::SRInterface(std::shared_ptr<Gui::GLContext> context,
      int frameInitLimit) :
      mSelectedID(0),
      mZoomSpeed(65),
      mMouseMode(MOUSE_OLDSCIRUN),
      mScreenWidth(640),
      mScreenHeight(480),
      axesFailCount_(0),
      mContext(context),
      clippingPlaneIndex_(0),
      mMatAmbient(0.2),
      mMatDiffuse(1.0),
      mMatSpecular(0.0),
      mMatShine(2.0),
      mFogIntensity(0.0),
      mFogStart(0.0),
      mFogEnd(1.0),
      mFogColor(glm::vec4(0.0)),
      frameInitLimit_(frameInitLimit),
      mCamera(new SRCamera(*this))  // Should come after all vars have been initialized.
    {
      showOrientation_ = true;
      autoRotate_ = false;
      selectWidget_ = false;
      widgetSelected_ = false;
      widgetExists_ = false;
      mRenderSortType = RenderState::TransparencySortType::UPDATE_SORT;
      // Construct ESCore. We will need to bootstrap the core. We should also
      // probably add utility static classes.
      setupCore();
      setupLights();
    }

    //------------------------------------------------------------------------------
    SRInterface::~SRInterface()
    {
      glDeleteTextures(1, &mFontTexture);
    }

    //------------------------------------------------------------------------------
    void SRInterface::setupCore()
    {
      mCore.addUserSystem(getSystemName_CoreBootstrap());

      // Add screen height / width static component.
      {
        gen::StaticScreenDims dims;
        dims.width = static_cast<uint32_t>(mScreenWidth);
        dims.height = static_cast<uint32_t>(mScreenHeight);
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
        std::string filesystemRoot = Core::Application::Instance().executablePath().string();
        filesystemRoot += boost::filesystem::path::preferred_separator;
        fs::StaticFS fileSystem((std::make_shared<fs::FilesystemSync>(filesystemRoot)));
        mCore.addStaticComponent(fileSystem);
        mCore.disableComponentSerialization<fs::StaticFS>();
      }

      // Add StaticSRInterface
      {
        StaticSRInterface iface(this);
        mCore.addStaticComponent(iface);
      }

    }

    void SRInterface::setupLights()
    {
      //mLightPosition;
      mLightsOn.push_back(true);
      mLightPosition.push_back(glm::vec3(0, 0, 1));
      for (int i = 1; i < LIGHT_NUM; ++i)
      {
        mLightsOn.push_back(false);
        mLightPosition.push_back(glm::vec3(0, 0, 1));
      }
    }

    //------------------------------------------------------------------------------
    void SRInterface::setMouseMode(MouseMode mode)
    {
      mMouseMode = mode;
    }

    //------------------------------------------------------------------------------
    SRInterface::MouseMode SRInterface::getMouseMode() const
    {
      return mMouseMode;
    }

    //------------------------------------------------------------------------------
    void SRInterface::setZoomSpeed(int zoomSpeed)
    {
      mZoomSpeed = zoomSpeed;
    }

    //------------------------------------------------------------------------------
    void SRInterface::setZoomInverted(bool value)
    {
      mCamera->setZoomInverted(value);
    }

    //------------------------------------------------------------------------------
    void SRInterface::eventResize(size_t width, size_t height)
    {
      mScreenWidth = width;
      mScreenHeight = height;

      mContext->makeCurrent();
      GL(glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));

      // Obtain StaticScreenDims component and populate.
      gen::StaticScreenDims* dims = mCore.getStaticComponent<gen::StaticScreenDims>();
      if (dims)
      {
        dims->width = static_cast<uint32_t>(width);
        dims->height = static_cast<uint32_t>(height);
      }

      // Setup default camera projection.
      gen::StaticCamera* cam = mCore.getStaticComponent<gen::StaticCamera>();
      gen::StaticOrthoCamera* orthoCam = mCore.getStaticComponent<gen::StaticOrthoCamera>();

      if (cam == nullptr || orthoCam == nullptr) return;

      float aspect = static_cast<float>(width) / static_cast<float>(height);

      float perspFOVY = 0.59f;
      float perspZNear = 0.01f;
      float perspZFar = 20000.0f;
      glm::mat4 proj = glm::perspective(perspFOVY, aspect, perspZNear, perspZFar);
      cam->data.setProjection(proj, perspFOVY, aspect, perspZNear, perspZFar);
      cam->data.winWidth = static_cast<float>(width);

      // Setup default ortho camera projection
      float orthoZNear = -1000.0f;
      float orthoZFar = 1000.0f;
      glm::mat4 orthoProj = glm::ortho(/*left*/   -1.0f,      /*right*/ 1.0f,
        /*bottom*/ -1.0f,      /*top*/   1.0f,
        /*znear*/  orthoZNear, /*zfar*/  orthoZFar);
      orthoCam->data.setOrthoProjection(orthoProj, aspect, 2.0f, 2.0f, orthoZNear, orthoZFar);
      orthoCam->data.winWidth = static_cast<float>(width);
    }

    //------------------------------------------------------------------------------
    void SRInterface::inputMouseDown(const glm::ivec2& pos, MouseButton btn)
    {
      if (selectWidget_ && widgetExists_)
      {
        if (btn == MouseButton::MOUSE_LEFT)
        {
          // //widgetSelected_ = foundWidget(pos);
          // std::cout << "widget exists" << std::endl;
        }
      }
      mCamera->mouseDownEvent(pos, btn);
    }

    void SRInterface::setLockZoom(bool lock)
    {
      mCamera->setLockZoom(lock);
    }

    void SRInterface::setLockPanning(bool lock)
    {
      mCamera->setLockPanning(lock);
    }

    void SRInterface::setLockRotation(bool lock)
    {
      mCamera->setLockRotation(lock);
    }

    //------------------------------------------------------------------------------
    void SRInterface::inputMouseMove(const glm::ivec2& pos, MouseButton btn)
    {
      if (widgetSelected_)
      {
        updateWidget(pos);
      }
      else
      {
        mCamera->mouseMoveEvent(pos, btn);
      }
    }

    //------------------------------------------------------------------------------
    void SRInterface::inputMouseWheel(int32_t delta)
    {
      mCamera->mouseWheelEvent(delta, mZoomSpeed);
    }

    //------------------------------------------------------------------------------
    void SRInterface::inputShiftKeyDown(bool shiftDown)
    {
      if (shiftDown) std::cout << "ShiftDown" << std::endl;
      else  std::cout << "ShiftUp" << std::endl;

      selectWidget_ = shiftDown;
    }

    void SRInterface::select(const glm::ivec2& pos,
      std::list<Graphics::Datatypes::GeometryHandle> &objList,
      int port)
    {
      mSelected = "";
      widgetSelected_ = false;
      // Ensure our rendering context is current on our thread.
      mContext->makeCurrent();

      //get vbo ibo man
      std::weak_ptr<ren::VBOMan> vm = mCore.getStaticComponent<ren::StaticVBOMan>()->instance_;
      std::weak_ptr<ren::IBOMan> im = mCore.getStaticComponent<ren::StaticIBOMan>()->instance_;
      std::shared_ptr<ren::VBOMan> vboMan = vm.lock();
      std::shared_ptr<ren::IBOMan> iboMan = im.lock();
      if (!vboMan || !iboMan)
        return;

      //retrieve and bind fbo for selection
      std::weak_ptr<ren::FBOMan> fm = mCore.getStaticComponent<ren::StaticFBOMan>()->instance_;
      std::shared_ptr<ren::FBOMan> fboMan = fm.lock();
      if (!fboMan)
        return;
      std::string fboName = "Selection:FBO:0";
      GLuint fboId = fboMan->getOrCreateFBO(mCore, GL_TEXTURE_2D,
        mScreenWidth, mScreenHeight, 1,
        fboName);
      fboMan->bindFBO(fboId);

      //a map from selection id to name
      std::map<uint32_t, std::string> selMap;
      std::vector<uint64_t> entityList;

      //modify and add each object to draw
      for (auto& obj : objList)
      {
        std::string objectName = obj->uniqueID();
        uint32_t selid = getSelectIDForName(objectName);
        selMap.insert(std::make_pair(selid, objectName));
        glm::vec4 selCol = getVectorForID(selid);

        // Add vertex buffer objects.
        std::vector<char*> vbo_buffer;
        std::vector<size_t> stride_vbo;

        int nameIndex = 0;
        for (auto it = obj->vbos().cbegin(); it != obj->vbos().cend(); ++it, ++nameIndex)
        {
          const auto& vbo = *it;

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

          vbo_buffer.push_back(reinterpret_cast<char*>(vbo.data->getBuffer()));
          size_t stride = 0;
          for (auto a : vbo.attributes)
            stride += a.sizeInBytes;
          stride_vbo.push_back(stride);
        }

        // Add index buffer objects.
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
            throw std::invalid_argument("Unable to determine index buffer depth.");
            break;
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
          default:
            primitive = GL_TRIANGLES;
            break;
          }

          int numPrimitives = ibo.data->getBufferSize() / ibo.indexSize;
          iboMan->addInMemoryIBO(ibo.data->getBuffer(), ibo.data->getBufferSize(), primitive, primType, numPrimitives, ibo.name);
        }

        std::weak_ptr<ren::ShaderMan> sm = mCore.getStaticComponent<ren::StaticShaderMan>()->instance_;
        if (auto shaderMan = sm.lock())
        {
          // Add passes
          for (auto& pass : obj->passes())
          {
            uint64_t entityID = getEntityIDForName(pass.passName, port);

            if (pass.renderType == RenderType::RENDER_VBO_IBO)
            {
              addVBOToEntity(entityID, pass.vboName);
              addIBOToEntity(entityID, pass.iboName);
            }
            else
            {
              // We will be constructing a render list from the VBO and IBO.
              RenderList list;

              for (const auto& vbo : obj->vbos())
              {
                if (vbo.name == pass.vboName)
                {
                  list.data = vbo.data;
                  list.attributes = vbo.attributes;
                  list.renderType = pass.renderType;
                  list.numElements = vbo.numElements;
                  mCore.addComponent(entityID, list);
                  break;
                }
              }

              // Lookup the VBOs and IBOs associated with this particular draw list
              // and add them to our entity in question.
              std::string assetName = "Assets/sphere.geom";

              addVBOToEntity(entityID, assetName);
              addIBOToEntity(entityID, assetName);
            }

            // Load vertex and fragment shader will use an already loaded program.
            //shaderMan->loadVertexAndFragmentShader(mCore, entityID, "Shaders/Selection");
            //					addShaderToEntity(entityID, "Shaders/Selection");
            //					shaderMan->loadVertexAndFragmentShader(mCore, entityID, pass.programName);
            const char* selectionShaderName = "Shaders/Selection";
            GLuint shaderID = shaderMan->getIDForAsset(selectionShaderName);
            if (shaderID == 0)
            {
              const char* vs =
                "uniform mat4 uProjIVObject;\n"
                "uniform vec4 uColor;\n"
                "attribute vec3 aPos;\n"
                "varying vec4 fColor;\n"
                "void main()\n"
                "{\n"
                "  gl_Position = uProjIVObject * vec4(aPos, 1.0);\n"
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

              shaderID = shaderMan->addInMemoryVSFS(vs, fs, selectionShaderName);
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

            SpireSubPass::Uniform uniform(
              "uColor", selCol);
            applyUniform(entityID, uniform);

            // Add components associated with entity. We just need a base class which
            // we can pass in an entity ID, then a derived class which bundles
            // all associated components (including types) together. We can use
            // a variadic template for this. This will allow us to place any components
            // we want on the objects in question in show field. This could lead to
            // much simpler customization.

            pass.renderState.mSortType = mRenderSortType;
            pass.renderState.set(RenderState::ActionFlags::USE_BLEND, false);
            mCore.addComponent(entityID, pass);
            entityList.push_back(entityID);
          }
        }
      }

      updateCamera();
      updateWorldLight();

      mCore.execute(0, 50);

      GLuint value;
      GLfloat depth;
      if (fboMan->readFBO(mCore, fboName, pos.x, pos.y, 1, 1,
        (GLvoid*)&value, (GLvoid*)&depth))
      {
        auto it = selMap.find(value);
        if (it != selMap.end())
          mSelected = it->second;
      }
      //release and restore fbo
      fboMan->unbindFBO();

      //calculate position
      if (mSelected != "")
      {
        widgetSelected_ = true;
        glm::vec4 spos((float(2 * pos.x) - float(mScreenWidth)) / float(mScreenWidth),
          (float(mScreenHeight) - float(2 * pos.y)) / float(mScreenHeight),
          depth * 2 - 1, 1.0f);
        mSelectedPos = spos;
        //selPos = cam->data.projIV * spos;
        //std::cout << selPos.x << "\t" << selPos.y << "\t" << selPos.z << "\n";
      }

      for (auto& it : entityList)
        mCore.removeEntity(it);
    }

    //------------------------------------------------------------------------------
    void SRInterface::doAutoView()
    {
      if (mSceneBBox.valid())
      {
        mCamera->doAutoView(mSceneBBox);

        //std::cout << mSceneBBox.get_min() << "\t" << mSceneBBox.get_max() << "\n";
      }
    }

    //------------------------------------------------------------------------------
    void SRInterface::setView(const glm::vec3& view, const glm::vec3& up)
    {
      mCamera->setView(view, up);
    }

    //------------------------------------------------------------------------------
    void SRInterface::showOrientation(bool value)
    {
      showOrientation_ = value;
    }

    //------------------------------------------------------------------------------
    void SRInterface::setBackgroundColor(QColor color)
    {
      mCore.setBackgroundColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }

    //------------------------------------------------------------------------------
    void SRInterface::setTransparencyRendertype(RenderState::TransparencySortType rType)
    {
      mRenderSortType = rType;
    }

    //------------------------------------------------------------------------------
    std::string &SRInterface::getSelection()
    {
      return mSelected;
    }

    gen::Transform &SRInterface::getWidgetTransform()
    {
      return mWidgetTransform;
    }

    //------------------------------------------------------------------------------
    //--------------Clipping Plane Tools--------------------------------------------
    void SRInterface::checkClippingPlanes(int n)
    {
      while (n >= clippingPlanes_.size())
      {
        ClippingPlane plane;
        plane.visible = false;
        plane.showFrame = false;
        plane.reverseNormal = false;
        plane.x = 0.0;
        plane.y = 0.0;
        plane.z = 0.0;
        plane.d = 0.0;
        clippingPlanes_.push_back(plane);
      }
    }

    void SRInterface::setClippingPlaneIndex(int index)
    {
      clippingPlaneIndex_ = index;
    }

    void SRInterface::setClippingPlaneVisible(bool value)
    {
      checkClippingPlanes(clippingPlaneIndex_);
      clippingPlanes_[clippingPlaneIndex_].visible = value;
      updateClippingPlanes();
    }

    void SRInterface::setClippingPlaneFrameOn(bool value)
    {
      checkClippingPlanes(clippingPlaneIndex_);
      clippingPlanes_[clippingPlaneIndex_].showFrame = value;
      updateClippingPlanes();
    }

    void SRInterface::reverseClippingPlaneNormal(bool value)
    {
      checkClippingPlanes(clippingPlaneIndex_);
      clippingPlanes_[clippingPlaneIndex_].reverseNormal = value;
      updateClippingPlanes();
    }

    void SRInterface::setClippingPlaneX(double value)
    {
      checkClippingPlanes(clippingPlaneIndex_);
      clippingPlanes_[clippingPlaneIndex_].x = value;
      updateClippingPlanes();
    }

    void SRInterface::setClippingPlaneY(double value)
    {
      checkClippingPlanes(clippingPlaneIndex_);
      clippingPlanes_[clippingPlaneIndex_].y = value;
      updateClippingPlanes();
    }

    void SRInterface::setClippingPlaneZ(double value)
    {
      checkClippingPlanes(clippingPlaneIndex_);
      clippingPlanes_[clippingPlaneIndex_].z = value;
      updateClippingPlanes();
    }

    void SRInterface::setClippingPlaneD(double value)
    {
      checkClippingPlanes(clippingPlaneIndex_);
      clippingPlanes_[clippingPlaneIndex_].d = value;
      updateClippingPlanes();
    }

    //set material factors
    void SRInterface::setMaterialFactor(MatFactor factor, double value)
    {
      switch (factor)
      {
      case MAT_AMBIENT:
        mMatAmbient = value;
        break;
      case MAT_DIFFUSE:
        mMatDiffuse = value;
        break;
      case MAT_SPECULAR:
        mMatSpecular = value;
        break;
      case MAT_SHINE:
        mMatShine = value;
        break;
      }
    }

    //set fog
    void SRInterface::setFog(FogFactor factor, double value)
    {
      switch (factor)
      {
      case FOG_INTENSITY:
        mFogIntensity = value;
        break;
      case FOG_START:
        mFogStart = value;
        break;
      case FOG_END:
        mFogEnd = value;
        break;
      }
    }

    void SRInterface::setFogColor(const glm::vec4 &color)
    {
      mFogColor = color;
    }

    const glm::mat4& SRInterface::getWorldToProjection() const
    { return mCamera->getWorldToProjection(); }

    const glm::mat4& SRInterface::getWorldToView() const
    { return mCamera->getWorldToView(); }

    const glm::mat4& SRInterface::getViewToWorld() const
    { return mCamera->getViewToWorld(); }

    const glm::mat4& SRInterface::getViewToProjection() const
    { return mCamera->getViewToProjection(); }

    //------------------------------------------------------------------------------
    /*void SRInterface::setScaleBar(const ScaleBar &scaleBarData)
    {
      scaleBar_.visible = scaleBarData.visible;
      scaleBar_.fontSize = scaleBarData.fontSize;
      scaleBar_.length = scaleBarData.length;
      scaleBar_.height = scaleBarData.height;
      scaleBar_.multiplier = scaleBarData.multiplier;
      scaleBar_.numTicks = scaleBarData.numTicks;
      scaleBar_.lineWidth = scaleBarData.lineWidth;
      scaleBar_.unit = scaleBarData.unit;
      if (scaleBar_.visible)
      {
        updateScaleBarLength();
        updateGeometryScaleBar();
      }
    }*/

    //------------------------------------------------------------------------------
    void SRInterface::inputMouseUp(const glm::ivec2& /*pos*/, MouseButton /*btn*/)
    {
      widgetSelected_ = false;
    }

    //------------------------------------------------------------------------------
    uint64_t SRInterface::getEntityIDForName(const std::string& name, int port)
    {
      return (static_cast<uint64_t>(std::hash<std::string>()(name)) >> 8) + (static_cast<uint64_t>(port) << 56);
    }

    uint32_t SRInterface::getSelectIDForName(const std::string& name)
    {
      return (static_cast<uint32_t>(std::hash<std::string>()(name)));
    }

    glm::vec4 SRInterface::getVectorForID(const uint32_t id)
    {
      float a = ((id >> 24) & 0xff) / 255.0f;
      float b = ((id >> 16) & 0xff) / 255.0f;
      float g = ((id >> 8) & 0xff) / 255.0f;
      float r = (id & 0xff) / 255.0f;
      glm::vec4 vec(r, g, b, a);
      return vec;
    }

    uint32_t SRInterface::getIDForVector(const glm::vec4& vec)
    {
      return 0;
    }

    //------------------------------------------------------------------------------
    void SRInterface::handleGeomObject(GeometryHandle obj, int port)
    {
      //logRendererInfo("Handling geom object on port {}", port);
      RENDERER_LOG_FUNCTION_SCOPE;
      RENDERER_LOG("Ensure our rendering context is current on our thread.");
      DEBUG_LOG_LINE_INFO
      mContext->makeCurrent();

      std::string objectName = obj->uniqueID();
      BBox bbox; // Bounding box containing all vertex buffer objects.

      RENDERER_LOG("Check to see if the object already exists in our list. "
        "If so, then remove the object. We will re-add it.");
      auto foundObject = std::find_if(
        mSRObjects.begin(), mSRObjects.end(),
        [&objectName](const SRObject& sro)
      {
        return (sro.mName == objectName);
      });

      DEBUG_LOG_LINE_INFO

      std::weak_ptr<ren::VBOMan> vm = mCore.getStaticComponent<ren::StaticVBOMan>()->instance_;
      std::weak_ptr<ren::IBOMan> im = mCore.getStaticComponent<ren::StaticIBOMan>()->instance_;
      if (std::shared_ptr<ren::VBOMan> vboMan = vm.lock())
      {
        DEBUG_LOG_LINE_INFO
        if (std::shared_ptr<ren::IBOMan> iboMan = im.lock())
        {
          DEBUG_LOG_LINE_INFO
          if (foundObject != mSRObjects.end())
          {
            DEBUG_LOG_LINE_INFO

            RENDERER_LOG("Iterate through each of the passes and remove their associated entity ID.");
            for (const auto& pass : foundObject->mPasses)
            {
              uint64_t entityID = getEntityIDForName(pass.passName, port);
              mCore.removeEntity(entityID);
            }

            RENDERER_LOG("We need to renormalize the core after removing entities. We don't need"
              "to run a new pass however. Renormalization is enough to remove"
              "old entities from the system.");
            mCore.renormalize(true);

            RENDERER_LOG("Run a garbage collection cycle for the VBOs and IBOs. We will likely"
              " be using similar VBO and IBO names.");
            vboMan->runGCCycle(mCore);
            iboMan->runGCCycle(mCore);

            RENDERER_LOG("Remove the object from the entity system.");
            mSRObjects.erase(foundObject);
          }

          DEBUG_LOG_LINE_INFO
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
              std::vector<std::tuple<std::string, size_t, bool>> attributeData;
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

          DEBUG_LOG_LINE_INFO
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
              break;
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
            default:
              primitive = GL_TRIANGLES;
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
                  char* sbuffer = !sorted_buffer.empty() ? reinterpret_cast<char*>(&sorted_buffer[0]) : 0;

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
          glm::mat4 xform;
          mSRObjects.push_back(SRObject(objectName, xform, bbox, obj->colorMap(), port));
          SRObject& elem = mSRObjects.back();

          std::weak_ptr<ren::ShaderMan> sm = mCore.getStaticComponent<ren::StaticShaderMan>()->instance_;
          if (auto shaderMan = sm.lock())
          {
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
              }
              else
              {
                RENDERER_LOG("We will be constructing a render list from the VBO and IBO.");
                RenderList list;

                for (const auto& vbo : obj->vbos())
                {
                  if (vbo.name == pass.vboName)
                  {
                    list.data = vbo.data;
                    list.attributes = vbo.attributes;
                    list.renderType = pass.renderType;
                    list.numElements = vbo.numElements;
                    mCore.addComponent(entityID, list);
                    break;
                  }
                }

                RENDERER_LOG("Lookup the VBOs and IBOs associated with this particular draw list "
                  "and add them to our entity in question.");
                std::string assetName = "Assets/sphere.geom";

                if (pass.renderType == RenderType::RENDER_RLIST_SPHERE)
                {
                  assetName = "Assets/sphere.geom";
                }

                if (pass.renderType == RenderType::RENDER_RLIST_CYLINDER)
                {
                  assetName = "Assests/arrow.geom";
                }

                addVBOToEntity(entityID, assetName);
                addIBOToEntity(entityID, assetName);
              }

              RENDERER_LOG("Load vertex and fragment shader will use an already loaded program.");
              shaderMan->loadVertexAndFragmentShader(mCore, entityID, pass.programName);

              RENDERER_LOG("Add transformation");
              gen::Transform trafo;

              if (pass.renderState.get(RenderState::IS_WIDGET))
              {
                widgetExists_ = true;
              }

              if (pass.renderType == RenderType::RENDER_RLIST_SPHERE)
              {
                double scale = pass.scalar;
                trafo.transform[0].x = scale;
                trafo.transform[1].y = scale;
                trafo.transform[2].z = scale;
              }
              if (widgetSelected_ && objectName == mSelected)
              {
                mSelectedID = entityID;
              }
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
                Graphics::Datatypes::SpireSubPass::Uniform uniform;
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

            RENDERER_LOG("Recalculate scene bounding box. Should only be done when an object is added.");
            mSceneBBox.reset();
            for (auto it = mSRObjects.begin(); it != mSRObjects.end(); ++it)
            {
              if (it->mBBox.valid())
              {
                mSceneBBox.extend(it->mBBox);
              }
            }
          }
        }
      }
      DEBUG_LOG_LINE_INFO
    }

    //------------------------------------------------------------------------------
    void SRInterface::addVBOToEntity(uint64_t entityID, const std::string& vboName)
    {
      std::weak_ptr<ren::VBOMan> vm = mCore.getStaticComponent<ren::StaticVBOMan>()->instance_;
      if (std::shared_ptr<ren::VBOMan> vboMan = vm.lock()) {
        ren::VBO vbo;
        vbo.glid = vboMan->hasVBO(vboName);
        mCore.addComponent(entityID, vbo);
      }
    }

    //------------------------------------------------------------------------------
    void SRInterface::addIBOToEntity(uint64_t entityID, const std::string& iboName)
    {
      std::weak_ptr<ren::IBOMan> im = mCore.getStaticComponent<ren::StaticIBOMan>()->instance_;
      if (std::shared_ptr<ren::IBOMan> iboMan = im.lock()) {
        ren::IBO ibo;
        auto iboData = iboMan->getIBOData(iboName);
        ibo.glid = iboMan->hasIBO(iboName);
        ibo.primType = iboData.primType;
        ibo.primMode = iboData.primMode;
        ibo.numPrims = iboData.numPrims;

        mCore.addComponent(entityID, ibo);
      }
    }

    //------------------------------------------------------------------------------
    void SRInterface::addTextToEntity(uint64_t entityID, const SpireText& text)
    {
      if (text.name == "")
        return;

       //texture man
      std::weak_ptr<ren::TextureMan> tm = mCore.getStaticComponent<ren::StaticTextureMan>()->instance_;
      std::shared_ptr<ren::TextureMan> textureMan = tm.lock();
      if (!textureMan)
        return;

      std::stringstream ss;
      ss << "FontTexture:" << entityID << text.name << text.width << text.height;
      std::string assetName = ss.str();

      ren::Texture texture;

      spire::CerealHeap<ren::Texture>* contTex =
        mCore.getOrCreateComponentContainer<ren::Texture>();
      std::pair<const ren::Texture*, size_t> component =
        contTex->getComponent(entityID);
      if (component.first == nullptr)
        texture = textureMan->createTexture(assetName, text.width, text.height, text.bitmap);
      else
        texture = *component.first;

      texture.textureUnit = 0;
      texture.setUniformName("uTX0");
      mCore.addComponent(entityID, texture);
    }

    //------------------------------------------------------------------------------
    void SRInterface::addShaderToEntity(uint64_t entityID, const std::string& shaderName)
    {
      std::weak_ptr<ren::ShaderMan> sm = mCore.getStaticComponent<ren::StaticShaderMan>()->instance_;
      if (std::shared_ptr<ren::ShaderMan> shaderMan = sm.lock()) {
        ren::Shader shader;
        shader.glid = shaderMan->getIDForAsset(shaderName.c_str());
        mCore.addComponent(entityID, shader);
      }
    }

    //------------------------------------------------------------------------------
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

    //apply material factors
    void SRInterface::applyMatFactors(Graphics::Datatypes::SpireSubPass::Uniform& uniform)
    {
      if (uniform.name == "uAmbientColor")
        uniform.data = glm::vec4(mMatAmbient);
      else if (uniform.name == "uSpecularColor")
        uniform.data = glm::vec4(mMatSpecular);
      else if (uniform.name == "uSpecularPower")
        uniform.data = glm::vec4(mMatShine);
    }

    //apply fog
    void SRInterface::applyFog(Graphics::Datatypes::SpireSubPass::Uniform& uniform)
    {
      if (uniform.name == "uFogSettings")
      {
        double start, end, zdist, ddist;
        glm::vec4 center(mSceneBBox.center().x(), mSceneBBox.center().y(), mSceneBBox.center().z(), 1.0);
        glm::mat4 worldToView = mCamera->getWorldToView();
        center = worldToView * center;
        center /= center.w;
        glm::vec3 c3(center.x, center.y, center.z);
        zdist = glm::length(c3);
        ddist = 1.1 * mSceneBBox.diagonal().length();
        start = zdist + (mFogStart - 0.5) * ddist;
        end = zdist + (mFogEnd - 0.5) * ddist;
        uniform.data = glm::vec4(mFogIntensity, start, end, 0.0);
      }
      else if (uniform.name == "uFogColor")
        uniform.data = mFogColor;
      uniform.type = Graphics::Datatypes::SpireSubPass::Uniform::UniformType::UNIFORM_VEC4;
    }

    //------------------------------------------------------------------------------
    bool SRInterface::foundWidget(const glm::ivec2& pos)
    {
      mContext->makeCurrent();
      for (auto it = mSRObjects.begin(); it != mSRObjects.end(); ++it)
      {
        for (const auto& pass : it->mPasses)
        {
          uint64_t entityID = getEntityIDForName(pass.passName, it->mPort);
          mCore.getComponentContainer(entityID);
        }
      }
      return true;
    }

    void SRInterface::updateWidget(const glm::ivec2& pos)
    {
      gen::StaticCamera* cam = mCore.getStaticComponent<gen::StaticCamera>();
      glm::vec4 spos((float(2 * pos.x) - float(mScreenWidth)) / float(mScreenWidth),
        (float(mScreenHeight) - float(2 * pos.y)) / float(mScreenHeight),
        mSelectedPos.z, 1.0f);

      mWidgetTransform = gen::Transform();
      mWidgetTransform.setPosition((spos - mSelectedPos).xyz());
      mWidgetTransform.transform = glm::inverse(cam->data.projIV) *
        mWidgetTransform.transform * cam->data.projIV;

      spire::CerealHeap<gen::Transform>* contTrans =
        mCore.getOrCreateComponentContainer<gen::Transform>();
      std::pair<const gen::Transform*, size_t> component =
        contTrans->getComponent(mSelectedID);

      if (component.first != nullptr)
        contTrans->modifyIndex(mWidgetTransform, component.second, 0);
    }

    //------------------------------------------------------------------------------
    void SRInterface::removeAllGeomObjects()
    {
      mContext->makeCurrent();
      for (auto it = mSRObjects.begin(); it != mSRObjects.end(); ++it)
      {
        // Iterate through each of the passes and remove their associated
        // entity ID.
        for (const auto& pass : it->mPasses)
        {
          uint64_t entityID = getEntityIDForName(pass.passName, it->mPort);
          mCore.removeEntity(entityID);
        }
      }

      mCore.renormalize(true);
      widgetExists_ = false;
      mSRObjects.clear();
    }

    //------------------------------------------------------------------------------
    void SRInterface::gcInvalidObjects(const std::vector<std::string>& validObjects)
    {
      for (auto it = mSRObjects.begin(); it != mSRObjects.end();)
      {
        if (std::find(validObjects.begin(), validObjects.end(), it->mName) == validObjects.end())
        {
          for (const auto& pass : it->mPasses)
          {
            uint64_t entityID = getEntityIDForName(pass.passName, it->mPort);
            mCore.removeEntity(entityID);
          }
          it = mSRObjects.erase(it);
        }
        else
        {
          ++it;
        }
      }

      mCore.renormalize(true);
    }

    //------------------------------------------------------------------------------
    void SRInterface::doFrame(double currentTime, double constantDeltaTime)
    {
      /// \todo Only render a frame if something has changed (new or deleted
      ///       objects, or the view point has changed).

      mContext->makeCurrent();

      updateCamera();
      updateWorldLight();

      mCore.execute(currentTime, constantDeltaTime);

      if (showOrientation_)
      {
        // Do not even attempt to render if the framebuffer is not complete.
        // This can happen when the rendering window is hidden (in SCIRun5 for
        // example);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
        {
          renderCoordinateAxes();
        }
      }


      // Set directional light source (in world space).
      // Need to set a static light source component which will be used when
      // uLightDirWorld is requested from a shader.
      // glm::vec3 viewDir = viewToWorld[2].xyz();
      // viewDir = -viewDir; // Cameras look down -Z.
      //mSpire->addGlobalUniform("uLightDirWorld", viewDir);
    }

    //------------------------------------------------------------------------------
    void SRInterface::updateCamera()
    {
      // Update the static camera with the appropriate world to view transform.
      mCamera->applyTransform();
      glm::mat4 viewToWorld = mCamera->getViewToWorld();

      gen::StaticCamera* camera = mCore.getStaticComponent<gen::StaticCamera>();
      if (camera)
      {
        camera->data.setView(viewToWorld);
      }
    }

    //------------------------------------------------------------------------------
    void SRInterface::updateWorldLight()
    {
      /*
      getWorldToProjection() const;
      getWorldToView() const;
      getViewToWorld() const;
      getViewToProjection() const;
      */
      glm::mat4 viewToWorld = mCamera->getViewToWorld();
      //glm::mat4 viewToWorld = mCamera->getWorldToView();

      // Set directional light source (in world space).
      StaticWorldLight* light = mCore.getStaticComponent<StaticWorldLight>();
      if (light)
      {
        for (int i = 0; i < LIGHT_NUM; ++i)
        {
          /*
          glm::vec3 viewDir = glm::vec3(0.0, 0.0, -1.0) - mLightPosition[i];
          glm::vec4 newDir(viewDir.x, viewDir.y, viewDir.z, 1.0);
          viewToWorld *= newDir;
          glm::vec3 lightDir = viewToWorld[4].xyz();
          */

          glm::vec3 viewDir = viewToWorld[2].xyz();
          viewDir = -viewDir; // Cameras look down -Z.
          //light->lightDir[i] = mLightsOn[i] ? viewDir - mLightPosition[i] : glm::vec3(0.0, 0.0, 0.0);
          light->lightDir[i] = mLightsOn[i] ? viewDir : glm::vec3(0.0, 0.0, 0.0);
          //light->lightDir[i] = mLightsOn[i] ? lightDir : glm::vec3(0.0, 0.0, 0.0);
        }
      }
    }

    void SRInterface::setLightColor(int index, float r, float g, float b)
    {
      if (index >= LIGHT_NUM)
        return;
      StaticWorldLight* light = mCore.getStaticComponent<StaticWorldLight>();
      if (light)
      {
        light->lightColor[index] = glm::vec3(r, g, b);
      }
    }


    void SRInterface::setLightPosition(int index, float x, float y)
    {
      if (index >= LIGHT_NUM)
        return;

      glm::mat4 viewToWorld = mCamera->getViewToWorld();
      if (mLightPosition.size() > 0)
      {
        mLightPosition[index] = glm::vec3(x, y, 0.0);
      }
      /*
      // Set directional light source (in world space).
      StaticWorldLight* light = mCore.getStaticComponent<StaticWorldLight>();
      if (light)
      {
        glm::vec3 viewDir = viewToWorld[2].xyz();
        viewDir = -viewDir; // Cameras look down -Z.
        light->lightDir[index] = mLightsOn[index] ? viewDir-position : glm::vec3(0.0, 0.0, 0.0);

        glm::vec3 view1 = viewToWorld[0].xyz();
        glm::vec3 view2 = viewToWorld[1].xyz();
        glm::vec3 view4 = viewToWorld[3].xyz();
        std::cout << "size: " << viewToWorld.length() << std::endl;
        std::cout << "view1x: " << view1.x << " view1y: " << view1.y << " view1z: " << view1.z << std::endl;
        std::cout << "view2x: " << view2.x << " view2y: " << view2.y << " view2z: " << view2.z << std::endl;
        std::cout << "view3x: " << viewDir.x << " view3y: " << viewDir.y << " view3z: " << viewDir.z << std::endl;
        std::cout << "view4x: " << view4.x << " view4y: " << view4.y << " view4z: " << view4.z << std::endl;
        std::cout << "x: " << x << " y: " << y << " z: " << 0 << std::endl;
      }
      */
    }

    void SRInterface::setLightOn(int index, bool value)
    {
      if (mLightsOn.size() > 0 && index < LIGHT_NUM)
      {
        mLightsOn[index] = value;
      }
    }

    //
    double SRInterface::getMaxProjLength(const glm::vec3 &n)
    {
      glm::vec3 a1(-1.0, 1.0, -1.0);
      glm::vec3 a2(-1.0, 1.0, 1.0);
      glm::vec3 a3(1.0, 1.0, -1.0);
      glm::vec3 a4(1.0, 1.0, 1.0);
      return std::max(
        std::max(
        std::abs(glm::dot(n, a1)),
        std::abs(glm::dot(n, a2))),
        std::max(
        std::abs(glm::dot(n, a3)),
        std::abs(glm::dot(n, a4))));
    }

    void SRInterface::updateClippingPlanes()
    {
      StaticClippingPlanes* clippingPlanes = mCore.getStaticComponent<StaticClippingPlanes>();
      if (clippingPlanes)
      {
        clippingPlanes->clippingPlanes.clear();
        clippingPlanes->clippingPlaneCtrls.clear();
        //boundbox transformation
        glm::mat4 trans_bb = glm::mat4();
        glm::vec3 scale_bb(mSceneBBox.x_length() / 2.0, mSceneBBox.y_length() / 2.0, mSceneBBox.z_length() / 2.0);
        glm::vec3 center_bb(mSceneBBox.center().x(), mSceneBBox.center().y(), mSceneBBox.center().z());
        glm::mat4 temp = glm::scale(glm::mat4(), scale_bb);
        trans_bb = temp * trans_bb;
        temp = glm::translate(glm::mat4(), center_bb);
        trans_bb = temp * trans_bb;
        //std::cout << trans_bb[0][0] << "\t" << trans_bb[1][0] << "\t" << trans_bb[2][0] << "\t" << trans_bb[3][0] << "\n" <<
        //  trans_bb[0][1] << "\t" << trans_bb[1][1] << "\t" << trans_bb[2][1] << "\t" << trans_bb[3][1] << "\n" <<
        //  trans_bb[0][2] << "\t" << trans_bb[1][2] << "\t" << trans_bb[2][2] << "\t" << trans_bb[3][2] << "\n" <<
        //  trans_bb[0][3] << "\t" << trans_bb[1][3] << "\t" << trans_bb[2][3] << "\t" << trans_bb[3][3] << "\n";
        int index = 0;
        for (auto i : clippingPlanes_)
        {
          glm::vec3 n3(i.x, i.y, i.z);
          double d = i.d;
          glm::vec4 n(0.0);
          if (glm::length(n3) > 0.0)
          {
            n3 = glm::normalize(n3);
            n = glm::vec4(n3, 0.0);
            d *= getMaxProjLength(n3);
          }
          //std::cout << i.d << "\t" << getMaxProjLength(n3) << "\t" << d << "\n";
          glm::vec4 o = glm::vec4(n.x, n.y, n.z, 1.0) * d;
          o.w = 1;
          o = trans_bb * o;
          n = glm::inverseTranspose(trans_bb) * n;
          o.w = 0;
          n.w = 0;
          n = glm::normalize(n);
          n.w = -glm::dot(o, n);
          //std::cout << n.x << "\t" << n.y << "\t" << n.z << "\t" << n.w << "\n";
          clippingPlanes->clippingPlanes.push_back(n);
          glm::vec4 control(i.visible ? 1.0 : 0.0,
            i.showFrame ? 1.0 : 0.0,
            i.reverseNormal ? 1.0 : 0.0, 0.0);
          clippingPlanes->clippingPlaneCtrls.push_back(control);
          index++;
        }
      }
    }

    StaticClippingPlanes* SRInterface::getClippingPlanes()
    {
      StaticClippingPlanes* clippingPlanes = mCore.getStaticComponent<StaticClippingPlanes>();
      return clippingPlanes;
    }

    //get scenenox
    Core::Geometry::BBox SRInterface::getSceneBox()
    {
      return mSceneBBox;
    }

    //------------------------------------------------------------------------------
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
            GLuint shader = shaderMan->getIDForAsset("Shaders/DirPhongNoClipping");

            // Bail if assets have not been loaded yet (asynchronous loading may take a
            // few frames).
            if (arrowVBO == 0 || arrowIBO == 0 || shader == 0)
            {
              axesFailCount_++;
              if (axesFailCount_ > frameInitLimit_)
                throw SRInterfaceFailure("Failed to initialize axes after many attempts. ViewScene is unusable. Halting renderer loop.");
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

            // Note that we can pull aspect ratio from the screen dimensions static
            // variable.
            gen::StaticScreenDims* dims = mCore.getStaticComponent<gen::StaticScreenDims>();
            float aspect = static_cast<float>(dims->width) / static_cast<float>(dims->height);
            glm::mat4 projection = glm::perspective(0.59f, aspect, 1.0f, 2000.0f);

            // Build world transform for all axes. Rotates about uninverted camera's
            // view, then translates to a specified corner on the screen.
            glm::mat4 axesRot = mCamera->getWorldToView();
            axesRot[3][0] = 0.0f;
            axesRot[3][1] = 0.0f;
            axesRot[3][2] = 0.0f;
            glm::mat4 invCamTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0.375f * aspect, 0.37f, -1.5f));
            glm::mat4 axesScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f));
            glm::mat4 axesTransform = axesScale * axesRot;

            GLint locCamViewVec = glGetUniformLocation(shader, "uCamViewVec");
            GLint locLightDirWorld = glGetUniformLocation(shader, "uLightDirWorld");

            GLint locAmbientColor = glGetUniformLocation(shader, "uAmbientColor");
            GLint locDiffuseColor = glGetUniformLocation(shader, "uDiffuseColor");
            GLint locSpecularColor = glGetUniformLocation(shader, "uSpecularColor");
            GLint locSpecularPower = glGetUniformLocation(shader, "uSpecularPower");

            GLint locProjIVObject = glGetUniformLocation(shader, "uProjIVObject");
            GLint locObject = glGetUniformLocation(shader, "uObject");

            GL(glUniform3f(locCamViewVec, 0.0f, 0.0f, -1.0f));
            GL(glUniform3f(locLightDirWorld, 0.0f, 0.0f, -1.0f));

            // Build projection for the axes to use on the screen. The arrors will not
            // use the camera, but will use the camera's transformation matrix.

            mArrowAttribs.bind();

            // X Axis (dark)
            {
              glm::mat4 xform = glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 2.0f, glm::vec3(0.0, 1.0, 0.0));
              glm::mat4 finalTrafo = axesTransform * xform;

              GL(glUniform4f(locAmbientColor, 0.1f, 0.01f, 0.01f, 1.0f));
              GL(glUniform4f(locDiffuseColor, 0.25f, 0.0f, 0.0f, 1.0f));
              GL(glUniform4f(locSpecularColor, 0.0f, 0.0f, 0.0f, 1.0f));
              GL(glUniform1f(locSpecularPower, 16.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = glm::value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = glm::value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
            }

            // X Axis
            {
              glm::mat4 xform = glm::rotate(glm::mat4(1.0f), -glm::pi<float>() / 2.0f, glm::vec3(0.0, 1.0, 0.0));
              glm::mat4 finalTrafo = axesTransform * xform;

              GL(glUniform4f(locAmbientColor, 0.5f, 0.01f, 0.01f, 1.0f));
              GL(glUniform4f(locDiffuseColor, 1.0f, 0.0f, 0.0f, 1.0f));
              GL(glUniform4f(locSpecularColor, 0.5f, 0.5f, 0.5f, 1.0f));
              GL(glUniform1f(locSpecularPower, 16.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = glm::value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = glm::value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
            }

            // Y Axis (dark)
            {
              glm::mat4 xform = glm::rotate(glm::mat4(1.0f), -glm::pi<float>() / 2.0f, glm::vec3(1.0, 0.0, 0.0));
              glm::mat4 finalTrafo = axesTransform * xform;

              GL(glUniform4f(locAmbientColor, 0.01f, 0.1f, 0.01f, 1.0f));
              GL(glUniform4f(locDiffuseColor, 0.0f, 0.25f, 0.0f, 1.0f));
              GL(glUniform4f(locSpecularColor, 0.0f, 0.0f, 0.0f, 1.0f));
              GL(glUniform1f(locSpecularPower, 16.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = glm::value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = glm::value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
            }

            // Y Axis
            {
              glm::mat4 xform = glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 2.0f, glm::vec3(1.0, 0.0, 0.0));
              glm::mat4 finalTrafo = axesTransform * xform;

              GL(glUniform4f(locAmbientColor, 0.01f, 0.5f, 0.01f, 1.0f));
              GL(glUniform4f(locDiffuseColor, 0.0f, 1.0f, 0.0f, 1.0f));
              GL(glUniform4f(locSpecularColor, 0.5f, 0.5f, 0.5f, 1.0f));
              GL(glUniform1f(locSpecularPower, 16.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = glm::value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = glm::value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
            }

            // Z Axis (dark)
            {
              // No rotation at all
              glm::mat4 finalTrafo = axesTransform;

              GL(glUniform4f(locAmbientColor, 0.01f, 0.01f, 0.1f, 1.0f));
              GL(glUniform4f(locDiffuseColor, 0.0f, 0.0f, 0.25f, 1.0f));
              GL(glUniform4f(locSpecularColor, 0.0f, 0.0f, 0.0f, 1.0f));
              GL(glUniform1f(locSpecularPower, 16.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = glm::value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = glm::value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
            }

            // Z Axis
            {
              // No rotation at all
              glm::mat4 xform = glm::rotate(glm::mat4(1.0f), glm::pi<float>(), glm::vec3(1.0, 0.0, 0.0));
              glm::mat4 finalTrafo = axesTransform * xform;

              GL(glUniform4f(locAmbientColor, 0.01f, 0.01f, 0.5f, 1.0f));
              GL(glUniform4f(locDiffuseColor, 0.0f, 0.0f, 1.0f, 1.0f));
              GL(glUniform4f(locSpecularColor, 0.5f, 0.5f, 0.5f, 1.0f));
              GL(glUniform1f(locSpecularPower, 16.0f));

              glm::mat4 worldToProj = projection * invCamTrans * finalTrafo;
              const GLfloat* ptr = glm::value_ptr(worldToProj);
              GL(glUniformMatrix4fv(locProjIVObject, 1, false, ptr));

              glm::mat4 objectSpace = finalTrafo;
              ptr = glm::value_ptr(objectSpace);
              GL(glUniformMatrix4fv(locObject, 1, false, ptr));

              GL(glDrawElements(iboData->primMode, iboData->numPrims, iboData->primType, 0));
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

    // Create default colormaps.
    void SRInterface::generateTextures()
    {
      //font texture
      //read in the font data
      bool success = true;
      auto fontPath = SCIRun::Core::Application::Instance().executablePath() / "Assets" / "times_new_roman.font";
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
        uint16_t *font_data = new uint16_t[w*h];
        in.read(reinterpret_cast<char*>(font_data), sizeof(uint16_t)*w*h);
        in.close();
        //std::vector<uint8_t> font;
        //font.reserve(w * h * 4);
        char* font = new char[w * h * 4];
        for (size_t i = 0; i < w*h; i++)
        {
          uint16_t pixel = font_data[i];
/*          font.push_back(static_cast<uint8_t>(pixel >> 8));
          font.push_back(static_cast<uint8_t>(pixel >> 8));
          font.push_back(static_cast<uint8_t>(pixel >> 8));
          font.push_back(static_cast<uint8_t>(pixel & 0x00ff));*/
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
        //GL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
        GL(glTexImage2D(GL_TEXTURE_2D, 0,
          GL_RGBA,
          GLsizei(w), GLsizei(h), 0,
          GL_RGBA,
          GL_UNSIGNED_BYTE, (GLvoid*)font));
        delete [] font_data;
        delete [] font;
      }
    }
  } // namespace Render
} // namespace SCIRun
