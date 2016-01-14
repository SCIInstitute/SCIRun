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
#include "Core.h"

// CPM Modules
#include <es-render/util/Shader.hpp>
#include <es-render/comp/CommonUniforms.hpp>

#include <Graphics/Datatypes/GeometryImpl.h>
#include <Interface/Modules/Render/share.h>

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
      friend class AssetBootstrap;  ///< For assigning asset entity ids.
      ///< This can be removed if we use a static
      ///< component for assigning entity IDs.
    public:
      explicit SRInterface(std::shared_ptr<Gui::GLContext> context, int frameInitLimit = 100);
      ~SRInterface();

      /// Call this whenever the window is resized. This will modify the viewport
      /// appropriately.
      void eventResize(size_t width, size_t height);

      /// \todo Specify what buttons are pressed.
      /// @{
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

      void inputMouseDown(const glm::ivec2& pos, MouseButton btn);
      void inputMouseMove(const glm::ivec2& pos, MouseButton btn);
      void inputMouseUp(const glm::ivec2& pos, MouseButton btn);
      /// @}

      void inputMouseWheel(int32_t delta);

      void inputShiftKeyDown(bool shiftDown);

      /// \todo Selecting objects...
      void select(const glm::ivec2& pos, std::list<Graphics::Datatypes::GeometryHandle> &objList, int port);

      /// \todo Obtaining data from mesh objects in order to spatially partition
      ///       them and provide quick object feedback.

      /// Screen width retrieval. Dimensions are pixels.
      size_t getScreenWidthPixels() const       { return mScreenWidth; }
      size_t getScreenHeightPixels() const      { return mScreenHeight; }

      /// Remove all SCIRun 5 objects.
      void removeAllGeomObjects();

      /// Garbage collect all invalid objects not given in the valid objects vector.
      void gcInvalidObjects(const std::vector<std::string>& validObjects);

      /// Handles a new geometry object.
      void handleGeomObject(Graphics::Datatypes::GeometryHandle object, int port);

      /// Performs a frame.
      void doFrame(double currentTime, double constantDeltaTime);

      /// Sets the mouse interaction mode.
      void setMouseMode(MouseMode mode);

      /// Retrieves mouse interaction mode.
      MouseMode getMouseMode() const;

      /// Sets zoom speed
      void setZoomSpeed(int zoomSpeed);

      /// Sets zoom inverted/not inverted
      void setZoomInverted(bool value);

      /// Performs an autoview.
      void doAutoView();

      /// Sets the selected View of the window
      void setView(const glm::vec3& view, const glm::vec3& up);

      /// Toggle Orientation Axes
      void showOrientation(bool value);

      /// Set the Background Color
      void setBackgroundColor(QColor color);

      /// Set Transparency Rener Type
      void setTransparencyRendertype(RenderState::TransparencySortType rType);

      /// get name of the selection
      std::string &getSelection();

    private:

      class DepthIndex {
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

          std::string                 passName;
          std::list<ObjectTransforms> transforms;
          Graphics::Datatypes::RenderType renderType;
        };

        std::string                     mName;
        glm::mat4                       mObjectToWorld;
        std::list<SRPass>               mPasses;
        Core::Geometry::BBox            mBBox;          ///< Objects bounding box (calculated from VBO).

        boost::optional<std::string>    mColorMap;

        int										          mPort;
      };
      // Sets up ESCore.
      void setupCore();

      // Places mCamera's transform into our static camera component.
      void updateCamera();

      // Updates the world light.
      void updateWorldLight();

      // Renders coordinate axes on the screen.
      void renderCoordinateAxes();

      // Generates the various colormaps that we use for rendering SCIRun geometry.
      void generateTextures();

      // Simple hash function. Modify if hash collisions occur due to string
      // hashing. The simplest approach would be to have all names placed in a
      // hash multimap with a list which assigns ids to names.
      uint64_t getEntityIDForName(const std::string& name, int port);
      uint32_t getSelectIDForName(const std::string& name);
      glm::vec4 getVectorForID(const uint32_t id);
      uint32_t getIDForVector(const glm::vec4& vec);

      // Adds a VBO to the given entityID.
      void addVBOToEntity(uint64_t entityID, const std::string& vboName);

      // Adds an IBO to the given entityID.
      void addIBOToEntity(uint64_t entityID, const std::string& iboName);

      // Adds a shader to the given entityID. Represents different materials
      // associated with different passes.
      void addShaderToEntity(uint64_t entityID, const std::string& shaderName);

      // Apply uniform.
      void applyUniform(uint64_t entityID, const Graphics::Datatypes::SpireSubPass::Uniform& uniform);

      // search for a widget at mouse position
      bool foundWidget(const glm::ivec2& pos);


      bool                              showOrientation_; ///< Whether the coordinate axes will render or not.
      bool                              autoRotate_;      ///< Whether the scene will continue to rotate.
      bool                              selectWidget_;    ///< Whether mouse click will select a widget.
      bool                              widgetSelected_;  ///< Whether or not a widget is currently selected.
      bool                              widgetExists_;    ///< Geometry contains a widget to find.

      int                               mZoomSpeed;
      MouseMode                         mMouseMode;       ///< Current mouse mode.

      std::string                       mSelected;        ///< Current selection
      size_t                            mScreenWidth;     ///< Screen width in pixels.
      size_t                            mScreenHeight;    ///< Screen height in pixels.

      GLuint                            mFontTexture;     /// 2D texture for fonts

      int axesFailCount_;
      std::shared_ptr<Gui::GLContext>   mContext;         ///< Context to use for rendering.
      std::vector<SRObject>             mSRObjects;       ///< All SCIRun objects.
      Core::Geometry::BBox              mSceneBBox;       ///< Scene's AABB. Recomputed per-frame.


      ESCore                            mCore;            ///< Entity system core.


      std::string                       mArrowVBOName;    ///< VBO for one axis of the coordinate axes.
      std::string                       mArrowIBOName;    ///< IBO for one axis of the coordinate axes.
      std::string                       mArrowObjectName; ///< Object name for profile arrow.


      ren::ShaderVBOAttribs<5>          mArrowAttribs;    ///< Pre-applied shader / VBO attributes.
      ren::CommonUniforms               mArrowUniforms;   ///< Common uniforms used in the arrow shader.
      RenderState::TransparencySortType mRenderSortType;  ///< Which strategy will be used to render transparency
      const int frameInitLimit_;
      std::unique_ptr<SRCamera>         mCamera;          ///< Primary camera.
    };

  } // namespace Render
} // namespace SCIRun 

#endif 
