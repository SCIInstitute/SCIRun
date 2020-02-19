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


#include <Core/Algorithms/Legacy/Fields/DistanceField/CalculateDistanceField.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Logging/Log.h>
#include <Core/Thread/Parallel.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;

ALGORITHM_PARAMETER_DEF(Fields, BasisType);
ALGORITHM_PARAMETER_DEF(Fields, Truncate);
ALGORITHM_PARAMETER_DEF(Fields, TruncateDistance);
ALGORITHM_PARAMETER_DEF(Fields, OutputFieldDatatype);
ALGORITHM_PARAMETER_DEF(Fields, OutputValueField);

CalculateDistanceFieldAlgo::CalculateDistanceFieldAlgo()
{
  using namespace Parameters;
  addParameter(Truncate, false);
  addParameter(TruncateDistance, 1.0);
  addParameter(OutputValueField, false);
  addOption(BasisType, "same as input","same as input|constant|linear");
  addOption(OutputFieldDatatype, "double","char|unsigned char|short|unsigned short|int|unsigned int|float|double");
}

namespace detail
{

class CalculateDistanceFieldP : public Interruptible
{
  public:
    CalculateDistanceFieldP(VMesh* imesh, VMesh* objmesh, VField*  ofield, const AlgorithmBase* algo) :
      imesh(imesh), objmesh(objmesh), objfield(0), ofield(ofield), vfield(0), algo_(algo) {}

    CalculateDistanceFieldP(VMesh* imesh, VMesh* objmesh, VField* objfield, VField*  ofield, VField* vfield, const AlgorithmBase* algo) :
      imesh(imesh), objmesh(objmesh), objfield(objfield), ofield(ofield), vfield(vfield), algo_(algo)  {}

    void parallel(int proc, int nproc)
    {
      VMesh::size_type num_values = ofield->num_values();
      VMesh::size_type num_evalues = ofield->num_evalues();

      double max = DBL_MAX;
      if (algo_->get(Parameters::Truncate).toBool())
      {
        max = algo_->get(Parameters::TruncateDistance).toDouble();
      }

      double val = 0.0;
      int cnt = 0;

      if (ofield->basis_order() == 0)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);

        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          checkForInterruption();
          Point p, p2;
          imesh->get_center(p,idx);
          if(!(objmesh->find_closest_elem(val,p2,fidx,p,max))) val = max;
          ofield->set_value(val,idx);

          if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
        }
      }
      else if (ofield->basis_order() == 1)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);

        for (VMesh::Node::index_type idx=start; idx<end; idx++)
        {
          checkForInterruption();
          Point p, p2;
          imesh->get_center(p,idx);
          if(!(objmesh->find_closest_elem(val,p2,fidx,p,max))) val = max;
          ofield->set_value(val,idx);

          if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
        }
      }
      else if (ofield->basis_order() > 1)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_evalues);

        for (VMesh::ENode::index_type idx=start; idx<end; idx++)
        {
          checkForInterruption();
          Point p, p2;
          imesh->get_center(p,idx);
          if(!(objmesh->find_closest_elem(val,p2,fidx,p,max))) val = max;
          ofield->set_value(val,idx);

          if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
        }
      }
    }

    void parallel2(int proc, int nproc)
    {
      VMesh::size_type num_values = ofield->num_values();
      VMesh::size_type num_evalues = ofield->num_evalues();

      if (algo_->get(Parameters::Truncate).toBool())
      {
        // Cannot do both at the same time
        if (proc == 0) algo_->warning("Closest value has been requested, disabling truncated distance map.");
      }

      double val = 0.0;
      int cnt = 0;

      if (ofield->basis_order() == 0)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        VMesh::coords_type coords;
        Point p, p2;
        range(proc,nproc,start,end,num_values);

        if (objfield->is_scalar())
        {
          double scalar;

          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            checkForInterruption();
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(scalar,coords,fidx);
            vfield->set_value(scalar,idx);
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
          }
        }
        else if (objfield->is_vector())
        {
          Vector vec;

          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            checkForInterruption();
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(vec,coords,fidx);
            vfield->set_value(vec,idx);
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
          }
        }
        else if (objfield->is_tensor())
        {
          Tensor tensor;

          for (VMesh::Elem::index_type idx=start; idx<end; idx++)
          {
            checkForInterruption();
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(tensor,coords,fidx);
            vfield->set_value(tensor,idx);
            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
          }
        }
      }
      else if (ofield->basis_order() == 1)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        VMesh::coords_type coords;
        range(proc,nproc,start,end,num_values);
        Point p, p2;

        if (objfield->is_scalar())
        {
          double scalar;

          for (VMesh::Node::index_type idx=start; idx<end; idx++)
          {
            checkForInterruption();
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(scalar,coords,fidx);
            vfield->set_value(scalar,idx);

            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
          }
        }
        else if (objfield->is_vector())
        {
          Vector vec;

          for (VMesh::Node::index_type idx=start; idx<end; idx++)
          {
            checkForInterruption();
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(vec,coords,fidx);
            vfield->set_value(vec,idx);

            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
          }
        }
        else if (objfield->is_tensor())
        {
          Tensor tensor;

          for (VMesh::Node::index_type idx=start; idx<end; idx++)
          {
            checkForInterruption();
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(tensor,coords,fidx);
            vfield->set_value(tensor,idx);

            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
          }
        }
      }
      else if (ofield->basis_order() > 1)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        VMesh::coords_type coords;
        range(proc,nproc,start,end,num_evalues);
        Point p, p2;

        if (objfield->is_scalar())
        {
          double scalar;
          for (VMesh::ENode::index_type idx=start; idx<end; idx++)
          {
            checkForInterruption();
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(scalar,coords,fidx);
            vfield->set_value(scalar,idx);

            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
          }
        }
        else if (objfield->is_vector())
        {
          Vector vec;
          for (VMesh::ENode::index_type idx=start; idx<end; idx++)
          {
            checkForInterruption();
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(vec,coords,fidx);
            vfield->set_value(vec,idx);

            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
          }
        }
        else if (objfield->is_tensor())
        {
          Tensor tensor;
          for (VMesh::ENode::index_type idx=start; idx<end; idx++)
          {
            checkForInterruption();
            imesh->get_center(p,idx);
            objmesh->find_closest_elem(val,p2,coords,fidx,p);
            ofield->set_value(val,idx);
            objfield->interpolate(tensor,coords,fidx);
            vfield->set_value(tensor,idx);

            if (proc == 0) { cnt++; if (cnt == 100) { algo_->update_progress_max(idx,end); cnt = 0; } }
          }
        }
      }
    }


    void range(int proc, int nproc,
               VMesh::index_type& start, VMesh::index_type& end,
               VMesh::size_type size)
    {
      VMesh::size_type m = size/nproc;
      start = proc*m;
      end = (proc+1)*m;
      if (proc == nproc-1) end = size;
    }

  private:
    VMesh*   imesh;
    VMesh*   objmesh;
    VField*  objfield;
    VField*  ofield;
    VField*  vfield;
    const AlgorithmBase* algo_;
};
}

//TODO refactor duplication

bool
CalculateDistanceFieldAlgo::runImpl(FieldHandle input, FieldHandle object, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "CalculateDistanceField");

  if (!input)
  {
    error("No input field");
    return (false);
  }

  if (!object)
  {
    error("No object field");
    return (false);
  }

  // Determine output type
  FieldInformation fo(input);
  if (fo.is_nodata()) fo.make_lineardata();

  if (checkOption(Parameters::BasisType,"linear")) fo.make_lineardata();
  if (checkOption(Parameters::BasisType,"constant")) fo.make_constantdata();

  fo.set_data_type(getOption(Parameters::OutputFieldDatatype));

  fo.make_double();
  output = CreateField(fo,input->mesh());

  if (!output)
  {
    error("Could not create output field");
    return (false);
  }

  VMesh* imesh = input->vmesh();
  VMesh* objmesh = object->vmesh();
  VField* ofield = output->vfield();
  ofield->resize_values();

  if (imesh->num_nodes() == 0)
  {
    warning("Input Field does not contain any nodes, setting distance to maximum.");
    return (true);
  }

  if (objmesh->num_nodes() == 0)
  {
    warning("Object Field does not contain any nodes, setting distance to maximum.");
    ofield->set_all_values(DBL_MAX);

    return (true);
  }

  objmesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E);

  if (ofield->basis_order() > 2)
  {
    error("Cannot add distance data to field");
    return (false);
  }

  detail::CalculateDistanceFieldP palgo(imesh,objmesh,ofield,this);
  auto task_i = [&palgo](int i) { palgo.parallel(i, Parallel::NumCores()); };
  Parallel::RunTasks(task_i, Parallel::NumCores());

  return (true);
}

bool
CalculateDistanceFieldAlgo::runImpl(FieldHandle input, FieldHandle object, FieldHandle& distance, FieldHandle& value) const
{
  ScopedAlgorithmStatusReporter asr(this, "CalculateDistanceField");

  if (!input)
  {
    error("No input field");
    return (false);
  }

  if (!object)
  {
    error("No object field");
    return (false);
  }

  // Determine output type
  FieldInformation fo(input);
  if (fo.is_nodata()) fo.make_lineardata();

  FieldInformation fb(object);

  if (fb.is_nodata())
  {
    error("Object field does not contain any values");
    return (false);
  }
  // Create Value mesh with same type as object type

  fo.set_data_type(fb.get_data_type());
  value = CreateField(fo,input->mesh());

  if (checkOption(Parameters::BasisType,"linear")) fo.make_lineardata();
  if (checkOption(Parameters::BasisType,"constant")) fo.make_constantdata();

  fo.set_data_type(getOption(Parameters::OutputFieldDatatype));
  distance = CreateField(fo,input->mesh());

  if (!distance)
  {
    error("Could not create output field");
    return (false);
  }

  if (!value)
  {
    error("Could not create output field");
    return (false);
  }

  VMesh* imesh = input->vmesh();
  VMesh* objmesh = object->vmesh();
  VField* objfield = object->vfield();

  VField* dfield = distance->vfield();
  dfield->resize_values();

  VField* vfield = value->vfield();
  vfield->resize_values();

  if (objmesh->num_nodes() == 0)
  {
    warning("Object Field does not contain any nodes, setting distance to maximum.");
    dfield->set_all_values(DBL_MAX);
    vfield->clear_all_values();

    return (true);
  }

  objmesh->synchronize(Mesh::FIND_CLOSEST_ELEM_E);

  if (distance->basis_order() > 2)
  {
    error("Cannot add distance data to field");
    return (false);
  }

  detail::CalculateDistanceFieldP palgo(imesh,objmesh,objfield,dfield,vfield,this);
  auto task_i = [&palgo](int i) { palgo.parallel2(i, Parallel::NumCores()); };
  Parallel::RunTasks(task_i, Parallel::NumCores());

  return (true);
}

const AlgorithmOutputName CalculateDistanceFieldAlgo::DistanceField("DistanceField");
const AlgorithmOutputName CalculateDistanceFieldAlgo::ValueField("ValueField");

AlgorithmOutput CalculateDistanceFieldAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  auto objectField = input.get<Field>(Variables::ObjectField);

  FieldHandle distance, value;
  if (get(Parameters::OutputValueField).toBool())
  {
    if (!runImpl(inputField, objectField, distance, value))
      THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");
  }
  else
  {
    if (!runImpl(inputField, objectField, distance))
      THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");
  }

  AlgorithmOutput output;
  output[DistanceField] = distance;
  output[ValueField] = value;
  return output;
}
