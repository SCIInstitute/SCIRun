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
#include <Core/Algorithms/Base/Variable.h>
#include <algorithm>

using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Dataflow::Networks;

MODULE_INFO_DEF(ConvertMatrixToColorMap, Converters, SCIRun)

ALGORITHM_PARAMETER_DEF(Visualization, AutoDetectColumns);
ALGORITHM_PARAMETER_DEF(Visualization, ColumnRoles);
ALGORITHM_PARAMETER_DEF(Visualization, MatrixColumnCount);
ALGORITHM_PARAMETER_DEF(Visualization, DetectedColumnRoles);

namespace SCIRun::Modules::Visualization
{
  const std::vector<std::string>& columnRoleNames()
  {
    static const std::vector<std::string> names =
      { "Ignore", "Position", "Red", "Green", "Blue", "Alpha" };
    return names;
  }
}

namespace
{
  std::string roleName(ColumnRole role)
  {
    return columnRoleNames()[static_cast<size_t>(role)];
  }

  ColumnRole roleFromName(const std::string& name)
  {
    const auto& names = columnRoleNames();
    const auto pos = std::find(names.begin(), names.end(), name);
    return pos == names.end() ? ColumnRole::Ignore
      : static_cast<ColumnRole>(std::distance(names.begin(), pos));
  }

  // The layouts a colormap matrix arrives in: bare RGB, RGB with alpha, and the
  // shape an XML colormap converts to, position first and alpha last (#2578).
  std::vector<ColumnRole> autoDetectRoles(size_t cols)
  {
    switch (cols)
    {
      case 3: return { ColumnRole::Red, ColumnRole::Green, ColumnRole::Blue };
      case 4: return { ColumnRole::Red, ColumnRole::Green, ColumnRole::Blue, ColumnRole::Alpha };
      case 5: return { ColumnRole::Position, ColumnRole::Red, ColumnRole::Green,
        ColumnRole::Blue, ColumnRole::Alpha };
      default: return std::vector<ColumnRole>(cols, ColumnRole::Ignore);
    }
  }

  int findColumn(const std::vector<ColumnRole>& roles, ColumnRole role)
  {
    const auto pos = std::find(roles.begin(), roles.end(), role);
    return pos == roles.end() ? -1 : static_cast<int>(std::distance(roles.begin(), pos));
  }

  size_t countColumns(const std::vector<ColumnRole>& roles, ColumnRole role)
  {
    return std::count(roles.begin(), roles.end(), role);
  }

  struct Stop
  {
    double x, r, g, b, a;
  };

  // ColorMap stores colors as an evenly spaced list, so an explicit position
  // column can only be honored by resampling the piecewise-linear ramp the
  // stops describe.
  std::vector<ColorRGB> resampleColors(const std::vector<Stop>& stops, size_t samples)
  {
    std::vector<ColorRGB> colors;
    colors.reserve(samples);
    for (size_t s = 0; s < samples; ++s)
    {
      const double t = (samples > 1) ? static_cast<double>(s) / (samples - 1) : 0.0;
      const auto upper = std::find_if(stops.begin(), stops.end(),
        [t](const Stop& stop) { return stop.x >= t; });

      if (upper == stops.begin())
        colors.emplace_back(stops.front().r, stops.front().g, stops.front().b);
      else if (upper == stops.end())
        colors.emplace_back(stops.back().r, stops.back().g, stops.back().b);
      else
      {
        const auto& lo = *(upper - 1);
        const auto& hi = *upper;
        const double span = hi.x - lo.x;
        const double f = (span > 0.0) ? (t - lo.x) / span : 0.0;
        colors.emplace_back(lo.r + f * (hi.r - lo.r), lo.g + f * (hi.g - lo.g),
          lo.b + f * (hi.b - lo.b));
      }
    }
    return colors;
  }
}

ConvertMatrixToColorMap::ConvertMatrixToColorMap() : Module(staticInfo_)
{
  INITIALIZE_PORT(InputMatrix);
  INITIALIZE_PORT(OutputColorMap);
}

void ConvertMatrixToColorMap::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::AutoDetectColumns, true);
  state->setValue(Parameters::ColumnRoles, VariableList());
}

void ConvertMatrixToColorMap::execute()
{
  auto input = getRequiredInput(InputMatrix);

  if (needToExecute())
  {
    auto dense = castMatrix::toDense(input);
    if (!dense)
    {
      error("Input must be a dense matrix of color values.");
      return;
    }

    const auto rows = dense->nrows();
    const auto cols = dense->ncols();

    if (rows < 1 || cols < 1)
    {
      error("Input matrix is empty; expected at least one color control point.");
      return;
    }

    auto state = get_state();
    const bool autoDetect = state->getValue(Parameters::AutoDetectColumns).toBool();

    std::vector<ColumnRole> roles;
    if (autoDetect)
      roles = autoDetectRoles(cols);
    else
    {
      const auto saved = state->getValue(Parameters::ColumnRoles).toVector();
      roles.assign(cols, ColumnRole::Ignore);
      for (size_t i = 0; i < std::min(saved.size(), cols); ++i)
        roles[i] = roleFromName(saved[i].toString());
    }

    // Publish before validating so the dialog can show the column list even
    // when the mapping is rejected below.
    state->setTransientValue(Parameters::MatrixColumnCount, static_cast<int>(cols));
    std::vector<std::string> roleNames;
    std::transform(roles.begin(), roles.end(), std::back_inserter(roleNames), roleName);
    state->setTransientValue(Parameters::DetectedColumnRoles, roleNames);

    const auto redCol = findColumn(roles, ColumnRole::Red);
    const auto greenCol = findColumn(roles, ColumnRole::Green);
    const auto blueCol = findColumn(roles, ColumnRole::Blue);
    if (redCol < 0 || greenCol < 0 || blueCol < 0)
    {
      if (autoDetect)
        error("Cannot infer the layout of a matrix with " + std::to_string(cols) +
          " columns; expected 3 (RGB), 4 (RGBA) or 5 (position + RGBA). "
          "Turn off auto-detection to assign the columns yourself.");
      else
        error("The red, green and blue columns must all be assigned.");
      return;
    }

    for (auto role : { ColumnRole::Position, ColumnRole::Red, ColumnRole::Green,
      ColumnRole::Blue, ColumnRole::Alpha })
    {
      if (countColumns(roles, role) > 1)
      {
        error("Column role '" + roleName(role) + "' is assigned to more than one column.");
        return;
      }
    }

    const auto alphaCol = findColumn(roles, ColumnRole::Alpha);
    const auto positionCol = findColumn(roles, ColumnRole::Position);

    // Scale the color channels alone: alpha is conventionally 0-1 even in a
    // 0-255 matrix, and a position column is not a color at all.
    const double colorMax = std::max({ dense->col(redCol).maxCoeff(),
      dense->col(greenCol).maxCoeff(), dense->col(blueCol).maxCoeff() });
    const double colorScale = (colorMax > 1.0) ? (1.0 / 255.0) : 1.0;
    if (colorScale != 1.0)
      remark("Color values exceed 1; interpreting them as 0-255 and scaling to 0-1.");

    double alphaScale = 1.0;
    if (alphaCol >= 0 && dense->col(alphaCol).maxCoeff() > 1.0)
      alphaScale = 1.0 / 255.0;

    // Positions are normalized to the [0, 1] domain ColorMap looks up in.
    double positionMin = 0.0, positionSpan = 1.0;
    if (positionCol >= 0)
    {
      positionMin = dense->col(positionCol).minCoeff();
      const double positionMax = dense->col(positionCol).maxCoeff();
      positionSpan = (positionMax > positionMin) ? (positionMax - positionMin) : 1.0;
    }

    std::vector<Stop> stops;
    stops.reserve(rows);
    for (size_t i = 0; i < rows; ++i)
    {
      const double x = (positionCol >= 0)
        ? ((*dense)(i, positionCol) - positionMin) / positionSpan
        : ((rows > 1) ? static_cast<double>(i) / (rows - 1) : 0.5);
      stops.push_back({ x,
        (*dense)(i, redCol) * colorScale,
        (*dense)(i, greenCol) * colorScale,
        (*dense)(i, blueCol) * colorScale,
        (alphaCol >= 0) ? (*dense)(i, alphaCol) * alphaScale : 1.0 });
    }

    std::vector<ColorRGB> colors;
    if (positionCol >= 0 && rows > 1)
    {
      std::stable_sort(stops.begin(), stops.end(),
        [](const Stop& a, const Stop& b) { return a.x < b.x; });
      colors = resampleColors(stops, 256);
    }
    else
    {
      std::transform(stops.begin(), stops.end(), std::back_inserter(colors),
        [](const Stop& s) { return ColorRGB(s.r, s.g, s.b); });
    }

    // ColorMap stores alpha separately, as (position, alpha) control points.
    std::vector<double> alphaPoints;
    if (alphaCol >= 0)
    {
      for (const auto& stop : stops)
      {
        alphaPoints.push_back(stop.x);
        alphaPoints.push_back(stop.a);
      }
    }

    auto colorMap = StandardColorMapFactory::create(colors, "Custom",
      256, 0.0, false, 0.5, 1.0, alphaPoints);

    sendOutput(OutputColorMap, colorMap);
  }
}
