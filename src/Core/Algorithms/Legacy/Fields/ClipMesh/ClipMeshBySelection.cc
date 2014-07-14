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

#include <Core/Algorithms/Fields/ClipMesh/ClipMeshBySelection.h>

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>

#include <sci_hash_map.h>
#include <algorithm>
#include <set>

ClipMeshBySelectionAlgo()
{
  add_option("method","one","element|onenode|majoritynodes|allnodes");
  add_bool("build_mapping",true);
}

namespace SCIRunAlgo {

using namespace SCIRun;

// Version without building mapping matrix
bool
ClipMeshBySelectionAlgo::run(FieldHandle input,
                             FieldHandle selection,
                             FieldHandle& output)
{
  // Store old setting
  bool temp = get_bool("build_mapping");
  set_bool("build_mapping",false);
  MatrixHandle dummy;
  // Run full algorithm
  bool ret = run(input,selection,output,dummy);
  // Reset old setting
  set_bool("build_mapping",temp);
  // Return result
  return (ret);
}

// Version with building mapping matrix

bool
ClipMeshBySelectionAlgo::run(FieldHandle input,
                             FieldHandle selection,
                             FieldHandle& output,
                             MatrixHandle& mapping)
{
  // Mark that we are starting the algorithm, but do not report progress
  algo_start("ClipMeshBySelection");
  // Step 0:
  // Safety test:
  // Test whether we received actually a field. A handle can point to no object.
  // Using a null handle will cause the program to crash. Hence it is a good
  // policy to check all incoming handles and to see whether they point to actual
  // objects.

  // Handle: the function get_rep() returns the pointer contained in the handle
  if (input.get_rep() == 0)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error.
    error("No input field.");
    algo_end(); return (false);
  }

  if (selection.get_rep() == 0)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error.
    error("No selection input field.");
    algo_end(); return (false);
  }


  // Step 1: determine the type of the input fields and determine what type the
  // output field should be.

  FieldInformation fi(input);
  // Here we test whether the class is part of any of these newly defined
  // non-linear classes. If so we return an error.
  if (fi.is_nonlinear())
  {
    error("This algorithm has not yet been defined for non-linear elements yet.");
    algo_end(); return (false);
  }

  printf("Num Elems %d; Num Tets %d\n",static_cast<int>(input->vmesh()->num_elems()),static_cast<int>(selection->vfield()->num_values()));

  FieldInformation fo(input);
  fo.make_unstructuredmesh();

  output = CreateField(fo);
  if (output.get_rep() == 0)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error.
    error("Could not generate output field.");
    algo_end(); return (false);
  }

  std::string method = get_option("method");


  VMesh*   imesh = input->vmesh();
  VMesh*   omesh =  output->vmesh();
  VField*  ifield = input->vfield();
  VField*  ofield = output->vfield();
  VField*  sfield = selection->vfield();

  VMesh::size_type num_elems = imesh->num_elems();
  VMesh::size_type num_nodes = imesh->num_nodes();

  // For point clouds elements and nodes are the same, but the basis order is
  // 0, so force it to run through the element method.
  if (imesh->is_pointcloudmesh()) method = "element";

  if (method == "element")
  {
    std::vector<index_type> node_mapping(imesh->num_nodes(),-1);
    std::vector<index_type> elem_mapping(imesh->num_elems(),-1);
    std::vector<index_type> node_mapping2(imesh->num_nodes(),-1);
    std::vector<index_type> elem_mapping2(imesh->num_elems(),-1);

    printf("Num Elems %d; Num Tets %d\n",static_cast<int>(imesh->num_elems()),static_cast<int>(sfield->num_values()));


    if (imesh->num_elems() != sfield->num_values())
    {
      error("Number of elements in input mesh does not match number of values in selection mesh.");
      algo_end(); return (false);
    }

    VMesh::Node::array_type nodes;
    VMesh::points_type points;

    int cnt = 0;

    for(VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
    {
      char val;
      sfield->get_value(val,idx);
      if (val)
      {
        imesh->get_nodes(nodes,idx);
        imesh->get_centers(points,nodes);

        for (size_t j=0; j<nodes.size();j++)
        {
          if (node_mapping[nodes[j]] < 0)
          {
            node_mapping[nodes[j]] = omesh->add_node(points[j]);
            node_mapping2[node_mapping[nodes[j]]] = nodes[j];
          }
          nodes[j] = node_mapping[nodes[j]];
        }

        elem_mapping[idx] = omesh->add_elem(nodes);
        elem_mapping2[elem_mapping[idx]] = idx;
      }
      cnt++; if (cnt == 100) { cnt=0; update_progress(idx,num_elems);}
    }

    ofield->resize_values();
    VMesh::size_type num_oelems = omesh->num_elems();
    VMesh::size_type num_onodes = omesh->num_nodes();

    if (ofield->basis_order() == 0)
    {
      for(VMesh::Elem::index_type idx=0; idx<num_oelems; idx++)
      {
        ofield->copy_value(ifield,elem_mapping2[idx],idx);
      }
    }
    else if (ofield->basis_order() == 1)
    {
      for(VMesh::Node::index_type idx=0; idx<num_onodes; idx++)
      {
        ofield->copy_value(ifield,node_mapping2[idx],idx);
      }
    }

    bool build_mapping = get_bool("build_mapping");
    if (build_mapping)
    {
      size_type m,n,nnz;

      if (ofield->basis_order() == 0)
      {
        if (num_elems > 0 && num_oelems > 0)
        {
          n =   num_elems;
          nnz = num_oelems;
          m =   num_oelems;
          SparseRowMatrix::Data data(m+1, m);

          if (!data.allocated())
          {
            error("Could not allocate enough memory.");
            algo_end(); return (false);     
          }
          const SparseRowMatrix::Rows& rr = data.rows();
          const SparseRowMatrix::Columns& cc = data.columns();
          const SparseRowMatrix::Storage& vv = data.data();
          
          for (index_type idx=0;idx<m+1;idx++)
          {
            rr[idx] = idx;
          }

          for (index_type idx=0;idx<m;idx++)
          {
            cc[idx] = elem_mapping2[idx];
            vv[idx] = 1.0;
          }

          mapping = new SparseRowMatrix(m,n,data,nnz);
        }
      }
      else if (ofield->basis_order() == 1)
      {
        if (num_nodes > 0 && num_onodes >0)
        {
          n =   num_nodes;
          nnz = num_onodes;
          m =   num_onodes;
          SparseRowMatrix::Data data(m+1, m);

          if (!data.allocated())
          {
            error("Could not allocate enough memory.");
            algo_end(); return (false);     
          }
          const SparseRowMatrix::Rows& rr = data.rows();
          const SparseRowMatrix::Columns& cc = data.columns();
          const SparseRowMatrix::Storage& vv = data.data();

          for (index_type idx=0;idx<m+1;idx++)
          {
            rr[idx] = idx;
          }

          for (index_type idx=0;idx<m;idx++)
          {
            cc[idx] = node_mapping2[idx];
            vv[idx] = 1.0;
          }

          mapping = new SparseRowMatrix(m,n,data,nnz);
        }
      }
      // provide an empty matrix
      if (mapping.get_rep() == 0) mapping = new DenseMatrix(0,0);
    }
  }
  else
  {
    int target = 1;
    if (method == "onenode") target = 1;
    else if (method == "majoritynodes") target = omesh->num_nodes_per_elem()/2;
    else if (method == "allnodes") target = omesh->num_nodes_per_elem();

    std::vector<index_type> node_mapping(imesh->num_nodes(),-1);
    std::vector<index_type> elem_mapping(imesh->num_elems(),-1);
    std::vector<index_type> node_mapping2(imesh->num_nodes(),-1);
    std::vector<index_type> elem_mapping2(imesh->num_elems(),-1);

    printf("Num Nodes %d; Num Tets %d\n",static_cast<int>(imesh->num_nodes()),static_cast<int>(sfield->num_values()));

    if (imesh->num_nodes() != sfield->num_values())
    {
      error("Number of nodes in input mesh does not match number of values in selection mesh.");
      algo_end(); return (false);
    }

    VMesh::Node::array_type nodes;
    VMesh::points_type points;
    std::vector<char> values;

    int cnt = 0;

    for(VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
    {
      imesh->get_nodes(nodes,idx);
      sfield->get_values(values,nodes);
      int ctarget = 0;
      for (size_t j=0; j<values.size(); j++) if (values[j]) ctarget++;

      if (ctarget >= target)
      {
        imesh->get_centers(points,nodes);
        for (size_t j=0; j<nodes.size();j++)
        {
          if (node_mapping[nodes[j]] < 0)
          {
            node_mapping[nodes[j]] = omesh->add_node(points[j]);
            // reverse mapping
            node_mapping2[node_mapping[nodes[j]]] = nodes[j];
          }
          nodes[j] = node_mapping[nodes[j]];
        }

        elem_mapping[idx] = omesh->add_elem(nodes);
        elem_mapping2[elem_mapping[idx]] = idx;
      }

      cnt++; if (cnt == 100) { cnt=0; update_progress(idx,num_elems);}
    }

    ofield->resize_values();
    VMesh::size_type num_oelems = omesh->num_elems();
    VMesh::size_type num_onodes = omesh->num_nodes();

    if (ofield->basis_order() == 0)
    {
      for(VMesh::Elem::index_type idx=0; idx<num_oelems; idx++)
      {
        ofield->copy_value(ifield,elem_mapping2[idx],idx);
      }
    }
    else if (ofield->basis_order() == 1)
    {
      for(VMesh::Node::index_type idx=0; idx<num_onodes; idx++)
      {
        ofield->copy_value(ifield,node_mapping2[idx],idx);
      }
    }

    bool build_mapping = get_bool("build_mapping");
    if (build_mapping)
    {
      size_type m,n,nnz;

      if (ofield->basis_order() == 0)
      {
        if (num_elems > 0 && num_oelems > 0)
        {
          n =   num_elems;
          nnz = num_oelems;
          m =   num_oelems;
          SparseRowMatrix::Data data(m+1, m);

          if (!data.allocated())
          {
            error("Could not allocate enough memory.");
            algo_end(); return (false);     
          }
          const SparseRowMatrix::Rows& rr = data.rows();
          const SparseRowMatrix::Columns& cc = data.columns();
          const SparseRowMatrix::Storage& vv = data.data();

          for (index_type idx=0;idx<m+1;idx++)
          {
            rr[idx] = idx;
          }

          for (index_type idx=0;idx<m;idx++)
          {
            cc[idx] = elem_mapping2[idx];
            vv[idx] = 1.0;
          }

          mapping = new SparseRowMatrix(m,n,data,nnz);
        }
      }
      else if (ofield->basis_order() == 1)
      {
        if (num_nodes > 0 && num_onodes > 0)
        {
          n =   num_nodes;
          nnz = num_onodes;
          m =   num_onodes;
          SparseRowMatrix::Data data(m+1, m);

          if (!data.allocated())
          {
            error("Could not allocate enough memory.");
            algo_end(); return (false);     
          }
          const SparseRowMatrix::Rows& rr = data.rows();
          const SparseRowMatrix::Columns& cc = data.columns();
          const SparseRowMatrix::Storage& vv = data.data();

          for (index_type idx=0;idx<m+1;idx++)
          {
            rr[idx] = idx;
          }

          for (index_type idx=0;idx<m;idx++)
          {
            cc[idx] = node_mapping2[idx];
            vv[idx] = 1.0;
          }
          mapping = new SparseRowMatrix(m,n,data,nnz);
        }
      }
      // provide an empty matrix
      if (mapping.get_rep() == 0) mapping = new DenseMatrix(0,0);
    }
  }

  /// Copy properties of the property manager
	output->copy_properties(input.get_rep());

  // Success:
  algo_end(); return (true);
}

} // End namespace SCIRunAlgo
