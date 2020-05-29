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

#include "OSPRayDataManager.h"

#include <cstring>

using namespace SCIRun::Render;

OSPRayDataManager::OSPRayDataManager()
{
  ospInit();
}

OSPRayDataManager::~OSPRayDataManager()
{
  ospShutdown();
}

OSPGeometry OSPRayDataManager::updateAndGetMesh(uint64_t id, uint64_t version,
  float* vertices, float* normals, float* colors, float* texCoords, uint32_t* indices,
  size_t numVertices, size_t numPolygons, uint32_t vertsPerPoly)
{
  auto search = geomMap.find(id);
  if(search != geomMap.end())
  {
    if(search->second.version == version) return search->second.geometry;
    else                                  geomMap.erase(search);
  }
  return addMesh(id, version, vertices, normals, colors, texCoords, indices, numVertices, numPolygons, vertsPerPoly);
}

OSPVolume OSPRayDataManager::updateAndgetStructuredVolume(uint64_t id, uint64_t version,
  float gridOrigin[3], float gridSpacing[3], uint32_t dataSize[3], float* data)
{
  auto search = geomMap.find(id);
  if(search != geomMap.end())
  {
    if(search->second.version == version) return search->second.volume;
    else                                  geomMap.erase(search);
  }
  return addStructuredVolume(id, version, gridOrigin, gridSpacing, dataSize, data);
}

OSPGeometry OSPRayDataManager::addMesh(uint64_t id, uint64_t version, float* vertices, float* normals, float* colors,
  float* texCoords, uint32_t* indices, size_t numVertices, size_t numPolygons, uint32_t vertsPerPoly)
{
  geomMap.emplace(id, ObjectData());
  ObjectData* obj = &geomMap[id];

  obj->geometry = ospNewGeometry("mesh");
  obj->version = version;

  if(vertices)
  {
    printf("v\n");

    OSPData sdata = ospNewSharedData(vertices, OSP_VEC3F, numVertices);
    OSPData odata = ospNewData(OSP_VEC3F, numVertices);
    ospCopyData(sdata, odata);

    ospSetParam(obj->geometry, "vertex.position", OSP_DATA, &odata);
    ospRelease(sdata);
    ospRelease(odata);
  }

  if(normals)
  {
    printf("n\n");

    OSPData sdata = ospNewSharedData(normals, OSP_VEC3F, numVertices);
    OSPData odata = ospNewData(OSP_VEC3F, numVertices);
    ospCopyData(sdata, odata);
    ospRelease(sdata);

    ospSetParam(obj->geometry, "vertex.normal", OSP_DATA, &odata);
    ospRelease(odata);
  }

  if(colors)
  {
    printf("c\n");

    OSPData sdata = ospNewSharedData(colors, OSP_VEC4F, numVertices);
    OSPData odata = ospNewData(OSP_VEC4F, numVertices);
    ospCopyData(sdata, odata);
    ospRelease(sdata);

    ospSetParam(obj->geometry, "vertex.color", OSP_DATA, &odata);
    ospRelease(odata);
  }

  if(texCoords)
  {
    printf("t\n");

    OSPData sdata = ospNewSharedData(texCoords, OSP_VEC2F, numVertices);
    OSPData odata = ospNewData(OSP_VEC2F, numVertices);
    ospCopyData(sdata, odata);
    ospRelease(sdata);

    ospSetParam(obj->geometry, "vertex.texcoord", OSP_DATA, &odata);
    ospRelease(odata);
  }

  if(indices)
  {
    printf("i\n");

    OSPDataType dataType = vertsPerPoly == 3 ? OSP_VEC3UI : OSP_VEC4UI;
    OSPData sdata = ospNewSharedData(indices, dataType, numPolygons);
    OSPData odata = ospNewData(dataType, numPolygons);
    ospCopyData(sdata, odata);
    ospRelease(sdata);

    ospSetParam(obj->geometry, "index", OSP_DATA, &odata);
    ospRelease(odata);
  }

  printf("\n");

  ospCommit(obj->geometry);
  return obj->geometry;
}

OSPVolume OSPRayDataManager::addStructuredVolume(uint64_t id, uint64_t version, float gridOrigin[3],
  float gridSpacing[3], uint32_t dataSize[3], float* data)
{
  geomMap.emplace(id, ObjectData());
  ObjectData* obj = &geomMap[id];

  obj->volume = ospNewVolume("structuredRegular");
  obj->version = version;

  ospSetParam(obj->volume, "gridOrigin", OSP_VEC3F, gridOrigin);
  ospSetParam(obj->volume, "gridSpacing", OSP_VEC3F, gridSpacing);

  {
    OSPData sdata = ospNewSharedData(data, OSP_FLOAT, dataSize[0], 0, dataSize[1], 0, dataSize[2], 0);
    OSPData odata = ospNewData(OSP_FLOAT, dataSize[0], dataSize[1], dataSize[2]);
    ospCopyData(sdata, odata);
    ospRelease(sdata);

    ospSetParam(obj->volume, "data", OSP_DATA, &odata);
    ospRelease(odata);
  }

  ospCommit(obj->volume);
  return obj->volume;
}

OSPGeometry OSPRayDataManager::getMesh(uint64_t id)
{
  auto search = geomMap.find(id);
  if(search != geomMap.end())
    return search->second.geometry;
  return nullptr;
}

OSPVolume OSPRayDataManager::getVolume(uint64_t id)
{
  auto search = geomMap.find(id);
  if(search != geomMap.end())
    return search->second.volume;
  return nullptr;
}

void OSPRayDataManager::removeObject(uint64_t id)
{
  auto search = geomMap.find(id);
  if(search != geomMap.end())
    geomMap.erase(search);
}
