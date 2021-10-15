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
#include <stdio.h>

using namespace SCIRun::Render;
using namespace SCIRun::Core::Datatypes;

OSPRayDataManager::OSPRayDataManager()
{
  ospInit();
}

OSPRayDataManager::~OSPRayDataManager()
{
  ospShutdown();
}

OSPGeometry OSPRayDataManager::updateAndGetMesh(OsprayGeometryObject* obj, uint32_t vertsPerPoly)
{
  auto search = geomMap_.find(obj->id);
  if(search != geomMap_.end())
  {
    if(search->second.version == obj->version) return search->second.geometry;
    else                                  geomMap_.erase(search);
  }
  return addMesh(obj, vertsPerPoly);
}

OSPVolume OSPRayDataManager::updateAndgetStructuredVolume(OsprayGeometryObject* obj)
{
  auto search = geomMap_.find(obj->id);
  if(search != geomMap_.end())
  {
    if(search->second.version == obj->version) return search->second.volume;
    else                                  geomMap_.erase(search);
  }
  return addStructuredVolume(obj);
}

OSPGeometry OSPRayDataManager::addMesh(OsprayGeometryObject* obj, uint32_t vertsPerPoly)
{
  OsprayGeometryObject::FieldData& data = obj->data;
  float* vertices   = data.vertex.size()   > 0 ? data.vertex.data()   : nullptr;
  float* colors     = data.color.size()    > 0 ? data.color.data()    : nullptr;
  float* normals    = data.normal.size()   > 0 ? data.normal.data()   : nullptr;
  float* texCoords  = data.texCoord.size() > 0 ? data.texCoord.data() : nullptr;
  uint32_t* indices = data.index.size()    > 0 ? data.index.data()    : nullptr;

  uint32_t numVertices = data.vertex.size() / DIMENSIONS_;
  size_t numPolygons = data.index.size() / vertsPerPoly;

  geomMap_.emplace(obj->id, ObjectData());
  ObjectData* obj_data = &geomMap_[obj->id];

  obj_data->geometry = ospNewGeometry("mesh");
  obj_data->version = obj->version;

  std::vector<OSPData> sdata, odata;
  std::vector<std::string> paramName;
  if(vertices)
  {
    paramName.push_back("vertex.position");
    sdata.push_back(ospNewSharedData(vertices, OSP_VEC3F, numVertices));
    odata.push_back(ospNewData(OSP_VEC3F, numVertices));
  }

  if(normals)
  {
    paramName.push_back("vertex.normal");
    sdata.push_back(ospNewSharedData(normals, OSP_VEC3F, numVertices));
    odata.push_back(ospNewData(OSP_VEC3F, numVertices));
  }

  if(colors)
  {
    sdata.push_back(ospNewSharedData(colors, OSP_VEC4F, numVertices));
    paramName.push_back("vertex.color");
    odata.push_back(ospNewData(OSP_VEC4F, numVertices));
  }

  if(texCoords)
  {
    paramName.push_back("vertex.texcoord");
    sdata.push_back(ospNewSharedData(texCoords, OSP_VEC2F, numVertices));
    odata.push_back(ospNewData(OSP_VEC2F, numVertices));
  }

  if(indices)
  {
    paramName.push_back("index");
    OSPDataType dataType = vertsPerPoly == 3 ? OSP_VEC3UI : OSP_VEC4UI;
    sdata.push_back(ospNewSharedData(indices, dataType, numPolygons));
    odata.push_back(ospNewData(dataType, numPolygons));
  }

  for (int i = 0; i < sdata.size(); ++i)
  {
    ospCopyData(sdata[i], odata[i]);
    ospSetParam(obj_data->geometry, paramName[i].data(), OSP_DATA, &(odata[i]));
    ospRelease(sdata[i]);
    ospRelease(odata[i]);
  }

  ospCommit(obj_data->geometry);
  return obj_data->geometry;
}

OSPVolume OSPRayDataManager::addStructuredVolume(OsprayGeometryObject* obj)
{
  geomMap_.emplace(obj->id, ObjectData());
  ObjectData* obj_data = &geomMap_[obj->id];

  obj_data->volume = ospNewVolume("structuredRegular");
  obj_data->version = obj->version;

  ospSetParam(obj_data->volume, "gridOrigin", OSP_VEC3F, obj->data.origin);
  ospSetParam(obj_data->volume, "gridSpacing", OSP_VEC3F, obj->data.spacing);

  {
    OSPData sdata = ospNewSharedData(obj->data.color.data(), OSP_FLOAT, obj->data.dim[0], 0, obj->data.dim[1], 0, obj->data.dim[2], 0);
    OSPData odata = ospNewData(OSP_FLOAT, obj->data.dim[0], obj->data.dim[1], obj->data.dim[2]);
    ospCopyData(sdata, odata);
    ospRelease(sdata);

    ospSetParam(obj_data->volume, "data", OSP_DATA, &odata);
    ospRelease(odata);
  }

  ospCommit(obj_data->volume);
  return obj_data->volume;
}

OSPGeometry OSPRayDataManager::getMesh(uint64_t id)
{
  auto search = geomMap_.find(id);
  if(search != geomMap_.end())
    return search->second.geometry;
  return nullptr;
}

OSPVolume OSPRayDataManager::getVolume(uint64_t id)
{
  auto search = geomMap_.find(id);
  if(search != geomMap_.end())
    return search->second.volume;
  return nullptr;
}

void OSPRayDataManager::removeObject(uint64_t id)
{
  auto search = geomMap_.find(id);
  if(search != geomMap_.end())
    geomMap_.erase(search);
}
