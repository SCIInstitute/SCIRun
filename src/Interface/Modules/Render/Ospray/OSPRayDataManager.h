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
  OSPGeometry getMesh(uint64_t id);
  void removeMesh(uint64_t id);

private:


  struct MeshData
  {
    //float*      vertices     {nullptr};
    //float*      normals      {nullptr};
    //float*      colors       {nullptr};
    //float*      texCoords    {nullptr};
    //uint32_t*   indices      {nullptr};
    //size_t      numVertices  {0};
    //size_t      numPolygons  {0};
    //uint32_t    vertsPerPoly {0};
    uint64_t    version      {0};
    OSPGeometry geometry     {nullptr};

    MeshData()
    {
      //geometry = ospNewGeometry("mesh");
    }
    ~MeshData()
    {
      if(geometry) ospRelease(geometry);
      //if(vertices) delete[] vertices;
      //if(normals) delete[] normals;
      //if(colors) delete[] colors;
      //if(indices) delete[] indices;
    }
  };

  std::map<uint64_t, MeshData> geomMap;
};

}}
