//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Modules/Legacy/Fields/InterfaceWithTetGenImpl.h>

#include <Core/Thread/Mutex.h>
#include <Core/Logging/LoggerInterface.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Dataflow/Network/Module.h>

#define TETLIBRARY   // Required definition for use of tetgen library
#include <tetgen.h>

#include <sstream>

#include <sci_debug.h>

using namespace SCIRun;
using namespace Dataflow::Networks;
using namespace Modules::Fields;
using namespace Core::Thread;
using namespace Core::Geometry;

namespace SCIRun {
namespace Modules {
namespace Fields {
namespace detail {

/// @class InterfaceWithTetGen
/// @brief This module interfaces with TetGen.

class InterfaceWithTetGenImplImpl //: public AlgorithmBase
{
  public:
    explicit InterfaceWithTetGenImplImpl(Module* mod);

    FieldHandle runImpl(const std::deque<FieldHandle>& surfaces,
      FieldHandle points, FieldHandle region_attribs, const InterfaceWithTetGenInput& input) const;

  private:
    // translate the ui variables into the string with options
    // that TetGen uses
    Module* module_;
    static Mutex TetGenMutex;
};

Mutex InterfaceWithTetGenImplImpl::TetGenMutex("Protect TetGen from running in parallel");
}}}}

detail::InterfaceWithTetGenImplImpl::InterfaceWithTetGenImplImpl(Module* module) : module_(module)
{}

std::string InterfaceWithTetGenInput::fillCommandOptions(bool addPoints) const
{
  // Collect the options in this output string stream
  std::ostringstream oss;
  if (addPoints)
  {
    oss << "i";
  }

  if (piecewiseFlag_)
  {
    oss << "p";
  }

  if (suppressSplitFlag_)
  {
    if (setSplitFlag_)
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

  if (qualityFlag_)
  {
    oss << "q";
    if (setRatioFlag_)
    {
      oss << minRadius_;
    }
  }
  if (volConstraintFlag_)
  {
    oss << "a";
    if (setMaxVolConstraintFlag_)
    {
      oss << maxVolConstraint_;
    }
  }
  if (detectIntersectionsFlag_)
  {
    oss << "d";
  }
  if (assignFlag_)
  {
    if (setNonzeroAttributeFlag_)
    {
      oss << "AA";
    }
    else
    {
      oss << "A";
    }
  }

  // Add in whatever the user wants to add additionally.
  oss << moreSwitches_;
  return oss.str();
}


FieldHandle detail::InterfaceWithTetGenImplImpl::runImpl(const std::deque<FieldHandle>& surfaces,
  FieldHandle points, FieldHandle region_attribs, const InterfaceWithTetGenInput& input) const
{
#if 0
  if (inputs_changed_ || piecewiseFlag_.changed() ||
      assignFlag_.changed() || setNonzeroAttributeFlag_.changed() ||
      suppressSplitFlag_.changed() || setSplitFlag_.changed() ||
      qualityFlag_.changed() || setRatioFlag_.changed() ||
      volConstraintFlag_.changed() || setMaxVolConstraintFlag_.changed() ||
      minRadius_.changed() || maxVolConstraint_.changed() ||
      detectIntersectionsFlag_.changed() || moreSwitches_.changed() ||
      !oport_cached("TetVol"))
  #endif
  //{
    // Tell SCIRun we are actually doing some work
//    update_state(Executing);

  //TODO DAN: check for proper memory management with these classes.
    tetgenio in, addin, out;

    bool add_points = false;
    if (points)
    {
      VMesh* mesh = points->vmesh();
      VMesh::Node::size_type num_nodes = mesh->num_nodes();

      addin.numberofpoints = num_nodes;
      addin.pointlist = new REAL[num_nodes*3];
      for(VMesh::Node::index_type idx=0; idx<num_nodes; ++idx)
      {
        Point p;
        mesh->get_center(p, idx);
        addin.pointlist[idx * 3] = p.x();
        addin.pointlist[idx * 3 + 1] = p.y();
        addin.pointlist[idx * 3 + 2] = p.z();
      }
      add_points = true;
      module_->remark("Added extra interior points from Points port.");
    }

    if (region_attribs)
    {
      VMesh* mesh = region_attribs->vmesh();
      VField* field = region_attribs->vfield();
      VMesh::Node::size_type num_nodes = mesh->num_nodes();

      in.regionlist = new REAL[num_nodes*5];
      in.numberofregions = num_nodes;
      for(VMesh::Node::index_type idx=0; idx<num_nodes; ++idx)
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
    VMesh::index_type off;

    for (size_t j=0; j< surfaces.size(); j++)
    {
      VMesh*  mesh = surfaces[j]->vmesh();
      VMesh::Node::size_type num_nodes = mesh->num_nodes();
      VMesh::Elem::size_type num_elems = mesh->num_elems();

      off = idx;
      for(VMesh::Node::index_type nidx=0; nidx<num_nodes; ++nidx)
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

      for(VMesh::Elem::index_type eidx=0; eidx<num_elems; ++eidx)
      {
        tetgenio::facet *f = &in.facetlist[fidx];
        f->numberofpolygons = 1;
        f->polygonlist = new tetgenio::polygon[1];
        f->numberofholes = 0;
        f->holelist = nullptr;
        tetgenio::polygon *p = &f->polygonlist[0];
        p->numberofvertices = vert_per_face;
        p->vertexlist = new int[p->numberofvertices];

        mesh->get_nodes(nodes, eidx);
        for (size_t i=0; i<nodes.size(); i++)
        {
          p->vertexlist[i] = VMesh::index_type(nodes[i]) + off;
        }

        in.facetmarkerlist[fidx] = marker;
        ++fidx;
      }

      marker *= 2;
    }

    module_->getUpdaterFunc()(.2);
    // Save files for later debugging.
    // std::string filename = std::string(sci_getenv("SCIRUN_TMP_DIR")) + "/tgIN";
    // in.save_nodes((char*)filename.c_str());
    // in.save_poly((char*)filename.c_str());

    std::string cmmd_ln = input.fillCommandOptions(add_points);
  #if DEBUG
    std::cerr << "\nTetgen command line: " << cmmd_ln << std::endl;
  #endif
    tetgenio *addtgio = nullptr;
    if (add_points)
    {
       addtgio = &addin;
    }
    // Create the new mesh.

    // Make sure only one thread accesses TetGen
    // It is not thread safe :)
    {
      Guard g(TetGenMutex.get());
      try
      {
        tetrahedralize(const_cast<char*>(cmmd_ln.c_str()), &in, &out, addtgio);
      }
      catch(std::exception& e)
      {
        module_->error(std::string("TetGen failed to generate a mesh: ") + e.what());
        return nullptr;
      }
      catch (...)
      {
        module_->error("TetGen failed to generate a mesh");
        return nullptr;
      }
    }

    module_->getUpdaterFunc()(.9);
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
        module_->error("TetGen failed to produce a valid tetrahedralization");
        return nullptr;
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

    module_->getUpdaterFunc()(1.0);
    return tetvol_out;
}

InterfaceWithTetGenInput::InterfaceWithTetGenInput() :
piecewiseFlag_(true),
assignFlag_(true),
setNonzeroAttributeFlag_(false),
suppressSplitFlag_(true),
setSplitFlag_(false),
qualityFlag_(true),
setRatioFlag_(false),
volConstraintFlag_(false),
setMaxVolConstraintFlag_(false),
// see TetGen documentation for "-q" switch: default is 2.0
minRadius_(2.0),
maxVolConstraint_(0.1),
detectIntersectionsFlag_(false),
moreSwitches_("")
{
}

InterfaceWithTetGenImpl::InterfaceWithTetGenImpl(Module* module, const InterfaceWithTetGenInput& input) :
  impl_(new detail::InterfaceWithTetGenImplImpl(module)),
  inputFlags_(input)
{}

FieldHandle InterfaceWithTetGenImpl::runImpl(const std::deque<FieldHandle>& surfaces, FieldHandle points, FieldHandle region_attribs) const
{
  return impl_->runImpl(surfaces, points, region_attribs, inputFlags_);
}
