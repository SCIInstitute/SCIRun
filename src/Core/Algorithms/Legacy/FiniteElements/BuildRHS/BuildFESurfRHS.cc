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


#include <Core/Algorithms/Legacy/FiniteElements/BuildRHS/BuildFESurfRHS.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <boost/lexical_cast.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
//using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Logging;

ALGORITHM_PARAMETER_DEF(FiniteElements, InputField);
ALGORITHM_PARAMETER_DEF(FiniteElements, BoundaryField);
ALGORITHM_PARAMETER_DEF(FiniteElements, RHSMatrix);

struct IndexHash {
  static const size_t bucket_size = 4;
  static const size_t min_buckets = 8;

  size_t operator()(const index_type &idx) const
    { return (static_cast<size_t>(idx)); }

  bool operator()(const index_type &i1, const index_type &i2) const
    { return (i1 < i2); }
};

bool BuildFESurfRHSAlgo::run(FieldHandle input, FieldHandle& output,  MatrixHandle& mat_output) const
{

   /// Define types we need for mapping
#ifdef HAVE_HASH_MAP
  typedef hash_map<index_type,index_type,IndexHash> hash_map_type;
#else
  typedef std::map<index_type,index_type,IndexHash> hash_map_type;
#endif
  hash_map_type node_map;
  hash_map_type elem_map;

  /// Check whether we have an input field
  if (!input)
  {
    error("No input field");
    return false;
  }

  FieldInformation fi(input);
  FieldInformation fo(input);

  /// We do not yet support Quadratic and Cubic Meshes here
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    return false;
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
    output=input;
    return true;
  }

  /// Check whether we could make a conversion
  if (!found_method)
  {
    error("No method available for mesh of type: " + fi.get_mesh_type());
    output=input;
    return false;
  }

  /// Create the output field
  output = CreateField(fo);
  if (!output)
  {
    error("Could not create output field");
    return false;
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
    output=input;
    return false;
  }

  imesh->begin(be);
  imesh->end(ee);

  // Set up output matrix
  VMesh::Node::size_type mns;
  DenseMatrix *rhsmatrix_;

  imesh->size(mns);  // this assumes basis order < 2
   try
  {
    rhsmatrix_ = new DenseMatrix(mns, 1);
  }
  catch (...)
  {
    error("Error alocating output matrix");
    return false;
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
	      output=input;
              return false;
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

	  (*rhsmatrix_)(inodes[q],0)+=l_val;  // in SCIRun4 it used to be: rhsmatrix_->add(inodes[q], 0, l_val);
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

 mat_output = (MatrixHandle)rhsmatrix_;
 return true;

}


AlgorithmOutput BuildFESurfRHSAlgo::run(const AlgorithmInput& input) const
{
  auto input_field = input.get<Field>(Parameters::InputField);

  FieldHandle out;
  MatrixHandle rhs;
  if(!run(input_field, out,  rhs))
  {
    error("Error: Algorithm of BuildFESurfRHS failed.");
  }
  AlgorithmOutput output;
  output[Parameters::BoundaryField] = out;
  output[Parameters::RHSMatrix] = rhs;

  return output;
}
