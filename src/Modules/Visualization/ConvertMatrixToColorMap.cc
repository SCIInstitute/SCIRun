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

  std::vector<ColumnRole> resolveRoles(bool autoDetect, size_t cols, const VariableList& saved)
  {
    if (autoDetect)
      return autoDetectRoles(cols);

    std::vector<ColumnRole> roles(cols, ColumnRole::Ignore);
    for (size_t i = 0; i < std::min(saved.size(), cols); ++i)
      roles[i] = roleFromName(saved[i].toString());
    return roles;
  }

  struct ColumnMapping
  {
    int red = -1, green = -1, blue = -1, alpha = -1, position = -1;
  };

  // Returns the reason the roles are unusable, or an empty string if they are.
  std::string mapColumns(const std::vector<ColumnRole>& roles, bool autoDetect,
    ColumnMapping& mapping)
  {
    mapping.red = findColumn(roles, ColumnRole::Red);
    mapping.green = findColumn(roles, ColumnRole::Green);
    mapping.blue = findColumn(roles, ColumnRole::Blue);
    mapping.alpha = findColumn(roles, ColumnRole::Alpha);
    mapping.position = findColumn(roles, ColumnRole::Position);

    if (mapping.red < 0 || mapping.green < 0 || mapping.blue < 0)
    {
      if (autoDetect)
        return "Cannot infer the layout of a matrix with " + std::to_string(roles.size()) +
          " columns; expected 3 (RGB), 4 (RGBA) or 5 (position + RGBA). "
          "Turn off auto-detection to assign the columns yourself.";
      return "The red, green and blue columns must all be assigned.";
    }

    for (auto role : { ColumnRole::Position, ColumnRole::Red, ColumnRole::Green,
      ColumnRole::Blue, ColumnRole::Alpha })
    {
      if (countColumns(roles, role) > 1)
        return "Column role '" + roleName(role) + "' is assigned to more than one column.";
    }
    return {};
  }

  // If any color channel exceeds 1 the matrix is in [0, 255]. Alpha and position
  // are excluded: alpha is conventionally 0-1 either way, and a position column
  // is not a color at all.
  double colorScaleFor(const DenseMatrix& m, const ColumnMapping& mapping)
  {
    const double colorMax = std::max({ m.col(mapping.red).maxCoeff(),
      m.col(mapping.green).maxCoeff(), m.col(mapping.blue).maxCoeff() });
    return (colorMax > 1.0) ? (1.0 / 255.0) : 1.0;
  }

  struct Stop
  {
    double x, r, g, b, a;
  };

  std::vector<Stop> buildStops(const DenseMatrix& m, const ColumnMapping& mapping,
    double colorScale)
  {
    const auto rows = m.nrows();
    const double alphaScale =
      (mapping.alpha >= 0 && m.col(mapping.alpha).maxCoeff() > 1.0) ? (1.0 / 255.0) : 1.0;

    // Positions are normalized to the [0, 1] domain ColorMap looks up in.
    double positionMin = 0.0, positionSpan = 1.0;
    if (mapping.position >= 0)
    {
      positionMin = m.col(mapping.position).minCoeff();
      const double positionMax = m.col(mapping.position).maxCoeff();
      positionSpan = (positionMax > positionMin) ? (positionMax - positionMin) : 1.0;
    }

    std::vector<Stop> stops;
    stops.reserve(rows);
    for (size_t i = 0; i < rows; ++i)
    {
      const double x = (mapping.position >= 0)
        ? (m(i, mapping.position) - positionMin) / positionSpan
        : ((rows > 1) ? static_cast<double>(i) / (rows - 1) : 0.5);
      stops.push_back({ x,
        m(i, mapping.red) * colorScale,
        m(i, mapping.green) * colorScale,
        m(i, mapping.blue) * colorScale,
        (mapping.alpha >= 0) ? m(i, mapping.alpha) * alphaScale : 1.0 });
    }

    if (mapping.position >= 0)
    {
      std::stable_sort(stops.begin(), stops.end(),
        [](const Stop& a, const Stop& b) { return a.x < b.x; });
    }
    return stops;
  }

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

  ColorMapHandle makeColorMap(const std::vector<Stop>& stops, const ColumnMapping& mapping)
  {
    std::vector<ColorRGB> colors;
    if (mapping.position >= 0 && stops.size() > 1)
      colors = resampleColors(stops, 256);
    else
    {
      std::transform(stops.begin(), stops.end(), std::back_inserter(colors),
        [](const Stop& s) { return ColorRGB(s.r, s.g, s.b); });
    }

    // ColorMap stores alpha separately, as (position, alpha) control points.
    std::vector<double> alphaPoints;
    if (mapping.alpha >= 0)
    {
      for (const auto& stop : stops)
      {
        alphaPoints.push_back(stop.x);
        alphaPoints.push_back(stop.a);
      }
    }

    return StandardColorMapFactory::create(colors, "Custom",
      256, 0.0, false, 0.5, 1.0, alphaPoints);
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

  if (!needToExecute())
    return;

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
  const auto roles = resolveRoles(autoDetect, cols,
    state->getValue(Parameters::ColumnRoles).toVector());
  publishColumns(roles);

  ColumnMapping mapping;
  const auto problem = mapColumns(roles, autoDetect, mapping);
  if (!problem.empty())
  {
    error(problem);
    return;
  }

  const auto colorScale = colorScaleFor(*dense, mapping);
  if (colorScale != 1.0)
    remark("Color values exceed 1; interpreting them as 0-255 and scaling to 0-1.");

  sendOutput(OutputColorMap,
    makeColorMap(buildStops(*dense, mapping, colorScale), mapping));
}

// Reported to the dialog before the mapping is validated, so the column list
// still appears when the roles are rejected.
void ConvertMatrixToColorMap::publishColumns(const std::vector<ColumnRole>& roles)
{
  auto state = get_state();
  state->setTransientValue(Parameters::MatrixColumnCount, static_cast<int>(roles.size()));

  std::vector<std::string> roleNames;
  std::transform(roles.begin(), roles.end(), std::back_inserter(roleNames), roleName);
  state->setTransientValue(Parameters::DetectedColumnRoles, roleNames);
}
