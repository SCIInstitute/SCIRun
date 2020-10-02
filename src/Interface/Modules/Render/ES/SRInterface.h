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


#ifndef INTERFACE_MODULES_RENDER_SPIRESCIRUN_SRINTERFACE_H
#define INTERFACE_MODULES_RENDER_SPIRESCIRUN_SRINTERFACE_H

#include <unordered_map>
#include <cstdint>
#include <memory>
#include <Interface/Modules/Render/GLContext.h>
#include <Interface/Modules/Render/ES/Core.h>

//freetype
#include <ft2build.h>
#include FT_FREETYPE_H

#include <es-render/util/Shader.hpp>
#include <es-render/comp/CommonUniforms.hpp>
#include <Interface/Modules/Render/ES/comp/StaticClippingPlanes.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <glm/gtc/quaternion.hpp>
#include <QOpenGLContext>
#include <Interface/Modules/Render/ES/RendererInterface.h>
#include <Interface/Modules/Render/ES/RendererCollaborators.h>
#include <Interface/Modules/Render/share.h>


namespace SCIRun
{
  namespace Render
  {
    // This class will be executing on a remote thread using boost lock free
    // structures. The view scene dialog on qt widgets only serve one purpose:
    // to relay information to this thread so that rendering can take place.
    // Information such as mouse clicks and user settings.
    class SCISHARE SRInterface : public RendererInterface, public ObjectTransformer
    {
      //we use a static component for assigning entity IDs
      //For assigning asset entity ids. This can be removed if
      friend class AssetBootstrap;

    public:
      explicit SRInterface(int frameInitLimit = 100);
      ~SRInterface();
      std::string toString(std::string prefix) const override;

      void setContext(QOpenGLContext* context) override {mContext = context;}

      // todo Obtaining data from mesh objects in order to spatially partition
      //       them and provide quick object feedback.

      //---------------- Input ---------------------------------------------------------------------
      void widgetMouseDown(MouseButton btn, int x, int y) override;
      void widgetMouseMove(MouseButton btn, int x, int y) override;
      void widgetMouseUp() override;
      void inputMouseDown(MouseButton btn, float x, float y) override;
      void inputMouseMove(MouseButton btn, float x, float y) override;
      void inputMouseUp() override;
      void inputMouseWheel(int32_t delta) override;
      void setMouseMode(MouseMode mode) override {mMouseMode = mode;}
      MouseMode getMouseMode() const override    {return mMouseMode;}
      void calculateScreenSpaceCoords(int x_in, int y_in, float& x_out, float& y_out) override;


      //---------------- Camera --------------------------------------------------------------------
      // Call this whenever the window is resized. This will modify the viewport appropriately.
      void eventResize(size_t width, size_t height) override;
      void doAutoView() override;
      // Sets the selected View of the window
      void setCameraDistance(const float distance) override;
      float getCameraDistance() const override;
      void setCameraLookAt(const glm::vec3& lookAt) override;
      glm::vec3 getCameraLookAt() const override;
      void setCameraRotation(const glm::quat& rotation) override;
      glm::quat getCameraRotation() const override;
      void setView(const glm::vec3& view, const glm::vec3& up) override;
      void setZoomSpeed(int zoomSpeed) override {mZoomSpeed = zoomSpeed;}
      void setZoomInverted(bool value) override;
      void setLockZoom(bool lock) override;
      void setLockPanning(bool lock) override;
      void setLockRotation(bool lock) override;
      void setAutoRotateVector(const glm::vec2& axis) override;
      void setAutoRotateSpeed(double speed) override;
      const glm::mat4& getWorldToView() const override;
      const glm::mat4& getViewToProjection() const override;

      //---------------- Widgets -------------------------------------------------------------------
      // todo Selecting objects...
      Graphics::Datatypes::WidgetHandle select(int x, int y, Graphics::Datatypes::WidgetList& widgets) override;
      glm::mat4 getWidgetTransform() override { return widgetUpdater_.widgetTransform(); }

      //---------------- Clipping Planes -----------------------------------------------------------
      StaticClippingPlanes* getClippingPlanes() override;
      void setClippingPlaneVisible(bool value) override;
      void setClippingPlaneFrameOn(bool value) override;
      void reverseClippingPlaneNormal(bool value) override;
      void setClippingPlaneX(double value) override;
      void setClippingPlaneY(double value) override;
      void setClippingPlaneZ(double value) override;
      void setClippingPlaneD(double value) override;
      void setClippingPlaneIndex(int index) override {clippingPlaneIndex_ = index;}
      void doInitialWidgetUpdate(Graphics::Datatypes::WidgetHandle& widget, int x, int y) override;

      //---------------- Data Handling ------------------------------------------------------------
      // Handles a new geometry object.
      void handleGeomObject(Graphics::Datatypes::GeometryHandle object, int port) override;
      // Remove all SCIRun 5 objects.
      void removeAllGeomObjects() override;
      bool hasObject(const std::string& object) override;
      // Garbage collect all invalid objects not given in the valid objects vector.
      void gcInvalidObjects(const std::vector<std::string>& validObjects) override;
      Core::Geometry::BBox getSceneBox() override {return mSceneBBox;}

      bool hasShaderPromise() const override;
      void runGCOnNextExecution() override;

      //---------------- Rendering -----------------------------------------------------------------
      void doFrame(double constantDeltaTime) override; // Performs a frame.
      void setLightColor(int index, float r, float g, float b) override;
      void setLightOn(int index, bool value) override;
      void setLightAzimuth(int index, float azimuth) override;
      void setLightInclination(int index, float inclination) override;
      void updateLightDirection(int index);
      void setMaterialFactor(MatFactor factor, double value) override;
      void setFog(FogFactor factor, double value) override;
      void setOrientSize(int size) override {orientSize = size/10.0f;}      //Remap 1:100 to 0.1:10
      void setOrientPosX(int pos) override  {orientPosX = (pos-50)/100.0f;} //Remap 0:100 to -0.5:0.5
      void setOrientPosY(int pos) override  {orientPosY = (pos-50)/100.0f;} //Remap 0:100 to -0.5:0.5
      void showOrientation(bool value) override {showOrientation_ = value;}
      void setBackgroundColor(const QColor& color) override;
      void setFogColor(const glm::vec4 &color) override {mFogColor = color;}
      void setTransparencyRendertype(RenderState::TransparencySortType rType) override {mRenderSortType = rType;}

      // Screen width retrieval. Dimensions are pixels.
      size_t getScreenWidthPixels() const override  { return screen_.width; }
      size_t getScreenHeightPixels() const override { return screen_.height; }
      glm::mat4 getStaticCameraViewProjection() override;

      void modifyObject(const std::string& id, const gen::Transform& trans) override;
      glm::mat4 getWorldToProjection() const override;

    private:
      void setupCore();
      void setupLights();

      // Simple hash function. Modify if hash collisions occur due to string
      // hashing. The simplest approach would be to have all names placed in a
      // hash multimap with a list which assigns ids to names.
      static uint64_t getEntityIDForName(const std::string& name, int port);

      //---------------- Camera ----------------------------------------------------------------------
      void applyAutoRotation();
      void updateCamera(); // Places mCamera's transform into our static camera component.

      static uint32_t getSelectIDForName(const std::string& name);
      static glm::vec4 getVectorForID(const uint32_t id);
      static uint32_t getIDForVector(const glm::vec4& vec);

      //---------------- Clipping Planes -----------------------------------------------------------
      void checkClippingPlanes(unsigned int n);// make sure clipping plane number matches
      double getMaxProjLength(const glm::vec3 &n);
      void updateClippingPlanes();

      //---------------- Data Handling ------------------------------------------------------------
      // Adds a VBO to the given entityID.
      void addVBOToEntity(uint64_t entityID, const std::string& vboName);
      // Adds an IBO to the given entityID.
      void addIBOToEntity(uint64_t entityID, const std::string& iboName);
      //add a texture to the given entityID.
      void addTextToEntity(uint64_t entityID, const Graphics::Datatypes::SpireText& text);
      void addTextureToEntity(uint64_t entityID, const Graphics::Datatypes::SpireTexture2D& texture);
      // Adds a shader to the given entityID. Represents different materials
      // associated with different passes.
      void addShaderToEntity(uint64_t entityID, const std::string& shaderName);
      // Generates the various colormaps that we use for rendering SCIRun geometry.
      void generateTextures();

      //---------------- Rendering -----------------------------------------------------------------
      void renderCoordinateAxes();
      void updateWorldLight();
      void applyUniform(uint64_t entityID, const Graphics::Datatypes::SpireSubPass::Uniform& uniform);
      void applyMatFactors(Graphics::Datatypes::SpireSubPass::Uniform& uniform);
      void applyFog(Graphics::Datatypes::SpireSubPass::Uniform& uniform);

      bool                                showOrientation_    {true};   // Whether the coordinate axes will render or not.
      bool                                autoRotate_         {false};  // Whether the scene will continue to rotate.
      bool                                tryAutoRotate       {false};
      bool                                doAutoRotateOnDrag  {false};

      float                               orientSize          {1.0};    //  Size of coordinate axes
      float                               orientPosX          {0.5};    //  X Position of coordinate axes
      float                               orientPosY          {0.5};    //  Y Position of coordinate axes

      uint64_t                            mSelectedID         {0};
      int                                 mZoomSpeed          {65};
      MouseMode                           mMouseMode          {MouseMode::MOUSE_OLDSCIRUN};  // Current mouse mode.

      ScreenParams screen_;
      WidgetUpdateService widgetUpdater_;

      GLuint                              mFontTexture        {};       // 2D texture for fonts

      int                                 axesFailCount_      {0};
      std::vector<SRObject>               mSRObjects          {};       // All SCIRun objects.
      Core::Geometry::BBox				  mSceneBBox          {};       // Scene's AABB. Recomputed per-frame.
      std::unordered_map<std::string, uint64_t> mEntityIdMap  {};

      ESCore                              mCore               {};       // Entity system core.

      std::vector<ClippingPlane>          clippingPlanes_     {};
      int                                 clippingPlaneIndex_ {0};

      ren::ShaderVBOAttribs<5>            mArrowAttribs       {};       // Pre-applied shader / VBO attributes.
      ren::CommonUniforms                 mArrowUniforms      {};       // Common uniforms used in the arrow shader.
      RenderState::TransparencySortType   mRenderSortType     {RenderState::TransparencySortType::UPDATE_SORT};       // Which strategy will be used to render transparency

      //material settings
      double                              mMatAmbient         {};
      double                              mMatDiffuse         {};
      double                              mMatSpecular        {};
      double                              mMatShine           {};
      GLfloat                             mLastSelectionDepth {0.0};

      //fog settings
      double                              mFogIntensity       {};
      double                              mFogStart           {};
      double                              mFogEnd             {};
      glm::vec4                           mFogColor           {};

      //light settings
      std::vector<glm::vec2>              mLightDirectionPolar{};
      std::vector<glm::vec3>              mLightDirectionView {};
      std::vector<bool>                   mLightsOn           {};

      glm::vec2                         autoRotateVector      {0.0, 0.0};
      float                             autoRotateSpeed       {0.01f};

      const int                         frameInitLimit_ {};
      QOpenGLContext*                   mContext        {};
	    std::unique_ptr<SRCamera>         mCamera;			// Primary camera.
    };

  } // namespace Render
} // namespace SCIRun

#endif
