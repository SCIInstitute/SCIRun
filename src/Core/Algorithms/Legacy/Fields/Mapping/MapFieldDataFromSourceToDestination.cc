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


#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromSourceToDestination.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Thread/Parallel.h>
#include <Core/Thread/Barrier.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

#include <boost/scoped_ptr.hpp>

using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Thread;
using namespace SCIRun;

ALGORITHM_PARAMETER_DEF(Fields, DefaultValue);
ALGORITHM_PARAMETER_DEF(Fields, MappingMethod);

const AlgorithmOutputName MapFieldDataFromSourceToDestinationAlgo::Remapped_Destination("Remapped_Destination");

MapFieldDataFromSourceToDestinationAlgo::MapFieldDataFromSourceToDestinationAlgo()
{
  using namespace Parameters;
  addParameter(DefaultValue, 0.0);
  addParameter(MaxDistance, -1.0);
  addOption(MappingMethod, "closestdata", "interpolateddata|closestdata|singledestination");
}

namespace detail
{
  class MapFieldDataFromSourceToDestinationPAlgoBase : public Interruptible
  {
  public:
    explicit MapFieldDataFromSourceToDestinationPAlgoBase(const std::string& name, int nproc) :
      sfield_(0), dfield_(0), smesh_(0), dmesh_(0), maxdist_(0), algo_(0),
      barrier_(name, nproc), nproc_(nproc) {}

    virtual ~MapFieldDataFromSourceToDestinationPAlgoBase() {}
    virtual void parallel(int proc) = 0;

    VField* sfield_;
    VField* dfield_;
    VMesh*  smesh_;
    VMesh*  dmesh_;

    double  maxdist_;
    const AlgorithmBase* algo_;

  protected:
    Barrier barrier_;
    int nproc_;
  };


//------------------------------------------------------------
// Algorithm - each destination has its closest source

class MapFieldDataFromSourceToDestinationClosestDataPAlgo : public MapFieldDataFromSourceToDestinationPAlgoBase
{
  public:
    explicit MapFieldDataFromSourceToDestinationClosestDataPAlgo(int nproc) :
      MapFieldDataFromSourceToDestinationPAlgoBase(" MapFieldDataFromSourceToDestinationClosestDataPAlgo Barrier", nproc) {}

    virtual void parallel(int proc) override;
};

void
MapFieldDataFromSourceToDestinationClosestDataPAlgo::parallel(int proc)
{
  // Determine which ones to run
  VField::size_type num_values = dfield_->num_values();
  VField::size_type localsize = num_values/nproc_;
  VField::index_type start = localsize*proc;
  VField::index_type end = localsize*(proc+1);
  if (proc == nproc_-1) end = num_values;

  barrier_.wait();

  int cnt = 0;

  if (dfield_->basis_order() == 0 && sfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::Elem::index_type didx;

    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      checkForInterruption();
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_elem(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
    }
  }
  else if (dfield_->basis_order() == 1 && sfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::Elem::index_type didx;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      checkForInterruption();
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_elem(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
    }
  }
  else if (dfield_->basis_order() == 0 && sfield_->basis_order() == 1)
  {
    Point p, r;
    VMesh::Node::index_type didx;
    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      checkForInterruption();
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_node(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
    }
  }
  else if (dfield_->basis_order() == 1 && sfield_->basis_order() == 1)
  {
    Point p, r;
    VMesh::Node::index_type didx;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      checkForInterruption();
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_node(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
    }
  }

  barrier_.wait();
}



//------------------------------------------------------------
// Algorithm - each source will map to one destination

class MapFieldDataFromSourceToDestinationSingleDestinationPAlgo : public MapFieldDataFromSourceToDestinationPAlgoBase
{
  public:
    explicit MapFieldDataFromSourceToDestinationSingleDestinationPAlgo(int nproc) :
      MapFieldDataFromSourceToDestinationPAlgoBase(" MapFieldDataFromSourceToDestinationSingleDestinationPAlgo Barrier", nproc) {}

    virtual void parallel(int proc) override;

    std::vector<index_type> tcc_;
    std::vector<index_type> cc_;
};

void
MapFieldDataFromSourceToDestinationSingleDestinationPAlgo::parallel(int proc)
{
  // Determine which ones to run
  VField::size_type num_values = sfield_->num_values();
  VField::size_type localsize = num_values/nproc_;
  VField::index_type start = localsize*proc;
  VField::index_type end = localsize*(proc+1);
  if (proc == nproc_-1) end = num_values;

  if (proc == 0)
  {
    tcc_.resize(dfield_->num_values(),-1);
    cc_.resize(sfield_->num_values(),-1);
  }

  barrier_.wait();
  int cnt = 0;

  if (sfield_->basis_order() == 0 && dfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::coords_type coords;
    VMesh::Elem::index_type didx;
    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      checkForInterruption();
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
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
    }
  }
  else if (sfield_->basis_order() == 1 && dfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::coords_type coords;
    VMesh::Elem::index_type didx;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      checkForInterruption();
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
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
    }
  }
  else if (sfield_->basis_order() == 0 && dfield_->basis_order() == 1)
  {
    Point p, r;
    VMesh::Node::index_type didx;
    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      checkForInterruption();
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
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
    }
  }
  else if (sfield_->basis_order() == 1 && dfield_->basis_order() == 1)
  {
    Point p, r;
    VMesh::Node::index_type didx;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      checkForInterruption();
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
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
    }
  }

  barrier_.wait();

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


class MapFieldDataFromSourceToDestinationInterpolatedDataPAlgo : public MapFieldDataFromSourceToDestinationPAlgoBase
{
  public:
    explicit MapFieldDataFromSourceToDestinationInterpolatedDataPAlgo(int nproc) :
      MapFieldDataFromSourceToDestinationPAlgoBase(" MapFieldDataFromSourceToDestinationInterpolatedDataPAlgo Barrier", nproc) {}

    virtual void parallel(int proc) override;
};

void
MapFieldDataFromSourceToDestinationInterpolatedDataPAlgo::parallel(int proc)
{
  // Determine which ones to run
  VField::size_type num_values = dfield_->num_values();
  VField::size_type localsize = num_values/nproc_;
  VField::index_type start = localsize*proc;
  VField::index_type end = localsize*(proc+1);
  if (proc == nproc_-1) end = num_values;

  barrier_.wait();

  int cnt = 0;
  if (dfield_->basis_order() == 0 && sfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::Elem::index_type didx;

    for (VMesh::Elem::index_type idx=start; idx<end;idx++)
    {
      checkForInterruption();
      dmesh_->get_center(p,idx);

      double dist;
      if(smesh_->find_closest_elem(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
    }
  }
  else if (dfield_->basis_order() == 1 && sfield_->basis_order() == 0)
  {
    Point p, r;
    VMesh::Elem::index_type didx;
    for (VMesh::Node::index_type idx=start; idx<end;idx++)
    {
      checkForInterruption();
      dmesh_->get_center(p,idx);
      double dist;
      if(smesh_->find_closest_elem(dist,r,didx,p))
      {
        if (maxdist_ < 0.0 || dist < maxdist_)
        {
          dfield_->copy_value(sfield_,didx,idx);
        }
      }
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
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
      checkForInterruption();
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
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
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
      checkForInterruption();
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
      if (proc == 0) { cnt++; if (cnt == 200) {cnt = 0; algo_->update_progress_max(idx,end); } }
    }
  }

  barrier_.wait();
}

}

bool
MapFieldDataFromSourceToDestinationAlgo::runImpl(FieldHandle source, FieldHandle destination, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "MapFieldDataFromSourceToDestination");
  using namespace Parameters;

  if (!source)
  {
    error("No source field");
    return (false);
  }

  if (!destination)
  {
    error("No destination field");
    return (false);
  }

  FieldInformation fis(source);
  FieldInformation fid(destination);
  fid.set_data_type(fis.get_data_type());

  if (fid.is_nodata()) fid.make_lineardata();
  output = CreateField(fid,destination->mesh());

  if (!output)
  {
    error("Could not allocate output field");
    return (false);
  }

  // Determine output type

  VMesh* smesh = source->vmesh();
  VMesh* dmesh = output->vmesh();
  VField* sfield = source->vfield();
  VField* dfield = output->vfield();

  // Make sure output field is all empty and of right size
  dfield->resize_values();
  dfield->clear_all_values();
  dfield->set_all_values(get(DefaultValue).toDouble());

  std::string method = getOption(MappingMethod);
  int sbasis_order = sfield->basis_order();
  int dbasis_order = dfield->basis_order();

  if (sbasis_order < 0)
  {
    error("Source field basis order needs to constant or linear");
    return (false);
  }

  if (dbasis_order < 0)
  {
    error("Destination field basis order needs to constant or linear");
    return (false);
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
      return (false);
    }
  }

  if (fis.is_pointcloud() && method != "closestdata")
  {
    warning("Point cloud source data will produce the same mapping as a closestnodedata mapping since the data lacks a basis for interpolation. See https://github.com/SCIInstitute/SCIRun/issues/2154");
  }

  double maxdist = get(MaxDistance).toDouble();

  boost::scoped_ptr<detail::MapFieldDataFromSourceToDestinationPAlgoBase> algoP;
  int np = Parallel::NumCores();
  if (method == "closestdata")
  {
    algoP.reset(new detail::MapFieldDataFromSourceToDestinationClosestDataPAlgo(np));
  }
  else if(method == "singledestination")
  {
    np = 1; //TODO: ???
    algoP.reset(new detail::MapFieldDataFromSourceToDestinationSingleDestinationPAlgo(np));
  }
  else if (method == "interpolateddata")
  {
    algoP.reset(new detail::MapFieldDataFromSourceToDestinationInterpolatedDataPAlgo(np));
  }

  if (!algoP)
    THROW_ALGORITHM_INPUT_ERROR("Invalid mapping method");

  algoP->sfield_ = sfield;
  algoP->dfield_ = dfield;
  algoP->smesh_ = smesh;
  algoP->dmesh_ = dmesh;
  algoP->maxdist_ = maxdist;
  algoP->algo_ = this;

  auto task_i = [&algoP](int i) { algoP->parallel(i); };
  Parallel::RunTasks(task_i, np);

  CopyProperties(*destination, *output);

  return (true);
}

AlgorithmOutput MapFieldDataFromSourceToDestinationAlgo::run(const AlgorithmInput& input) const
{
  auto source = input.get<Field>(Variables::Source);
  auto destination = input.get<Field>(Variables::Destination);

  FieldHandle output_field;
  if (!runImpl(source, destination, output_field))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned from legacy run call");

  AlgorithmOutput output;
  output[Remapped_Destination] = output_field;

  return output;
}
