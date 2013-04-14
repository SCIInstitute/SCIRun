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

#include <Core/Algorithms/Fields/Mapping/BuildMappingMatrix.h>

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

class BuildMappingMatrixPAlgoBase
{
public:
  explicit BuildMappingMatrixPAlgoBase(const std::string& barrierName) :
      barrier_(barrierName.c_str()) {}
  VField* sfield_;
  VField* dfield_;
  VMesh*  smesh_;
  VMesh*  dmesh_;

  const SparseRowMatrix::Rows* rr_;
  const SparseRowMatrix::Columns* cc_;
  const SparseRowMatrix::Storage* vv_;

  double  maxdist_;
  AlgoBase* algo_;

protected:
  Barrier  barrier_;
};

class BuildMappingMatrixClosestDataPAlgo : public BuildMappingMatrixPAlgoBase
{
  public:
    BuildMappingMatrixClosestDataPAlgo() :
      BuildMappingMatrixPAlgoBase(" BuildMappingMatrixClosestDataPAlgo Barrier") {}
      
    void parallel(int proc, int nproc);
};

void
BuildMappingMatrixClosestDataPAlgo::parallel(int proc, int nproc)
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
    VMesh::coords_type coords;
    VMesh::Elem::index_type didx;
    
    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_elem(dist,r,coords,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          (*cc_)[idx] = didx;
        }
        else (*cc_)[idx] = -1;
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  else if (dfield_->basis_order() == 1 && sfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::coords_type coords;
    VMesh::Elem::index_type didx;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_elem(dist,r,coords,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          (*cc_)[idx] = didx;
        }
        else (*cc_)[idx] = -1;
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
          (*cc_)[idx] = didx;
        }
        else (*cc_)[idx] = -1;
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
          (*cc_)[idx] = didx;
        }
        else (*cc_)[idx] = -1;
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  
  barrier_.wait(nproc);
  
  if (proc == 0)
  {
    VField::size_type num_dvalues = dfield_->num_values();

    (*rr_)[0] = 0;
    VMesh::index_type k = 0;
    for (VMesh::index_type idx=0; idx<num_dvalues;idx++)
    {
      if ((*cc_)[idx] >= 0)
      {
        (*cc_)[k] = (*cc_)[idx];
        (*vv_)[k] = 1.0;
        k++;
      }
      (*rr_)[idx+1] = k;
    }
  }
}



//------------------------------------------------------------
// Algorithm - each source will map to one destination

class BuildMappingMatrixSingleDestinationPAlgo : public BuildMappingMatrixPAlgoBase
{
  public:
    BuildMappingMatrixSingleDestinationPAlgo() :
      BuildMappingMatrixPAlgoBase(" BuildMappingMatrixSingleDestinationPAlgo Barrier") {}
      
    void parallel(int proc, int nproc);

    std::vector<index_type> tcc_;
};

void
BuildMappingMatrixSingleDestinationPAlgo::parallel(int proc, int nproc)
{
  // Determine which ones to run
  VField::size_type num_values = sfield_->num_values();
  VField::size_type localsize = num_values/nproc;
  VField::index_type start = localsize*proc;
  VField::index_type end = localsize*(proc+1);
  if (proc == nproc-1) end = num_values;

  if (proc == 0)
  {
    tcc_.resize(dfield_->num_values());
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
          (*cc_)[idx] = didx;
        }
        else (*cc_)[idx] = -1;
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
          (*cc_)[idx] = didx;
        }
        else (*cc_)[idx] = -1;
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
          (*cc_)[idx] = didx;
        }
        else (*cc_)[idx] = -1;
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
          (*cc_)[idx] = didx;
        }
        else (*cc_)[idx] = -1;
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  
  barrier_.wait(nproc);
  
  if (proc == 0)
  {
    VField::size_type num_dvalues = dfield_->num_values();
    for (VMesh::index_type idx=0; idx<num_dvalues;idx++)
    {
      tcc_[idx] = -1;
    }
    for (VMesh::index_type idx=0; idx<num_values;idx++)
    {
      tcc_[(*cc_)[idx]] = idx;
    }

    (*rr_)[0] = 0;
    VMesh::index_type k = 0;
    for (VMesh::index_type idx=0; idx<num_dvalues;idx++)
    {
      if (tcc_[idx] >= 0)
      {
        (*cc_)[k] = tcc_[idx];
        (*vv_)[k] = 1.0;
        k++;
      }
      (*rr_)[idx+1] = k;
    }
  }

}


//------------------------------------------------------------
// Algorithm - get interpolated data


class BuildMappingMatrixInterpolatedDataPAlgo : public BuildMappingMatrixPAlgoBase
{
  public:
    BuildMappingMatrixInterpolatedDataPAlgo() :
      BuildMappingMatrixPAlgoBase(" BuildMappingMatrixInterpolatedDataPAlgo Barrier") {}
      
    void parallel(int proc, int nproc);

    size_type e_;
};

void
BuildMappingMatrixInterpolatedDataPAlgo::parallel(int proc, int nproc)
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
          (*cc_)[idx] = didx;
          (*vv_)[idx] = 1.0;
        }
        else 
        {
          (*cc_)[idx] = -1;
          (*vv_)[idx] = 1.0;
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
          (*cc_)[idx] = didx;
          (*vv_)[idx] = 1.0;
        }
        else 
        {
          (*cc_)[idx] = -1;
          (*vv_)[idx] = 1.0;
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
          for (index_type j=0;j<e_;j++)
          {
            (*cc_)[idx*e_+j] = interp.node_index[j];
            (*vv_)[idx*e_+j] = interp.weights[j];
          }
        }
        else
        {
          for (index_type j=0;j<e_;j++)
          {
            (*cc_)[idx*e_+j] = -1;
            (*vv_)[idx*e_+j] = 0.0;          
          }
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
          for (index_type j=0;j<e_;j++)
          {
            (*cc_)[idx*e_+j] = interp.node_index[j];
            (*vv_)[idx*e_+j] = interp.weights[j];
          }
        }
        else
        {
          for (index_type j=0;j<e_;j++)
          {
            (*cc_)[idx*e_+j] = -1;
            (*vv_)[idx*e_+j] = 0.0;          
          }
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress(idx,end); } }
    }
  }
  
  barrier_.wait(nproc);
  
  if (proc == 0)
  {
    VField::size_type num_dvalues = dfield_->num_values();

    (*rr_)[0] = 0;
    VMesh::index_type k = 0;
    VMesh::index_type kk = 0;
    for (VMesh::index_type idx=0; idx<num_dvalues;idx++)
    {
      for (VMesh::index_type j=0;j<e_;j++)
      {
        if ((*cc_)[kk] >= 0)
        {
          (*cc_)[k] = (*cc_)[kk];
          (*vv_)[k] = (*vv_)[kk];
          k++;
        }
        kk++;
      }
      (*rr_)[idx+1] = k;
    }
  }
}


bool
BuildMappingMatrixAlgo::
run(FieldHandle source, FieldHandle destination, MatrixHandle& output)
{
  algo_start("BuildMappingMatrix");
  
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

  // Determine output type
  
  VMesh* smesh = source->vmesh();
  VMesh* dmesh = destination->vmesh();
  VField* sfield = source->vfield();
  VField* dfield = destination->vfield();

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

  size_type n,m,nnz,e; 
  
  if (method == "closestdata")
  {
    m = dfield->num_values();
    n = sfield->num_values();
    nnz = m;
    
    if (sbasis_order == 0) smesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
    else smesh->synchronize(Mesh::FIND_CLOSEST_NODE_E);
  } 
  else if(method == "singledestination")
  {
    m = dfield->num_values();
    n = sfield->num_values();
    nnz = n;
    
    if (dbasis_order == 0) dmesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
    else dmesh->synchronize(Mesh::FIND_CLOSEST_NODE_E);
  }
  else if (method == "interpolateddata")
  {
    m = dfield->num_values();
    n = sfield->num_values();
    if (smesh->num_elems() > 0)
    {
      smesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
      VMesh::coords_type cs; cs[0] =0.0; cs[1] = 0.0; cs[2] = 0.0;
      VMesh::ElemInterpolate ei;
      smesh->get_interpolate_weights(cs,0,ei,sbasis_order);
      if (sbasis_order == 0) 
        { nnz = m; e = 1; }
      else if (sbasis_order == 1) 
        { nnz = m*ei.node_index.size(); e = ei.node_index.size(); }
      else if (sbasis_order == 2) 
        { nnz = m*(ei.node_index.size() + ei.edge_index.size());
          e = (ei.node_index.size() + ei.edge_index.size()); }
    }
    else
    {
      error("Source does not have any elements, hence one cannot interpolate data in this field");
      error("Use a closestdata interpolation scheme for this data");
      algo_end(); return (false);
    }
  }

  SparseRowMatrix::Data outputData(m+1, nnz);

  if (!outputData.allocated())
  {
    error("Could not allocate enough memory for output matrix");
    algo_end(); return (false);     
  }
  const SparseRowMatrix::Rows& rr = outputData.rows();
  const SparseRowMatrix::Columns& cc = outputData.columns();
  const SparseRowMatrix::Storage& vv = outputData.data();
  
  double maxdist = get_scalar("max_distance");
  
  if (method == "closestdata")
  {
    BuildMappingMatrixClosestDataPAlgo algo;
    algo.sfield_ = sfield;
    algo.dfield_ = dfield;
    algo.smesh_ = smesh;
    algo.dmesh_ = dmesh;
    algo.rr_ = &rr;
    algo.cc_ = &cc;
    algo.vv_ = &vv;
    algo.maxdist_ = maxdist;
    algo.algo_ = this;
    
    int np = Thread::numProcessors();
    Thread::parallel(&algo,&BuildMappingMatrixClosestDataPAlgo::parallel,np,np);
  }
  else if(method == "singledestination")
  {
    BuildMappingMatrixSingleDestinationPAlgo algo;
    algo.sfield_ = sfield;
    algo.dfield_ = dfield;
    algo.smesh_ = smesh;
    algo.dmesh_ = dmesh;
    algo.rr_ = &rr;
    algo.cc_ = &cc;
    algo.vv_ = &vv;
    algo.maxdist_ = maxdist;
    algo.algo_ = this;
    
    int np = Thread::numProcessors();
    Thread::parallel(&algo,&BuildMappingMatrixSingleDestinationPAlgo::parallel,np,np);
  }
  else if (method == "interpolateddata")
  { 
    BuildMappingMatrixInterpolatedDataPAlgo algo;
    algo.sfield_ = sfield;
    algo.dfield_ = dfield;
    algo.smesh_ = smesh;
    algo.dmesh_ = dmesh;
    algo.rr_ = &rr;
    algo.cc_ = &cc;
    algo.vv_ = &vv;
    algo.e_ = e;
    algo.maxdist_ = maxdist;
    algo.algo_ = this;
    
    int np = Thread::numProcessors();
    Thread::parallel(&algo,&BuildMappingMatrixInterpolatedDataPAlgo::parallel,np,np);  
  }
  
  output = new SparseRowMatrix(m,n,outputData,nnz);
  if (output.get_rep() == 0)
  {
    error("Could not create output matrix");
    algo_end(); return (false);
  }

  algo_end(); return (true);
}


} // end namespace SCIRunAlgo
