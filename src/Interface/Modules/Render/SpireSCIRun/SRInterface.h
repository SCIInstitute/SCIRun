/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

/// \author James Hughes
/// \date   February 2013

#ifndef SPIRE_APPSPECIFIC_SCIRUN_SCIRUNINTERFACE_H
#define SPIRE_APPSPECIFIC_SCIRUN_SCIRUNINTERFACE_H

#include <cstdint>

#include <Core/Datatypes/Geometry.h>
#include <boost/shared_ptr.hpp>

#include "../namespaces.h"
#include <spire/Interface.h>
#include <gl-state/GLState.hpp>

namespace SCIRun {
namespace Gui {

class SRCamera;
class SciBall;

// This class will be executing on a remote thread using boost lock free
// structures. The view scene dialog on qt widgets only serve one purpose:
// to relay information to this thread so that rendering can take place.
// Information such as mouse clicks and user settings.
class SRInterface
{
public:
  SRInterface(std::shared_ptr<spire::Context> context,
              const std::vector<std::string>& shaderDirs,
              spire::Interface::LogFunction logFP = spire::Interface::LogFunction());
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
  size_t getScreenWidthPixels() const       {return mScreenWidth;}
  size_t getScreenHeightPixels() const      {return mScreenHeight;}

  /// Reads an asset file and returns the associated vertex buffer and index
  /// buffer.
  void readAsset(const std::string& filename,
                 std::vector<uint8_t> vbo, std::vector<uint8_t> ibo);

  /// Remove all SCIRun 5 objects.
  void removeAllGeomObjects();

  /// Handles a new geometry object.
  void handleGeomObject(boost::shared_ptr<Core::Datatypes::GeometryObject> object);

  /// Performs a frame.
  void doFrame();

  /// Sets the mouse interaction mode.
  void setMouseMode(MouseMode mode);

  /// Retrieves mouse interaction mode.
  MouseMode getMouseMode();

  /// Performs an autoview.
  void doAutoView();

private:

  class SRObject
  {
  public:
    SRObject(const std::string& name, const spire::M44& objToWorld,
             const Core::Geometry::BBox& bbox) :
        mName(name),
        mObjectToWorld(objToWorld),
        mBBox(bbox)
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
      SRPass(const std::string& name) :
          passName(name)
      {}

      std::string                 passName;
      std::list<ObjectTransforms> transforms;
    };

    std::string           mName;
    spire::M44            mObjectToWorld;
    std::list<SRPass>     mPasses;
    Core::Geometry::BBox  mBBox;          ///< Objects bounding box (calculated from VBO).

    boost::optional<std::string>    mColorMap;
  };

  // Begins the frame.
  void beginFrame();

  void generateColormaps();

  std::shared_ptr<spire::Interface>           mSpire;

  MouseMode                 mMouseMode;       ///< Current mouse mode.

  size_t                    mScreenWidth;     ///< Screen width in pixels.
  size_t                    mScreenHeight;    ///< Screen height in pixels.

  GLuint                    mRainbowCMap;     ///< Rainbow color map.
  GLuint                    mGrayscaleCMap;   ///< Grayscale color map.

  std::unique_ptr<SRCamera> mCamera;          ///< Primary camera.
  std::vector<SRObject>     mSRObjects;       ///< All SCIRun objects.
  Core::Geometry::BBox      mSceneBBox;       ///< Scene's AABB. Recomputed per-frame.

  std::string               mArrowVBOName;    ///< VBO for one axis of the coordinate axes.
  std::string               mArrowIBOName;    ///< IBO for one axis of the coordinate axes.
  std::string               mArrowObjectName; ///< Object name for profile arrow.

};

} // namespace Gui
} // namespace SCIRun 

#endif 
