/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/FiniteElements/BuildRHS/BuildFESurfRHS.h>

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixOperations.h>

namespace SCIRunAlgo {

using namespace SCIRun;

struct IndexHash {
  static const size_t bucket_size = 4;
  static const size_t min_buckets = 8;
  
  size_t operator()(const index_type &idx) const
    { return (static_cast<size_t>(idx)); }
  
  bool operator()(const index_type &i1, const index_type &i2) const
    { return (i1 < i2); }
};



/// A copy of the algorithm without creating the mapping matrix. 
/// Need this for the various algorithms that only use the boundary to
/// project nodes on.

bool 
BuildFESurfRHSAlgo::
run(FieldHandle input, FieldHandle& output,  MatrixHandle& mat_output)
{
  /// Define types we need for mapping
#ifdef HAVE_HASH_MAP
  typedef hash_map<index_type,index_type,IndexHash> hash_map_type;
#else
  typedef std::map<index_type,index_type,IndexHash> hash_map_type;
#endif
  hash_map_type node_map;
  hash_map_type elem_map;
  
  algo_start("BuildFESurfRHS");
  
  /// Check whether we have an input field
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  /// Figure out what the input type and output type have to be
  FieldInformation fi(input);
  FieldInformation fo(input);
  
  /// We do not yet support Quadratic and Cubic Meshes here
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    algo_end(); return (false);
  }
  
  /// Figure out which type of field the output is:
  bool found_method = false;
  if (fi.is_hex_element())    { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_prism_element())  { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_tet_element())    { fo.make_trisurfmesh(); found_method = true; }
  if (fi.is_quad_element())   { fo.make_curvemesh(); found_method = true; }
  if (fi.is_tri_element())    { fo.make_curvemesh(); found_method = true; }
  if (fi.is_pnt_element())
  {
    remark("The field boundary of a point cloud is the same point cloud");
    output = input;
    algo_end(); return (true);
  }
  
  /// Check whether we could make a conversion
  if (!found_method)
  {
    error("No method available for mesh of type: " + fi.get_mesh_type());
    algo_end(); return (false);
  }

  /// Create the output field
  output = CreateField(fo);
  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
  }
  
  /// Get the virtual interfaces:
  VMesh* imesh = input->vmesh();
  VMesh* omesh = output->vmesh();
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  
  imesh->synchronize(Mesh::DELEMS_E|Mesh::ELEM_NEIGHBORS_E);
  
  /// These are all virtual iterators, virtual index_types and array_types
  VMesh::Elem::iterator be, ee;
  VMesh::Elem::index_type nci, ci;
  VMesh::DElem::array_type delems; 
  VMesh::Node::array_type inodes; 
  VMesh::Node::array_type onodes; 
  VMesh::Node::index_type a;

  inodes.clear();
  onodes.clear();  
  Point point;
  
  /// Create numerical integration scheme
  int int_basis = 2;
  std::vector<VMesh::coords_type> points;
  std::vector<double> weights;
  std::vector<std::vector<double> > basis_vals;
  
  omesh->get_gaussian_scheme(points,weights,int_basis);
  basis_vals.resize(points.size());
  
  for (size_t j = 0; j < points.size(); j++){
    omesh->get_weights(points[j], basis_vals[j], 1);   // assumes a basis order of 1
  }
  
  double vol = omesh->get_element_size();
  const int dim = omesh->dimensionality();
  if(dim < 1 || dim > 3){
    error("Surface mesh dimension is 0 or larger than 3, for which no FE implementation is available");
    algo_end(); return (false);
  }
    
  imesh->begin(be); 
  imesh->end(ee);

  // Set up output matrix
  VMesh::Node::size_type mns;
  DenseMatrix *rhsmatrix_;
  MatrixHandle rhsmatrixhandle_;
  
  imesh->size(mns);  // this assumes basis order < 2
  try
  {
    rhsmatrix_ = new DenseMatrix(mns, 1);
    rhsmatrixhandle_ = rhsmatrix_;
  }
  catch (...)
  {
    error("Error alocating output matrix");
    algo_end(); return (false);
  }
  
  // Loop over output values and zero
  for(index_type i = 0; i < mns; i++){
    rhsmatrix_->put(i, 0, 0.0);
  }
  
  while (be != ee) 
  {
    ci = *be;
    imesh->get_delems(delems,ci);
    for (size_t p =0; p < delems.size(); p++)
    {
      bool includeface = false;
      
      if(!(imesh->get_neighbor(nci,ci,delems[p]))) includeface = true;

      if (includeface)
      {
        imesh->get_nodes(inodes,delems[p]);
        if (onodes.size() == 0) onodes.resize(inodes.size());
        for (size_t q=0; q<onodes.size(); q++)
        {
          a = inodes[q];
          hash_map_type::iterator it = node_map.find(a);
          if (it == node_map.end())
          {            
            imesh->get_center(point,a);
            onodes[q] = omesh->add_node(point);
            node_map[a] = onodes[q];            
          }
          else
          {
            onodes[q] = node_map[a];
          }
        }
        VMesh::Elem::index_type new_elem = omesh->add_elem(onodes);
        elem_map[new_elem] = ci;        
  
        // start integration routine
        
        // get the center of a standard element in the output surface
        VMesh::coords_type center;
        omesh->get_element_center(center);
        
        // find the normal at the center of the element
        Vector normal;
        omesh->get_normal(normal, center, new_elem);
        
        // Get field value
        Vector V;
        ifield->get_value(V,ci);
        
        for (size_t q=0; q < onodes.size(); q++)
        {
          // Let q be high.
    
          double l_val = 0.0;
          for(size_t i = 0; i < basis_vals.size(); i++){
            double Ji[9];
            double detJ = omesh->inverse_jacobian(points[i], new_elem, Ji);
            
            // If Jacobian is negative, there is a problem with the mesh
            if (detJ <= 0.0){
              error("Mesh has elements with negative jacobians, check the order of the nodes that define an element");
              algo_end(); return (false);
            }
            
            // Volume associated with the local Gaussian Quadrature point:
            // weightfactor * Volume Unit Element * Volume ratio (ral element/unit element)
            detJ *= weights[i] * vol;
            
            // build local RHS value
            // Get the local weights of the basis functions in the basis element
            // They are all the same and are thus precomputed in matrix basis_weights
            const double *Wi = &basis_vals[i][0];
            
            // Weight associated with the node we are calculating
            const double &Wip = Wi[q];
            
            // Calculating input vector dotted with normal, times basis_vals, times volume scaling factor
            l_val += Dot(V, normal) * Wip * detJ;
          }
          
          rhsmatrix_->add(inodes[q], 0, l_val);
        }            
      }
    }
    ++be;
  }
  
  ofield->resize_fdata();
  
  if (ifield->basis_order() == 0)
  {
    hash_map_type::iterator it, it_end;
    it = elem_map.begin();
    it_end = elem_map.end();
    
    while (it != it_end)
    {      
      VMesh::Elem::index_type idx1((*it).second);
      VMesh::Elem::index_type idx2((*it).first);  
      
      /// Copying values
      ofield->copy_value(ifield,idx1,idx2);
      ++it;
    }
  }
  else if (input->basis_order() == 1)
  {
    hash_map_type::iterator it, it_end;
    it = node_map.begin();
    it_end = node_map.end();
    
    while (it != it_end)
    {
      VMesh::Node::index_type idx1((*it).first);
      VMesh::Node::index_type idx2((*it).second);
      ofield->copy_value(ifield,idx1,idx2);
      ++it;
    }
  }
  
  // copy property manager
	output->copy_properties(input.get_rep());
  mat_output = rhsmatrixhandle_;
  
  algo_end(); return (true);
}


} // End namespace SCIRunAlgo
