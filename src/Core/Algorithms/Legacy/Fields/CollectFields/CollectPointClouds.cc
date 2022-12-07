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


#include <Core/Algorithms/Fields/CollectFields/CollectPointClouds.h>

#include <Core/Datatypes/FieldInformation.h>


namespace SCIRunAlgo {


bool
CollectPointCloudsAlgo::run( FieldHandle field_input_handle,
			     FieldHandle field_acc_handle,
			     FieldHandle &field_output_handle)
{
  algo_start("CollectPointClouds");

  // ----------------------------
  // Check input pointcloud field
  if (field_input_handle.get_rep() == 0)
  {
    error("No input mesh.");
    algo_end(); return (false);
  }

  FieldInformation fi(field_input_handle);

  if (!(fi.is_pointcloudmesh()))
  {
    error("Input mesh needs to be a PointCloudMesh.");
    algo_end(); return (false);
  }

  VMesh::size_type num_nodes = field_input_handle->vmesh()->num_nodes();

  if( num_nodes == 0 )
  {
    error("Input mesh has no nodes.");
    algo_end(); return (false);
  }

  VField* ifield = field_input_handle->vfield();
  VMesh*  imesh =  field_input_handle->vmesh();

  // ----------------------------
  // Check accumulation field

  VField* afield = 0;
  VMesh*  amesh  = 0;

  if (field_acc_handle.get_rep())
  {
    FieldInformation fi2(field_acc_handle);

    if ((!(fi2.is_curvemesh()))||(!(fi2.is_lineardata())))
    {
      error("Output mesh needs to be a Curve mesh with linear data.");
      algo_end(); return (false);
    }
    VMesh::size_type num_acc_nodes = field_acc_handle->vmesh()->num_nodes();

    if ( num_acc_nodes % num_nodes != 0)
    {
      error("Number of nodes in accumulation field is not a multiple of the number of nodes in the pointcloud field.");
      algo_end(); return (false);
    }

    afield = field_acc_handle->vfield();
    amesh  = field_acc_handle->vmesh();
  }

  FieldInformation fo(field_input_handle);
  fo.make_curvemesh();
  fo.make_lineardata();
  fo.make_linearmesh();

  field_output_handle = CreateField(fo);

  if (field_output_handle.get_rep() == 0)
  {
    error("Could not create output mesh.");
    algo_end(); return (false);
  }

  VField* ofield = field_output_handle->vfield();
  VMesh*  omesh =  field_output_handle->vmesh();

  if (afield)
  {
    VMesh::size_type num_acc_nodes = field_acc_handle->vmesh()->num_nodes();

    int fields = num_acc_nodes / num_nodes;
    int max_num_fields = get_int("max_num_fields");

    int start = fields-(max_num_fields-1);
    int end = fields;
    if (start < 0) start = 0;

    VMesh::Node::index_type idx;

    // Get the previous points and move all but perhaps the last to
    // the output field.
    for (idx = start*num_nodes; idx<end*num_nodes; idx++)
    {
      Point p;
      amesh->get_center(p,idx);
      omesh->add_point(p);
    }

    // Add in the next new point to the output field.
    for (idx=0; idx<num_nodes;idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_point(p);
    }

    // Create all of the new elements
    VMesh::Node::array_type nodes(2);
    VMesh::index_type m = 0;

    for (VMesh::index_type k = start*num_nodes; k<end*num_nodes; k++, m++)
    {
      nodes[0] = m;
      nodes[1] = m+num_nodes;
      omesh->add_elem(nodes);
    }
  }

  // No previous field so add all off the points to the output field.
  else
  {
    for (VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
    {
      Point p;
      imesh->get_center(p,idx);
      omesh->add_point(p);
    }
  }

  ofield->resize_values();

  if (afield)
  {
    VMesh::size_type num_acc_nodes = field_acc_handle->vmesh()->num_nodes();

    int fields = num_acc_nodes / num_nodes;
    int max_num_fields = get_int("max_num_fields");

    int start = fields-(max_num_fields-1);
    int end = fields;
    if (start < 0) start = 0;

    // Get the previous data values and move all but perhaps the last to
    // the output field.

    VMesh::Node::index_type idx, odx = 0;

    for (idx = start*num_nodes; idx<end*num_nodes; idx++, odx++)
    {
      ofield->copy_value(afield, idx, odx);
    }

    // Add in the next new value to the output field.
    for (idx=0; idx<num_nodes; idx++, odx++)
    {
      ofield->copy_value(ifield, idx, odx);
    }
  }
  else
  {
    // No previous field so add all off the points to the output field.
    for (VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
    {
      ofield->copy_value(ifield,idx,idx);
    }
  }

  ofield->copy_properties(ifield);
  omesh->copy_properties(imesh);

  algo_end();  return (true);
}

} // end namespace SCIRunAlgo
