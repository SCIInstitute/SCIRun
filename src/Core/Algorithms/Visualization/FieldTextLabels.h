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


#ifndef CORE_ALGORITHMS_VISUALIZATION_FIELDTEXTLABELS_H
#define CORE_ALGORITHMS_VISUALIZATION_FIELDTEXTLABELS_H

#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/Legacy/Field/FieldFwd.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <optional>
#include <string>
#include <vector>
#include <Core/Algorithms/Visualization/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Visualization
      {
        struct SCISHARE FieldTextLabel
        {
          Geometry::Point anchor;
          std::string text;
          // Set only when backface culling applies to this label.
          std::optional<Geometry::Vector> normal;
          // Empty means "use the default text color".
          std::optional<Datatypes::ColorRGB> color;
        };

        struct SCISHARE FieldTextLabelOptions
        {
          bool showDataValues {true};
          bool showNodeIndices {false};
          bool showEdgeIndices {false};
          bool showFaceIndices {false};
          bool showCellIndices {false};
          // Print each element's position instead of its index.
          bool renderAsLocation {false};
          bool cullBackfacing {false};
          bool useColorMap {false};
          int precision {3};
          // 0 means no limit.
          size_t maxLabels {0};
        };

        struct SCISHARE FieldTextLabels
        {
          std::vector<FieldTextLabel> labels;
          // Labels dropped because of maxLabels.
          size_t omitted {0};
        };

        // Renderer-independent half of ShowField's Text tab (#2148): which
        // labels exist, where they sit, and what they say. Follows v4's
        // RenderFieldV::render_text.
        SCISHARE FieldTextLabels buildFieldTextLabels(FieldHandle field,
          Datatypes::ColorMapHandle colorMap, const FieldTextLabelOptions& options);
      }
    }
  }
}

#endif
