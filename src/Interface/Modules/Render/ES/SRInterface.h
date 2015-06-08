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

#include <Core/Datatypes/Geometry.h>
#include <boost/shared_ptr.hpp>

#include <Interface/Modules/Render/GLContext.h>
#include <Interface/Modules/Render/namespaces.h>

#include "Core.h"
#include "AssetBootstrap.h"

// CPM Modules
#include <gl-state/GLState.hpp>
#include <es-render/util/Shader.hpp>
#include <es-render/comp/CommonUniforms.hpp>
#include <glm/glm.hpp>

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
        class SRInterface
        {
            friend class AssetBootstrap;  ///< For assigning asset entity ids.
            ///< This can be removed if we use a static
            ///< component for assigning entity IDs.
        public:
            SRInterface(std::shared_ptr<Gui::GLContext> context,
                        const std::vector<std::string>& shaderDirs, int frameInitLimit);
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
            
            /// \todo Selecting objects...
            
            /// \todo Obtaining data from mesh objects in order to spatially partition
            ///       them and provide quick object feedback.
            
            /// Screen width retrieval. Dimensions are pixels.
            size_t getScreenWidthPixels() const       { return mScreenWidth; }
            size_t getScreenHeightPixels() const      { return mScreenHeight; }
            
            /// Reads an asset file and returns the associated vertex buffer and index
            /// buffer.
            void readAsset(const std::string& filename,
                           std::vector<uint8_t> vbo, std::vector<uint8_t> ibo);
            
            /// Remove all SCIRun 5 objects.
            void removeAllGeomObjects();
            
            /// Garbage collect all invalid objects not given in the valid objects vector.
            void gcInvalidObjects(const std::vector<std::string>& validObjects);
            
            /// Handles a new geometry object.
            void handleGeomObject(boost::shared_ptr<Core::Datatypes::GeometryObject> object, int port);
            
            /// Performs a frame.
            void doFrame(double currentTime, double constantDeltaTime);
            
            /// Sets the mouse interaction mode.
            void setMouseMode(MouseMode mode);
            
            /// Retrieves mouse interaction mode.
            MouseMode getMouseMode();
            
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
                    SRPass(const std::string& name, Core::Datatypes::GeometryObject::RenderType renType) :
                    passName(name),
                    renderType(renType)
                    {}
                    
                    std::string                 passName;
                    std::list<ObjectTransforms> transforms;
                    Core::Datatypes::GeometryObject::RenderType renderType;
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
            
            // Adds a VBO to the given entityID.
            void addVBOToEntity(uint64_t entityID, const std::string& vboName);
            
            // Adds an IBO to the given entityID.
            void addIBOToEntity(uint64_t entityID, const std::string& iboName);
            
            void reorderIBO(Core::Datatypes::GeometryObject::SpireSubPass& pass);
            
            // Adds a shader to the given entityID. Represents different materials
            // associated with different passes.
            void addShaderToEntity(uint64_t entityID, const std::string& shaderName);
            
            // Apply uniform.
            void applyUniform(uint64_t entityID, const Core::Datatypes::GeometryObject::SpireSubPass::Uniform& uniform);
            
            
            bool                              showOrientation_; ///< Whether the coordinate axes will render or not.
            bool                              autoRotate_;      ///< Whether the scene will continue to rotate.
            
            
            MouseMode                         mMouseMode;       ///< Current mouse mode.
            
            
            size_t                            mScreenWidth;     ///< Screen width in pixels.
            size_t                            mScreenHeight;    ///< Screen height in pixels.
            
            GLuint                            mFontTexture;     /// 2D texture for fonts
            
            int axesFailCount_;
            std::shared_ptr<Gui::GLContext>   mContext;         ///< Context to use for rendering.
            std::unique_ptr<SRCamera>         mCamera;          ///< Primary camera.
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
        };
        
    } // namespace Render
} // namespace SCIRun 

#endif 
