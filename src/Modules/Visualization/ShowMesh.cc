/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <Modules/Visualization/ShowMesh.h>
#include <Core/Datatypes/Mesh/Mesh.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/Datatypes/Geometry.h>
#include <boost/foreach.hpp>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

ShowMeshModule::ShowMeshModule() : Module(ModuleLookupInfo("ShowMesh", "Visualization", "SCIRun")) {}

void ShowMeshModule::execute()
{
  auto mesh = getRequiredInput(Mesh);
  MeshFacadeHandle facade(mesh->getFacade());

  bool showEdges = get_state()->getValue(ShowEdges).getBool();
  bool showfaces = get_state()->getValue(ShowFaces).getBool();
  bool zTestOn = get_state()->getValue(ZTestOn).getBool();

  /// \todo Split the mesh into chunks of about ~32,000 vertices. May be able to
  ///       eek out better coherency and use a 16 bit index buffer instead of
  ///       a 32 bit index buffer.

  // We are going to get no lighting in this first pass. The unfortunate reality
  // is that I cannot get access to face normals in vertex shaders based off of
  // the winding orders of the incoming geometry.

  // Allocate memory for vertex buffer (*NOT* the index buffer, which is a
  // a function of the number of faces). Only allocating enough memory to hold
  // points associated with the faces.
  // Edges *and* faces should use the same vbo!
  size_t vboSize = sizeof(float) * 3 * facade->numNodes();
  float* vbo = static_cast<float*>(std::malloc(vboSize));

  // Build index buffer. Based off of the node indices that came out of old
  // SCIRun, TnL cache coherency will be poor. Maybe room for improvement later.
  size_t i = 0;
  BOOST_FOREACH(const NodeInfo& node, facade->nodes())
  {
    vbo[i+0] = node.point().x(); vbo[i+1] = node.point().y(); vbo[i+2] = node.point().z();
    i+=3;
  }

  // Build the faces.
  size_t iboFacesSize = sizeof(uint32_t) * facade->numFaces() * 6;
  uint32_t* iboFaces = static_cast<uint32_t*>(std::malloc(iboFacesSize));
  i = 0;
  BOOST_FOREACH(const FaceInfo& face, facade->faces())
  {
    // There should *only* be four indicies.
    VirtualMesh::Node::array_type nodes = face.nodeIndices();
    assert(nodes.size() == 4);
    // Winding order looks good from tests.
    // Render two triangles.
    iboFaces[i  ] = nodes[0]; iboFaces[i+1] = nodes[1]; iboFaces[i+2] = nodes[2];
    iboFaces[i+3] = nodes[0]; iboFaces[i+4] = nodes[2]; iboFaces[i+5] = nodes[3];
    i += 6;
  }

  // Build the edges
  size_t iboEdgesSize = sizeof(uint32_t) * facade->numEdges() * 2;
  uint32_t* iboEdges = static_cast<uint32_t*>(std::malloc(iboEdgesSize));
  i = 0;
  BOOST_FOREACH(const EdgeInfo& edge, facade->edges())
  {
    // There should *only* be two indicies (linestrip would be better...)
    VirtualMesh::Node::array_type nodes = edge.nodeIndices();
    assert(nodes.size() == 2);
    // Winding order looks good from tests.
    // Render two triangles.
    iboEdges[i] = nodes[0]; iboEdges[i+1] = nodes[1];
    i += 2;
  }

  GeometryHandle geom(new GeometryObject(mesh));
  geom->vboCommon = (uint8_t*)vbo;
  geom->vboCommonSize = vboSize;
  geom->iboFaces = (uint8_t*)iboFaces;
  geom->iboFacesSize = iboFacesSize;
  geom->iboEdges = (uint8_t*)iboEdges;
  geom->iboEdgesSize = iboEdgesSize;
  sendOutput(SceneGraph, geom);
}

AlgorithmParameterName ShowMeshModule::ShowEdges("Show edges");
AlgorithmParameterName ShowMeshModule::ShowFaces("Show faces");
AlgorithmParameterName ShowMeshModule::ZTestOn("Z Test");