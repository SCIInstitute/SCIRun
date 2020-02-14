/*/*
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


/// @todo Documentation Modules/Visualization/MeshConstruction.h

#ifndef MODULES_VISUALIZATION_MESH_CONSTRUCTION_H
#define MODULES_VISUALIZATION_MESH_CONSTRUCTION_H

#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Mesh/VMesh.h>
#include <Core/Datatypes/Mesh/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Mesh/MeshFacade.h>

namespace SCIRun {
namespace Modules {
namespace Visualization {

namespace MeshConstruction
{
  /// Constructs faces without normal information. We can share the primary
  /// VBO with the nodes and the edges in this case.
  template <typename VMeshType>
  static void buildFacesNoNormals(typename SCIRun::Core::Datatypes::MeshTraits<VMeshType>::MeshFacadeHandle facade,
                           SCIRun::Core::Datatypes::GeometryHandle geom,
                           const std::string& primaryVBOName,
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
                                  Spire::StuInterface::TRIANGLES);
    if (faceTransparency)
      pass.addUniform("uColor", Spire::V4(1.0f, 1.0f, 1.0f, 0.2f));
    else
      pass.addUniform("uColor", Spire::V4(1.0f, 1.0f, 1.0f, 1.0f));

    geom->mPasses.emplace_back(pass);
  }

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
};

}}}

#endif
