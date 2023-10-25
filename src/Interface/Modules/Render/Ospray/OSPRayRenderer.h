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

#pragma once

#include <vector>
#include <cstdio>


#ifdef (WITH_OSPRAY) || defined(WITH_OSPRAY_LOCAL)
#include <ospray/ospray.h>
#include <ospray/ospray_util.h>

#include "OSPRayDataManager.h"
#include "OSPRayCamera.h"

#include <Core/Datatypes/Geometry.h>
#include <Interface/Modules/Render/Ospray/share.h>

namespace SCIRun { namespace Render {

class SCISHARE OSPRayRenderer
{
public:
  OSPRayRenderer();
  virtual ~OSPRayRenderer();

  //Rendering---------------------------------------------------------------------------------------
  void renderFrame(); //renders frame with current OpenGL context

  //Interaction-------------------------------------------------------------------------------------
  void resize(uint32_t width, uint32_t height);
  void mousePress(float x, float y, MouseButton btn);
  void mouseMove(float x, float y, MouseButton btn);
  void mouseRelease();
  void mouseWheel(int32_t delta);
  void autoView();

  //Data--------------------------------------------------------------------------------------------
  void updateGeometries(const std::vector<Core::Datatypes::OsprayGeometryObjectHandle>& geometries);

  //Getters-----------------------------------------------------------------------------------------
  uint32_t width() {return width_;}
  uint32_t height() {return height_;}

private:
  void addGroup();
  void addInstaceOfGroup();
  void addMeshToGroup(Core::Datatypes::OsprayGeometryObject* geometryObject, uint32_t vertsPerPoly);
  void addStructuredVolumeToGroup(Core::Datatypes::OsprayGeometryObject* geometryObject);
  void addMaterial(OSPGeometricModel model, Core::Datatypes::OsprayGeometryObject::Material& mat);
  void addTransferFunction(OSPVolumetricModel model,  Core::Datatypes::OsprayGeometryObject::TransferFunc& transFunc);
  void addDirectionalLight(glm::vec3 col, glm::vec3 dir);
  void addAmbientLight(glm::vec3 col, float intensity);
  void addSphereLight(glm::vec3 col, glm::vec3 position, float radius, float intensity);
  void addQuadLight(glm::vec3 col, glm::vec3 position, glm::vec3 edge1, glm::vec3 edge2, float intensity);
  void setLightsAsObject();

  static int osprayRendererInstances;
  static OSPRayDataManager dataManager;
  std::vector<OSPLight> lights_;

  uint32_t width_  {16};
  uint32_t height_ {16};
  uint32_t framesAccumulated {0};

  bool isScivis {true};

  OSPFrameBuffer frameBuffer_ {nullptr};
  OSPRenderer    renderer_    {nullptr};
  OSPRayCamera*  camera_      {nullptr};
  OSPCamera      parentCamera_{nullptr};
  OSPWorld       world_       {nullptr};


  OSPGroup group_ {nullptr};
};

}}

#endif
