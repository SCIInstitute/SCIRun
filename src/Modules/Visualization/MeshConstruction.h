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

class MeshConstruction
{
public:
  /// Constructs faces without normal information. We can share the primary
  /// VBO with the nodes and the edges in this case.
  template <typename VMeshType>
  void buildFacesNoNormals(typename SCIRun::Core::Datatypes::MeshTraits<VMeshType>::MeshFacadeHandle facade,
                           SCIRun::Core::Datatypes::GeometryHandle geom,
                           const std::string& primaryVBOName,
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
};

}}}

#endif

