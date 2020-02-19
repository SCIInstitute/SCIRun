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


/// @todo Documentation Core/Datatypes/Mesh/MeshFacadeIterators.h

#ifndef CORE_DATATYPES_MESH_FACADE_ITERATORS_H
#define CORE_DATATYPES_MESH_FACADE_ITERATORS_H

#include <boost/iterator/iterator_facade.hpp>
#include <Core/Utils/Exception.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/Datatypes/Mesh/share.h>

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

  /// @todo: Being conservative with mesh synchronization flags until more
  // synchronize infrastructure is implemented and tested,
  // so clear_synchronization() is always called after synchronize and operation
  // that requires synchronize to be called.
  template <class VirtualMeshType, template <typename> class MeshComponent>
  class SmartMeshIterator : public boost::iterator_facade<SmartMeshIterator<VirtualMeshType, MeshComponent>, MeshComponent<VirtualMeshType>, boost::bidirectional_traversal_tag>
  {
  public:
    /// @todo: need to look up pattern for creating "end" iterators.
    explicit SmartMeshIterator(VirtualMeshType* vmesh = 0, bool isEnd = false) : iter_(0), vmesh_(vmesh), current_(vmesh)
    {
      ENSURE_NOT_NULL(vmesh, "virtual mesh");
      if (!isEnd)
      {
        vmesh_->begin(iter_);
      }
      else
      {
        /// @todo: need to split out that Synchronize enum
        //TODO: pass in the correct synch constant
        vmesh_->synchronize(/*Mesh::EDGES_E*/ 2);
        vmesh_->end(iter_);
      }
      current_.setIndex(*iter_);
    }
  private:
    friend class boost::iterator_core_access;

    void increment() { ++iter_; }
    void decrement() { --iter_; }

    bool equal(const SmartMeshIterator<VirtualMeshType, MeshComponent>& other) const
    {
      return this->vmesh_ == other.vmesh_
        && this->iter_ == other.iter_;
    }

    MeshComponent<VirtualMeshType>& dereference() const
    {
      current_.setIndex(*iter_);
      return current_;
    }

    typename MeshComponent<VirtualMeshType>::iterator iter_;
    VirtualMeshType* vmesh_;
    mutable MeshComponent<VirtualMeshType> current_;
  };

  /// @todo: templatize with traits and stuff. for now, a specialized version for edges.
  template <class VirtualMeshType>
  class EdgeInfo
  {
  public:
    typedef typename VirtualMeshType::Edge::iterator iterator;
    enum
    {
      sync_enum = 2
    };
    explicit EdgeInfo(VirtualMeshType* mesh) : index_(0), vmesh_(mesh)
    {
      /// @todo: need to split out that Synchronize enum
      vmesh_->synchronize(/*Mesh::EDGES_E*/ sync_enum);
    }
    void setIndex(typename VirtualMeshType::Edge::index_type i) { index_ = i; }

    typename VirtualMeshType::Edge::index_type index() const { return index_; }
    typename VirtualMeshType::Node::array_type nodeIndices() const
    {
      typename VirtualMeshType::Node::array_type nodesFromEdge(2);
      vmesh_->get_nodes(nodesFromEdge, index_);

      return nodesFromEdge;
    }

    std::vector<Geometry::Point> nodePoints() const
    {
      auto indices = nodeIndices();
      std::vector<Geometry::Point> ps(2);
      for (size_t i = 0; i < ps.size(); ++i)
        vmesh_->get_point(ps[i], indices[i]);

      return ps;
    }
  private:
    typename VirtualMeshType::Edge::index_type index_;
    VirtualMeshType* vmesh_;
  };

  template <typename VirtualMeshType>
  struct SmartEdgeIterator
  {
    typedef SmartMeshIterator<VirtualMeshType, EdgeInfo> Type;
  };

  template <class VirtualMeshType>
  class FaceInfo
  {
  public:
    typedef typename VirtualMeshType::Face::iterator iterator;
    enum
    {
      sync_enum = 4
    };
    explicit FaceInfo(VirtualMeshType* mesh) : index_(0), vmesh_(mesh)
    {
      vmesh_->synchronize(/*Mesh::FACES_E*/ sync_enum);
    }
    void setIndex(typename VirtualMeshType::Face::index_type i) { index_ = i; }

    typename VirtualMeshType::Face::index_type index() const { return index_; }
    typename VirtualMeshType::Node::array_type nodeIndices() const
    {
      typename VirtualMeshType::Node::array_type nodesFromFace(4);
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
    typename VirtualMeshType::Edge::array_type edgeIndices() const
    {
      typename VirtualMeshType::Edge::array_type edgesFromFace(4);
      vmesh_->get_edges(edgesFromFace, index_);
      return edgesFromFace;
    }
  private:
    typename VirtualMeshType::Face::index_type index_;
    VirtualMeshType* vmesh_;
  };

  template <typename VirtualMeshType>
  struct SmartFaceIterator
  {
    typedef SmartMeshIterator<VirtualMeshType, FaceInfo> Type;
  };

  template <class VirtualMeshType>
  class NodeInfo
  {
  public:
    typedef typename VirtualMeshType::Node::iterator iterator;
    enum
    {
      sync_enum = 1 << 8
    };
    explicit NodeInfo(VirtualMeshType* mesh) : synched_(false), index_(0), vmesh_(mesh) {}
    void setIndex(typename VirtualMeshType::Node::index_type i) { index_ = i; }

    typename VirtualMeshType::Node::index_type index() const { return index_; }
    Geometry::Point point() const
    {
      Geometry::Point p;
      vmesh_->get_point(p, index_);
      return p;
    }
    typename VirtualMeshType::Edge::array_type edgeIndices() const
    {
      if (!synched_)
      {
        /// @todo: need to split out that Synchronize enum
        vmesh_->synchronize(/*Mesh::NODE_NEIGHBORS_E*/sync_enum);
        synched_ = true;
      }
      typename VirtualMeshType::Edge::array_type edgesFromNode(6);
      vmesh_->get_edges(edgesFromNode, index_);
      return edgesFromNode;
    }
  private:
    mutable bool synched_;
    typename VirtualMeshType::Node::index_type index_;
    VirtualMeshType* vmesh_;
  };

  template <typename VirtualMeshType>
  struct SmartNodeIterator
  {
    typedef SmartMeshIterator<VirtualMeshType, NodeInfo> Type;
  };

  template <class VirtualMeshType>
  class CellInfo
  {
  public:
    typedef typename VirtualMeshType::Cell::iterator iterator;
    typedef typename VirtualMeshType::Cell::index_type index_type;
    enum
    {
      sync_enum = 1 << 3
    };
    explicit CellInfo(VirtualMeshType* mesh) : synched_(false), index_(0), vmesh_(mesh) {}
    void setIndex(index_type i) { index_ = i; }

    index_type index() const { return index_; }
    Geometry::Point center() const
    {
      Geometry::Point p;
      vmesh_->get_center(p, index_);
      return p;
    }
    typename VirtualMeshType::Edge::array_type edgeIndices() const
    {
      if (!synched_)
      {
        /// @todo: need to split out that Synchronize enum
        vmesh_->synchronize(/*Mesh::??*/sync_enum);
        synched_ = true;
      }
      typename VirtualMeshType::Edge::array_type edgesFromNode(6);
      vmesh_->get_edges(edgesFromNode, index_);
      return edgesFromNode;
    }
  private:
    mutable bool synched_;
    index_type index_;
    VirtualMeshType* vmesh_;
  };

  template <typename VirtualMeshType>
  struct SmartCellIterator
  {
    typedef SmartMeshIterator<VirtualMeshType, CellInfo> Type;
  };

}}}

#endif
