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

#ifndef INTERFACE_MODULES_RENDER_SPIRESCIRUN_SRINTERFACE_H
#define INTERFACE_MODULES_RENDER_SPIRESCIRUN_SRINTERFACE_H

#include <cstdint>
#include <memory>
#include <Interface/Modules/Render/GLContext.h>
#include <Interface/Modules/Render/ES/Core.h>
#include <es-general/comp/Transform.hpp>

//freetype
#include <ft2build.h>
#include FT_FREETYPE_H

#include <es-render/util/Shader.hpp>
#include <es-render/comp/CommonUniforms.hpp>
#include <Interface/Modules/Render/ES/comp/StaticClippingPlanes.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Interface/Modules/Render/share.h>
#include <glm/gtc/quaternion.hpp>

namespace SCIRun {
  namespace Render {

    class SRCamera;

    class SRInterfaceFailure : public std::runtime_error
    {
    public:
      explicit SRInterfaceFailure(const std::string& message) : std::runtime_error(message) {}
    };

    // This class will be executing on a remote thread using boost lock free
    // structures. The view scene dialog on qt widgets only serve one purpose:
    // to relay information to this thread so that rendering can take place.
    // Information such as mouse clicks and user settings.
    class SCISHARE SRInterface
    {
      //we use a static component for assigning entity IDs
      //For assigning asset entity ids. This can be removed if
      friend class AssetBootstrap;

    public:
      explicit SRInterface(std::shared_ptr<Gui::GLContext> context, int frameInitLimit = 100);
      ~SRInterface();
      std::string toString(std::string prefix) const;

      /// todo Specify what buttons are pressed.
      enum MouseButton
      {
        MOUSE_NONE,
        MOUSE_LEFT,
        MOUSE_RIGHT,
        MOUSE_MIDDLE,
      };

      enum MouseMode
      {
        MOUSE_OLDSCIRUN,
        MOUSE_NEWSCIRUN
      };

      enum MatFactor
      {
        MAT_AMBIENT,
        MAT_DIFFUSE,
        MAT_SPECULAR,
        MAT_SHINE
      };

      enum FogFactor
      {
        FOG_INTENSITY,
        FOG_START,
        FOG_END
      };

      struct ClippingPlane
      {
        bool visible, showFrame, reverseNormal;
        double x, y, z, d;
      };

      // todo Obtaining data from mesh objects in order to spatially partition
      //       them and provide quick object feedback.

      //---------------- Input ---------------------------------------------------------------------
      void inputMouseDown(const glm::ivec2& pos, MouseButton btn);
      void inputMouseMove(const glm::ivec2& pos, MouseButton btn);
      void inputMouseUp(const glm::ivec2& pos, MouseButton btn);
      void inputMouseWheel(int32_t delta);
      void inputShiftKeyDown(bool shiftDown);
      void setMouseMode(MouseMode mode) {mMouseMode = mode;}
      MouseMode getMouseMode() const    {return mMouseMode;}

      //---------------- Camera --------------------------------------------------------------------
      // Call this whenever the window is resized. This will modify the viewport appropriately.
      void eventResize(size_t width, size_t height);
      void doAutoView();
      // Sets the selected View of the window
      void setCameraDistance(float distance);
      float getCameraDistance();
      void setCameraLookAt(glm::vec3 lookAt);
      glm::vec3 getCameraLookAt();
      void setCameraRotation(glm::quat rotation);
      glm::quat getCameraRotation();
      void setView(const glm::vec3& view, const glm::vec3& up);
      void setZoomSpeed(int zoomSpeed) {mZoomSpeed = zoomSpeed;}
      void setZoomInverted(bool value);
      void setLockZoom(bool lock);
      void setLockPanning(bool lock);
      void setLockRotation(bool lock);
      void setAutoRotateVector(glm::vec2 axis);
      void setAutoRotateSpeed(double speed);
      void setAutoRotateOnDrag(bool value);
      const glm::mat4& getWorldToProjection() const;
      const glm::mat4& getWorldToView() const;
      const glm::mat4& getViewToWorld() const;
      const glm::mat4& getViewToProjection() const;

      //---------------- Widgets -------------------------------------------------------------------
      // todo Selecting objects...
      void select(const glm::ivec2& pos, std::list<Graphics::Datatypes::GeometryHandle> &objList, int port);
      std::string &getSelection()          {return mSelected;}
      gen::Transform &getWidgetTransform() {return mWidgetTransform;}

      //---------------- Clipping Planes -----------------------------------------------------------
      StaticClippingPlanes* getClippingPlanes();
      void setClippingPlaneVisible(bool value);
      void setClippingPlaneFrameOn(bool value);
      void reverseClippingPlaneNormal(bool value);
      void setClippingPlaneX(double value);
      void setClippingPlaneY(double value);
      void setClippingPlaneZ(double value);
      void setClippingPlaneD(double value);
      void setClippingPlaneIndex(int index) {clippingPlaneIndex_ = index;}

      //---------------- Data Handeling ------------------------------------------------------------
      // Handles a new geometry object.
      void handleGeomObject(Graphics::Datatypes::GeometryHandle object, int port);
      // Remove all SCIRun 5 objects.
      void removeAllGeomObjects();
      bool hasObject(const std::string& object);
      // Garbage collect all invalid objects not given in the valid objects vector.
      void gcInvalidObjects(const std::vector<std::string>& validObjects);
      // Simple hash function. Modify if hash collisions occur due to string
      // hashing. The simplest approach would be to have all names placed in a
      // hash multimap with a list which assigns ids to names.
      uint64_t getEntityIDForName(const std::string& name, int port);
      Core::Geometry::BBox getSceneBox() {return mSceneBBox;}

      //---------------- Rendering -----------------------------------------------------------------
      void doFrame(double currentTime, double constantDeltaTime); // Performs a frame.
      void setLightColor(int index, float r, float g, float b);
      void setLightOn(int index, bool value);
      void setLightAzimuth(int index, float azimuth);
      void setLightInclination(int index, float inclination);
      void updateLightDirection(int index);
      void setMaterialFactor(MatFactor factor, double value);
      void setFog(FogFactor factor, double value);
      void setOrientSize(int size) {orientSize = size/10.0f;}      //Remap 1:100 to 0.1:10
      void setOrientPosX(int pos)  {orientPosX = (pos-50)/100.0f;} //Remap 0:100 to -0.5:0.5
      void setOrientPosY(int pos)  {orientPosY = (pos-50)/100.0f;} //Remap 0:100 to -0.5:0.5
      void showOrientation(bool value) {showOrientation_ = value;}
      void setBackgroundColor(const QColor& color);
      void setFogColor(const glm::vec4 &color) {mFogColor = color;}
      void setTransparencyRendertype(RenderState::TransparencySortType rType) {mRenderSortType = rType;}

      // Screen width retrieval. Dimensions are pixels.
      size_t getScreenWidthPixels() const  {return mScreenWidth;}
      size_t getScreenHeightPixels() const {return mScreenHeight;}

    private:
      void setupCore();
      void setupLights();

      //---------------- Camera ----------------------------------------------------------------------
      void updateCamera(); // Places mCamera's transform into our static camera component.
      void applyAutoRotation();

      //---------------- Widgets -------------------------------------------------------------------
      bool foundWidget(const glm::ivec2& pos); // search for a widget at mouse position
      void updateWidget(const glm::ivec2& pos); // update selected widget
      uint32_t getSelectIDForName(const std::string& name);
      glm::vec4 getVectorForID(const uint32_t id);
      uint32_t getIDForVector(const glm::vec4& vec);

      //---------------- Clipping Planes -----------------------------------------------------------
      void checkClippingPlanes(int n);// make sure clipping plane number matches
      double getMaxProjLength(const glm::vec3 &n);
      void updateClippingPlanes();

      //---------------- Data Handeling ------------------------------------------------------------
      // Adds a VBO to the given entityID.
      void addVBOToEntity(uint64_t entityID, const std::string& vboName);
      // Adds an IBO to the given entityID.
      void addIBOToEntity(uint64_t entityID, const std::string& iboName);
      //add a texture to the given entityID.
      void addTextToEntity(uint64_t entityID, const Graphics::Datatypes::SpireText& text);
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


      class DepthIndex
      {
      public:
        size_t mIndex;
        double mDepth;

        DepthIndex() :
          mIndex(0),
          mDepth(0.0)
        {}

        DepthIndex(size_t index, double depth) :
          mIndex(index),
          mDepth(depth)
        {}

        bool operator<(const DepthIndex& di) const
        {
          return this->mDepth < di.mDepth;
        }
      };

      class SRObject
      {
      public:
        SRObject(const std::string& name, const glm::mat4& objToWorld,
          const Core::Geometry::BBox& bbox, boost::optional<std::string> colorMap, int port) :
          mName(name),
          mObjectToWorld(objToWorld),
          mBBox(bbox),
          mColorMap(colorMap),
          mPort(port)
        {}

        // Different types of uniform transformations that are associated
        // with the object (based off of the unsatisfied uniforms detected
        // by the Spire object).
        enum ObjectTransforms
        {
          OBJECT_TO_WORLD,
          OBJECT_TO_CAMERA,
          OBJECT_TO_CAMERA_PROJECTION,
        };

        struct SRPass
        {
          SRPass(const std::string& name, Graphics::Datatypes::RenderType renType) :
            passName(name),
            renderType(renType)
          {}

          std::string passName;
          std::list<ObjectTransforms> transforms;
          Graphics::Datatypes::RenderType renderType;
        };

        std::string mName;
        glm::mat4 mObjectToWorld;
        std::list<SRPass> mPasses;
        Core::Geometry::BBox mBBox;          // Objects bounding box (calculated from VBO).

        boost::optional<std::string> mColorMap;

        int	mPort;
      };


      bool                              showOrientation_    {true};   // Whether the coordinate axes will render or not.
      bool                              autoRotate_         {false};  // Whether the scene will continue to rotate.
      bool                              selectWidget_       {false};  // Whether mouse click will select a widget.
      bool                              widgetSelected_     {false};  // Whether or not a widget is currently selected.
      bool                              widgetExists_       {false};  // Geometry contains a widget to find.
      bool                              tryAutoRotate       {false};
      bool                              doAutoRotateOnDrag  {false};

      float                             orientSize          {1.0};    //  Size of coordinate axes
      float                             orientPosX          {0.5};    //  X Position of coordinate axes
      float                             orientPosY          {0.5};    //  Y Position of coordinate axes

      uint64_t                          mSelectedID         {0};
      int                               mZoomSpeed          {65};
      MouseMode                         mMouseMode          {MOUSE_OLDSCIRUN};  // Current mouse mode.

      std::string                       mSelected           {};       // Current selection
      glm::vec4                         mSelectedPos        {};
      gen::Transform                    mWidgetTransform    {};

      size_t                            mScreenWidth        {640};    // Screen width in pixels.
      size_t                            mScreenHeight       {480};    // Screen height in pixels.

      GLuint                            mFontTexture        {};       // 2D texture for fonts

      int                               axesFailCount_      {0};
      std::shared_ptr<Gui::GLContext>   mContext            {};       // Context to use for rendering.
      std::vector<SRObject>             mSRObjects          {};       // All SCIRun objects.
      Core::Geometry::BBox              mSceneBBox          {};       // Scene's AABB. Recomputed per-frame.


      ESCore                            mCore               {};       // Entity system core.

      std::vector<ClippingPlane>        clippingPlanes_     {};
      int                               clippingPlaneIndex_ {0};

      ren::ShaderVBOAttribs<5>          mArrowAttribs       {};       // Pre-applied shader / VBO attributes.
      ren::CommonUniforms               mArrowUniforms      {};       // Common uniforms used in the arrow shader.
      RenderState::TransparencySortType mRenderSortType     {};       // Which strategy will be used to render transparency

      //material settings
      double                            mMatAmbient         {};
      double                            mMatDiffuse         {};
      double                            mMatSpecular        {};
      double                            mMatShine           {};

      //fog settings
      double                            mFogIntensity       {};
      double                            mFogStart           {};
      double                            mFogEnd             {};
      glm::vec4                         mFogColor           {};

      //light settings
      std::vector<glm::vec2>            mLightDirectionPolar{};
      std::vector<glm::vec3>            mLightDirectionView {};
      std::vector<bool>                 mLightsOn           {};

      glm::vec2                         autoRotateVector    {0.0, 0.0};
      float                             autoRotateSpeed     {0.01};

      const int                         frameInitLimit_     {};
      std::unique_ptr<SRCamera>         mCamera;       // Primary camera.
    };

  } // namespace Render
} // namespace SCIRun

#endif
