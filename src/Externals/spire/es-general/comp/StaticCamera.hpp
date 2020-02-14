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


#ifndef SPIRE_GENERAL_COMPONENTS_STATIC_CAMERA_HPP
#define SPIRE_GENERAL_COMPONENTS_STATIC_CAMERA_HPP

#include <es-log/trace-log.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <es-cereal/ComponentSerialize.hpp>
#include <tuple>
#include <spire/scishare.h>

namespace gen {

// Structure for shared camera data. Shared with StaticOrthoCamera.
// This data is used to pass into functions expecting camera data, but don't
// care anything about the camera itself.
struct StaticCameraData
{
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 viewProjection;

  // Misc variables. fovy only applies to perspective matrices.
  float fovy;
  float aspect;
  float znear;
  float zfar;

  // Information regarding the truncated pyramid this projection represents
  glm::vec3 nearTopLeft;  // (+,+)
  //glm::vec3 nearBotRight; // (-,-) <-- This can be calculated by negating x/y in nearTopLeft

  glm::vec3 farTopLeft;   // (+,+)
  //glm::vec3 farBotRight;  // (-,-) <-- This can be calculated by negating x/y in farTopLeft

  // Information related to 'plane of play' if we are 2.5D.
  float zpp;        ///< z plane of play (exact Z value to use).
  float zppHWidth;  ///< z plane of play half width.
  float zppHHeight; ///< z plane of play half height.
  float winWidth;   ///< window width

  StaticCameraData()
  {
    fovy   = 0.0f;
    aspect = 0.0f;
    znear  = 0.0f;
    zfar   = 0.0f;

    zpp        = 0.0f;
    zppHWidth  = 0.0f;
    zppHHeight = 0.0f;
    winWidth   = 0.0f;
  }

  void setView(const glm::mat4& view_in)
  {
    view = view_in;
    viewProjection = projection * view;
  }

  void setProjection(const glm::mat4& projection_in, float fovy_in, float aspect_in,
                     float znear_in, float zfar_in)
  {
    projection = projection_in;
    fovy       = fovy_in;
    aspect     = aspect_in;
    znear      = znear_in;
    zfar       = zfar_in;

    // Setup frustum details.
		float zDist  = fabs(zfar - znear);
		float tanVal = tanf(0.5f * (fovy));
		//for (int i = 0; i < mNumFrustumSections + 1; i++)

    // Nearest point (we will calculate farthest points and we will be able to
    // calculate the full frustum based on this)
    {
      // Projection matrix is down negative z.
			float zs = -znear;

			// For width and height, we use the farthest point away, which will have
      // the larger compontents in all directions.
			float ywh = fabs(zs) * tanVal;	// y width half -- multiplycation by fabs(zs)
                                      // removes the 'adjacent' component in o/a for tan.
			float xwh = (ywh * aspect);

			// NOTE! The order in which the local points are specified matters! Specifically, the first point, array element
			// index 0, both components MUST be positive.
			// Win32 never rotates the screen.
      nearTopLeft   = glm::vec3( xwh, ywh, zs);
      //nearBotRight  = glm::vec3(-xwh,-ywh, zs);
    }

    // Farthest point
    {
      float zs = -znear - zDist;

      float ywh = fabs(zs) * tanVal;
      float xwh = (ywh * aspect);

      farTopLeft  = glm::vec3( xwh, ywh, zs );
      //farBotRight = glm::vec3(-xwh,-ywh, zs );
    }
  }

  void setOrthoProjection(const glm::mat4& orthoProj, float aspect_in,
                          float width, float height,
                          float znear_in, float zfar_in)
  {
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    fovy   = 0.0f;
    aspect = aspect_in;
    znear  = znear_in;
    zfar   = zfar_in;

    projection = orthoProj;

    // Nearest plane.
    {
      float zs = -znear;

			float xwh = halfWidth;
			float ywh = halfHeight;

      nearTopLeft   = glm::vec3( xwh, ywh, zs );
      //nearBotRight  = glm::vec3(-xwh,-ywh, zs );
    }

    // Farest pane.
    {
      float zs = -znear - zfar;

      float xwh = halfWidth;
      float ywh = halfHeight;

      farTopLeft  = glm::vec3( xwh, ywh, zs );
      //farBotRight = glm::vec3(-xwh,-ywh, zs );
    }
  }

  // Returns the half target plane width and height.
  std::tuple<float, float> getFrustumPlaneWidthHeightAtZ(float z)
  {
    float distFromNearPlane = (-z) - znear;
    float ratio = distFromNearPlane / (zfar - znear);
    if (ratio > 1.0f)
    {
      std::cerr << "Warning: Attempting to get frustum plane passed the far plane." << std::endl;
      ratio = 1.0f;
    }
    else if (ratio < 0.0f)
    {
      std::cerr << "Warning: Attempting to get frustum plane in front of the near plane." << std::endl;
      ratio = 0.0f;
    }

    // Interpolate closest width / height with farthest width / height in a linear fashion.
    float nw = fabs(nearTopLeft.x);
    float nh = fabs(nearTopLeft.y);
    float fw = fabs(farTopLeft.x);
    float fh = fabs(farTopLeft.y);

    float hwidth	= (1.0f - ratio) * nw + ratio * fw;
    float hheight	= (1.0f - ratio) * nh + ratio * fh;

    // Return location of frustum in local coordinates along the z axis.
    return std::make_tuple(hwidth, hheight);
  }


  /// Get plane of play desired width. Returns the Z depth as the first tuple
  /// parameter and the resulting height as the second parameter
  /// \param dhw  Desired half width.
  std::tuple<float, float> getPPDesWidth(float width)
  {
    float dhw = width / 2.0f; // Desired half width

    // Interpolate closest width / height with farthest width / height in a linear fashion.
    float nw = fabs(nearTopLeft.x);
    float nh = fabs(nearTopLeft.y);
    float fw = fabs(farTopLeft.x);
    float fh = fabs(farTopLeft.y);

    if (dhw < nw)
    {
      std::cerr << "Desired half width is less than the near plane width." << std::endl;
      dhw = nw;
    }

    if (dhw > fw)
    {
      std::cerr << "Desired half width is greater than the far plane width." << std::endl;
      dhw = fw;
    }

    // Determine the ratio from dhw
    float ratio = (dhw - nw) / (fw - nw);

    // Return values
    return std::make_tuple(-znear - (zfar - znear) * ratio,
                           (1.0f - ratio) * nh + ratio * fh);
  }

  // This function will also calculate the extents of the plane of play given
  // the current settings.
  void setZPlaneOfPlay(float z)
  {
    zpp = z;

    auto halfWH = getFrustumPlaneWidthHeightAtZ(z);
    zppHWidth = std::get<0>(halfWH);
    zppHHeight = std::get<1>(halfWH);
  }

  glm::mat4 getInverseView() const
  {
    return glm::affineInverse(view);
  }
};

struct StaticCamera
{
  // -- Data --
  StaticCameraData data;

  // -- Functions --
  static const char* getName() {return "gen:StaticCamera";}

  bool serialize(spire::ComponentSerialize& /* s */, uint64_t /* entityID */)
  {
    /// \todo Serialize camera settings.
    return true;
  }
};

} // namespace gen

#endif
