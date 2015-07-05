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

#include <Core/Algorithms/Fields/MeshDerivatives/CalculateMeshCenter.h>

#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>

namespace SCIRunAlgo {

bool
CalculateMeshCenterAlgo::run(FieldHandle input, FieldHandle& output)
{
  algo_start("CalculateMeshCenter");
  
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

  if (input->vmesh()->num_nodes() == 0)
  {
    warning("Input field does not have any nodes, output will be an empty field.");
    output = CreateField(fo);
    algo_end(); return (true);
  }
  
  VMesh* imesh = input->vmesh(); 
   
  MeshHandle mesh = CreateMesh(fo);
  
  if (imesh->num_nodes() == 0)
  {
    warning("Input field does contain any nodes, output will be an empty field");  
    output = CreateField(fo);
    algo_end(); return (true);
  }    
  
  Point center(0.0,0.0,0.0);
  
  if (check_option("method","node-center"))    
  {
    Point c(0.0,0.0,0.0);
    VField::size_type num_nodes = imesh->num_nodes();
    for (VMesh::Node::index_type idx=0; idx < num_nodes; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      c = Point(c + p);
    }
    if (num_nodes)
    {
      center = (c*(1.0/static_cast<double>(num_nodes)));
    }
  }
  else if (check_option("method","elem-center"))  
  {
    Point c(0.0,0.0,0.0);
    VField::size_type num_elems = imesh->num_elems();
    for (VMesh::Elem::index_type idx=0; idx < num_elems; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      c = Point(c + p);
    }
    center = (c*(1.0/static_cast<double>(num_elems)));
  }  
  else if (check_option("method","weighted-elem-center"))  
  {
    Point c(0.0,0.0,0.0);
    VField::size_type num_elems = imesh->num_elems();
    double size =0.0;
    double weight;
    for (VMesh::Elem::index_type idx=0; idx < num_elems; idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      weight = Abs(imesh->get_size(idx));
      size += weight;
      c = Point(c + weight*p);
    }
    center = (c*(1.0/size));
  }  
  else if (check_option("method","bounding-box-center"))  
  {
    BBox b = imesh->get_bounding_box();
    center = (0.5*(b.max() + b.min())).point();
  } 
  else if (check_option("method","mid-node-index"))  
  {
    VMesh::Node::index_type mid_idx = (imesh->num_nodes()>>1);
    imesh->get_center(center,mid_idx);
  }
  else if (check_option("method","mid-elem-index"))  
  {
    VMesh::Elem::index_type mid_idx = (imesh->num_elems()>>1);
    imesh->get_center(center,mid_idx);
  }

  output = CreateField(fo);
  output->vmesh()->add_point(center);
  output->vfield()->resize_values();
    
  algo_end(); return (true);
}

} // end namespace SCIRunAlgo
