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


#include <Modules/Visualization/ConvertMatrixToColorMap.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Dataflow::Networks;

MODULE_INFO_DEF(ConvertMatrixToColorMap, Converters, SCIRun)

ConvertMatrixToColorMap::ConvertMatrixToColorMap() : Module(staticInfo_, false)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputColorMap);
}

void ConvertMatrixToColorMap::execute()
{
  auto input = getRequiredInput(InputMatrix);

  if (needToExecute())
  {
    auto dense = castMatrix::toDense(input);
    if (!dense)
    {
      error("Input must be a dense matrix of color values (Nx3 for RGB or Nx4 for RGBA).");
      return;
    }

    const auto rows = dense->nrows();
    const auto cols = dense->ncols();

    if (rows < 1)
    {
      error("Input matrix has no rows; expected at least one color control point.");
      return;
    }
    if (cols != 3 && cols != 4)
    {
      error("Input matrix must have 3 columns (RGB) or 4 columns (RGBA); got " +
        std::to_string(cols) + ".");
      return;
    }

    // Decide the value range: if any color channel exceeds 1, assume [0, 255].
    const double maxValue = dense->maxCoeff();
    const double scale = (maxValue > 1.0) ? (1.0 / 255.0) : 1.0;
    if (scale != 1.0)
      remark("Values exceed 1; interpreting the matrix as 0-255 and scaling to 0-1.");

    std::vector<ColorRGB> colors;
    colors.reserve(rows);
    std::vector<double> alphaPoints;
    const bool hasAlpha = (cols == 4);

    for (size_t i = 0; i < rows; ++i)
    {
      const double r = (*dense)(i, 0) * scale;
      const double g = (*dense)(i, 1) * scale;
      const double b = (*dense)(i, 2) * scale;
      colors.emplace_back(r, g, b);

      if (hasAlpha)
      {
        // ColorMap stores alpha separately as (position, alpha) control points
        // spanning the map domain [0, 1].
        const double position = (rows > 1) ? (static_cast<double>(i) / (rows - 1)) : 0.5;
        alphaPoints.push_back(position);
        alphaPoints.push_back((*dense)(i, 3) * scale);
      }
    }

    auto colorMap = StandardColorMapFactory::create(colors, "Custom",
      256, 0.0, false, 0.5, 1.0, alphaPoints);

    sendOutput(OutputColorMap, colorMap);
  }
}
