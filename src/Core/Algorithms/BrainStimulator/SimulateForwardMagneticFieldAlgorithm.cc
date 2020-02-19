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


#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Thread/Barrier.h>
#include <Core/Thread/Parallel.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Logging/ScopedTimeRemarker.h>
#include <Core/Logging/Log.h>
#include <Core/Algorithms/BrainStimulator/SimulateForwardMagneticFieldAlgorithm.h>
#include <string>
#include <vector>
#include <algorithm>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::BrainStimulator;
using namespace SCIRun::Core::Logging;

AlgorithmInputName SimulateForwardMagneticFieldAlgo::ElectricField("ElectricField");
AlgorithmInputName SimulateForwardMagneticFieldAlgo::ConductivityTensor("ConductivityTensor");
AlgorithmInputName SimulateForwardMagneticFieldAlgo::DipoleSources("DipoleSources");
AlgorithmInputName SimulateForwardMagneticFieldAlgo::DetectorLocations("DetectorLocations");
AlgorithmOutputName SimulateForwardMagneticFieldAlgo::MagneticField("MagneticField");
AlgorithmOutputName SimulateForwardMagneticFieldAlgo::MagneticFieldMagnitudes("MagneticFieldMagnitudes");

class CalcFMField
{
  public:

    CalcFMField(const AlgorithmBase* algo) : algo_(algo),
      np_(-1),efld_(0),ctfld_(0),dipfld_(0),detfld_(0),emsh_(0),ctmsh_(0),dipmsh_(0),detmsh_(0),magfld_(0),magmagfld_(0)
    {
    }

    boost::tuple<FieldHandle, FieldHandle> calc_forward_magnetic_field(FieldHandle efield,
					   FieldHandle ctfield,
					   FieldHandle dipoles,
					   FieldHandle detectors);

  private:
    void interpolate(int proc, Point p);
    void set_up_cell_cache();
    void calc_parallel(int proc);

    const AlgorithmBase* algo_;
    int np_;
    std::vector<Vector> interp_value_;
    std::vector<std::pair<std::string, Tensor> > tens_;
    bool have_tensors_;

    struct per_cell_cache {
      Vector cur_density_;
      Point  center_;
      double volume_;
    };

    std::vector<per_cell_cache>  cell_cache_;

    VField* efld_; // Electric Field
    VField* ctfld_; // Conductivity Field
    VField* dipfld_; // Dipole Field
    VField* detfld_; // Detector Field

    VMesh* emsh_; // Electric Field
    VMesh* ctmsh_; // Conductivity Field
    VMesh* dipmsh_; // Dipole Field
    VMesh* detmsh_; // Detector Field

    VField* magfld_; // Magnetic Field
    VField* magmagfld_; // Magnetic Field Magnitudes
};

void CalcFMField::interpolate(int proc, Point p)
{
  emsh_->synchronize(Mesh::ELEM_LOCATE_E);

  VMesh::Elem::index_type inside_cell = 0;
  bool outside = !(emsh_->locate(inside_cell, p));

  VMesh::size_type num_elems = emsh_->num_elems();

  for (VMesh::Elem::index_type idx; idx<num_elems; idx++)
  {
    if (outside || idx != inside_cell)
    {
      per_cell_cache &c = cell_cache_[idx];
      Vector radius = p - c.center_;

      Vector valueJXR = Cross(c.cur_density_, radius);
      double length = radius.length();

      interp_value_[proc] += ((valueJXR / (length * length * length)) * c.volume_);
    }
  }
}

void CalcFMField::set_up_cell_cache()
{
  VMesh::size_type num_elems = emsh_->num_elems();
  Vector elemField;
  cell_cache_.resize(num_elems);

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  if (have_tensors_)
  {
    int material = -1;
    for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
    {
      per_cell_cache c;
      emsh_->get_center(c.center_,idx);
      efld_->get_value(elemField,idx);
      ctfld_->get_value(material,idx);

      c.cur_density_ = tens_[material].second * -1 * elemField;
      c.volume_ = emsh_->get_volume(idx);
      cell_cache_[idx] = c;
    }
  } else
#endif

  if (ctfld_ && ctfld_->is_tensor())
  {
    Tensor ten;
    for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
    {
      per_cell_cache c;
      emsh_->get_center(c.center_,idx);
      efld_->get_value(elemField,idx);
      ctfld_->get_value(ten,idx);

      c.cur_density_ = ten * -1 * elemField;
      c.volume_ = emsh_->get_volume(idx);
      cell_cache_[idx] = c;
    }
  }
  else
  {
    double val;
    for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
    {
      per_cell_cache c;
      emsh_->get_center(c.center_,idx);
      efld_->get_value(elemField,idx);
      ctfld_->get_value(val,idx);
      c.cur_density_ = val * -1 * elemField;
      c.volume_ = emsh_->get_volume(idx);
      cell_cache_[idx] = c;
    }
  }
}

void CalcFMField::calc_parallel(int proc)
{

  VMesh::size_type num_nodes = detmsh_->num_nodes();
  VMesh::size_type nodes_per_thread = num_nodes/np_;

  VMesh::Node::index_type start = proc*nodes_per_thread;
  VMesh::Node::index_type end = (proc+1)*nodes_per_thread;
  if (proc == (np_-1)) end = num_nodes;

  Vector mag_field;
  Point  pt;
  Point  pt2;
  Vector P;
  const double one_over_4_pi = 1.0 / (4 * M_PI);

  VMesh::size_type num_dipoles = dipmsh_->num_nodes();

  int cnt = 0;
  for (VMesh::Node::index_type idx = start; idx < end; idx++ )
  {
    // finish loop iteration.

    detmsh_->get_center(pt, idx);

    // init the interp val to 0
    interp_value_[proc] = Vector(0,0,0);
    interpolate(proc, pt);

    mag_field = interp_value_[proc];

    Vector normal;
    detfld_->get_value(normal,idx);

    // iterate over the dipoles.
    for (VMesh::Node::index_type dip_idx = 0; dip_idx < num_dipoles; dip_idx++)
    {
      dipmsh_->get_center(pt2, dip_idx);
      dipfld_->value(P,dip_idx);

      Vector radius = pt - pt2; // detector - source
      Vector valuePXR = Cross(P, radius);
      double length = radius.length();

      mag_field += valuePXR / (length * length * length);
    }

    mag_field *= one_over_4_pi;
    magmagfld_->set_value(Dot(mag_field, normal),idx);
    magfld_->set_value(mag_field,idx);

    if (proc == 0)
    {
      cnt++;
      if (cnt == 100)
      {
      	cnt = 0;
      	algo_->update_progress_max(idx,end);
      }
    }
  }

}

boost::tuple<FieldHandle,FieldHandle> CalcFMField::calc_forward_magnetic_field(FieldHandle efield, FieldHandle ctfield, FieldHandle dipoles, FieldHandle detectors)
{
  efld_ = efield->vfield();
  ctfld_ = ctfield->vfield();
  ctmsh_ = ctfield->vmesh();
  dipfld_ = dipoles->vfield();
  detfld_ = detectors->vfield();
  emsh_ = efield->vmesh();
  dipmsh_ = dipoles->vmesh();
  detmsh_ = detectors->vmesh();

  if (!efld_ || !ctfld_ || !ctmsh_ || !dipfld_ || !detfld_ || !emsh_ || !dipmsh_ || !detmsh_)
  {
     algo_->error("At least one required input field/mesh has a NULL pointer.");
  }

  have_tensors_=false; /// we dont support a conductivity_table in a FieldHandle (could not be set in SCIRun4 as well)

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  // this code should be able to handle Field<Tensor> as well
  have_tensors_ = ctfld_->get_property("conductivity_table", tens_);
#endif
  LOG_DEBUG(" Note: The original SCIRun4 module looked for a field attribute ''conductivity_table'' of the second module input which could only be set outside of SCIRun4. This function is not available in SCIRun5. ");

  FieldInformation mfi(detectors);
  mfi.make_lineardata();

  mfi.make_double();
  FieldHandle magnetic_field_magnitudes = CreateField(mfi,detectors->mesh());
  if (!magnetic_field_magnitudes)
  {
    algo_->error("Could not allocate field for magnetic field magnitudes");
  }

  magmagfld_ = magnetic_field_magnitudes->vfield();
  magmagfld_->resize_values();
  mfi.make_vector();
  FieldHandle magnetic_field = CreateField(mfi,detectors->mesh());
  if (!magnetic_field)
  {
    algo_->error("Could not allocate field for magnetic field");
  }

  magfld_ = magnetic_field->vfield();
  magfld_->resize_values();

  // Make sure we have more than zero threads
  np_ = Parallel::NumCores();
  interp_value_.resize(np_,Vector(0.0,0.0,0.0));

  // cache per cell calculations that are used over and over again.
  set_up_cell_cache();

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  // do the parallel work.
  Thread::parallel(this, &CalcFMField::calc_parallel, np_, mod);
#endif

  Parallel::RunTasks([this](int i) { calc_parallel(i); }, np_);

  return boost::make_tuple(magnetic_field, magnetic_field_magnitudes);

}

boost::tuple<FieldHandle, FieldHandle> SimulateForwardMagneticFieldAlgo::run(FieldHandle ElectricField, FieldHandle ConductivityTensors, FieldHandle DipoleSources, FieldHandle DetectorLocations) const
{
  if (!ElectricField || !DipoleSources || !DetectorLocations || !ConductivityTensors)
  {
    THROW_ALGORITHM_INPUT_ERROR("At least one required input has a NULL pointer.");
  }

  if (!ElectricField->vfield()->is_vector())
  {
    THROW_ALGORITHM_INPUT_ERROR("Must have Vector field as Electric Field input");
  }

  if (!DipoleSources->vfield()->is_vector())
  {
    THROW_ALGORITHM_INPUT_ERROR("Must have Vector field as Dipole Sources input");
  }

  if (!DetectorLocations->vfield()->is_vector())
  {
    THROW_ALGORITHM_INPUT_ERROR("Must have Vector field as Detector Locations input");
  }

  CalcFMField algo(this);
  FieldHandle MField, MFieldMagnitudes;

  boost::tie(MField,MFieldMagnitudes) = algo.calc_forward_magnetic_field(ElectricField, ConductivityTensors, DipoleSources, DetectorLocations);

  return boost::make_tuple(MField, MFieldMagnitudes);
}

AlgorithmOutput SimulateForwardMagneticFieldAlgo::run(const AlgorithmInput& input) const
{
  AlgorithmOutput output;

  auto efield = input.get<Field>(ElectricField);
  auto condtensor = input.get<Field>(ConductivityTensor);
  auto dipoles = input.get<Field>(DipoleSources);
  auto detectors = input.get<Field>(DetectorLocations);
  FieldHandle MField, MFieldMagnitudes;

  boost::tie(MField,MFieldMagnitudes) = run(efield, condtensor, dipoles, detectors);

  output[MagneticField] = MField;
  output[MagneticFieldMagnitudes] = MFieldMagnitudes;

  return output;
}
