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

/// Class definition of this one
#include <Core/Algorithms/Fields/TetGen/InterfaceWithTetGen.h>

/// Need to find out what type of field we are dealing with
#include <Core/Datatypes/FieldInformation.h>

#define TETLIBRARY   // Required definition for use of tetgen library
#include <tetgen.h>

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>

namespace SCIRunAlgo {

bool
InterfaceWithTetGenAlgo::run(FieldHandle mainFldH,
			     std::vector<FieldHandle> &regionsFldH,
			     FieldHandle ptsFldH,
			     FieldHandle regionAttribsFldH,
			     FieldHandle &tetvol_out)
{
  // Mark that we are starting the algorithm
  algo_start("InterfaceWithTetGen");

  tetgenio in, addin, out;
  std::vector<FieldHandle> surfaces;
  surfaces.push_back(mainFldH);
  for (size_t j=0; j<regionsFldH.size(); j++)
    surfaces.push_back(regionsFldH[j]);
  bool add_points = false;

  if (ptsFldH.get_rep())
  {
    VMesh* mesh = ptsFldH->vmesh();
    VMesh::Node::size_type num_nodes = mesh->num_nodes();

    addin.numberofpoints = num_nodes;
    addin.pointlist = new REAL[num_nodes*3];
    for(VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
    {
      Point p;
      mesh->get_center(p, idx);
      addin.pointlist[idx * 3] = p.x();
      addin.pointlist[idx * 3 + 1] = p.y();
      addin.pointlist[idx * 3 + 2] = p.z();
    }
    add_points = true;
    remark("Added extra interior points from Points port.");
  }
  if (regionAttribsFldH.get_rep())
  {
    VMesh* mesh = regionAttribsFldH->vmesh();
    VField* field = regionAttribsFldH->vfield();
    VMesh::Node::size_type num_nodes = mesh->num_nodes();

    in.regionlist = new REAL[num_nodes*5];
    in.numberofregions = num_nodes;
    for(VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
    {
      Point p; double val;
      mesh->get_center(p, idx);
      in.regionlist[idx * 5] = p.x();
      in.regionlist[idx * 5 + 1] = p.y();
      in.regionlist[idx * 5 + 2] = p.z();
      in.regionlist[idx * 5 + 3] = idx;
      field->get_value(val, idx);
      in.regionlist[idx * 5 + 4] = val;
    }
  }
  // indices start from 0.
  in.firstnumber = 0;
  in.mesh_dim = 3;

  int marker = -10;

  VMesh::size_type tot_num_nodes = 0;
  VMesh::size_type tot_num_elems = 0;

  for (size_t j=0; j<surfaces.size(); j++)
  {
    VMesh*  mesh = surfaces[j]->vmesh();
    VMesh::Node::size_type num_nodes = mesh->num_nodes();
    VMesh::Elem::size_type num_elems = 0;

	if (!mesh->is_pointcloudmesh()) {
		num_elems = mesh->num_elems();
	}

    tot_num_nodes += num_nodes;
    tot_num_elems += num_elems;
  }

  in.pointlist = new REAL[(tot_num_nodes) * 3];
  in.numberofpoints = tot_num_nodes;

  in.facetlist = new tetgenio::facet[tot_num_elems];
  in.facetmarkerlist = new int[tot_num_elems];
  in.numberoffacets = tot_num_elems;

  VMesh::index_type idx = 0;
  VMesh::index_type fidx = 0;
  VMesh::index_type off = 0;

  for (size_t j=0; j<surfaces.size(); j++)
  {
    VMesh*  mesh = surfaces[j]->vmesh();
    VMesh::Node::size_type num_nodes = mesh->num_nodes();

    VMesh::Elem::size_type num_elems = 0;

    if (!mesh->is_pointcloudmesh()) {
		num_elems = mesh->num_elems();
	}

    off = idx;
    for(VMesh::Node::index_type nidx=0; nidx<num_nodes; nidx++)
    {
      Point p;
      mesh->get_center(p, nidx);
      in.pointlist[idx * 3] = p.x();
      in.pointlist[idx * 3 + 1] = p.y();
      in.pointlist[idx * 3 + 2] = p.z();
      ++idx;
    }

    // we can skip the rest if this is a point cloud mesh.
    if (mesh->is_pointcloudmesh())
    	continue;

    unsigned int vert_per_face = mesh->num_nodes_per_elem();

    // iterate over faces.
    VMesh::Node::array_type nodes;

    for(VMesh::Elem::index_type eidx=0; eidx<num_elems; eidx++)
    {
      tetgenio::facet *f = &in.facetlist[fidx];
      f->numberofpolygons = 1;
      f->polygonlist = new tetgenio::polygon[1];
      f->numberofholes = 0;
      f->holelist = 0;
      tetgenio::polygon *p = &f->polygonlist[0];
      p->numberofvertices = vert_per_face;
      p->vertexlist = new int[p->numberofvertices];

      mesh->get_nodes(nodes, eidx);
      for (size_t j=0; j<nodes.size(); j++)
        {
          p->vertexlist[j] = VMesh::index_type(nodes[j]) + off;
        }

      in.facetmarkerlist[fidx] = marker;
      ++fidx;
    }

    marker *= 2;
  }

  update_progress(.2);
  // Save files for later debugging.
  // string filename = string(sci_getenv("SCIRUN_TMP_DIR")) + "/tgIN";
  // in.save_nodes((char*)filename.c_str());
  // in.save_poly((char*)filename.c_str());

#if DEBUG
  std::cerr << "\nTetgen command line: " << get_string("cmmd_ln") << std::endl;
#endif
  tetgenio *addtgio = 0;
  if (add_points)
    {
      addtgio = &addin;
    }
  // Create the new mesh.

  try
  {
    tetrahedralize((char*)get_string("cmmd_ln").c_str(), &in, &out, addtgio);
  }
  catch(...)
  {
    error("TetGen failed to generate a mesh");
    algo_end();
    return false;
  }

  update_progress(.9);
  FieldInformation fi(TETVOLMESH_E,CONSTANTDATA_E,DOUBLE_E);
  tetvol_out = CreateField(fi);
  // Convert to a SCIRun TetVol.
  VMesh* mesh = tetvol_out->vmesh();
  VField* field = tetvol_out->vfield();
  for (int i = 0; i < out.numberofpoints; i++)
  {
    Point p(out.pointlist[i*3], out.pointlist[i*3+1], out.pointlist[i*3+2]);
    mesh->add_point(p);
  }

  VMesh::Node::array_type nodes(4);
  VMesh::Node::size_type numnodes = mesh->num_nodes();

  for (int i = 0; i < out.numberoftetrahedra; i++)
  {
    nodes[0] = out.tetrahedronlist[i*4];
    nodes[1] = out.tetrahedronlist[i*4+1];
    nodes[2] = out.tetrahedronlist[i*4+2];
    nodes[3] = out.tetrahedronlist[i*4+3];

    if (nodes[0] >= numnodes ||nodes[1] >= numnodes ||
        nodes[2] >= numnodes ||nodes[3] >= numnodes )
    {
      error("TetGen failed to produce a valid tetrahedralization");
      algo_end();
      return (false);
    }
    mesh->add_elem(nodes);
  }

  field->resize_values();

  int atts =  out.numberoftetrahedronattributes;

  for (int i = 0; i < out.numberoftetrahedra; i++)
    {
      for (int j = 0; j < atts; j++)
      {
        double val = out.tetrahedronattributelist[i * atts + j];
        field->set_value(val, VMesh::Elem::index_type(i));
      }
    }

  update_progress(1.0);
  algo_end(); return (true);
}

} // end namespace SCIRunAlgo
