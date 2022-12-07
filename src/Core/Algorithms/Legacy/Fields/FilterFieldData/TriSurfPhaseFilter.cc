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


#include <Core/Algorithms/Fields/FilterFieldData/TriSurfPhaseFilter.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
TriSurfPhaseFilterAlgo::
run(FieldHandle& input,
    FieldHandle& output,
    FieldHandle& phaseline,
    FieldHandle& phasepoint)
{
  algo_start("TriSurfPhaseFilter");

  if (input.get_rep() == 0)
  {
    algo_end(); error("No input field");
    return (false);
  }

  // no precompiled version available, so compile one

  FieldInformation fi(input);
  FieldInformation fo(input);
  FieldInformation fo2(input);
  FieldInformation fo3(input);

  if (!(fi.is_linear()))
  {
    algo_end(); error("This function has not yet been defined for non-linear elements");
    return (false);
  }

  if ((!fi.is_trisurf())||(!fi.is_lineardata()))
  {
    algo_end(); error("This function works only for TriSurf Meshes with linear interpolated data as input");
    return (false);
  }

  fo.make_trisurfmesh();
  fo.make_double();
  fo2.make_curvemesh();
  fo2.make_double();
  fo3.make_pointcloudmesh();
  fo3.make_double();

  VField* ifield = input->vfield();
  VMesh*  imesh  = input->vmesh();

  output = CreateField(fo);
  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
  }

  VField* ofield = output->vfield();
  VMesh* omesh = output->vmesh();

  phaseline = CreateField(fo2);
  if (phaseline.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
  }

  VMesh*  lmesh = phaseline->vmesh();

  phasepoint = CreateField(fo3);
  if (phasepoint.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
  }

  VField* pfield = phasepoint->vfield();
  VMesh*  pmesh = phasepoint->vmesh();

  VMesh::Node::array_type nodes;
  VMesh::Node::array_type lnodes(2);
  VMesh::Node::array_type nnodes[6];
  double vals[3], temp, v1,v2,v3;
  VMesh::Node::index_type tidx;
  VMesh::Node::index_type idx1, idx2, idx3, idx4, idx5, idx6, idx7, idx8;
  VMesh::Node::index_type idx9, idx10, idx11, idx12, idx13;
  Point points[3], opoints[3], tpoint;


  std::vector<double> fvals;
  std::vector<double> pvals;

  for (int q=0; q<6; q++)
  {
    nnodes[q].resize(3);
  }

  VMesh::index_type cnt = 0;

  double minval;
  ifield->min(minval);
  double offset = 2*M_PI*ceil(fabs(minval)/(2*M_PI)) + M_PI;

  VMesh::size_type num_elems = imesh->num_elems();
  for (VMesh::Elem::index_type idx=0; idx < num_elems; idx++)
  {
    imesh->get_nodes(nodes,idx);
    for (int p = 0; p < 3; p++)
    {
      double t;
      ifield->get_value(t,nodes[p]);
      vals[p] = fmod(t+offset,2*M_PI);
      imesh->get_center(points[p],nodes[p]);
      imesh->get_center(opoints[p],nodes[p]);
    }

    for (int p = 0; p < 2; p++)
    {
      for (int q=p; q<3;q++)
      {
        if (vals[p] < vals[q])
        {
          temp = vals[q]; vals[q] =vals[p]; vals[p] = temp;
          tidx = nodes[q]; nodes[q] = nodes[p]; nodes[p] = tidx;
          tpoint = points[q]; points[q] = points[p]; points[p] = tpoint;
        }
      }
    }

    if (fabs(vals[0]-vals[2]) > M_PI)
    {
      if (fabs(vals[0]-vals[1]) > M_PI)
      {

        Point ph1,ph2;
        v1 = 2*M_PI - fabs(vals[0]-vals[1]);
        if (v1) ph1 = Point(points[0] + (2*M_PI-vals[0])/v1*(points[1]-points[0])); else ph1 = Point(0.5*(points[0]+points[1]));
        v2 = 2*M_PI - fabs(vals[0]-vals[2]);
        if (v2) ph2 = Point(points[0] + (2*M_PI-vals[0])/v2*(points[2]-points[0])); else ph2 = Point(0.5*(points[0]+points[2]));

        idx1 = omesh->add_point(points[0]); fvals.push_back(vals[0]);
        idx2 = omesh->add_point(points[1]); fvals.push_back(vals[1]);
        idx3 = omesh->add_point(points[2]); fvals.push_back(vals[2]);
        idx4 = omesh->add_point(ph1); fvals.push_back(2*M_PI);
        idx5 = omesh->add_point(ph2); fvals.push_back(2*M_PI);
        idx6 = omesh->add_point(ph1); fvals.push_back(0.0);
        idx7 = omesh->add_point(ph2); fvals.push_back(0.0);

        nnodes[0][0] = idx1;
        nnodes[0][1] = idx5;
        nnodes[0][2] = idx4;

        nnodes[1][0] = idx6;
        nnodes[1][1] = idx7;
        nnodes[1][2] = idx2;

        nnodes[2][0] = idx2;
        nnodes[2][1] = idx7;
        nnodes[2][2] = idx3;

        if (Dot(Cross(opoints[1]-opoints[0],opoints[2]-opoints[1]),Cross(points[1]-points[0],points[2]-points[1]))< 0.0)
        {
          for (int q=0; q<3;q++)
          {
            tidx = nnodes[q][0]; nnodes[q][0] = nnodes[q][1]; nnodes[q][1] = tidx;
          }
        }

        omesh->add_elem(nnodes[0]);
        omesh->add_elem(nnodes[1]);
        omesh->add_elem(nnodes[2]);

        lnodes[0] = lmesh->add_point(ph1);
        lnodes[1] = lmesh->add_point(ph2);
        lmesh->add_elem(lnodes);

      }
      else if (fabs(vals[1]-vals[2]) > M_PI)
      {

        Point ph1,ph2;
        v1 = 2*M_PI - fabs(vals[0]-vals[2]);
        if (v1) ph1 = Point(points[0] + (2*M_PI-vals[0])/v1*(points[2]-points[0])); else ph1 = Point(0.5*(points[0]+points[2]));
        v2 = 2*M_PI - fabs(vals[1]-vals[2]);
        if (v2) ph2 = Point(points[1] + (2*M_PI-vals[1])/v2*(points[2]-points[1])); else ph2 = Point(0.5*(points[1]+points[2]));

        idx1 = omesh->add_point(points[0]); fvals.push_back(vals[0]);
        idx2 = omesh->add_point(points[1]); fvals.push_back(vals[1]);
        idx3 = omesh->add_point(points[2]); fvals.push_back(vals[2]);
        idx4 = omesh->add_point(ph1); fvals.push_back(2*M_PI);
        idx5 = omesh->add_point(ph2); fvals.push_back(2*M_PI);
        idx6 = omesh->add_point(ph1); fvals.push_back(0.0);
        idx7 = omesh->add_point(ph2); fvals.push_back(0.0);


        nnodes[0][0] = idx1;
        nnodes[0][1] = idx2;
        nnodes[0][2] = idx5;

        nnodes[1][0] = idx5;
        nnodes[1][1] = idx4;
        nnodes[1][2] = idx1;

        nnodes[2][0] = idx7;
        nnodes[2][1] = idx3;
        nnodes[2][2] = idx6;

        if (Dot(Cross(opoints[1]-opoints[0],opoints[2]-opoints[1]),Cross(points[1]-points[0],points[2]-points[1]))< 0.0)
        {
          for (int q=0; q<3;q++)
          {
            tidx = nnodes[q][0]; nnodes[q][0] = nnodes[q][1]; nnodes[q][1] = tidx;
          }
        }

        omesh->add_elem(nnodes[0]);
        omesh->add_elem(nnodes[1]);
        omesh->add_elem(nnodes[2]);

        lnodes[0] = lmesh->add_point(ph1);
        lnodes[1] = lmesh->add_point(ph2);
        lmesh->add_elem(lnodes);

      }
      else
      {

        Point ph1,ph2,ph3,ph4,ph5,ph6;
        v1 = 2*M_PI - fabs(vals[0]-vals[2]);
        if (v1) ph1 = Point(points[0] + (2*M_PI-vals[0])/v1*(points[2]-points[0])); else ph1 = Point(0.5*(points[0]+points[2]));

        v2 = 2*M_PI - fabs(vals[1]-vals[2]);
        if (v2) ph2 = Point(points[1] + (2*M_PI-vals[1])/v2*(points[2]-points[1])); else ph2 = Point(0.5*(points[1]+points[2]));

        v3 = 2*M_PI - fabs(vals[0]-vals[1]);
        if (v3) ph3 = Point(points[0] + (2*M_PI-vals[0])/v3*(points[1]-points[0])); else ph3 = Point(0.5*(points[0]+points[1]));

        // estimate of rotation point
        double alfa = (M_PI-v2);
        double beta = (M_PI-v3);
        ph4 = Point((1-(M_PI-v1)/M_PI)*ph1 + ((M_PI-v1)/M_PI)*alfa/(alfa+beta)*ph2 + ((M_PI-v1)/M_PI)*beta/(alfa+beta)*ph3);

        ph5 = Point(0.5*(points[0]+points[1]));
        ph6 = Point(0.5*(points[1]+points[2]));

        // contains rotational point
        idx1 = omesh->add_point(points[0]); fvals.push_back(vals[0]);
        idx2 = omesh->add_point(points[1]); fvals.push_back(vals[1]);
        idx3 = omesh->add_point(points[2]); fvals.push_back(vals[2]);
        idx4 = omesh->add_point(ph1); fvals.push_back(2*M_PI);
        idx5 = omesh->add_point(ph1); fvals.push_back(0.0);
        idx6 = omesh->add_point(ph4); fvals.push_back(2*M_PI);
        idx7 = omesh->add_point(ph4); fvals.push_back(8/5*M_PI);
        idx8 = omesh->add_point(ph4); fvals.push_back(6/5*M_PI);
        idx9 = omesh->add_point(ph4); fvals.push_back(4/5*M_PI);
        idx10 = omesh->add_point(ph4); fvals.push_back(2/5*M_PI);
        idx11 = omesh->add_point(ph4); fvals.push_back(0.0);
        idx12 = omesh->add_point(ph5); fvals.push_back(0.5*(vals[0]+vals[1]));
        idx13 = omesh->add_point(ph6); fvals.push_back(0.5*(vals[1]+vals[2]));

        if (Dot(Cross(opoints[1]-opoints[0],opoints[2]-opoints[1]),Cross(points[1]-points[0],points[2]-points[1]))< 0.0)
        {
          nnodes[0][0] = idx4;
          nnodes[0][1] = idx1;
          nnodes[0][2] = idx6;

          nnodes[1][0] = idx1;
          nnodes[1][1] = idx12;
          nnodes[1][2] = idx7;

          nnodes[2][0] = idx12;
          nnodes[2][1] = idx2;
          nnodes[2][2] = idx8;

          nnodes[3][0] = idx2;
          nnodes[3][1] = idx13;
          nnodes[3][2] = idx9;

          nnodes[4][0] = idx13;
          nnodes[4][1] = idx3;
          nnodes[4][2] = idx10;

          nnodes[5][0] = idx3;
          nnodes[5][1] = idx5;
          nnodes[5][2] = idx11;
          pvals.push_back(1.0);

        }
        else
        {
          nnodes[0][1] = idx4;
          nnodes[0][0] = idx1;
          nnodes[0][2] = idx6;

          nnodes[1][1] = idx1;
          nnodes[1][0] = idx12;
          nnodes[1][2] = idx7;

          nnodes[2][1] = idx12;
          nnodes[2][0] = idx2;
          nnodes[2][2] = idx8;

          nnodes[3][1] = idx2;
          nnodes[3][0] = idx13;
          nnodes[3][2] = idx9;

          nnodes[4][1] = idx13;
          nnodes[4][0] = idx3;
          nnodes[4][2] = idx10;

          nnodes[5][1] = idx3;
          nnodes[5][0] = idx5;
          nnodes[5][2] = idx11;
          pvals.push_back(-1.0);
        }

        omesh->add_elem(nnodes[0]);
        omesh->add_elem(nnodes[1]);
        omesh->add_elem(nnodes[2]);
        omesh->add_elem(nnodes[3]);
        omesh->add_elem(nnodes[4]);
        omesh->add_elem(nnodes[5]);

        lnodes[0] = lmesh->add_point(ph1);
        lnodes[1] = lmesh->add_point(ph4);
        lmesh->add_elem(lnodes);

        pmesh->add_point(ph4);

      }

    }
    else
    {

      idx1 = omesh->add_point(points[0]); fvals.push_back(vals[0]);
      idx2 = omesh->add_point(points[1]); fvals.push_back(vals[1]);
      idx3 = omesh->add_point(points[2]); fvals.push_back(vals[2]);
      nnodes[0][0] = idx1;
      nnodes[0][1] = idx2;
      nnodes[0][2] = idx3;

      if (Dot(Cross(opoints[1]-opoints[0],opoints[2]-opoints[1]),Cross(points[1]-points[0],points[2]-points[1]))< 0.0)
      {
        for (int q=0; q<1;q++)
        {
          tidx = nnodes[q][0]; nnodes[q][0] = nnodes[q][1]; nnodes[q][1] = tidx;
        }
      }

      omesh->add_elem(nnodes[0]);
    }
    cnt++; if (cnt == 100) { cnt = 0; update_progress(idx,num_elems); }
  }

  VMesh::Node::iterator nit, nit_end;
  omesh->begin(nit);
  omesh->end(nit_end);
  ofield->resize_values();

  while (nit != nit_end)
  {
    ofield->set_value(fvals[*nit],*nit);
    ++nit;
  }

  VMesh::Node::iterator pnit, pnit_end;
  pmesh->begin(pnit);
  pmesh->end(pnit_end);
  pfield->resize_values();

  while (pnit != pnit_end)
  {
    pfield->set_value(pvals[*pnit],*pnit);
    ++pnit;
  }

  phaseline->vfield()->resize_values();

  // Success:
  algo_end(); return (true);
}

}
