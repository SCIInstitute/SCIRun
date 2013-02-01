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

#ifndef CORE_DATATYPES_MESH_FACADE_H
#define CORE_DATATYPES_MESH_FACADE_H 

#include <boost/iterator/iterator_facade.hpp>
#include <Core/Utils/Exception.h>
#include <Core/Datatypes/Mesh/VMesh.h>
#include <Core/Datatypes/Mesh/Share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  //class SmartNodeIterator {};

  //TODO: templatize with traits and stuff. for now, a specialized version for edges.
  class EdgeInfo
  {
  public:
    VirtualMesh::Edge::index_type index() const { return index_; }
    VirtualMesh::Node::array_type nodeIndices() const
    {
      VirtualMesh::Node::array_type nodesFromEdge(2);
      vmesh_->get_nodes(nodesFromEdge, index_);
      return nodesFromEdge;
    }
    std::vector<Geometry::Point> nodePoints() const 
    {
      auto indices = nodeIndices();
      std::vector<Geometry::Point> ps(2);
      vmesh_->get_point(ps[0], indices[0]);
      vmesh_->get_point(ps[1], indices[1]);
      return ps;
    }
  private:
    friend class SmartEdgeIterator;
    explicit EdgeInfo(VirtualMesh* mesh) : index_(0), vmesh_(mesh) {}
    void setIndex(VirtualMesh::Edge::index_type i) { index_ = i; }
    VirtualMesh::Edge::index_type index_;
    VirtualMesh* vmesh_;
  };

  class SmartEdgeIterator : public boost::iterator_facade<SmartEdgeIterator, EdgeInfo, boost::bidirectional_traversal_tag>
  {
  public:
    //TODO: need to look up pattern for creating "end" iterators.
    explicit SmartEdgeIterator(VirtualMesh* vmesh = 0, bool isEnd = false) : iter_(0), vmesh_(vmesh), current_(vmesh)
    {
      ENSURE_NOT_NULL(vmesh, "virtual mesh");
      if (!isEnd)
      {
        vmesh_->begin(iter_);
      }
      else
      {
        vmesh_->end(iter_);
      }
      current_.setIndex(*iter_);
    }
  private:
    friend class boost::iterator_core_access;

    void increment() { ++iter_; }
    void decrement() { --iter_; }

    bool equal(const SmartEdgeIterator& other) const
    {
      return this->vmesh_ == other.vmesh_ 
        && this->iter_ == other.iter_;
    }

    EdgeInfo& dereference() const
    { 
      current_.setIndex(*iter_);
      return current_; 
    }

    VirtualMesh::Edge::iterator iter_;
    VirtualMesh* vmesh_;
    mutable EdgeInfo current_;
  };

  //class SmartFaceIterator {};
  
  class SCISHARE MeshFacade
  {
  public:
    virtual ~MeshFacade() {}

    // prototype this for use with BOOST_FOREACH, so use pairs of iterators. When upgrading to C++11, will need to support a range concept.
    //typedef std::pair<SmartNodeIterator, SmartNodeIterator> Nodes;
    typedef std::pair<SmartEdgeIterator, SmartEdgeIterator> Edges;
    //typedef std::pair<SmartFaceIterator, SmartFaceIterator> Faces;

    //virtual Nodes nodes() const = 0;
    virtual Edges edges() const = 0;
    //virtual Faces faces() const = 0;

    virtual size_t numNodes() const = 0;
    virtual size_t numEdges() const = 0;
    virtual size_t numFaces() const = 0;
    virtual size_t numElements() const = 0;
  };

  /*
    IDEA: Mesh iterators should iterate over entire values, not just indexes, producing values on demand. 
    A "SmartIndex" for the mesh.
    E.G. for LatVols:
    class SmartNodeIterator : public std::iterator<std::forward_iterator_tag, SmartIndex<Mesh::index_type,Point> >
    class SmartEdgeIterator : public std::iterator<std::forward_iterator_tag, SmartIndex<Mesh::index_type,Point[2]> >
    class SmartFaceIterator : public std::iterator<std::forward_iterator_tag, SmartIndex<Mesh::index_type,Point[4],SmartIndex<Edge>[4]> >
  */

}}}

#endif

