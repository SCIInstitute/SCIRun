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


#include <Core/Algorithms/Fields/TracePoints/TracePoints.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool TracePointsAlgo::run(FieldHandle pointcloud,
                          FieldHandle old_curvefield,
                          FieldHandle& curvefield)
{
  algo_start("TracePoints");

  double val = get_scalar("value");
  double tol = get_scalar("tolerance");

  if (pointcloud.get_rep() == 0)
  {
    error("No input field.");
    algo_end(); return (false);
  }

  VField* pfield = pointcloud->vfield();
  VMesh*  pmesh  = pointcloud->vmesh();

  // no precompiled version available, so compile one

  FieldInformation fi(pointcloud);
  FieldInformation fo(pointcloud);

  if (!(fi.is_pointcloud()))
  {
    error("This function works only for PointCloud meshes as input.");
    algo_end(); return (false);
  }

  fo.make_curvemesh();

  VMesh  *cmesh;
  VField *cfield;

  if (old_curvefield.get_rep())
  {
    old_curvefield.detach();
    curvefield = old_curvefield;

    cfield = curvefield->vfield();
    cmesh = curvefield->vmesh();
  }
  else
  {
    curvefield = CreateField(fo);
    if (curvefield.get_rep() == 0)
    {
      error("Could not create output mesh");
      algo_end(); return (false);
    }

    cfield = curvefield->vfield();
    cmesh = curvefield->vmesh();
  }


  FieldHandle pointcloud_old;
  double val_old;

  if (cfield->is_property("end_points"))
  {
    cfield->get_property("end_points",pointcloud_old);
    cfield->set_property("end_points",pointcloud,false);
    cfield->get_property("value",val_old);
    cfield->set_property("value",val,false);
  }
  else
  {
    cfield->set_property("end_points",pointcloud,false);
    cfield->set_property("value",val,false);
    algo_end(); return (true);
  }

  if (pointcloud_old.get_rep() == 0)
  {
    error("TracePoints: Could not obtain end_points field");
    algo_end(); return (false);
  }

  VField *pfield2 = pointcloud_old->vfield();
  VMesh  *pmesh2  = pointcloud_old->vmesh();

  VMesh::Node::iterator it, it_end;
  VMesh::Node::iterator it2, it_end2;
  double pval, pval2;
  VMesh::Node::array_type na(2);
  Point p, p2,p3;
  double dist = DBL_MAX;
  double tol2 = tol*tol;

  pmesh->begin(it);
  pmesh->end(it_end);

  std::vector<double> vals;
  VMesh::size_type k = cfield->num_values();

  while (it != it_end)
  {
    pmesh->get_center(p,*it);
    pfield->value(pval,*it);
    pmesh2->begin(it2);
    pmesh2->end(it_end2);
    while (it2 != it_end2)
    {
      pmesh2->get_center(p2,*it2);
      pfield2->get_value(pval2,*it);
      Vector v(p2-p);
      if (pval2 == pval) if (v.length2() < dist) p3 = p2;
    }

    if (dist < tol2)
    {
      na[0] = cmesh->add_point(p);
      vals.push_back(val);
      na[1] = cmesh->add_point(p3);
      vals.push_back(val_old);
      pmesh2->add_elem(na);
    }
  }

  cfield->resize_values();
  VMesh::size_type sz = cfield->num_values();
  for (VMesh::index_type i=0 ;k<sz;k++,i++)
    cfield->set_value(vals[i],k);

  // Success:
  algo_end();
  return (true);
}


} // End namespace SCIRunAlgo
