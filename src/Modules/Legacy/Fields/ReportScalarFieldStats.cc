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


///@author
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  February 2001

#include <Modules/Legacy/Fields/ReportScalarFieldStats.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Algorithms/Legacy/Fields/DomainFields/GetDomainBoundaryAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/GeometryPrimitives/Vector.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Fields;

ALGORITHM_PARAMETER_DEF(Fields, AutoRangeEnabled);
ALGORITHM_PARAMETER_DEF(Fields, Mean);
ALGORITHM_PARAMETER_DEF(Fields, Median);
ALGORITHM_PARAMETER_DEF(Fields, StandardDeviation);
ALGORITHM_PARAMETER_DEF(Fields, HistogramBinCount);

/// @class ReportScalarFieldStats
/// @brief Analyze data from a scalarfield.

MODULE_INFO_DEF(ReportScalarFieldStats, MiscField, SCIRun)

ReportScalarFieldStats::ReportScalarFieldStats()
  : Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(HistogramData);
}

void ReportScalarFieldStats::setStateDefaults()
{
  auto state = get_state();
  state->setValue(Parameters::Mean, std::string(""));
  state->setValue(Parameters::Median, std::string(""));
  state->setValue(Parameters::StandardDeviation, std::string(""));
  state->setValue(Parameters::AutoRangeEnabled, 0);
  state->setValue(Parameters::MinRange, 0.0);
  state->setValue(Parameters::MaxRange, 0.0);
  state->setValue(Parameters::HistogramBinCount, 256);
}

void ReportScalarFieldStats::execute()
{
  auto inputField = getRequiredInput(InputField);

  if (!(inputField->vfield()->is_scalar()))
  {
    error("This module only works on scalar fields.");
    return;
  }

  VField* ifield = inputField->vfield();
  auto state = get_state();

  bool init = false;
  double value = 0;
  double min = 0;
  double max = 0;
  int counter = 0;
  std::vector<double> values;
  double mean = 0;
  double median = 0;
  double sigma = 0;   //standard deviation

  if (state->getValue(Parameters::AutoRangeEnabled).toInt() == 1)
  {
    const double mmin = state->getValue(Parameters::MinRange).toDouble();
    const double mmax = state->getValue(Parameters::MaxRange).toDouble();
    VField::size_type num_values = ifield->num_values();
    for (VField::index_type idx=0; idx < num_values ;idx++)
    {
      double val;
      ifield->get_value(val, idx);
      if ( val >= mmin && val <= mmax )
      {
        values.push_back( val );
        value += val;
        ++counter;
      }
    }

    min = mmin;
    max = mmax;
    mean = value/double(counter);
  }
  else
  {
    VField::size_type num_values = ifield->num_values();
    for (VField::index_type idx=0; idx < num_values ;idx++)
    {
      double val;
      ifield->get_value(val, idx);
      values.push_back( val );
      value += val;
      if ( !init )
      {
        min = max = val;
        init = true;
      }
      else
      {
        min = std::min(val, min);
        max = std::max(val, max);
      }
      ++counter;
    }
    mean = value/double(counter);
  }

  if ((max - min) > 1e-16 && values.size() > 0)
  {
    const int nbuckets = state->getValue(Parameters::HistogramBinCount).toInt();
    std::vector<double> hits;

    const double frac = nbuckets/(max - min);

    for (const auto v : values)
    {
      if( v >= min && v <= max)
      {
        const auto bin = std::floor((v - min)*frac);
        const auto histoVal = bin / frac + min;
        hits.push_back(histoVal);
      }
      sigma += (v - mean)*(v - mean);
    }
    sigma = sqrt( sigma / double(values.size()) );

    std::nth_element(values.begin(), values.begin() + values.size()/2, values.end());
    median = values[values.size()/2];
    auto hitsM = makeShared<DenseMatrix>(hits.size(), 1, 0);
    std::copy(hits.begin(), hits.end(), &(*hitsM)(0,0));
    sendOutput(HistogramData, hitsM);
  }
  else
  {
    warning("min - max less than precision or no values in range; clearing histogram");
    sendOutput(HistogramData, makeShared<DenseMatrix>(0,0,0));
  }

  state->setValue(Parameters::Mean, std::to_string(mean));
  state->setValue(Parameters::Median, std::to_string(median));
  state->setValue(Parameters::StandardDeviation, std::to_string(sigma));
}
