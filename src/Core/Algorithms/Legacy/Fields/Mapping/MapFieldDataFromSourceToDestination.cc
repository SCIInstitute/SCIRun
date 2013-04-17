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

#include <Core/Thread/Thread.h>
#include <Core/Thread/Barrier.h>

#include <Core/Algorithms/Fields/Mapping/MapFieldDataFromSourceToDestination.h>

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

//------------------------------------------------------------
// Algorithm - each destination has its closest source


class MapFieldDataFromSourceToDestinationClosestDataPAlgo
{
  public:
    MapFieldDataFromSourceToDestinationClosestDataPAlgo() :
      barrier_(" MapFieldDataFromSourceToDestinationClosestDataPAlgo Barrier") {}
      
    void parallel(int proc, int nproc);

    VField* sfield_;
    VField* dfield_;
    VMesh*  smesh_;
    VMesh*  dmesh_;
    
    double  maxdist_;
    AlgoBase * algo_;
    
  private:
    Barrier  barrier_;
};

void
MapFieldDataFromSourceToDestinationClosestDataPAlgo::parallel(int proc, int nproc)
{
  // Determine which ones to run
  VField::size_type num_values = dfield_->num_values();
  VField::size_type localsize = num_values/nproc;
  VField::index_type start = localsize*proc;
  VField::index_type end = localsize*(proc+1);
  if (proc == nproc-1) end = num_values;

  barrier_.wait(nproc);
  
  int cnt = 0;
  
  if (dfield_->basis_order() == 0 && sfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::Elem::index_type didx;
    
    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_elem(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  else if (dfield_->basis_order() == 1 && sfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::Elem::index_type didx;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_elem(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  else if (dfield_->basis_order() == 0 && sfield_->basis_order() == 1)
  {
    Point p, r;
    VMesh::Node::index_type didx;
    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_node(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  else if (dfield_->basis_order() == 1 && sfield_->basis_order() == 1)
  {
    Point p, r;
    VMesh::Node::index_type didx;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_node(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  
  barrier_.wait(nproc);
}



//------------------------------------------------------------
// Algorithm - each source will map to one destination

class MapFieldDataFromSourceToDestinationSingleDestinationPAlgo
{
  public:
    MapFieldDataFromSourceToDestinationSingleDestinationPAlgo() :
      barrier_(" MapFieldDataFromSourceToDestinationSingleDestinationPAlgo Barrier") {}
      
    void parallel(int proc, int nproc);

    VField* sfield_;
    VField* dfield_;
    VMesh*  smesh_;
    VMesh*  dmesh_;
    
    std::vector<index_type> tcc_;
    std::vector<index_type> cc_;
  
    double  maxdist_;
    AlgoBase* algo_;
    
  private:
    Barrier  barrier_;
};

void
MapFieldDataFromSourceToDestinationSingleDestinationPAlgo::parallel(int proc, int nproc)
{
  // Determine which ones to run
  VField::size_type num_values = sfield_->num_values();
  VField::size_type localsize = num_values/nproc;
  VField::index_type start = localsize*proc;
  VField::index_type end = localsize*(proc+1);
  if (proc == nproc-1) end = num_values;

  if (proc == 0)
  {
    tcc_.resize(dfield_->num_values(),-1);
    cc_.resize(sfield_->num_values(),-1);
  }

  barrier_.wait(nproc);
  int cnt = 0;
  
  if (sfield_->basis_order() == 0 && dfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::coords_type coords;
    VMesh::Elem::index_type didx;
    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      smesh_->get_center(p,idx);
      double dist;
      if(dmesh_->find_closest_elem(dist,r,coords,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          cc_[idx] = didx;
        }
        else cc_[idx] = -1;
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  else if (sfield_->basis_order() == 1 && dfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::coords_type coords;
    VMesh::Elem::index_type didx;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      smesh_->get_center(p,idx);
      double dist;
      if(dmesh_->find_closest_elem(dist,r,coords,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          cc_[idx] = didx;
        }
        else cc_[idx] = -1;
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  else if (sfield_->basis_order() == 0 && dfield_->basis_order() == 1)
  {
    Point p, r;
    VMesh::Node::index_type didx;
    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      smesh_->get_center(p,idx);
      double dist;
      if(dmesh_->find_closest_node(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          cc_[idx] = didx;
        }
        else cc_[idx] = -1;
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  else if (sfield_->basis_order() == 1 && dfield_->basis_order() == 1)
  {
    Point p, r;
    VMesh::Node::index_type didx;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      smesh_->get_center(p,idx);
      double dist;
      if(dmesh_->find_closest_node(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          cc_[idx] = didx;
        }
        else cc_[idx] = -1;
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  
  barrier_.wait(nproc);

  // Copy the data thread safe
  if (proc == 0)
  {
    VField::size_type num_dvalues = dfield_->num_values();

    for (VMesh::index_type idx=0; idx<num_values;idx++)
    {
      if (cc_[idx] >= 0) tcc_[cc_[idx]] = idx;
    }

    for (VMesh::index_type idx=0; idx<num_dvalues;idx++)
    {
      if (tcc_[idx] >= 0)
      {
        dfield_->copy_value(sfield_,tcc_[idx],idx);
      }
    }
  }
}


//------------------------------------------------------------
// Algorithm - get interpolated data


class MapFieldDataFromSourceToDestinationInterpolatedDataPAlgo
{
  public:
    MapFieldDataFromSourceToDestinationInterpolatedDataPAlgo() :
      barrier_(" MapFieldDataFromSourceToDestinationInterpolatedDataPAlgo Barrier") {}
      
    void parallel(int proc, int nproc);

    VField* sfield_;
    VField* dfield_;
    VMesh*  smesh_;
    VMesh*  dmesh_;
    
    double  maxdist_;
    AlgoBase*  algo_;
    
  private:
    Barrier  barrier_;
};

void
MapFieldDataFromSourceToDestinationInterpolatedDataPAlgo::parallel(int proc, int nproc)
{
  // Determine which ones to run
  VField::size_type num_values = dfield_->num_values();
  VField::size_type localsize = num_values/nproc;
  VField::index_type start = localsize*proc;
  VField::index_type end = localsize*(proc+1);
  if (proc == nproc-1) end = num_values;

  barrier_.wait(nproc);
  
  int cnt = 0;
  if (dfield_->basis_order() == 0 && sfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::Elem::index_type didx;
    
    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      dmesh_->get_center(p,idx);
    
      double dist;
      if(smesh_->find_closest_elem(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  else if (dfield_->basis_order() == 1 && sfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::Elem::index_type didx;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_elem(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  else if (dfield_->basis_order() == 0 && sfield_->basis_order() == 1)
  {
    Point p, r;
    VMesh::coords_type coords;
    VMesh::Elem::index_type didx;
    VMesh::ElemInterpolate interp;
    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_elem(dist,r,coords,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          smesh_->get_interpolate_weights(coords,didx,interp,1);
          dfield_->copy_weighted_value(sfield_,&(interp.node_index[0]),
              &(interp.weights[0]),interp.node_index.size(),idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  else if (dfield_->basis_order() == 1 && sfield_->basis_order() == 1)
  {
    Point p, r;
    VMesh::coords_type coords;
    VMesh::Elem::index_type didx;
    VMesh::ElemInterpolate interp;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_elem(dist,r,coords,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          smesh_->get_interpolate_weights(coords,didx,interp,1);
          dfield_->copy_weighted_value(sfield_,&(interp.node_index[0]),
              &(interp.weights[0]),interp.node_index.size(),idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }

  barrier_.wait(nproc);
}


bool
MapFieldDataFromSourceToDestinationAlgo::
run(FieldHandle source, FieldHandle destination, 
    FieldHandle& output)
{
  algo_start("MapFieldDataFromSourceToDestination");
  
  if (source.get_rep() == 0)
  {
    error("No source field");
    algo_end(); return (false);
  }

  if (destination.get_rep() == 0)
  {
    error("No destination field");
    algo_end(); return (false);
  }

  FieldInformation fis(source);
  FieldInformation fid(destination);
  fid.set_data_type(fis.get_data_type());
  
  if (fid.is_nodata()) fid.make_lineardata();
  output = CreateField(fid,destination->mesh());
  
  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);
  }
  
  // Determine output type

  VMesh* smesh = source->vmesh();
  VMesh* dmesh = output->vmesh();
  VField* sfield = source->vfield();
  VField* dfield = output->vfield();

  // Make sure output field is all empty and of right size
  dfield->resize_values();
  dfield->clear_all_values();
  dfield->set_all_values(get_scalar("default_value"));
  
  std::string method = get_option("method");
  int sbasis_order = sfield->basis_order();
  int dbasis_order = dfield->basis_order();

  if (sbasis_order < 0)
  {
    error("Source field basis order needs to constant or linear");
    algo_end(); return (false);  
  }

  if (dbasis_order < 0)
  {
    error("Destination field basis order needs to constant or linear");
    algo_end(); return (false);  
  }

  if (method == "closestdata")
  {
    if (sbasis_order == 0) smesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
    else smesh->synchronize(Mesh::FIND_CLOSEST_NODE_E);
  } 
  else if(method == "singledestination")
  {
    if (dbasis_order == 0) dmesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
    else dmesh->synchronize(Mesh::FIND_CLOSEST_NODE_E);
  }
  else if (method == "interpolateddata")
  {
    if (smesh->num_elems() > 0)
    {
      smesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
    }
    else
    {
      error("Source does not have any elements, hence one cannot interpolate data in this field");
      error("Use a closestdata interpolation scheme for this data");
      algo_end(); return (false);
    }
  }

  double maxdist = get_scalar("max_distance");
    
  if (method == "closestdata")
  {
    MapFieldDataFromSourceToDestinationClosestDataPAlgo algo;
    algo.sfield_ = sfield;
    algo.dfield_ = dfield;
    algo.smesh_ = smesh;
    algo.dmesh_ = dmesh;
    algo.maxdist_ = maxdist;
    algo.algo_ = this;
    
    int np = Thread::numProcessors();
    Thread::parallel(&algo,&MapFieldDataFromSourceToDestinationClosestDataPAlgo::parallel,np,np);
  }
  else if(method == "singledestination")
  {
    MapFieldDataFromSourceToDestinationSingleDestinationPAlgo algo;
    algo.sfield_ = sfield;
    algo.dfield_ = dfield;
    algo.smesh_ = smesh;
    algo.dmesh_ = dmesh;
    algo.maxdist_ = maxdist;
    algo.algo_ = this;
    
    int np = Thread::numProcessors();
    np = 1;
    Thread::parallel(&algo,&MapFieldDataFromSourceToDestinationSingleDestinationPAlgo::parallel,np,np);
  }
  else if (method == "interpolateddata")
  { 
    MapFieldDataFromSourceToDestinationInterpolatedDataPAlgo algo;
    algo.sfield_ = sfield;
    algo.dfield_ = dfield;
    algo.smesh_ = smesh;
    algo.dmesh_ = dmesh;
    algo.maxdist_ = maxdist;
    algo.algo_ = this;

    int np = Thread::numProcessors();
    Thread::parallel(&algo,&MapFieldDataFromSourceToDestinationInterpolatedDataPAlgo::parallel,np,np);  
  }
 
  dfield->copy_properties(destination->vfield());
  
  algo_end(); return (true);
}


} // end namespace SCIRunAlgo
