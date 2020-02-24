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


#include <vector>
#include <algorithm>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Algorithms/Fields/FieldData/CalculateFieldDataMetric.h>

#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
CalculateFieldDataMetricAlgo::
run(std::vector<FieldHandle>& input, MatrixHandle& output)
{
  algo_start("CalculateFieldDataMetric");

  if (!(input.size()))
  {
    error("No input fields were provided");
    algo_end(); return (false);
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
        algo_end(); return (false);
      }
    }
    else if (is_vector)
    {
      if(!(input[j]->vfield()->is_vector()))
      {
        error("Not all fields are of the same type");
        algo_end(); return (false);
      }
    }
    else if (is_tensor)
    {
      if(!(input[j]->vfield()->is_tensor()))
      {
        error("Not all fields are of the same type");
        algo_end(); return (false);
      }
    }
    else
    {
      if (input[j]->vfield()->is_scalar()) is_scalar = true;
      if (input[j]->vfield()->is_vector()) is_vector = true;
      if (input[j]->vfield()->is_tensor()) is_tensor = true;
    }
  }

  if (check_option("method","min"))
  {
    if (!is_scalar)
    {
      error("Minimum needs scalar data");
      algo_end(); return (true);
    }
    double min = DBL_MAX;
    for (size_t j=0;j<input.size();j++)
    {
      double fmin;
      if (input[j]->vfield()->min(fmin)) if (fmin < min) min = fmin;
    }
    output = new DenseMatrix(min);
    algo_end(); return (true);
  }
  else if (check_option("method","max"))
  {
    if (!is_scalar)
    {
      error("Minimum needs scalar data");
      algo_end(); return (true);
    }
    double max = -(DBL_MAX);
    for (size_t j=0;j<input.size();j++)
    {
      double fmax;
      if(input[j]->vfield()->max(fmax)) if (fmax > max) max = fmax;
    }
    output = new DenseMatrix(max);
    algo_end(); return (true);
  }
  else if (check_option("method","median"))
  {
    if (!is_scalar)
    {
      error("Minimum needs scalar data");
      algo_end(); return (true);
    }

    double sum = 0.0;
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
    output = new DenseMatrix(values[(values.size()/2)]);
    algo_end(); return (true);
  }
  else if (check_option("method","sum"))
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
      output = new DenseMatrix(sum);
      algo_end(); return (true);
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
      output = new DenseMatrix(sum);
      algo_end(); return (true);
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
      output = new DenseMatrix(sum);
      algo_end(); return (true);
    }
  }
  else if (check_option("method","value-mean"))
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
      output = new DenseMatrix(sum/vals);
      algo_end(); return (true);
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
      output = new DenseMatrix(sum*(1.0/vals));
      algo_end(); return (true);
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
      output = new DenseMatrix(sum*(1.0/vals));
      algo_end(); return (true);
    }
  }
  else if (check_option("method","integral"))
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
      output = new DenseMatrix(integral);
      algo_end(); return (true);
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
      output = new DenseMatrix(integral);
      algo_end(); return (true);
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
      output = new DenseMatrix(integral);
      algo_end(); return (true);
    }
  }
  else if (check_option("method","geom-mean"))
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
      output = new DenseMatrix(integral/volume);
      algo_end(); return (true);
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
      output = new DenseMatrix(integral*(1.0/volume));
      algo_end(); return (true);
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
      output = new DenseMatrix(integral*(1.0/volume));
      algo_end(); return (true);
    }
  }

  algo_end(); return (false);
}


bool
CalculateFieldDataMetricAlgo::
run(FieldHandle input, MatrixHandle& output)
{
  std::vector<FieldHandle> inputs(1);
  inputs[0] = input;
  return(run(inputs,output));
}


} // namespace SCIRunAlgo
