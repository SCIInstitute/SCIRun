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

ShowMeshModule::ShowMeshModule() : Module(ModuleLookupInfo("ShowMesh", "Visualization", "SCIRun")) 
{
  get_state()->setValue(ShowEdges, true);
  get_state()->setValue(ShowFaces, true);
}

void ShowMeshModule::execute()
{
  auto mesh = getRequiredInput(Mesh);
  MeshFacadeHandle facade(mesh->getFacade());

  /// \todo Determine a better way of handling all of the various object state.
  bool showEdges = get_state()->getValue(ShowEdges).getBool();
  bool showFaces = get_state()->getValue(ShowFaces).getBool();
  bool zTestOn = get_state()->getValue(ZTestOn).getBool();
  bool nodeTransparency = get_state()->getValue(NodeTransparency).getBool();
  bool edgeTransparency = get_state()->getValue(EdgeTransparency).getBool();
  bool faceTransparency = get_state()->getValue(FaceTransparency).getBool();

  GeometryHandle geom(new GeometryObject(mesh));
  geom->objectName = get_id();

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
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  size_t vboSize = sizeof(float) * 3 * facade->numNodes();
  rawVBO->resize(vboSize); // linear complexity.
  float* vbo = reinterpret_cast<float*>(&(*rawVBO)[0]); // Remember, standard guarantees that vectors are contiguous in memory.

  // Add shared VBO to the geometry object.
  std::string primVBOName = "primaryVBO";
  std::vector<std::string> attribs;   ///< \todo Switch to initializer lists when msvc supports it.
  attribs.push_back("aPos");          ///< \todo Find a good place to pull these names from.
  geom->mVBOs.emplace_back(GeometryObject::SpireVBO(primVBOName, attribs, rawVBO));

  // Build index buffer. Based off of the node indices that came out of old
  // SCIRun, TnL cache coherency will be poor. Maybe room for improvement later.
  size_t i = 0;
  BOOST_FOREACH(const NodeInfo& node, facade->nodes())
  {
    vbo[i+0] = node.point().x(); vbo[i+1] = node.point().y(); vbo[i+2] = node.point().z();
    i+=3;
  }


  // Build the edges
  size_t iboEdgesSize = sizeof(uint32_t) * facade->numEdges() * 2;
  uint32_t* iboEdges = nullptr;
  if (showEdges)
  {
    std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
    //rawIBO->reserve(iboEdgesSize);
    rawIBO->resize(iboEdgesSize);   // Linear in complexity... If we need more performance,
                                    // use malloc to generate buffer and then vector::assign.
    iboEdges = reinterpret_cast<uint32_t*>(&(*rawIBO)[0]);
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

    // Add IBO for the faces.
    std::string edgesIBOName = "edgesIBO";
    geom->mIBOs.emplace_back(GeometryObject::SpireIBO(edgesIBOName, sizeof(uint32_t), rawIBO));

    // Build pass for the faces.
    /// \todo Find an appropriate place to put program names like UniformColor.
    GeometryObject::SpirePass pass = 
        GeometryObject::SpirePass("edgesPass", primVBOName,
                                  edgesIBOName, "UniformColor",
                                  Spire::StuInterface::LINES);

    // Add appropriate uniforms to the pass (in this case, uColor).
    if (edgeTransparency)
      pass.addUniform("uColor", Spire::V4(0.6f, 0.6f, 0.6f, 0.5f));
    else
      pass.addUniform("uColor", Spire::V4(0.6f, 0.6f, 0.6f, 1.0f));

    geom->mPasses.emplace_back(pass);
  }
  //mStuInterface->addPassUniform(obj1, pass1, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f));

  // Build the faces.
  size_t iboFacesSize = sizeof(uint32_t) * facade->numFaces() * 6;
  uint32_t* iboFaces = nullptr;
  if (showFaces)
  {
    std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
    //rawIBO->reserve(iboFacesSize);
    rawIBO->resize(iboFacesSize);   // Linear in complexity... If we need more performance,
                                    // use malloc to generate buffer and then vector::assign.
    iboFaces = reinterpret_cast<uint32_t*>(&(*rawIBO)[0]);
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

    // Add IBO for the faces.
    std::string facesIBOName = "facesIBO";
    geom->mIBOs.emplace_back(GeometryObject::SpireIBO(facesIBOName, sizeof(uint32_t), rawIBO));

    // Build pass for the faces.
    /// \todo Find an appropriate place to put program names like UniformColor.
    GeometryObject::SpirePass pass = 
        GeometryObject::SpirePass("facesPass", primVBOName,
                                  facesIBOName, "UniformColor",
                                  Spire::StuInterface::TRIANGLES);
    if (faceTransparency)
      pass.addUniform("uColor", Spire::V4(1.0f, 1.0f, 1.0f, 0.4f));
    else
      pass.addUniform("uColor", Spire::V4(1.0f, 1.0f, 1.0f, 1.0f));

    geom->mPasses.emplace_back(pass);
  }

  sendOutput(SceneGraph, geom);
}

AlgorithmParameterName ShowMeshModule::ShowEdges("Show edges");
AlgorithmParameterName ShowMeshModule::ShowFaces("Show faces");
AlgorithmParameterName ShowMeshModule::ZTestOn("Z Test");
AlgorithmParameterName ShowMeshModule::NodeTransparency("Node Transparency");
AlgorithmParameterName ShowMeshModule::EdgeTransparency("Edge Transparency");
AlgorithmParameterName ShowMeshModule::FaceTransparency("Face Transparency");
