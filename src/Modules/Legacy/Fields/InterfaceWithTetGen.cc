//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2009 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

///
///    @file    InterfaceWithTetGen.cc
///    @author  Martin Cole
///    @date    Wed Mar 22 07:56:22 2006
///

#include <Core/Thread/Mutex.h>

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#define TETLIBRARY   // Required definition for use of tetgen library
#include <tetgen.h>

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>

#include <sci_debug.h>

namespace SCIRun {

Mutex TetGenMutex("Protect TetGen from running in parallel");

class InterfaceWithTetGen : public Module
{
  public:
    // Constructor/destructor
    InterfaceWithTetGen(GuiContext* ctx);
    virtual ~InterfaceWithTetGen() {}

    // to run the module
    virtual void execute();
    
  private:
    // translate the ui variables into the string with options
    // that TetGen uses
    void fillCommandOptions(std::string& commandLine, bool addPoints);

    GuiInt piecewiseFlag_;            // -p
    GuiInt assignFlag_;               // -A
    GuiInt setNonzeroAttributeFlag_;  // -AA
    GuiInt suppressSplitFlag_;        // -Y
    GuiInt setSplitFlag_;             // -YY
    GuiInt qualityFlag_;              // -q
    GuiInt setRatioFlag_;             // -q
    GuiInt volConstraintFlag_;        // -a
    GuiInt setMaxVolConstraintFlag_;  // -a
    GuiDouble minRadius_;
    GuiDouble maxVolConstraint_;
    GuiInt detectIntersectionsFlag_;  // -d
    GuiString moreSwitches_;          // additional flags
};

DECLARE_MAKER(InterfaceWithTetGen)
  
InterfaceWithTetGen::InterfaceWithTetGen(GuiContext* ctx) : 
  Module("InterfaceWithTetGen", ctx, Filter, "NewField", "SCIRun"),
  piecewiseFlag_(ctx->subVar("piecewiseFlag"), 1),
  assignFlag_(ctx->subVar("assignFlag"), 1),
  setNonzeroAttributeFlag_(ctx->subVar("setNonzeroAttributeFlag"), 0),
  suppressSplitFlag_(ctx->subVar("suppressSplitFlag"), 1),
  setSplitFlag_(ctx->subVar("setSplitFlag"), 0),
  qualityFlag_(ctx->subVar("qualityFlag"), 1),
  setRatioFlag_(ctx->subVar("setRatioFlag"), 0),
  volConstraintFlag_(ctx->subVar("volConstraintFlag"), 0),
  setMaxVolConstraintFlag_(ctx->subVar("setMaxVolConstraintFlag"), 0),
  // see TetGen documentation for "-q" switch: default is 2.0
  minRadius_(ctx->subVar("minRadius"), 2.0),
  maxVolConstraint_(ctx->subVar("maxVolConstraint"), 0.1),
  detectIntersectionsFlag_(ctx->subVar("detectIntersectionsFlag"), 0),
  moreSwitches_(ctx->subVar("moreSwitches"), "")
{
}

void
InterfaceWithTetGen::fillCommandOptions(std::string& commandLine, bool addPoints)
{
  // Collect the options in this output string stream
  std::ostringstream oss;
  commandLine.clear();
  if (addPoints) 
  {
    oss << "i";
  }

  if (piecewiseFlag_.get()) 
  {
    oss << "p";
  }

  if (suppressSplitFlag_.get()) 
  {
    if (setSplitFlag_.get()) 
    {
      oss << "YY";
    }
    else 
    {
      oss << "Y";
    }
  }
  
  // Always use zero indexing that is the only thing SCIRun knows....
  oss << "z";
  
  if (qualityFlag_.get()) 
  {
    oss << "q";
    if (setRatioFlag_.get()) 
    {
      oss << minRadius_.get();
    }
  }
  if (volConstraintFlag_.get()) 
  {
    oss << "a";
    if (setMaxVolConstraintFlag_.get()) 
    {
      oss << maxVolConstraint_.get();
    }
  }
  if (detectIntersectionsFlag_.get()) 
  {
    oss << "d";
  }
  if (assignFlag_.get()) 
  {
    if (setNonzeroAttributeFlag_.get()) 
    {
      oss << "AA";
    }
    else 
    {
      oss << "A";
    }
  }
  
  // Add in whatever the user wants to add additionally.
  std::string s = moreSwitches_.get();
  if (s.size() > 0) 
  {
    oss << s;
  }
  commandLine = oss.str();
}

void
InterfaceWithTetGen::execute() 
{
  tetgenio in, addin, out;

  FieldHandle first_surface;
  std::vector<FieldHandle> surfaces, tsurfaces;

  get_input_handle("Main", first_surface, true);
  surfaces.push_back(first_surface);

  get_dynamic_input_handles("Regions", tsurfaces, false);
  for (size_t j=0; j< tsurfaces.size(); j++) surfaces.push_back(tsurfaces[j]);
  
  FieldHandle points;
  get_input_handle("Points", points, false);
  
  FieldHandle region_attribs;
  get_input_handle( "Region Attribs", region_attribs, false);

  if (inputs_changed_ || piecewiseFlag_.changed() ||
      assignFlag_.changed() || setNonzeroAttributeFlag_.changed() ||
      suppressSplitFlag_.changed() || setSplitFlag_.changed() ||
      qualityFlag_.changed() || setRatioFlag_.changed() ||
      volConstraintFlag_.changed() || setMaxVolConstraintFlag_.changed() ||
      minRadius_.changed() || maxVolConstraint_.changed() ||
      detectIntersectionsFlag_.changed() || moreSwitches_.changed() ||
      !oport_cached("TetVol"))
  {
    // Tell SCIRun we are actually doing some work
    update_state(Executing);

    bool add_points = false;  
    if (points.get_rep()) 
    {
      VMesh* mesh = points->vmesh();    
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

    if (region_attribs.get_rep()) 
    {
      VMesh* mesh = region_attribs->vmesh();
      VField* field = region_attribs->vfield();
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

    for (size_t j=0; j< surfaces.size(); j++)
    {
      VMesh*  mesh = surfaces[j]->vmesh();
      VMesh::Node::size_type num_nodes = mesh->num_nodes();
      VMesh::Elem::size_type num_elems = mesh->num_elems();

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
    
    for (size_t j=0; j< surfaces.size(); j++)
    {
      VMesh*  mesh = surfaces[j]->vmesh();
      VMesh::Node::size_type num_nodes = mesh->num_nodes();
      VMesh::Elem::size_type num_elems = mesh->num_elems();

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
    // std::string filename = std::string(sci_getenv("SCIRUN_TMP_DIR")) + "/tgIN";
    // in.save_nodes((char*)filename.c_str());
    // in.save_poly((char*)filename.c_str());

    std::string cmmd_ln;
    fillCommandOptions(cmmd_ln, add_points);
  #if DEBUG
    std::cerr << "\nTetgen command line: " << cmmd_ln << std::endl;
  #endif
    tetgenio *addtgio = 0;
    if (add_points) 
    {
       addtgio = &addin;
    }
    // Create the new mesh.

    // Make sure only one thread accesses TetGen
    // It is not thread safe :)
    TetGenMutex.lock();
    try
    {
      tetrahedralize((char*)cmmd_ln.c_str(), &in, &out, addtgio); 
    }
    catch(...)
    {
      TetGenMutex.unlock();
      error("TetGen failed to generate a mesh");
      return;
    }
    TetGenMutex.unlock();
    
    update_progress(.9);
    FieldInformation fi(TETVOLMESH_E,CONSTANTDATA_E,DOUBLE_E);
    FieldHandle tetvol_out = CreateField(fi);
    // Convert to a SCIRun TetVol.
    
    VMesh* mesh = tetvol_out->vmesh();
    VField* field = tetvol_out->vfield();
    for (int i = 0; i < out.numberofpoints; i++) 
    {
      Point p(out.pointlist[i*3], out.pointlist[i*3+1], out.pointlist[i*3+2]);
      mesh->add_point(p);
    }  

    VMesh::Node::size_type numnodes = mesh->num_nodes();
    VMesh::Node::array_type nodes(4);
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
        return;
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

    // Send the output to SCIRun
    send_output_handle("TetVol", tetvol_out);
  }
}


} // end namespace SCIRun

