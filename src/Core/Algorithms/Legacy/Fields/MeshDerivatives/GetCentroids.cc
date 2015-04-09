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

#include <Core/Algorithms/Fields/MeshDerivatives/GetCentroids.h>

#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>

namespace SCIRunAlgo {

bool
GetCentroidsAlgo::run(FieldHandle input, FieldHandle& output)
{
  algo_start("GetCentroids");
  
  /// Safety check
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }
  
  /// Get the information of the input field
  FieldInformation fo(input);
  fo.make_pointcloudmesh();
  fo.make_nodata();
  
  std::string centroids;
  get_option("centroid",centroids);
    
  VMesh* imesh = input->vmesh();  
  MeshHandle mesh = CreateMesh(fo);
  VMesh* omesh = mesh->vmesh();
      
  if (centroids == "elem")
  {
    VField::size_type num_elems = imesh->num_elems();
    omesh->reserve_nodes(num_elems);
    for (VMesh::Elem::index_type idx=0; idx < num_elems; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }

    output = CreateField(fo,mesh);
    output->vfield()->resize_values();
    
    algo_end(); return (true);
  }

  if (centroids == "node")
  {
    VMesh::size_type num_nodes = imesh->num_nodes();
    omesh->reserve_nodes(num_nodes);
    for (VMesh::Node::index_type idx=0; idx < num_nodes; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }

    output = CreateField(fo,mesh);
    output->vfield()->resize_values();
    
    algo_end(); return (true);
  }

  if (centroids == "edge")
  {
    VMesh::size_type num_edges = imesh->num_edges();
    omesh->reserve_nodes(num_edges);
    for (VMesh::Edge::index_type idx=0; idx < num_edges; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }

    output = CreateField(fo,mesh);
    output->vfield()->resize_values();
    
    algo_end(); return (true);
  }

  if (centroids == "face")
  {
    VMesh::size_type num_faces = imesh->num_faces();
    omesh->reserve_nodes(num_faces);
    for (VMesh::Face::index_type idx=0; idx < num_faces; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }

    output = CreateField(fo,mesh);
    output->vfield()->resize_values();
    
    algo_end(); return (true);
  }

  if (centroids == "cell")
  {
    VMesh::size_type num_cells = imesh->num_cells();
    omesh->reserve_nodes(num_cells);
    for (VMesh::Cell::index_type idx=0; idx < num_cells; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }

    output = CreateField(fo,mesh);
    output->vfield()->resize_values();
    
    algo_end(); return (true);
  }


  if (centroids == "delem")
  {
    VMesh::size_type num_delems = imesh->num_delems();
    omesh->reserve_nodes(num_delems);
    for (VMesh::DElem::index_type idx=0; idx < num_delems; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_node(p);
    }

    output = CreateField(fo,mesh);
    output->vfield()->resize_values();
    
    algo_end(); return (true);
  }

  return false; // nothing done
}

} // end namespace SCIRunAlgo
