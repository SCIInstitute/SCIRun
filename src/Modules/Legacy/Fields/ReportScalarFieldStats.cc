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
#include <Core/Algorithms/Legacy/Fields/RegisterWithCorrespondences.h>
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

/// @class ReportScalarFieldStats
/// @brief Analyze data from a scalarfield.

namespace SCIRun::Modules::Fields {
class ReportScalarFieldStatsImpl
{
  public:
    explicit ReportScalarFieldStatsImpl(ReportScalarFieldStats* module) : module_(module) {}

    void fill_histogram(const std::vector<int>& hits);
    void clear_histogram() {}

    ReportScalarFieldStats* module_;
    double min_;
    double max_;
    double mean_;
    double median_;
    double sigma_;   //standard deviation
    int is_fixed_;
    int nbuckets_ {256};
};
}


MODULE_INFO_DEF(ReportScalarFieldStats, MiscField, SCIRun)

ReportScalarFieldStats::ReportScalarFieldStats()
  : Module(staticInfo_)
    // min_(get_ctx()->subVar("min"), 0.0),
    // max_(get_ctx()->subVar("max"), 0.0),
    // mean_(get_ctx()->subVar("mean"), 0.0),
    // median_(get_ctx()->subVar("median"), 0.0),
    // sigma_(get_ctx()->subVar("sigma"),0.0),
    // is_fixed_(get_ctx()->subVar("is_fixed"), 0),
    // nbuckets_(get_ctx()->subVar("nbuckets"), 256)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(HistogramData);
}



void ReportScalarFieldStatsImpl::fill_histogram(const std::vector<int>& hits)
{
  std::ostringstream ostr;
  int nmin, nmax;
  auto it = hits.begin();
  nmin = 0;  nmax = *it;
  ostr << *it;  ++it;

  for(; it != hits.end(); ++it)
  {
    ostr <<" "<<*it;
    nmin = ((nmin < *it) ? nmin : *it );
    nmax = ((nmax > *it) ? nmax : *it );
  }
  ostr << std::ends;
  std::string data = ostr.str();

  // TCLInterface::execute();
  module_->remark(" graph_data " + std::to_string(nmin) + " " + std::to_string(nmax) + " " + data );
}

ReportScalarFieldStats::~ReportScalarFieldStats() = default;

void ReportScalarFieldStats::setStateDefaults()
{

}

void ReportScalarFieldStats::execute()
{
  auto inputField = getRequiredInput(InputField);

  if (!(inputField->vfield()->is_scalar()))
  {
    error("This module only works on scalar fields.");
    return;
  }

  //update_state(Executing);

  VField* ifield = inputField->vfield();

  bool init = false;
  double value = 0;
  double min = 0;
  double max = 0;
  int counter = 0;
  std::vector<double> values;

  //update_progress(0.3);
  double mean = 0;
  double mmin = 0;
  double mmax = 0;

  if ( impl_->is_fixed_ == 1 )
  {
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

    mean = value/double(counter);
    impl_->mean_ = mean;
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
        min = (val < min) ? val:min;
        max = (val > max) ? val:max;
      }
      ++counter;
    }
    mean = value/double(counter);
    impl_->mean_ = mean;

    impl_->min_ = min;
    impl_->max_ = max;
  }

  //update_progress(0.6);

  if ((impl_->max_ - impl_->min_) > 1e-16 && values.size() > 0)
  {
    int nbuckets = impl_->nbuckets_;
    std::vector<int> hits(nbuckets, 0);

    double frac = 1.0;
    frac = (nbuckets-1)/(impl_->max_ - impl_->min_);

    double sigma = 0.0;
    auto vit = values.begin();
    auto vit_end = values.end();
    for(; vit != vit_end; ++vit)
    {
      if( *vit >= impl_->min_ && *vit <= impl_->max_)
      {
        double value = (*vit - impl_->min_)*frac;
        hits[int(value)]++;
      }
      sigma += (*vit - mean)*(*vit - mean);
    }
    impl_->sigma_ = sqrt( sigma / double(values.size()) );

    vit = values.begin();
    nth_element(vit, vit+values.size()/2, vit_end);
    impl_->median_ = values[values.size()/2];
    impl_->fill_histogram(hits);
  }
  else
  {
    warning("min - max < precision or no values in range; clearing histogram");
    impl_->clear_histogram();
  }
  auto state = get_state();
  state->setValue(Parameters::Mean, impl_->mean_);
  state->setValue(Parameters::Median, impl_->median_);
  state->setValue(Parameters::StandardDeviation, impl_->sigma_);
}
