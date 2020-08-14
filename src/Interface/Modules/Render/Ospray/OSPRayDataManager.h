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

#include <map>

#include <ospray/ospray.h>
#include <ospray/ospray_util.h>

namespace SCIRun { namespace Render {

class OSPRayDataManager
{
public:
  OSPRayDataManager();
  virtual ~OSPRayDataManager();

  OSPGeometry updateAndGetMesh(uint64_t id, uint64_t version,
    float* vertices, float* normals, float* colors, float* texCoords, uint32_t* indices,
    size_t numVertices, size_t numPolygons, uint32_t vertsPerPoly);
  OSPGeometry addMesh(uint64_t id, uint64_t version, float* vertices, float* normals, float* colors,
    float* texCoords, uint32_t* indices, size_t numVertices, size_t numPolygons, uint32_t vertsPerPoly);


  OSPVolume updateAndgetStructuredVolume(uint64_t id, uint64_t version, float gridOrigin[3],
    float gridSpacing[3], uint32_t dataSize[3], float* data);
  OSPVolume addStructuredVolume(uint64_t id, uint64_t version, float gridOrigin[3],
    float gridSpacing[3], uint32_t dataSize[3], float* data);

  OSPGeometry getMesh(uint64_t id);
  OSPVolume getVolume(uint64_t id);

  void removeObject(uint64_t id);

private:


  struct ObjectData
  {
    uint64_t    version      {0};
    union
    {
      OSPObject   object;
      OSPGeometry geometry;
      OSPVolume   volume;
    };

    ObjectData()
    {
      object = nullptr;
    }

    ~ObjectData()
    {
      if(object)
      {
        //printf("Delete Version: %d\n", version);
        ospRelease(object);
      }
    }
  };

  std::map<uint64_t, ObjectData> geomMap;
};

}}
