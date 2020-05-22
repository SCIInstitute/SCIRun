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

#include <ospray/ospray.h>
#include <ospray/ospray_util.h>

#include "OSPRayDataManager.h"
#include "OSPRayCamera.h"
#include "Core/Datatypes/Geometry.h"

namespace SCIRun { namespace Render {

class OSPRayRenderer
{
public:
  OSPRayRenderer();
  virtual ~OSPRayRenderer();

  //Rendering---------------------------------------------------------------------------------------
  void renderFrame(); //renders frame with current OpenGL context

  //Interaction-------------------------------------------------------------------------------------
  void resize(int width, int height);
  void mousePress(float x, float y, MouseButton btn);
  void mouseMove(float x, float y, MouseButton btn);
  void mouseRelease();
  void mouseWheel(int delta);

  //Data--------------------------------------------------------------------------------------------
  void updateGeometries(const std::vector<Core::Datatypes::OsprayGeometryObjectHandle>& geometries);

  //Getters-----------------------------------------------------------------------------------------
  int width() {return width_;}
  int height() {return height_;}

private:
  void addGroup();
  void addInstaceOfGroup();
  void addMeshToGroup(uint64_t id, uint64_t version,
    Core::Datatypes::OsprayGeometryObject::FieldData& data, uint32_t vertsPerPoly,
    Core::Datatypes::OsprayGeometryObject::Material& material);
  void addMaterial(OSPGeometricModel model, Core::Datatypes::OsprayGeometryObject::Material& mat);

  static int osprayRendererInstances;
  static OSPRayDataManager dataManager;

  int width_  {16};
  int height_ {16};
  uint32_t framesAccumulated {0};

  bool isScivis {true};

  OSPFrameBuffer frameBuffer_ {nullptr};
  OSPRenderer    renderer_    {nullptr};
  OSPRayCamera*  camera_      {nullptr};
  OSPWorld       world_       {nullptr};


  OSPGroup group_ {nullptr};
};

}}
