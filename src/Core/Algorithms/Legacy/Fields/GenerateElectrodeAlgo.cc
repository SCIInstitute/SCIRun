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


#include <Core/Algorithms/Legacy/Fields/GenerateElectrodeAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;

ALGORITHM_PARAMETER_DEF(Fields, ElectrodeLength);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeThickness);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeWidth);
ALGORITHM_PARAMETER_DEF(Fields, NumberOfControlPoints);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeType);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeResolution);
ALGORITHM_PARAMETER_DEF(Fields, ElectrodeProjection);
ALGORITHM_PARAMETER_DEF(Fields, MoveAll);
ALGORITHM_PARAMETER_DEF(Fields, UseFieldNodes);

GenerateElectrodeAlgo::GenerateElectrodeAlgo()
{
  addParameter(Parameters::ElectrodeLength, 0.1);
  addParameter(Parameters::ElectrodeThickness, 0.003);
  addParameter(Parameters::ElectrodeWidth, 0.02);
  addOption(Parameters::ElectrodeType,"wire","wire|planar");
  addOption(Parameters::ElectrodeProjection,"midway","positive|midway|negative");
  addParameter(Parameters::NumberOfControlPoints,5);
  addParameter(Parameters::ElectrodeResolution,10);
  addParameter(Parameters::UseFieldNodes,true);
  addParameter(Parameters::MoveAll,false);
  
}

namespace detail
{
class GenerateElectrodeAlgoF {
  public:
    typedef std::pair<double, VMesh::Elem::index_type> weight_type;
    typedef std::vector<weight_type> table_type;

    bool build_table(VMesh *mesh, VField* vfield,
                     std::vector<weight_type> &table,
                     std::string& method);

    static bool
    weight_less(const weight_type &a, const weight_type &b)
    {
      return (a.first < b.first);
    }
};

bool
GenerateElectrodeAlgoF::build_table(VMesh *vmesh,
                                                VField* vfield,
                                                std::vector<weight_type> &table,
                                                std::string& method)
{
  VMesh::size_type num_elems = vmesh->num_elems();

  long double sum = 0.0;
  for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
  {
    double elemsize = 0.0;
    if (method == "impuni")
    { // Size of element * data at element.
      Point p;
      vmesh->get_center(p, idx);
      if (vfield->is_vector())
      {
        Vector v;
        if (vfield->interpolate(v, p))
        {
          elemsize = v.length() * vmesh->get_size(idx);
        }
      }
      if (vfield->is_scalar())
      {
        double d;
        if (vfield->interpolate(d, p) && d > 0.0)
        {
          elemsize = d * vmesh->get_size(idx);
        }
      }
    }
    else if (method == "impscat")
    { // data at element
      Point p;
      vmesh->get_center(p, idx);
      if (vfield->is_vector())
      {
        Vector v;
        if (vfield->interpolate(v, p))
        {
          elemsize = v.length();
        }
      }
      if (vfield->is_scalar())
      {
        double d;
        if (vfield->interpolate(d, p) && d > 0.0)
        {
          elemsize = d;
        }
      }
    }
    else if (method == "uniuni")
    { // size of element only
      elemsize = vmesh->get_size(idx);
    }
    else if (method == "uniscat")
    {
      elemsize = 1.0;
    }

    if (elemsize > 0.0)
    {
      sum += elemsize;
      table.push_back(weight_type(sum, idx));
    }
  }
  if (table.size() > 0)
  {
    return (true);
  }

  return (false);
}
}


// equivalent to the interp1 command in matlab.  uses the parameters p and t to perform a cubic spline interpolation pp in one direction.

bool GenerateElectrodeAlgo::CalculateSpline(std::vector<double>& t, std::vector<double>& x, std::vector<double>& tt, std::vector<double>& xx)
{
  // need to have at least 3 nodes
  if (t.size() < 3) return (false);
  if (x.size() != t.size()) return (false);

  size_t size = x.size();
  std::vector<double> z(size), h(size - 1), b(size - 1), v(size - 1), u(size - 1);

  for (size_t k = 0; k < size - 1; k++)
  {
    h[k] = (t[k + 1] - t[k]);
    b[k] = (6 * (x[k + 1] - x[k]) / h[k]);
  }

  u[1] = 2 * (h[0] + h[1]);
  v[1] = b[1] - b[0];

  for (size_t k = 2; k < size - 1; k++)
  {
    u[k] = 2 * (h[k] + h[k - 1]) - (h[k - 1] * h[k - 1]) / u[k - 1];
    v[k] = b[k] - b[k - 1] - h[k - 1] * v[k - 1] / u[k - 1];
  }

  z[size - 1] = 0;

  for (size_t k = size - 2; k > 0; k--)
  {
    z[k] = (v[k] - h[k] * z[k + 1]) / u[k];
  }

  z[0] = 0;

  size_t segment = 0;

  xx.resize(tt.size());
  for (size_t k = 0; k < tt.size(); k++)
  {
    while (segment < (size - 2) && t[segment + 1] < tt[k])
    {
      segment++;
    }

    double w0, w1, w2, w3, a, b, c, d;

    w3 = (t[segment + 1] - tt[k]);
    w0 = w3 * w3*w3;
    w2 = (tt[k] - t[segment]);
    w1 = w2 * w2*w2;

    a = z[segment] / (6 * h[segment]);
    b = z[segment + 1] / (6 * h[segment]);
    c = (x[segment + 1] / h[segment] - (z[segment + 1] * h[segment]) / 6);
    d = (x[segment] / h[segment] - z[segment] * h[segment] / 6);

    xx[k] = a * w0 + b * w1 + c * w2 + d * w3;
  }

  return (true);
}

// this is a spline function.  pp is the final points that are in between the original points p.
// t and tt are the original and final desired spacing, respectively.

bool GenerateElectrodeAlgo::CalculateSpline(std::vector<double>& t, std::vector<Point>& p, std::vector<double>& tt, std::vector<Point>& pp)
{
  // need to have at least 3 nodes
  if (t.size() < 3) return (false);
  if (p.size() != t.size()) return (false);

  size_t size = p.size();

  std::vector<double> x(size), y(size), z(size);
  std::vector<double> xx, yy, zz;

  for (size_t k = 0; k < p.size(); k++)
  {
    x[k] = p[k].x();
    y[k] = p[k].y();
    z[k] = p[k].z();
  }

  CalculateSpline(t, x, tt, xx);
  CalculateSpline(t, y, tt, yy);
  CalculateSpline(t, z, tt, zz);

  for (size_t k = 0; k < tt.size(); k++)
    pp.emplace_back(xx[k], yy[k], zz[k]);

  return (true);
}

bool GenerateElectrodeAlgo::runImpl(FieldHandle input, FieldHandle& output) const
{
  
}

AlgorithmOutput GenerateElectrodeAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);

  FieldHandle outputField;
  if (!runImpl(inputField, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[Samples] = outputField;
  return output;
}

