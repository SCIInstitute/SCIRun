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


#include <Core/Algorithms/Visualization/FieldTextLabels.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <sstream>
#include <type_traits>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

namespace
{
  template <class T>
  std::string format(const T& value, int precision)
  {
    std::ostringstream os;
    os.precision(precision);
    os << value;
    return os.str();
  }

  template <class Index>
  std::string dataText(VField* fld, Index idx, int precision)
  {
    if (fld->is_scalar())
    {
      double v;
      fld->get_value(v, idx);
      return format(v, precision);
    }
    if (fld->is_vector())
    {
      Vector v;
      fld->get_value(v, idx);
      return format(v, precision);
    }
    if (fld->is_tensor())
    {
      Tensor t;
      fld->get_value(t, idx);
      return format(t, precision);
    }
    return {};
  }

  template <class Index>
  std::optional<ColorRGB> dataColor(VField* fld, const ColorMap& colorMap, Index idx)
  {
    if (fld->is_scalar())
    {
      double v;
      fld->get_value(v, idx);
      return colorMap.valueToColor(v);
    }
    if (fld->is_vector())
    {
      Vector v;
      fld->get_value(v, idx);
      return colorMap.valueToColor(v);
    }
    if (fld->is_tensor())
    {
      Tensor t;
      fld->get_value(t, idx);
      return colorMap.valueToColor(t);
    }
    return {};
  }

  enum class Text { Data, Index, Location };

  struct Builder
  {
    VMesh* mesh;
    VField* fld;
    ColorMapHandle colorMap;
    const FieldTextLabelOptions& options;
    FieldTextLabels result;

    // Location is VMesh::Node, Edge, Face, Cell or Elem.
    template <class Location>
    void add(Text what, bool hasDataHere)
    {
      const bool colored = hasDataHere && options.useColorMap && colorMap;
      bool culled = false;
      if constexpr (std::is_same_v<Location, VMesh::Node>)
      {
        culled = options.cullBackfacing && mesh->has_normals();
        if (culled)
          mesh->synchronize(Mesh::NORMALS_E);
      }

      typename Location::iterator it, end;
      mesh->begin(it);
      mesh->end(end);
      for (; it != end; ++it)
      {
        if (options.maxLabels && result.labels.size() >= options.maxLabels)
        {
          ++result.omitted;
          continue;
        }

        FieldTextLabel label;
        mesh->get_center(label.anchor, *it);
        switch (what)
        {
        case Text::Data: label.text = dataText(fld, *it, options.precision); break;
        case Text::Index: label.text = std::to_string(static_cast<long long>(*it)); break;
        case Text::Location: label.text = format(label.anchor, options.precision); break;
        }
        if (colored)
          label.color = dataColor(fld, *colorMap, *it);
        if constexpr (std::is_same_v<Location, VMesh::Node>)
        {
          if (culled)
          {
            Vector n;
            mesh->get_normal(n, *it);
            label.normal = n;
          }
        }
        result.labels.push_back(std::move(label));
      }
    }
  };
}

FieldTextLabels SCIRun::Core::Algorithms::Visualization::buildFieldTextLabels(FieldHandle field,
  ColorMapHandle colorMap, const FieldTextLabelOptions& options)
{
  Builder b{ field->vmesh(), field->vfield(), colorMap, options, {} };
  const int basis = b.fld->basis_order();
  const int dim = b.mesh->dimensionality();
  const auto indexText = options.renderAsLocation ? Text::Location : Text::Index;

  if (options.showDataValues)
  {
    if (basis == 1)
      b.add<VMesh::Node>(Text::Data, true);
    else if (basis == 0)
      b.add<VMesh::Elem>(Text::Data, true);
  }
  // Index labels take a data color only when the data lives on that element type.
  if (options.showNodeIndices)
  {
    b.mesh->synchronize(Mesh::NODES_E);
    b.add<VMesh::Node>(indexText, basis == 1 || (basis == 0 && dim == 0));
  }
  if (options.showEdgeIndices && dim >= 1)
  {
    b.mesh->synchronize(Mesh::EDGES_E);
    b.add<VMesh::Edge>(indexText, basis == 0 && dim == 1);
  }
  if (options.showFaceIndices && dim >= 2)
  {
    b.mesh->synchronize(Mesh::FACES_E);
    b.add<VMesh::Face>(indexText, basis == 0 && dim == 2);
  }
  if (options.showCellIndices && dim == 3)
    b.add<VMesh::Cell>(indexText, basis == 0);

  return std::move(b.result);
}
