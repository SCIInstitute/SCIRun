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

#include <Modules/Visualization/ShowField.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/Datatypes/Color.h>

#include <boost/foreach.hpp>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;


namespace SCIRun {
  namespace Modules {
    namespace Visualization {

      class ShowFieldModuleImpl
      {
      public:
        explicit ShowFieldModuleImpl(SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc updater) : updater_(updater) {}
        GeometryHandle renderMesh(boost::shared_ptr<SCIRun::Field> field,
          ModuleStateHandle state, 
          const std::string& id);
      private:
        /// Constructs faces without normal information. We can share the primary
        /// VBO with the nodes and the edges in this case.
        template <typename VMeshType>
        void buildFacesNoNormals(typename SCIRun::Core::Datatypes::MeshTraits<VMeshType>::MeshFacadeHandle facade,
          SCIRun::Core::Datatypes::GeometryHandle geom,
          const std::string& primaryVBOName,
          float dataMin, float dataMax,
          ModuleStateHandle state);

        /// Constructs edges without normal information. We can share the primary
        /// VBO with faces and nodes.
        template <typename VMeshType>
        void buildEdgesNoNormals(typename SCIRun::Core::Datatypes::MeshTraits<VMeshType>::MeshFacadeHandle facade,
          SCIRun::Core::Datatypes::GeometryHandle geom,
          const std::string& primaryVBOName,
          ModuleStateHandle state);

        /// Constructs nodes without normal information. We can share the primary
        /// VBO with edges and faces.
        template <typename VMeshType>
        void buildNodesNoNormals(typename SCIRun::Core::Datatypes::MeshTraits<VMeshType>::MeshFacadeHandle facade,
          SCIRun::Core::Datatypes::GeometryHandle geom,
          const std::string& primaryVBOName,
          ModuleStateHandle state);

        SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc updater_;
      };

    }}}

ShowFieldModule::ShowFieldModule() : Module(ModuleLookupInfo("ShowField", "Visualization", "SCIRun")),
  impl_(new ShowFieldModuleImpl(getUpdaterFunc()))
{
}

void ShowFieldModule::execute()
{
  boost::shared_ptr<SCIRun::Field> field = getRequiredInput(Field);

  //pass in the field object, get vmesh, vfield, and facade
  // template<class T>  inline void VField::get_value(T& val, VMesh::Node::index_type idx) const
  //normals
  //virtual void VMesh::get_normal(Core::Geometry::Vector& norm,Node::index_type i) const;

  /*

  VMesh* v = field->vmesh();
  
  if (vmesh->has_normals())
    vmesh_->synchronize(Mesh::NORMALS_E);

  BOOST_FOREACH(const NodeInfo<VMeshType>& node, facade->nodes())
  {
    iboNodes[i] = static_cast<uint32_t>(node.index());
    i++;

    //data 
    double val;
    vfield->get_value(val, node.index());

    if (vmesh->has_normals())
    {
      Vector normal;
      vmesh->get_normal(normal, node.index());
    }

  } 
  */

  {
    //TODO
    ColorRGB color = any_cast_or_default<ColorRGB>(get_state()->getTransientValue(DefaultMeshColor.name_));
    std::cout << "Default mesh color is: " << color << std::endl;
  }

  GeometryHandle geom = impl_->renderMesh(field, get_state(), get_id());

  sendOutput(SceneGraph, geom);
}

/// \todo Merge ShowMesh and ShowField. The only difference between ShowMesh
///       ShowField is the field data element in the vertex buffer.
///       Also, the shaders that are to be used should be changed.
GeometryHandle ShowFieldModuleImpl::renderMesh(
    boost::shared_ptr<SCIRun::Field> field,
    ModuleStateHandle state, 
    const std::string& id)
{
  SCIRun::Core::Datatypes::MeshTraits<VMesh>::MeshFacadeHandle facade =
      field->mesh()->getFacade();
  VField* vfield = field->vfield();
  
  // Since we are rendering a field, we also need to handle data on the nodes.

  /// \todo Determine a better way of handling all of the various object state.
  bool showNodes = state->getValue(ShowFieldModule::ShowNodes).getBool();
  bool showEdges = state->getValue(ShowFieldModule::ShowEdges).getBool();
  bool showFaces = state->getValue(ShowFieldModule::ShowFaces).getBool();
  bool nodeTransparency = state->getValue(ShowFieldModule::NodeTransparency).getBool();

  GeometryHandle geom(new GeometryObject(field));
  geom->objectName = id;

  ENSURE_NOT_NULL(facade, "Mesh facade");

  // Grab the vmesh object so that we can extract the normals.
  VMesh* v = field->vmesh();
  
  if (vmesh->has_normals())
    vmesh_->synchronize(Mesh::NORMALS_E); // Ensure the normals are synchronized.

  /// \todo Split the mesh into chunks of about ~32,000 vertices. May be able to
  ///       eek out better coherency and use a 16 bit index buffer instead of
  ///       a 32 bit index buffer.

  // We are going to get no lighting in this first pass. The unfortunate reality
  // is that I cannot get access to face normals in vertex shaders based off of
  // the winding orders of the incoming geometry.

  int numFloats = 4;  // Position + field data.
  if (vmesh->has_normals())
    numFloats += 3;   // Position + field data + normals;

  // Allocate memory for vertex buffer (*NOT* the index buffer, which is a
  // a function of the number of faces). Only allocating enough memory to hold
  // points associated with the faces.
  // Edges *and* faces should use the same vbo!
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  size_t vboSize = sizeof(float) * numFloats * facade->numNodes();
  rawVBO->resize(vboSize); // linear complexity.
  float* vbo = reinterpret_cast<float*>(&(*rawVBO)[0]); // Remember, standard guarantees that vectors are contiguous in memory.

  // Add shared VBO to the geometry object.
  /// \note This 'primaryVBO' is dependent on the types present in the data.
  ///       If normals are specified, then this will NOT be the primary VBO.
  ///       Another VBO will be constructed containing normal information.
  ///       All of this is NOT necessary if we are on OpenGL 3.2+ where we
  ///       can compute all normals in the geometry shader (smooth and face).
  std::string primVBOName = "primaryVBO";
  std::vector<std::string> attribs;   ///< \todo Switch to initializer lists when msvc supports it.
  attribs.push_back("aPos");          ///< \todo Find a good place to pull these names from.
  attribs.push_back("aFieldData");
  if (vmesh->has_normals())
    attribs.push_back("aNormal");
  geom->mVBOs.emplace_back(GeometryObject::SpireVBO(primVBOName, attribs, rawVBO));

  if (updater_)
    updater_(0.1);

  // Build vertex buffer.
  size_t i = 0;
  BOOST_FOREACH(const NodeInfo<VMesh>& node, facade->nodes())
  {
    vbo[i+0] = node.point().x(); vbo[i+1] = node.point().y(); vbo[i+2] = node.point().z();

    if (node.index() < vfield->num_values())
    {
      double val = 0.0;
      vfield->get_value(val, node.index());
      vbo[i+3] = static_cast<float>(val);
    }
    else
    {
      vbo[i+3] = 0.0f;
    }

    if (vmesh->has_normals())
    {
      Vector normal;
      vmesh->get_normal(normal, node.index());

      // Need to use *real* indices instead of these constant offsets like 4, 5,
      // and 6. These don't take into account any variability in the attributes.
      vbo[i+4] = node.point().x(); vbo[i+5] = node.point().y(); vbo[i+6] = node.point().z();
    }
      
    //std::cout << static_cast<float>(val) << std::endl;
    i+=numFloats;
  }

  if (updater_)
    updater_(0.25);

  // Build the edges
  if (showEdges)
  {
    buildEdgesNoNormals<VMesh>(facade, geom, primVBOName, state);
  }

  if (updater_)
    updater_(0.5);

  // Build the faces
  if (showFaces)
  {
    double dataMin = 0.0;
    double dataMax = 0.0;
    vfield->min(dataMin);
    vfield->max(dataMax);
    buildFacesNoNormals<VMesh>(facade, geom, primVBOName, dataMin, dataMax, state);
  }

  if (updater_)
    updater_(0.75);

  // Build the nodes
  if (showNodes)
  {
    buildNodesNoNormals<VMesh>(facade, geom, primVBOName, state);
  }

  if (updater_)
    updater_(1);

  return geom;
}

template <typename VMeshType>
void ShowFieldModuleImpl::buildFacesNoNormals(typename SCIRun::Core::Datatypes::MeshTraits<VMeshType>::MeshFacadeHandle facade, 
  GeometryHandle geom,
  const std::string& primaryVBOName,
  float dataMin, float dataMax,   /// Dataset minimum / maximum.
  ModuleStateHandle state)
{
  bool faceTransparency = state->getValue(ShowFieldModule::FaceTransparency).getBool();
  uint32_t* iboFaces = nullptr;

  // Determine the size of the face structure (taking into account the varying
  // types of faces -- only quads and triangles are currently supported).
  size_t iboFacesSize = 0;
  BOOST_FOREACH(const FaceInfo<VMeshType>& face, facade->faces())
  {
    typename VMeshType::Node::array_type nodes = face.nodeIndices();
    if (nodes.size() == 4)
    {
      iboFacesSize += sizeof(uint32_t) * 6;
    }
    else if (nodes.size() == 3)
    {
      iboFacesSize += sizeof(uint32_t) * 3;
    }
    else
    {
      BOOST_THROW_EXCEPTION(SCIRun::Core::DimensionMismatch() 
        << SCIRun::Core::ErrorMessage("Only Quads and Triangles are supported."));
    }
  }
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  rawIBO->resize(iboFacesSize);   // Linear in complexity... If we need more performance,
  // use malloc to generate buffer and then vector::assign.
  iboFaces = reinterpret_cast<uint32_t*>(&(*rawIBO)[0]);
  size_t i = 0;
  BOOST_FOREACH(const FaceInfo<VMeshType>& face, facade->faces())
  {
    typename VMeshType::Node::array_type nodes = face.nodeIndices();
    if (nodes.size() == 4)
    {
      // Winding order looks good from tests.
      iboFaces[i  ] = static_cast<uint32_t>(nodes[0]); iboFaces[i+1] = static_cast<uint32_t>(nodes[1]); iboFaces[i+2] = static_cast<uint32_t>(nodes[2]);
      iboFaces[i+3] = static_cast<uint32_t>(nodes[0]); iboFaces[i+4] = static_cast<uint32_t>(nodes[2]); iboFaces[i+5] = static_cast<uint32_t>(nodes[3]);
      i += 6;
    }
    else if (nodes.size() == 3)
    {
      iboFaces[i  ] = static_cast<uint32_t>(nodes[0]); iboFaces[i+1] = static_cast<uint32_t>(nodes[1]); iboFaces[i+2] = static_cast<uint32_t>(nodes[2]);
      i += 3;
    }
    // All other cases have been checked in the loop above which determines
    // the size of the face IBO.
  }

  // Add IBO for the faces.
  std::string facesIBOName = "facesIBO";
  geom->mIBOs.emplace_back(GeometryObject::SpireIBO(facesIBOName, sizeof(uint32_t), rawIBO));

  // Build pass for the faces.
  /// \todo Find an appropriate place to put program names like UniformColor.
  GeometryObject::SpireSubPass pass = 
    GeometryObject::SpireSubPass("facesPass", primaryVBOName,
    facesIBOName, "UniformColor",
    Spire::Interface::TRIANGLES);
  float transparency = 1.0f;
  if (faceTransparency)
    transparency = 0.2f;
  //pass.addUniform("uColorZero", Spire::V4(1.0f, 0.0f, 0.0f, transparency));
  //pass.addUniform("uColorOne", Spire::V4(0.0f, 0.7f, 0.0f, transparency));
  //pass.addUniform("uMinMax", Spire::V2(dataMin, dataMax));
  pass.addUniform("uColor", Spire::V4(0.6f, 0.6f, 0.6f, 0.5f));

  // For color mapping.
  //GeometryObject::SpireSubPass pass = 
  //  GeometryObject::SpireSubPass("facesPass", primaryVBOName,
  //  facesIBOName, "ColorMap",
  //  Spire::Interface::TRIANGLES);
  //float transparency = 1.0f;
  //if (faceTransparency)
  //  transparency = 0.2f;
  //pass.addUniform("uColorZero", Spire::V4(1.0f, 0.0f, 0.0f, transparency));
  //pass.addUniform("uColorOne", Spire::V4(0.0f, 0.7f, 0.0f, transparency));
  //pass.addUniform("uMinMax", Spire::V2(dataMin, dataMax));

  geom->mPasses.emplace_back(pass);
}

template <typename VMeshType>
void ShowFieldModuleImpl::buildEdgesNoNormals(typename SCIRun::Core::Datatypes::MeshTraits<VMeshType>::MeshFacadeHandle facade,
  GeometryHandle geom,
  const std::string& primaryVBOName,
  ModuleStateHandle modState)
{
  bool edgeTransparency = modState->getValue(ShowFieldModule::EdgeTransparency).getBool();

  size_t iboEdgesSize = sizeof(uint32_t) * facade->numEdges() * 2;
  uint32_t* iboEdges = nullptr;

  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  rawIBO->resize(iboEdgesSize);   // Linear in complexity... If we need more performance,
  // use malloc to generate buffer and then vector::assign.
  iboEdges = reinterpret_cast<uint32_t*>(&(*rawIBO)[0]);
  size_t i = 0;
  BOOST_FOREACH(const EdgeInfo<VMeshType>& edge, facade->edges())
  {
    // There should *only* be two indicies (linestrip would be better...)
    typename VMeshType::Node::array_type nodes = edge.nodeIndices();
    ENSURE_DIMENSIONS_MATCH(nodes.size(), 2, "Edges require exactly 2 indices.");
    iboEdges[i] = static_cast<uint32_t>(nodes[0]); iboEdges[i+1] = static_cast<uint32_t>(nodes[1]);
    i += 2;
  }

  // Add IBO for the edges.
  std::string edgesIBOName = "edgesIBO";
  geom->mIBOs.emplace_back(GeometryObject::SpireIBO(edgesIBOName, sizeof(uint32_t), rawIBO));

  // Build pass for the edges.
  /// \todo Find an appropriate place to put program names like UniformColor.
  GeometryObject::SpireSubPass pass = 
    GeometryObject::SpireSubPass("edgesPass", primaryVBOName,
    edgesIBOName, "UniformColor",
    Spire::Interface::LINES);

  Spire::GPUState state;
  state.mLineWidth = 2.5f;
  pass.addGPUState(state);

  // Add appropriate uniforms to the pass (in this case, uColor).
  if (edgeTransparency)
    pass.addUniform("uColor", Spire::V4(0.6f, 0.6f, 0.6f, 0.5f));
  else
    pass.addUniform("uColor", Spire::V4(0.6f, 0.6f, 0.6f, 1.0f));

  geom->mPasses.emplace_back(pass);
}

template <typename VMeshType>
void ShowFieldModuleImpl::buildNodesNoNormals(typename SCIRun::Core::Datatypes::MeshTraits<VMeshType>::MeshFacadeHandle facade,
  GeometryHandle geom,
  const std::string& primaryVBOName,
  ModuleStateHandle state)
{
  bool nodeTransparency = state->getValue(ShowFieldModule::NodeTransparency).getBool();

  size_t iboNodesSize = sizeof(uint32_t) * facade->numNodes();
  uint32_t* iboNodes = nullptr;

  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  rawIBO->resize(iboNodesSize);   // Linear in complexity... If we need more performance,
  // use malloc to generate buffer and then vector::assign.
  iboNodes = reinterpret_cast<uint32_t*>(&(*rawIBO)[0]);
  size_t i = 0;
  BOOST_FOREACH(const NodeInfo<VMeshType>& node, facade->nodes())
  {
    // There should *only* be two indicies (linestrip would be better...)
    //node.index()
    //VirtualMesh::Node::array_type nodes = node.nodeIndices();
    //ENSURE_DIMENSIONS_MATCH(nodes.size(), 2, "Edges require exactly 2 indices.");
    iboNodes[i] = static_cast<uint32_t>(node.index());
    i++;
  }

  // Add IBO for the nodes.
  std::string nodesIBOName = "nodesIBO";
  geom->mIBOs.emplace_back(GeometryObject::SpireIBO(nodesIBOName, sizeof(uint32_t), rawIBO));

  // Build pass for the nodes.
  /// \todo Find an appropriate place to put program names like UniformColor.
  GeometryObject::SpireSubPass pass = 
    GeometryObject::SpireSubPass("nodesPass", primaryVBOName,
    nodesIBOName, "UniformColor",
    Spire::Interface::POINTS);

  // Add appropriate uniforms to the pass (in this case, uColor).
  if (nodeTransparency)
    pass.addUniform("uColor", Spire::V4(0.6f, 0.6f, 0.6f, 0.5f));
  else
    pass.addUniform("uColor", Spire::V4(0.6f, 0.6f, 0.6f, 1.0f));

  geom->mPasses.emplace_back(pass);
}

AlgorithmParameterName ShowFieldModule::ShowNodes("ShowNodes");
AlgorithmParameterName ShowFieldModule::ShowEdges("ShowEdges");
AlgorithmParameterName ShowFieldModule::ShowFaces("ShowFaces");
AlgorithmParameterName ShowFieldModule::NodeTransparency("NodeTransparency");
AlgorithmParameterName ShowFieldModule::EdgeTransparency("EdgeTransparency");
AlgorithmParameterName ShowFieldModule::FaceTransparency("FaceTransparency");
AlgorithmParameterName ShowFieldModule::DefaultMeshColor("DefaultMeshColor");
