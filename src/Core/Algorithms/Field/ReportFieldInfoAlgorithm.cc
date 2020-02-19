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


#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Field/ReportFieldInfoAlgorithm.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>

#include <iostream>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun;

ReportFieldInfoAlgorithm::Outputs ReportFieldInfoAlgorithm::update_input_attributes(FieldHandle f) const
{
  auto vfield = f->vfield();
  auto vmesh  = f->vmesh();

  Outputs output;

  if (vfield && vmesh)
  {
    auto td = f->get_type_description();
    if (td)
    {
      const auto& tname = td->get_name();
      output.type = tname;
    }
    else
      output.type = "Null";

    // Basis
    static const char *at_table[4] = { "Nodes", "Edges", "Faces", "Cells" };
    switch(f->basis_order())
    {
      case 3:
        output.dataLocation = "Nodes (cubic basis)";
        break;
      case 2:
        output.dataLocation = "Nodes (quadratic basis)";
        break;
      case 1:
        output.dataLocation = "Nodes (linear basis)";
        break;
      case 0:
        output.dataLocation = at_table[f->vmesh()->dimensionality()] +
                        std::string(" (constant basis)");
        break;
      case -1:
        output.dataLocation = "None (nodata basis)";
        break;
    }

    Point center;
    Vector size;

    const auto bbox = vmesh->get_bounding_box();
    if (bbox.valid())
    {
      size = bbox.diagonal();
      center = bbox.center();
      output.center = center;
      output.size = size;
    }
    else
    {
      warning("Input Field is empty.");

      const auto nan = std::numeric_limits<double>::quiet_NaN();
      output.center = Point(nan,nan,nan);
      output.size = Vector(nan, nan, nan);

    }

    double min, max;
    vfield->minmax(min,max);
    output.dataMin = min;
    output.dataMax = max;

    output.numdata_ = vfield->num_values();
    output.numnodes_ = vmesh->num_nodes();
    output.numelements_ = vmesh->num_elems();

    VMesh::dimension_type dim;
    vmesh->get_dimensions(dim);
    output.dims = Vector(1.0,1.0,1.0);
    for (size_t p=0; p < dim.size(); p++)
      output.dims[p] = static_cast<double>(dim[p]);

    output.geometricSize = 0.0;
    for (VMesh::Elem::index_type idx=0; idx< output.numelements_; ++idx)
    {
      output.geometricSize += vmesh->get_size(idx);
    }

  }
  return output;
}

ReportFieldInfoAlgorithm::Outputs::Outputs()
  : dataMin(0), dataMax(0), numdata_(0), numnodes_(0), numelements_(0), geometricSize(0)
{
}

ReportFieldInfoAlgorithm::Outputs ReportFieldInfoAlgorithm::run(const Inputs& input) const
{
  ENSURE_ALGORITHM_INPUT_NOT_NULL(input, "Null input field");

  return update_input_attributes(input);
}

AlgorithmOutput ReportFieldInfoAlgorithm::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);

  auto info = run(field);

  AlgorithmOutput output;
  output.setTransient(info);
  return output;
}

std::string ReportFieldInfoAlgorithm::summarize(const Outputs& info)
{
  std::ostringstream ostr;
  ostr << "Type: " << info.type << std::endl;
  ostr << "Center: " << info.center << std::endl;
  ostr << "Size: " << info.size << std::endl;
  ostr << "Data min,max: " << info.dataMin << " , " << info.dataMax << std::endl;
  ostr << "# nodes: " << info.numnodes_ << std::endl;
  ostr << "# elements: " << info.numelements_ << std::endl;
  ostr << "# data: " << info.numdata_ << std::endl;
  ostr << "Data location: " << info.dataLocation << std::endl;
  ostr << "Dims (x,y,z): " << info.dims << std::endl;
  ostr << "Geometric size: " << info.geometricSize << std::endl;
  return ostr.str();
}
