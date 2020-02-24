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


/// @todo Documentation Core/Datatypes/Mesh/MeshFacade.h

#ifndef CORE_DATATYPES_MESH_FACADE_H
#define CORE_DATATYPES_MESH_FACADE_H

#include <Core/Datatypes/Mesh/MeshFacadeIterators.h>
#include <Core/Datatypes/Mesh/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  template <class VirtualMeshType>
  class MeshFacade
  {
  public:
    virtual ~MeshFacade() {}

    // prototype this for use with BOOST_FOREACH, so use pairs of iterators. When upgrading to C++11, will need to support a range concept.
    typedef std::pair<typename SmartNodeIterator<VirtualMeshType>::Type, typename SmartNodeIterator<VirtualMeshType>::Type> Nodes;
    typedef std::pair<typename SmartEdgeIterator<VirtualMeshType>::Type, typename SmartEdgeIterator<VirtualMeshType>::Type> Edges;
    typedef std::pair<typename SmartFaceIterator<VirtualMeshType>::Type, typename SmartFaceIterator<VirtualMeshType>::Type> Faces;
    typedef std::pair<typename SmartCellIterator<VirtualMeshType>::Type, typename SmartCellIterator<VirtualMeshType>::Type> Cells;

    virtual Nodes nodes() const = 0;
    virtual Edges edges() const = 0;
    virtual Faces faces() const = 0;
    virtual Cells cells() const = 0;

    virtual size_t numNodes() const = 0;
    virtual size_t numEdges() const = 0;
    virtual size_t numFaces() const = 0;
    virtual size_t numCells() const = 0;
    virtual size_t numElements() const = 0;
  };

  template <class A, class B>
  const A& begin(const std::pair<A,B>& nodes) { return nodes.first; }
  template <class A, class B>
  const B& end(const std::pair<A, B>& nodes) { return nodes.second; }

}}}

#endif
