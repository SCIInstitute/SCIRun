/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

///////////////////////////
// PORTED SCIRUN v4 CODE //
///////////////////////////

#ifndef CORE_DATATYPES_MESH_LATVOLMESHFACADE_H
#define CORE_DATATYPES_MESH_LATVOLMESHFACADE_H 

#include <Core/Datatypes/Mesh/MeshFacade.h>
#include <Core/Datatypes/Mesh/Share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  class LatticeVolumeMeshFacade : public MeshFacade
  {
  public:
    explicit LatticeVolumeMeshFacade(VirtualMeshHandle vmesh) : vmesh_(vmesh) 
    {
      if (!(vmesh->is_latvolmesh() || vmesh->is_trisurfmesh()))
        THROW_INVALID_ARGUMENT("Incorrect mesh type for this facade type.");
    }

    virtual Edges edges() const 
    {
      return Edges(SmartEdgeIterator(vmesh_.get()), SmartEdgeIterator(vmesh_.get(), true));
    }

    virtual Faces faces() const 
    {
      return Faces(SmartFaceIterator(vmesh_.get()), SmartFaceIterator(vmesh_.get(), true));
    }

    virtual Nodes nodes() const
    {
      return Nodes(SmartNodeIterator(vmesh_.get()), SmartNodeIterator(vmesh_.get(), true));
    }

    virtual size_t numNodes() const
    {
      return vmesh_->num_nodes();
    }

    virtual size_t numEdges() const
    {
      return vmesh_->num_edges();
    }

    virtual size_t numFaces() const 
    {
      return vmesh_->num_faces();
    }

    virtual size_t numElements() const 
    {
      return vmesh_->num_elems();
    }
  private:
    VirtualMeshHandle vmesh_;
  };

}}}

#endif

