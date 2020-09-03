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

#include <Core/Thread/Parallel.h>
#include <Core/Thread/Barrier.h>

#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoNodes.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MappingDataSource.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Thread;
using namespace SCIRun;

MapFieldDataOntoNodesAlgo::MapFieldDataOntoNodesAlgo()
{
  using namespace Parameters;
  addOption(Quantity, "value", "value|gradient|gradientnorm|flux");
  addOption(InterpolationModel, "interpolateddata", "interpolateddata|closestnodedata|closestinterpolateddata");
  addParameter(OutsideValue, 0.0);
  addParameter(MaxDistance, std::numeric_limits<double>::max());
}

namespace detail {

class MapFieldDataOntoNodesPAlgo : public Interruptible
{
  public:
    explicit MapFieldDataOntoNodesPAlgo(unsigned int numProcs) :
    algo_(0), is_flux_(false),
      barrier_("MapFieldDataOntoNodesPAlgo Barrier", numProcs), nproc(numProcs) {}

    void parallel(int proc);

    FieldHandle sfield_;
    FieldHandle wfield_;
    FieldHandle ofield_;

    const AlgorithmBase* algo_;

    bool is_flux_;
    std::vector<bool> success_;

  private:
    Barrier barrier_;
    unsigned int nproc;
};

void
MapFieldDataOntoNodesPAlgo::parallel(int proc)
{
  success_[proc] = true;

  // Each thread has its own Datasource class, so it can preallocate array internally
  MappingDataSourceHandle datasource = CreateDataSource(sfield_,wfield_,algo_);

  if (!datasource)
  {
    success_[proc] = false;
  }

  barrier_.wait();

  for (unsigned int j=0; j<nproc; j++)
  {
    if (!success_[j])
      return;
  }

  barrier_.wait();

  VMesh* omesh = ofield_->vmesh();
  VField* ofield = ofield_->vfield();

  VMesh::Node::size_type  num_nodes = omesh->num_nodes();
  VField::size_type       localsize = num_nodes/nproc;
  VField::index_type      start = localsize*proc;
  VField::index_type      end = localsize*(proc+1);
  if (proc == nproc-1) end = num_nodes;

  int cnt = 0;
  if (is_flux_)
  {
    // To compute flux through a surface
    Point p; Vector val; Vector norm;
    for (VMesh::Node::index_type idx=start; idx<end; idx++)
    {
      checkForInterruption();
      omesh->get_center(p,idx);
      omesh->get_normal(norm,idx);
      datasource->get_data(val,p);
      ofield->set_value(Dot(val,norm),idx);
      if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress_max(idx,end); } }
    }
  }
  else
  {
    // To map value, gradient, or gradientnorm
    if (datasource->is_scalar())
    {
      Point p; double val;
      for (VMesh::Node::index_type idx=start; idx<end; idx++)
      {
        checkForInterruption();
        omesh->get_center(p,idx);
        datasource->get_data(val,p);
        ofield->set_value(val,idx);
        if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress_max(idx,end); } }
      }
    }
    else if (datasource->is_vector())
    {
      Point p; Vector val;
      for (VMesh::Node::index_type idx=start; idx<end; idx++)
      {
        checkForInterruption();
        omesh->get_center(p,idx);
        datasource->get_data(val,p);
        ofield->set_value(val,idx);
        if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress_max(idx,end); } }
      }
    }
    else
    {
      Point p; Tensor val;
      for (VMesh::Node::index_type idx=start; idx<end; idx++)
      {
        checkForInterruption();
        omesh->get_center(p,idx);
        datasource->get_data(val,p);
        ofield->set_value(val,idx);
        if (proc == 0) { cnt++; if (cnt == 400) {cnt = 0; algo_->update_progress_max(idx,end); } }
      }
    }
  }
  // Wait until all of the threads are done
  success_[proc] = true;
  barrier_.wait();
}
}

bool
MapFieldDataOntoNodesAlgo::runImpl(FieldHandle source, FieldHandle weights,
    FieldHandle destination, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "MapFieldDataOntoNodes");

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

  FieldInformation fi(source);
  FieldInformation fo(destination);
  fo.make_lineardata();

  std::string quantity = getOption(Parameters::Quantity);
  std::string mappingModel = getOption(Parameters::InterpolationModel);

  if (mappingModel == "closestnodedata")
  {
    if (!fi.is_lineardata() && !fi.is_pointcloud())
    {
      error("Closest node data mapping only works for source data located at the nodes with linear basis.");
      return (false);
    }
  }

  if (fi.is_pointcloud() && (mappingModel == "interpolateddata" || mappingModel == "closestinterpolateddata"))
  {
    warning("Point cloud source data will produce the same mapping as a closestnodedata mapping since the data lacks a basis for interpolation. See https://github.com/SCIInstitute/SCIRun/issues/2155");
  }

  if (fi.is_nodata())
  {
    error("No data in source field.");
    return (false);
  }

  if (weights)
  {
    FieldInformation wfi(weights);
    if (mappingModel == "closestnodedata")
    {
      if (!wfi.is_lineardata())
      {
        error("Closest node data only works for weights data located at the nodes.");
        return (false);
      }
    }

    if (wfi.is_nodata())
    {
      error("No data in weights field.");
      return (false);
    }
  }

  // Make sure output equals quantity to be computed

  if (quantity == "value")
  {
    // Copy the output datatype
    fo.set_data_type(fi.get_data_type());
  }
  else  if (quantity == "gradient")
  {
    // Output will be a vector
    if (!fi.is_scalar())
    {
      error("Gradient can only be calculated on a scalar field.");
      return (false);
    }
    fo.make_vector();
  }
  else if (quantity == "gradientnorm")
  {
    // Output will be a double
    if (!fi.is_scalar())
    {
      error("Gradient can only be calculated on a scalar field.");
      return (false);
    }
    fo.make_double();
  }
  else if (quantity == "flux")
  {
    // Only for output surfaces (only field with normals) and output will
    // be double
    if (!fi.is_scalar())
    {
      error("Flux can only be calculated on a scalar field.");
      return (false);
    }
    if (!fo.is_surface())
    {
      error("Flux can only be computed for surfaces meshes as destination");
      return (false);
    }
    fo.make_double();
  }

  // Incorporate the weights and alter the datatype to reflect that
  if (weights)
  {
    FieldInformation wfi(weights);
    if ((!wfi.is_tensor())&&(!wfi.is_scalar()))
    {
      error("Weights field needs to be a scalar or a tensor.");
      return (false);
    }

    if (fo.is_scalar() && wfi.is_tensor())
    {
      fo.make_tensor();
    }

    if (fo.is_tensor() && wfi.is_tensor())
    {
      error("Weights and source field cannot be both tensor data.");
      return (false);
    }
  }

  // Create new output field
  output = CreateField(fo,destination->mesh());
  output->vfield()->resize_values();

  if (!output)
  {
    error("Could not allocate output field");
    return (false);
  }

  // Number of threads is equal to the number of cores
  int np = Parallel::NumCores();
  // Run algorithm in parallel
  detail::MapFieldDataOntoNodesPAlgo algo(np);

  algo.sfield_ = source;
  algo.wfield_ = weights;
  algo.ofield_ = output;
  algo.algo_ = this;

  algo.success_.resize(np,true);
  // Mark whether it is a flux computation
  algo.is_flux_ = quantity == "flux";

  auto task_i = [&algo](int i) { algo.parallel(i); };
  Parallel::RunTasks(task_i, Parallel::NumCores());

 // Check whether algorithm succeeded
  for (int j=0; j<np; j++)
  {
    if (!algo.success_[j])
    {
      // Should not be able to get here
      error("The algorithm failed for an unknown reason.");
      return (false);
    }
  }
  CopyProperties(*destination, *output);

  return (true);
}

bool
MapFieldDataOntoNodesAlgo::runImpl(FieldHandle source, FieldHandle destination, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr (this, "MapFieldDataOntoNodes");

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

  FieldInformation fi(source);
  FieldInformation fo(destination);
  fo.make_lineardata();

  std::string quantity = getOption(Parameters::Quantity);
  std::string mappingModel = getOption(Parameters::InterpolationModel);

  if (mappingModel == "closestnodedata")
  {
    if (!fi.is_lineardata() && !fi.is_pointcloud())
    {
      error("Closest node data mapping only works for source data located at the nodes with linear basis.");
      return (false);
    }
  }
  
  if (fi.is_pointcloud() && (mappingModel == "interpolateddata" || mappingModel == "closestinterpolateddata"))
  {
    warning("Point cloud source data will produce the same mapping as a closestnodedata mapping since the data lacks a basis for interpolation. See https://github.com/SCIInstitute/SCIRun/issues/2155");
  }

  if (fi.is_nodata())
  {
    error("No data in source field.");
    return (false);
  }

  // Make sure output equals quantity to be computed

  if (quantity == "value")
  {
    // Copy the output datatype
    fo.set_data_type(fi.get_data_type());
  }
  else if (quantity == "gradient")
  {
    // Output will be a vector
    if (!fi.is_scalar())
    {
      error("Gradient can only be calculated on a scalar field.");
      return (false);
    }
    fo.make_vector();
  }
  else if (quantity == "gradientnorm")
  {
    // Output will be a double
    if (!fi.is_scalar())
    {
      error("Gradient can only be calculated on a scalar field.");
      return (false);
    }
    fo.make_double();
  }
  else if (quantity == "flux")
  {
    // Only for output surfaces (only field with normals) and output will
    // be double
    if (!fi.is_scalar())
    {
      error("Flux can only be calculated on a scalar field.");
      return (false);
    }
    if (!fo.is_surface())
    {
      error("Flux can only be computed for surfaces meshes as destination");
      return (false);
    }
    fo.make_double();
  }

  // Create new output field
  output = CreateField(fo,destination->mesh());
  output->vfield()->resize_values();

  if (!output)
  {
    error("Could not allocate output field");
    return (false);
  }

  // Number of threads is equal to the number of cores
  int np = Parallel::NumCores();
  // Run algorithm in parallel
  detail::MapFieldDataOntoNodesPAlgo algo(np);

  algo.sfield_ = source;
  algo.ofield_ = output;
  algo.algo_ = this;

  algo.success_.resize(np,true);
  // Mark whether it is a flux computation
  algo.is_flux_ = quantity == "flux";

  auto task_i = [&algo](int i) { algo.parallel(i); };
  Parallel::RunTasks(task_i, Parallel::NumCores());

 // Check whether algorithm succeeded
  for (int j=0; j<np; j++)
  {
    if (!algo.success_[j])
    {
      // Should not be able to get here
      error("The algorithm failed for an unknown reason.");
      return (false);
    }
  }
  CopyProperties(*destination, *output);

  return (true);
}

const AlgorithmInputName MapFieldDataOntoNodesAlgo::Source("Source");
const AlgorithmInputName MapFieldDataOntoNodesAlgo::Destination("Destination");
const AlgorithmInputName MapFieldDataOntoNodesAlgo::Weights("Weights");

AlgorithmOutput MapFieldDataOntoNodesAlgo::run(const AlgorithmInput& input) const
{
  auto source = input.get<Field>(Source);
  auto destination = input.get<Field>(Destination);
  auto weights = input.get<Field>(Weights);

  FieldHandle outputField;

  if (!runImpl(source, weights, destination, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False thrown on legacy run call");

  AlgorithmOutput output;
  output[Variables::OutputField] = outputField;

  return output;
}
