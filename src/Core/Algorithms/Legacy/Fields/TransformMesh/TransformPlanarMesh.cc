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


#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>

#include <Core/Algorithms/Fields/TransformMesh/TransformPlanarMesh.h>

namespace SCIRunAlgo {

using namespace SCIRun;


bool
TransformPlanarMeshAlgo::
run(FieldHandle input, FieldHandle& output)
{
  algo_start("TransformPlanarMesh");

  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  output = input->clone();
  output->mesh_detach();

  bool invert = get_bool("invert");

  Transform trans;
  Vector axisVec;

  double sign = (invert ? -1 : 1);

  Point p0;
  Point p1;

  VMesh* vmesh = output->vmesh();
  VMesh::size_type num_nodes = vmesh->num_nodes();

  /// Get the first point along the line.
  if (num_nodes > 0)
  {
    vmesh->get_center(p0,VMesh::Node::index_type(0));
  }
  else
  {
    error("Cannot get the first point - No mesh");
    algo_end(); return (false);
  }

  if (num_nodes > 1)
  {
    vmesh->get_center(p1,VMesh::Node::index_type(num_nodes-1));
  }
  else
  {
    error("Cannot get the last point - No mesh");
    algo_end(); return (false);
  }

  /// For a line assume that it is colinear.
  if (vmesh->dimensionality() == 1)
  {
    axisVec = Vector(p1 - p0);
  }
  else
  {
    /// For a surface assume that it is planar.

    // Translate the mesh to the center of the view.
    const BBox bbox = vmesh->get_bounding_box();

    if (bbox.valid())
    {
      Point center = -bbox.center();
      trans.post_translate( Vector( center ) );
    }

    Point p2;

    if (num_nodes > 1)
    {
      vmesh->get_center(p2,VMesh::Node::index_type(num_nodes/2));
    }
    else
    {
      error("Cannot get the middle point - No mesh");
      algo_end(); return (false);
    }

    Vector vec0 = Vector(p2 - p0);
    vec0.safe_normalize();

    Vector vec1 = Vector(p2 - p1);
    vec1.safe_normalize();

    axisVec = Cross( vec0, vec1 );

    axisVec.safe_normalize();

    std::cerr << axisVec << std::endl;
  }

  axisVec.safe_normalize();

  std::string axis = get_option("axis");
  /// Rotate only if not in the -Z or +Z axis.
  if( axis != "z" || fabs( fabs( axisVec.z() ) - 1.0 ) > 1.0e-4 )
  {
    double theta = atan2( axisVec.y(), axisVec.x() );
    double phi   = acos( axisVec.z() / axisVec.length() );

    /// Rotate the line into the xz plane.
    trans.pre_rotate( -theta, Vector(0,0,1) );
    /// Rotate the line into the z axis.
    trans.pre_rotate( -phi,   Vector(0,1,0) );
  }

  if( axis == "x" )
  {
    /// Rotate the line into the x axis.
    trans.pre_rotate( sign * M_PI/2.0, Vector(0,1,0));
  }
  else if( axis == "y" )
  {
    /// Rotate the line into the y axis.
    trans.pre_rotate( sign * -M_PI/2.0, Vector(1,0,0));
  }
  else if( invert )
  {
    /// Rotate the line into the z axis.
    trans.pre_rotate( M_PI, Vector(1,0,0));
  }

  vmesh->transform(trans);

  double tx = get_scalar("translate_x");
  double ty = get_scalar("translate_y");

  // Optionally translate the mesh away from the center of the view.
  if (tx || ty)
  {
    const BBox bbox = vmesh->get_bounding_box();

    if (bbox.valid())
    {
      Vector size = bbox.diagonal();

      Transform trans;
      trans.post_translate( Vector( 0, tx * size.y(), ty * size.z() ) );

      vmesh->transform(trans);
    }
  }

  algo_end(); return (true);
}


} // end namespace SCIRunAlgo
