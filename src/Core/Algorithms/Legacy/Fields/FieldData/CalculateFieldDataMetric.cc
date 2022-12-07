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
#include <Core/Datatypes/MatrixAlgorithms.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Algorithms/Legacy/Fields/FieldData/CalculateFieldDataMetric.h>

#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace SCIRun::MatrixAlgorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

CalculateFieldDataMetricAlgo::CalculateFieldDataMetricAlgo()
{
  /// keep scalar type defines whether we convert to double or not
  addOption(Variables::Method, "value-mean", "min|max|median|value-mean|geom-mean|sum|integral|volthreshold");
  addParameter(Threshold, 0.5);
}

const AlgorithmParameterName CalculateFieldDataMetricAlgo::Threshold("Threshold");

bool CalculateFieldDataMetricAlgo::runImpl(const std::vector<FieldHandle>& input, MatrixHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "CalculateFieldDataMetric");

  if (!input.size())
  {
    error("No input fields were provided");
    return (false);
  }

  bool is_scalar = false;
  bool is_vector = false;
  bool is_tensor = false;

  for (size_t j=0;j<input.size();j++)
  {
    if (is_scalar)
    {
      if(!(input[j]->vfield()->is_scalar()))
      {
        error("Not all fields are of the same type");
        return (false);
      }
    }
    else if (is_vector)
    {
      if(!(input[j]->vfield()->is_vector()))
      {
        error("Not all fields are of the same type");
        return (false);
      }
    }
    else if (is_tensor)
    {
      if(!(input[j]->vfield()->is_tensor()))
      {
        error("Not all fields are of the same type");
        return (false);
      }
    }
    else
    {
      if (input[j]->vfield()->is_scalar()) is_scalar = true;
      if (input[j]->vfield()->is_vector()) is_vector = true;
      if (input[j]->vfield()->is_tensor()) is_tensor = true;
    }
  }

  if (checkOption(Variables::Method, "min"))
  {
    if (!is_scalar)
    {
      error("Minimum needs scalar data");
      return (false);
    }
    double min = std::numeric_limits<double>::max();
    for (size_t j=0;j<input.size();j++)
    {
      double fmin;
      if (input[j]->vfield()->min(fmin)) if (fmin < min) min = fmin;
    }
    output.reset(new DenseMatrix(1, 1, min));
    return (true);
  }
  else if (checkOption(Variables::Method,"max"))
  {
    if (!is_scalar)
    {
      error("Minimum needs scalar data");
      return (false);
    }
    double max = -(std::numeric_limits<double>::max());
    for (size_t j=0;j<input.size();j++)
    {
      double fmax;
      if (input[j]->vfield()->max(fmax))
        if (fmax > max) max = fmax;
    }
    output.reset(new DenseMatrix(1, 1, max));
    return (true);
  }
  else if (checkOption(Variables::Method,"median"))
  {
    if (!is_scalar)
    {
      error("Minimum needs scalar data");
      return (false);
    }

    std::vector<double> values;

    VField::size_type num_values = 0;
    for (size_t j=0;j<input.size();j++)
    {
      num_values += input[j]->vfield()->num_values();
    }
    values.resize(num_values);

    VField::size_type offset = 0;
    for (size_t j=0;j<input.size();j++)
    {
      num_values = input[j]->vfield()->num_values();
      if (num_values) input[j]->vfield()->get_values(&(values[offset]),num_values);
    }

    std::sort(values.begin(),values.end());
    output.reset(new DenseMatrix(1, 1, values[(values.size()/2)]));
    return (true);
  }
  else if (checkOption(Variables::Method,"sum"))
  {
    if (is_scalar)
    {
      double sum = 0.0;
      std::vector<double> values;
      for (size_t j=0;j<input.size();j++)
      {
        input[j]->vfield()->get_values(values);
        for (size_t i=0;i<values.size();i++) sum +=values[i];
      }
      output.reset(new DenseMatrix(1, 1, sum));
      return (true);
    }
    else if (is_vector)
    {
      Vector sum(0.0,0.0,0.0);
      std::vector<Vector> values;
      for (size_t j=0;j<input.size();j++)
      {
        input[j]->vfield()->get_values(values);
        for (size_t i=0;i<values.size();i++) sum +=values[i];
      }
      output = matrixFromVector(sum);
      return (true);
    }
    else if (is_tensor)
    {
      Tensor sum(0.0);
      std::vector<Tensor> values;
      for (size_t j=0;j<input.size();j++)
      {
        input[j]->vfield()->get_values(values);
        for (size_t i=0;i<values.size();i++) sum +=values[i];
      }
      output = matrixFromTensor(sum);
      return (true);
    }
  }
  else if (checkOption(Variables::Method,"value-mean"))
  {
    if (is_scalar)
    {
      double sum = 0.0;
      double vals = 0.0;
      std::vector<double> values;
      for (size_t j=0;j<input.size();j++)
      {
        input[j]->vfield()->get_values(values);
        for (size_t i=0;i<values.size();i++) sum +=values[i];
        vals += static_cast<double>(input[j]->vfield()->num_values());
      }
      output.reset(new DenseMatrix(1, 1, sum/vals));
      return (true);
    }
    else if (is_vector)
    {
      Vector sum(0.0,0.0,0.0);
      double vals = 0.0;
      std::vector<Vector> values;
      for (size_t j=0;j<input.size();j++)
      {
        input[j]->vfield()->get_values(values);
        for (size_t i=0;i<values.size();i++) sum +=values[i];
        vals += static_cast<double>(input[j]->vfield()->num_values());
      }
      output = matrixFromVector(sum*(1.0/vals));
      return (true);
    }
    else if (is_tensor)
    {
      Tensor sum(0.0);
      double vals = 0.0;
      std::vector<Tensor> values;
      for (size_t j=0;j<input.size();j++)
      {
        input[j]->vfield()->get_values(values);
        for (size_t i=0;i<values.size();i++) sum +=values[i];
        vals += static_cast<double>(input[j]->vfield()->num_values());
      }
      output = matrixFromTensor(sum*(1.0/vals));
      return (true);
    }
  }
  else if (checkOption(Variables::Method,"integral"))
  {
    if (is_scalar)
    {
      double integral = 0.0;
      std::vector<VMesh::coords_type> coords;
      std::vector<double> weights;
      std::vector<double> vals;
      double val;
      double size;
      double weight;

      for (size_t j=0;j<input.size();j++)
      {
        VMesh*  vmesh  = input[j]->vmesh();
        VField* vfield = input[j]->vfield();
        VMesh::size_type num_elems = vmesh->num_elems();

        vmesh->get_gaussian_scheme(coords,weights,vfield->basis_order());

        if (coords.size() > 1)
        {
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->minterpolate(vals,coords,idx);
            for (size_t i=0;i<vals.size();i++)
            {
              integral += weights[i]*vals[i]*size;
            }
          }
        }
        else
        {
          weight = weights[0];
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->interpolate(val,coords[0],idx);
            integral += weight*val*size;
          }
        }
      }
      output.reset(new DenseMatrix(1, 1, integral));
      return (true);
    }
    else if (is_vector)
    {
      Vector integral(0.0,0.0,0.0);
      std::vector<VMesh::coords_type> coords;
      std::vector<double> weights;
      std::vector<Vector> vals;
      Vector val;
      double size;
      double weight;

      for (size_t j=0;j<input.size();j++)
      {
        VMesh*  vmesh  = input[j]->vmesh();
        VField* vfield = input[j]->vfield();
        VMesh::size_type num_elems = vmesh->num_elems();

        vmesh->get_gaussian_scheme(coords,weights,vfield->basis_order());

        if (coords.size() > 1)
        {
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->minterpolate(vals,coords,idx);
            for (size_t i=0;i<vals.size();i++)
            {
              integral += weights[i]*vals[i]*size;
            }
          }
        }
        else
        {
          weight = weights[0];
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->interpolate(val,coords[0],idx);
            integral += weight*val*size;
          }
        }
      }
      output = matrixFromVector(integral);
      return (true);
    }
    else if (is_tensor)
    {
      Tensor integral(0.0);
      std::vector<VMesh::coords_type> coords;
      std::vector<double> weights;
      std::vector<Tensor> vals;
      Tensor val;
      double size;
      double weight;

      for (size_t j=0;j<input.size();j++)
      {
        VMesh*  vmesh  = input[j]->vmesh();
        VField* vfield = input[j]->vfield();
        VMesh::size_type num_elems = vmesh->num_elems();

        vmesh->get_gaussian_scheme(coords,weights,vfield->basis_order());

        if (coords.size() > 1)
        {
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->minterpolate(vals,coords,idx);
            for (size_t i=0;i<vals.size();i++)
            {
              integral += weights[i]*vals[i]*size;
            }
          }
        }
        else
        {
          weight = weights[0];
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->interpolate(val,coords[0],idx);
            integral += weight*val*size;
          }
        }
      }
      output = matrixFromTensor(integral);
      return (true);
    }
  }
  else if (checkOption(Variables::Method,"geom-mean"))
  {
    if (is_scalar)
    {
      double integral = 0.0;
      double volume = 0.0;
      std::vector<VMesh::coords_type> coords;
      std::vector<double> weights;
      std::vector<double> vals;
      double val;
      double size;
      double weight;

      for (size_t j=0;j<input.size();j++)
      {
        VMesh*  vmesh  = input[j]->vmesh();
        VField* vfield = input[j]->vfield();
        VMesh::size_type num_elems = vmesh->num_elems();

        vmesh->get_gaussian_scheme(coords,weights,vfield->basis_order());

        if (coords.size() > 1)
        {
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->minterpolate(vals,coords,idx);
            for (size_t i=0;i<vals.size();i++)
            {
              integral += weights[i]*vals[i]*size;
            }
            volume += size;
          }
        }
        else
        {
          weight = weights[0];
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->interpolate(val,coords[0],idx);
            integral += weight*val*size;
            volume += size;
          }
        }
      }
      output.reset(new DenseMatrix(1, 1, integral/volume));
      return (true);
    }
    else if (is_vector)
    {
      Vector integral(0.0,0.0,0.0);
      double volume = 0.0;
      std::vector<VMesh::coords_type> coords;
      std::vector<double> weights;
      std::vector<Vector> vals;
      Vector val;
      double size;
      double weight;

      for (size_t j=0;j<input.size();j++)
      {
        VMesh*  vmesh  = input[j]->vmesh();
        VField* vfield = input[j]->vfield();
        VMesh::size_type num_elems = vmesh->num_elems();

        vmesh->get_gaussian_scheme(coords,weights,vfield->basis_order());

        if (coords.size() > 1)
        {
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->minterpolate(vals,coords,idx);
            for (size_t i=0;i<vals.size();i++)
            {
              integral += weights[i]*vals[i]*size;
            }
            volume += size;
          }
        }
        else
        {
          weight = weights[0];
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->interpolate(val,coords[0],idx);
            integral += weight*val*size;
            volume += size;
          }
        }
      }
      output = matrixFromVector(integral*(1.0/volume));
      return (true);
    }
    else if (is_tensor)
    {
      Tensor integral(0.0);
      double volume = 0.0;
      std::vector<VMesh::coords_type> coords;
      std::vector<double> weights;
      std::vector<Tensor> vals;
      Tensor val;
      double size;
      double weight;

      for (size_t j=0;j<input.size();j++)
      {
        VMesh*  vmesh  = input[j]->vmesh();
        VField* vfield = input[j]->vfield();
        VMesh::size_type num_elems = vmesh->num_elems();

        vmesh->get_gaussian_scheme(coords,weights,vfield->basis_order());

        if (coords.size() > 1)
        {
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->minterpolate(vals,coords,idx);
            for (size_t i=0;i<vals.size();i++)
            {
              integral += weights[i]*vals[i]*size;
            }
            volume += size;
          }
        }
        else
        {
          weight = weights[0];
          for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
          {
            size = vmesh->get_size(idx);
            vfield->interpolate(val,coords[0],idx);
            integral += weight*val*size;
            volume += size;
          }
        }
      }
      output = matrixFromTensor(integral*(1.0/volume));
      return (true);
    }
  }

  return (false);
}


bool CalculateFieldDataMetricAlgo::runImpl(FieldHandle input, MatrixHandle& output) const
{
  std::vector<FieldHandle> inputs {input};
  return runImpl(inputs, output);
}

AlgorithmOutput CalculateFieldDataMetricAlgo::run(const AlgorithmInput& input) const
{
  auto inputFields = input.getList<Field>(Variables::InputFields);

  MatrixHandle outputMatrix;
  if (!runImpl(inputFields, outputMatrix))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Variables::OutputMatrix] = outputMatrix;
  return output;
}
