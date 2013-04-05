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

#ifndef CORE_DATATYPES_MESH_FACADE_ITERATORS_H
#define CORE_DATATYPES_MESH_FACADE_ITERATORS_H 

#include <boost/iterator/iterator_facade.hpp>
#include <Core/Utils/Exception.h>
#include <Core/Datatypes/Mesh/VMesh.h>
#include <Core/Datatypes/Mesh/Share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {
  
  /*
    IDEA: Mesh iterators should iterate over entire values, not just indexes, producing values on demand. 
    A "SmartIndex" for the mesh.
    E.G. for LatVols:
    class SmartNodeIterator : public std::iterator<std::forward_iterator_tag, SmartIndex<Mesh::index_type,Point> >
    class SmartEdgeIterator : public std::iterator<std::forward_iterator_tag, SmartIndex<Mesh::index_type,Point[2]> >
    class SmartFaceIterator : public std::iterator<std::forward_iterator_tag, SmartIndex<Mesh::index_type,Point[4],SmartIndex<Edge>[4]> >
  */

  // TODO: Being conservative with mesh synchronization flags until more
  // synchronize infrastructure is implemented and tested,
  // so clear_synchronization() is always called after synchronize and operation
  // that requires synchronize to be called.
  template <typename MeshComponent>
  class SmartMeshIterator : public boost::iterator_facade<SmartMeshIterator<MeshComponent>, MeshComponent, boost::bidirectional_traversal_tag>
  {
  public:
    //TODO: need to look up pattern for creating "end" iterators.
    explicit SmartMeshIterator(VirtualMesh* vmesh = 0, bool isEnd = false) : iter_(0), vmesh_(vmesh), current_(vmesh)
    {
      ENSURE_NOT_NULL(vmesh, "virtual mesh");
      if (!isEnd)
      {
        vmesh_->begin(iter_);
      }
      else
      {
        vmesh_->synchronize(Mesh::EDGES_E);        

        vmesh_->end(iter_);

        vmesh_->clear_synchronization();
      }
      current_.setIndex(*iter_);
    }
  private:
    friend class boost::iterator_core_access;

    void increment() { ++iter_; }
    void decrement() { --iter_; }

    bool equal(const SmartMeshIterator<MeshComponent>& other) const
    {
      return this->vmesh_ == other.vmesh_ 
        && this->iter_ == other.iter_;
    }

    MeshComponent& dereference() const
    { 
      current_.setIndex(*iter_);
      return current_; 
    }

    typename MeshComponent::iterator iter_;
    VirtualMesh* vmesh_;
    mutable MeshComponent current_;
  };

  class EdgeInfo;
  typedef SmartMeshIterator<EdgeInfo> SmartEdgeIterator;

  //TODO: templatize with traits and stuff. for now, a specialized version for edges.
  class EdgeInfo
  {
  public:
    typedef VirtualMesh::Edge::iterator iterator;
    explicit EdgeInfo(VirtualMesh* mesh) : index_(0), vmesh_(mesh) 
    {
      //vmesh_->synchronize(Mesh::EDGES_E);
    }
    void setIndex(VirtualMesh::Edge::index_type i) { index_ = i; }

    VirtualMesh::Edge::index_type index() const { return index_; }
    VirtualMesh::Node::array_type nodeIndices() const
    {
      VirtualMesh::Node::array_type nodesFromEdge(2);
      vmesh_->synchronize(Mesh::EDGES_E);
      vmesh_->get_nodes(nodesFromEdge, index_);
      vmesh_->clear_synchronization();

      return nodesFromEdge;
    }

    std::vector<Geometry::Point> nodePoints() const 
    {
      auto indices = nodeIndices();
      std::vector<Geometry::Point> ps(2);
      vmesh_->synchronize(Mesh::EDGES_E);

      for (size_t i = 0; i < ps.size(); ++i)
        vmesh_->get_point(ps[i], indices[i]);
      vmesh_->clear_synchronization();

      return ps;
    }
  private:
    VirtualMesh::Edge::index_type index_;
    VirtualMesh* vmesh_;
  };

  class FaceInfo;
  typedef SmartMeshIterator<FaceInfo> SmartFaceIterator;

  class FaceInfo 
  {
  public:
    typedef VirtualMesh::Face::iterator iterator;
    explicit FaceInfo(VirtualMesh* mesh) : index_(0), vmesh_(mesh) {}
    void setIndex(VirtualMesh::Face::index_type i) { index_ = i; }

    VirtualMesh::Face::index_type index() const { return index_; }
    VirtualMesh::Node::array_type nodeIndices() const
    {
      VirtualMesh::Node::array_type nodesFromFace(4);
      vmesh_->get_nodes(nodesFromFace, index_);
      return nodesFromFace;
    }
    std::vector<Geometry::Point> nodePoints() const 
    {
      auto indices = nodeIndices();
      std::vector<Geometry::Point> ps(4);
      for (size_t i = 0; i < ps.size(); ++i)
        vmesh_->get_point(ps[i], indices[i]);
      return ps;
    }
    VirtualMesh::Edge::array_type edgeIndices() const
    {
      VirtualMesh::Edge::array_type edgesFromFace(4);
      vmesh_->get_edges(edgesFromFace, index_);
      return edgesFromFace;
    }
  private:
    VirtualMesh::Face::index_type index_;
    VirtualMesh* vmesh_;
  };

  class NodeInfo;
  typedef SmartMeshIterator<NodeInfo> SmartNodeIterator;

  class NodeInfo 
  {
  public:
    typedef VirtualMesh::Node::iterator iterator;
    explicit NodeInfo(VirtualMesh* mesh) : index_(0), vmesh_(mesh) {}
    void setIndex(VirtualMesh::Node::index_type i) { index_ = i; }

    VirtualMesh::Node::index_type index() const { return index_; }
    Geometry::Point point() const 
    {
      Geometry::Point p;
      vmesh_->get_point(p, index_);
      return p;
    }
    VirtualMesh::Edge::array_type edgeIndices() const
    {
      VirtualMesh::Edge::array_type edgesFromNode(6);
      vmesh_->synchronize(Mesh::NODE_NEIGHBORS_E);

      vmesh_->get_edges(edgesFromNode, index_);

      vmesh_->clear_synchronization();
      return edgesFromNode;
    }
  private:
    VirtualMesh::Node::index_type index_;
    VirtualMesh* vmesh_;
  };
  
}}}

#endif

