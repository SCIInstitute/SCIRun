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


#ifndef CORE_DATATYPES_VMESHSHARED_H
#define CORE_DATATYPES_VMESHSHARED_H

#include <iostream>
#include <Core/Datatypes/Mesh/VMesh.h>
#include <Core/Datatypes/Mesh/Share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

template <class MESH>
class SCISHARE VirtualMeshShared : public VirtualMesh 
{
public:
  
  // Constructor: this class maintain all the virtual function calls to the 
  // basis class, so we do not need to replicate this for each mesh class
  // This is only used in the mesh classes
  
  VirtualMeshShared(MESH* mesh) :
    mesh_(mesh)
  {
    basis_ = &(mesh_->get_basis());
  
    //! Collect general information on mesh type. These are constants and should
    //! be accessible down the road by inline function calls. The latter 
    //! construction was choosen to improve performance and reduce the need for
    //! for virtual function calls
    
    
    //! cache the mesh basis order (1=liner, 2=quadratic, 3=cubic)
    basis_order_   = mesh_->basis_order();
    //! cache the mesh dimension (0=point,1=curve, 2=surface, or 3=volume)
    dimension_     = mesh_->dimensionality();
    //! cache whether we can edit the mesh by adding nodes and elements
    is_editable_   = mesh_->is_editable();
    //! cache whether we haev surface normals
    has_normals_   = mesh_->has_normals();   
    //! cache whether we have node points defined or whether we derive them
    //! implicitly. 
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    is_regular_    = static_cast<bool>(mesh_->topology_geometry()&Mesh5::REGULAR);
    //! cache whether we  have connectivity defined or whether we assume the data
    //! to be structured enough so we can derive it implicitly
    is_structured_ = static_cast<bool>(mesh_->topology_geometry()&Mesh5::STRUCTURED);
#endif
    //! Store topology information on the mesh:
    //! This section converts the relevant data out of the underlying older data
    //! structures.
    
    //! Number of nodes in one element
    num_nodes_per_elem_  = basis_->number_of_mesh_vertices();
    
    //! Number of edge nodes for quadratic interpolation model
    num_enodes_per_elem_ = basis_->number_of_vertices() - basis_->number_of_mesh_vertices();
    
    //! Number of edges in one element
    num_edges_per_elem_  = basis_->number_of_edges();
    
    //! Number of faces in one element
    num_faces_per_elem_  = basis_->faces_of_cell();
    
    //! Number of nodes per face (volume and surface meshes only)
    num_nodes_per_face_  = basis_->vertices_of_face();
    
    //! Number of edges per face (volume and surface meshes only)
    num_edges_per_face_  = 0;
    if (basis_->vertices_of_face() > 0) num_edges_per_face_  = basis_->vertices_of_face()-1; 
    
    //! Number of gradients per node for cubic interpolation model
    num_gradients_per_node_ = basis_->num_hderivs();
    
    element_size_ = basis_->domain_size();
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    generation_ = mesh_->generation;
#endif

    unit_vertices_.resize(num_nodes_per_elem_);
    for (size_t k=0; k < num_nodes_per_elem_; k++)
    {
      VirtualMesh::coords_type c;//(dimension_);
      for (int p=0; p<dimension_; p++) c[p] = basis_->unit_vertices[k][p];
      unit_vertices_[k] = c;
    }
    
    unit_edges_.resize(num_edges_per_elem_);
    for (size_t k=0; k < num_edges_per_elem_; k++)
    {
      unit_edges_[k].resize(2);
      for (size_t p=0; p<2; p++) 
      {
        unit_edges_[k][p] = 0;
        unit_edges_[k][p] = basis_->unit_edges[k][p];
      }
    }
    
    unit_center_.resize(dimension_);
    for (int k=0; k < dimension_; k++) 
    {
      unit_center_[k] = basis_->unit_center[k];
    }
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    pm_ = static_cast<PropertyManager*>(mesh_);
#endif
  }
  
  virtual ~VirtualMeshShared() {}
  
  virtual MeshHandle5 mesh();
  virtual VirtualMesh*     vmesh();  
  virtual bool synchronize(unsigned int sync);
  virtual bool unsynchronize(unsigned int sync);
  virtual bool clear_synchronization();
  
#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  virtual BBox get_bounding_box() const;
  virtual void transform(const Geometry::Transform &t);  

  virtual double get_epsilon() const;
                          
  virtual void get_weights(const VirtualMesh::coords_type& coords, 
                           std::vector<double>& weights, 
                           int basis_order) const; 

  virtual void get_derivate_weights(const VirtualMesh::coords_type& coords, 
                           std::vector<double>& weights, 
                           int basis_order) const; 
  
  virtual void get_gaussian_scheme(std::vector<VirtualMesh::coords_type>& coords, 
                                   std::vector<double>& weights, int order) const;
  virtual void get_regular_scheme(std::vector<VirtualMesh::coords_type>& coords, 
                                  std::vector<double>& weights, int order) const;

  virtual void get_canonical_transform(Geometry::Transform &t);
#endif

protected:
  MESH*                           mesh_;
  typename MESH::basis_type*      basis_;
};


template <class MESH>
MeshHandle5
VirtualMeshShared<MESH>::mesh()
{
  return (MeshHandle5(mesh_));
}

template <class MESH>
VirtualMesh*
VirtualMeshShared<MESH>::vmesh()
{
  std::cerr << "VirtualMeshShared<MESH>" << std::endl;
  return (this);
}

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
template<class MESH>
void 
VirtualMeshShared<MESH>::get_canonical_transform(Transform &t)
{
  mesh_->get_canonical_transform(t);
}

template<class MESH>
void 
VirtualMeshShared<MESH>::get_weights(const VirtualMesh::coords_type& coords, 
                               std::vector<double>& weights, 
                               int basis_order) const
{
  switch(basis_order)
  {
    case 0:
      weights.resize(1); weights[0] = 1.0;
      return;
    case 1:  
      weights.resize(basis_->num_linear_weights());
      basis_->get_linear_weights(coords,&(weights[0]));
      return;
    case 2:
      weights.resize(basis_->num_quadratic_weights());
      basis_->get_quadratic_weights(coords,&(weights[0]));
      return;
    case 3:  
      weights.resize(basis_->num_cubic_weights());
      basis_->get_cubic_weights(coords,&(weights[0]));
      return;
  }
  ASSERTFAIL("Weights of unknown order requested");
}

template<class MESH>
void 
VirtualMeshShared<MESH>::get_derivate_weights(const VirtualMesh::coords_type& coords, 
                                        std::vector<double>& weights, 
                                        int basis_order) const
{
  switch(basis_order)
  {
    case 0:
      weights.resize(1); weights[0] = 1.0;
      return;
    case 1:  
      weights.resize(basis_->num_linear_derivate_weights());
      basis_->get_linear_derivate_weights(coords,&(weights[0]));
      return;
    case 2:
      weights.resize(basis_->num_quadratic_derivate_weights());
      basis_->get_quadratic_derivate_weights(coords,&(weights[0]));
      return;
    case 3:  
      weights.resize(basis_->num_cubic_derivate_weights());
      basis_->get_cubic_derivate_weights(coords,&(weights[0]));
      return;
  }
  ASSERTFAIL("Derivate weights of unknown order requested");
}



template <class MESH>
BBox
VirtualMeshShared<MESH>::get_bounding_box() const
{
  return(mesh_->get_bounding_box());
}

template <class MESH>
double
VirtualMeshShared<MESH>::get_epsilon() const
{
  return(mesh_->get_epsilon());
}

template<class MESH>
void 
VirtualMeshShared<MESH>::transform(const Transform &t)
{
  mesh_->transform(t);
}

#endif

template<class MESH>
bool
VirtualMeshShared<MESH>::synchronize(unsigned int sync)
{
  return(mesh_->synchronize(sync));
}

template<class MESH>
bool
VirtualMeshShared<MESH>::unsynchronize(unsigned int sync)
{
  return(mesh_->unsynchronize(sync));
}

template<class MESH>
bool
VirtualMeshShared<MESH>::clear_synchronization()
{
  return(mesh_->clear_synchronization());
}


#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER

template<class MESH>
void 
VirtualMeshShared<MESH>::get_gaussian_scheme(std::vector<coords_type>& coords, 
                                       std::vector<double>& weights, int order) const
{
  basis_->get_gaussian_scheme(coords,weights,order);
}
  
template<class MESH>
void 
VirtualMeshShared<MESH>::get_regular_scheme(std::vector<coords_type>& coords, 
                                      std::vector<double>& weights, int order) const
{
  basis_->get_regular_scheme(coords,weights,order);
}

#endif
}
}
}

#endif
