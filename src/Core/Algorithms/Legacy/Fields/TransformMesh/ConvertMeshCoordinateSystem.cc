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


#include <Core/Algorithms/Fields/TransformMesh/ConvertMeshCoordinateSystem.h>
#include <Core/Datatypes/VField.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/FieldInformation.h>


namespace SCIRunAlgo {

using namespace SCIRun;

/// @todo: check for input field's current coordinate system?
// Is there a way to check for input field's current coordinate system?

bool
ConvertMeshCoordinateSystemAlgo::
run(FieldHandle input, FieldHandle& output)
{
  // Mark that we are starting the algorithm, but do not report progress
  algo_start("ConvertMeshCoordinateSystem");

  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  FieldInformation fi(input);

  if (fi.is_nonlinear())
  {
    error("This algorithm is not yet formulated for a non-linear mesh or field");
    algo_end(); return (false);
  }

  fi.make_irregularmesh();

  MeshHandle mesh = CreateMesh(fi);

  if (mesh.get_rep() == 0)
  {
    error("Could not allocate output mesh");
    algo_end(); return (false);
  }

  VField* ifield = input->vfield();
  VMesh*  imesh = input->vmesh();
  VMesh*  omesh = mesh->vmesh();

  VMesh::size_type num_nodes = imesh->num_nodes();

  std::string oldsystem = get_option("input_system");
  std::string newsystem = get_option("output_system");

  VMesh::size_type tot_estimate = (VMesh::size_type)(num_nodes*1.3);
  int cnt = 0;

  double theta, phi, r, x, y, z;

  for (VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
  {
    Point pO, pE, pN; // point in "Old", "Cartesian", and "New" coordinates

    imesh->get_center(pO,idx);

    // transform from old system to Cartesian
    if (oldsystem=="cartesian")
    {
      pE=pO;
    }
    else if (oldsystem=="spherical")
    {
      // pO was in Spherical coordinates -- transform it to Cartesian
      theta=pO.x();
      phi=pO.y();
      r=pO.z();
      pE.x(r*sin(phi)*sin(theta));
      pE.y(r*sin(phi)*cos(theta));
      pE.z(r*cos(phi));
    }
    else if (oldsystem=="polar")
    {
      // pO was in Polar coordinates -- transform to Cartesian
      theta=pO.x();
      r=pO.y();
      z=pO.z();
      pE.x(r*sin(theta));
      pE.y(r*cos(theta));
      pE.z(z);
    }
    else if (oldsystem=="range")
    {
      // pO was in Range coordinates -- transform to Cartesian
          // first convert range-to-spherical, then spherical-to-cartesian
      theta=pO.x();
      phi=pO.y();
      r=pO.z();
      phi=atan(tan(phi)*sin(theta));
      phi=M_PI/2.-phi;    // phi went + to -, but we need it to be just pos
      pE.x(-r*sin(phi)*cos(theta)); // theta sweeps across x
      pE.y(r*cos(phi)); // phi (tilt of scanner) is y
      pE.z(r*sin(phi)*sin(theta)); // depth is z
    }

    x=pE.x();
    y=pE.y();
    z=pE.z();

    // transform from Cartesian to new system
    if (newsystem=="cartesian")
    {
      pN=pE;
    }
    else if (newsystem=="spherical")
    {
      theta=atan2(y,x);
      r=sqrt(x*x+y*y+z*z);
      phi=acos(z/r);
      pN.x(theta);
      pN.y(phi);
      pN.z(r);
    }
    else if (newsystem=="polar")
    {
      theta=atan2(x,y);
      r=sqrt(x*x+y*y);
      pN.x(theta);
      pN.y(r);
      pN.z(z);
    }
    else if (newsystem=="range")
    {
      // first convert cartesian-to-spherical, then spherical-to-range
      theta=atan2(z,x);    // theta's in the x/z plane (y is up)
      r=sqrt(x*x+y*y+z*z);
      phi=acos(y/r);       // y is up
      phi=M_PI/2.-phi; // zero at equator
      phi=atan(tan(phi)/sin(theta));  // wedge shaped
      if (phi>M_PI) phi=M_PI; else if (phi<-M_PI) phi=-M_PI;
      pN.x(-theta);
      pN.y(phi);
      pN.z(r);
    }


    omesh->add_point(pN);

    cnt++; if (cnt == 200) { cnt = 0; update_progress(idx,tot_estimate); }
  }

  output = CreateField(fi,mesh);
  VField* ofield = output->vfield();

  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);
  }

  omesh->copy_elems(imesh);
  ofield->copy_values(ifield);

  output->copy_properties(input.get_rep());

  algo_end(); return (true);
}

} // End namespace SCIRunAlgo
