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
#include "namespaces.h"
#include "spire/Interface.h"
#include "spire/src/LambdaInterface.h"
#include "spire/src/ObjectLambda.h"

namespace CPM_SPIRE_SCIRUN_NS {

class SRCamera;
class SciBall;

/// A wrapper around spire that provides higher level functionality required
/// to operate SCIRun.
/// \todo Think about how we are going to break apart this class.
///       There will be a lot of functionality behind it.
class SRInterface : public spire::Interface
{
public:
  SRInterface(std::shared_ptr<spire::Context> context,
              const std::vector<std::string>& shaderDirs,
              bool createThread, LogFunction logFP = LogFunction());
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


private:

  /// Calculates the screen space coordinates given the window coordinates.
  spire::V2 calculateScreenSpaceCoords(const glm::ivec2& mousePos);

  /// Recalculates camera transform using the most relevant data.
  void buildAndApplyCameraTransform();

  float                     mCamDistance;   ///< Camera's distance from the origin.

  size_t                    mScreenWidth;   ///< Screen width in pixels.
  size_t                    mScreenHeight;  ///< Screen height in pixels.

  spire::V2                 mTransClick;    ///< Start of the translation.

  spire::V3                 mCamAccumPosDown; ///< Accumulated translation / zoom when first clicked down.
  spire::V3                 mCamAccumPosNow;  ///< Accumulated translation / zoom currently.

  MouseButton               mActiveDrag;    ///< The button we are currently dragging.

  std::unique_ptr<SRCamera> mCamera;        ///< Primary camera.
  std::unique_ptr<SciBall>  mSciBall;       ///< SCIRun 4's arcball camera.

};

} // namespace CPM_SPIRE_SCIRUN_NS

#endif 
