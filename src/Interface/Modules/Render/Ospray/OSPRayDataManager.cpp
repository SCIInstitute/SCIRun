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

}

OSPRayDataManager::~OSPRayDataManager()
{

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
  addMesh(id, version, vertices, normals, colors, texCoords, indices, numVertices, numPolygons, vertsPerPoly);
}

OSPGeometry OSPRayDataManager::addMesh(uint64_t id, uint64_t version, float* vertices, float* normals, float* colors,
  float* texCoords, uint32_t* indices, size_t numVertices, size_t numPolygons, uint32_t vertsPerPoly)
{
  geomMap.emplace(id, MeshData());
  MeshData* mesh = &geomMap[id];

  //mesh->numVertices = numVertices;
  //mesh->numPolygons = numPolygons;
  //mesh->vertsPerPoly = vertsPerPoly;

  mesh->geometry = ospNewGeometry("mesh");
  mesh->version = version;

  if(vertices)
  {
    printf("v\n");

    OSPData sdata = ospNewSharedData(vertices, OSP_VEC3F, numVertices);
    OSPData odata = ospNewData(OSP_VEC3F, numVertices);
    ospCopyData(sdata, odata);

    ospSetParam(mesh->geometry, "vertex.position", OSP_DATA, &odata);
    ospRelease(sdata);
    ospRelease(odata);

    /*
    size_t vertexArraySize = mesh->numVertices * 3;
    mesh->vertices = new float[vertexArraySize];
    memcpy(mesh->vertices, vertices, vertexArraySize * sizeof(float));

    OSPData data = ospNewSharedData(mesh->vertices, OSP_VEC3F, mesh->numVertices);
    ospCommit(data);
    ospSetParam(mesh->geometry, "vertex.position", OSP_DATA, &data);
    ospRelease(data);
    */
  }

  if(normals)
  {
    printf("n\n");

    OSPData sdata = ospNewSharedData(normals, OSP_VEC3F, numVertices);
    OSPData odata = ospNewData(OSP_VEC3F, numVertices);
    ospCopyData(sdata, odata);
    ospRelease(sdata);

    ospSetParam(mesh->geometry, "vertex.normal", OSP_DATA, &odata);
    ospRelease(odata);

    /*
    size_t normalArraySize = mesh->numVertices * 3;
    mesh->normals = new float[normalArraySize];
    memcpy(mesh->normals, normals, normalArraySize * sizeof(float));

    OSPData data = ospNewSharedData(mesh->normals, OSP_VEC3F, mesh->numVertices);
    ospCommit(data);
    ospSetParam(mesh->geometry, "vertex.normal", OSP_DATA, &data);
    ospRelease(data);
    */
  }

  if(colors)
  {
    printf("c\n");

    OSPData sdata = ospNewSharedData(colors, OSP_VEC4F, numVertices);
    OSPData odata = ospNewData(OSP_VEC4F, numVertices);
    ospCopyData(sdata, odata);
    ospRelease(sdata);

    ospSetParam(mesh->geometry, "vertex.color", OSP_DATA, &odata);
    ospRelease(odata);

    /*
    size_t colorArraySize = mesh->numVertices * 4;
    mesh->colors = new float[colorArraySize];
    memcpy(mesh->colors, colors, colorArraySize * sizeof(float));

    OSPData data = ospNewSharedData(mesh->colors, OSP_VEC4F, mesh->numVertices);
    ospCommit(data);
    ospSetParam(mesh->geometry, "vertex.color", OSP_DATA, &data);
    ospRelease(data);
    */
  }

  if(texCoords)
  {
    printf("t\n");

    OSPData sdata = ospNewSharedData(texCoords, OSP_VEC2F, numVertices);
    OSPData odata = ospNewData(OSP_VEC2F, numVertices);
    ospCopyData(sdata, odata);
    ospRelease(sdata);

    ospSetParam(mesh->geometry, "vertex.texcoord", OSP_DATA, &odata);
    ospRelease(odata);

    /*
    size_t texCoordArraySize = mesh->numVertices * 2;
    mesh->texCoords = new float[texCoordArraySize];
    memcpy(mesh->texCoords, colors, texCoordArraySize * sizeof(float));

    OSPData data = ospNewSharedData(mesh->texCoords, OSP_VEC2F, mesh->numVertices);
    ospCommit(data);
    ospSetParam(mesh->geometry, "vertex.texcoord", OSP_DATA, &data);
    ospRelease(data);
    */
  }

  if(indices)
  {
    printf("i\n");

    OSPDataType dataType = vertsPerPoly == 3 ? OSP_VEC3UI : OSP_VEC4UI;
    OSPData sdata = ospNewSharedData(indices, dataType, numPolygons);
    OSPData odata = ospNewData(dataType, numPolygons);
    ospCopyData(sdata, odata);
    ospRelease(sdata);

    ospSetParam(mesh->geometry, "index", OSP_DATA, &odata);
    ospRelease(odata);

    /*
    size_t indexArraySize = mesh->numPolygons * mesh->vertsPerPoly;
    mesh->indices = new uint32_t[indexArraySize];
    memcpy(mesh->indices, indices, indexArraySize * sizeof(uint32_t));

    OSPDataType dataType = vertsPerPoly == 3 ? OSP_VEC3UI : OSP_VEC4UI;
    OSPData data = ospNewSharedData(mesh->indices, dataType, mesh->numPolygons);
    ospCommit(data);
    ospSetParam(mesh->geometry, "index", OSP_DATA, &data);
    ospRelease(data);
    */
  }

  printf("\n");

  ospCommit(mesh->geometry);
  return mesh->geometry;
}

OSPGeometry OSPRayDataManager::getMesh(uint64_t id)
{
  auto search = geomMap.find(id);
  if(search != geomMap.end())
    return search->second.geometry;
  return nullptr;
}

void OSPRayDataManager::removeMesh(uint64_t id)
{
  auto search = geomMap.find(id);
  if(search != geomMap.end())
    geomMap.erase(search);
}
