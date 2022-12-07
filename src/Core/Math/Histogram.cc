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
#include <limits>
#include <boost/algorithm/minmax_element.hpp>
#include <Core/Math/Histogram.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun::Core::Math;

namespace
{
  double Nan()
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
}

Histogram::Histogram()
{
  this->min_ = Nan();
  this->max_ = Nan();
  this->bin_start_ = Nan();
  this->bin_size_ = Nan();
  this->histogram_.resize( 0 );
}

Histogram::Histogram( const double* data, size_t size )
{
  this->compute( data, size );
}

// For char and short data we do a single pass over the data to speed up the computation. The
// first pass is used to generate the histogram. In the next step we use this data to compute
// min and max
bool Histogram::compute( const double* data, size_t size )
{
  this->min_ = Nan();
  this->max_ = Nan();
  this->bin_start_ = Nan();
  this->bin_size_ = Nan();
  this->histogram_.resize( 0 );
  if ( size == 0 )
    return false;

  try
  {
    this->min_ = std::numeric_limits<double>::max();
    this->max_ = std::numeric_limits<double>::min();

    for ( size_t j = 0 ; j < size ; j++ )
    {
      double val = data[ j ];
      if ( ! IsFinite( val ) ) continue;
      if ( val < this->min_ ) this->min_ = val;
      if ( val > this->max_ ) this->max_ = val;
    }

    if ( this->min_ > this->max_ )
    {
      // Most likely all the data is NaN
      this->min_ = Nan();
      this->max_ = Nan();
      this->bin_start_ = Nan();
      this->bin_size_ = Nan();
      this->histogram_.resize( 0 );
      return false;
    }

    if ( this->min_ == this->max_ )
    {
      this->bin_size_  = 1.0;
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );
      this->histogram_.resize( 1, 0 );
    }
    else
    {
      size_t hist_size = 0x100;
      this->histogram_.resize( hist_size, 0 );
      this->bin_size_ = ( this->max_ - this->min_ ) / static_cast<double>( hist_size - 1 );
      this->bin_start_ = this->min_ - ( this->bin_size_ * 0.5 );
    }

    double inv_bin_size = 1.0 / bin_size_;

    for ( size_t j = 1 ; j < size ; j++ )
    {
      double val = data[ j ];
      if ( IsFinite( val ) )
      {
        size_t idx = static_cast<size_t>( ( val - this->min_ ) * inv_bin_size );
        this->histogram_[ idx ]++;
      }
    }

    auto min_max = boost::minmax_element( this->histogram_.begin(), this->histogram_.end() );
    this->min_bin_ = (*min_max.first);
    this->max_bin_ = (*min_max.second);
  }
  catch( ... )
  {
    this->min_ = Nan();
    this->max_ = Nan();
    this->bin_start_ = Nan();
    this->bin_size_ = Nan();
    this->histogram_.resize( 0 );
    return false;
  }

  return true;
}

double Histogram::get_min() const
{
  return this->min_;
}

double Histogram::get_max() const
{
  return this->max_;
}

double Histogram::get_cum_value( double fraction ) const
{
  size_t tot_hist = 0;
  for ( size_t j = 0; j < this->histogram_.size(); j++ )
  {
    tot_hist += this->histogram_[ j ];
  }

  double multiplier = 1.0 / static_cast<double>( tot_hist );
  double jj = 0.0;
  size_t cur_hist = 0;

  for ( size_t j = 0; j < this->histogram_.size(); j++ )
  {
    double frac_start = static_cast<double>( cur_hist ) * multiplier;
    cur_hist += this->histogram_[ j ];
    double frac_end = static_cast<double>( cur_hist ) * multiplier;
    if ( fraction > frac_start && fraction <= frac_end )
    {
      jj = static_cast<double>( j ) + ( frac_end - fraction )/( frac_end - frac_start );
      break;
    }
  }

  return this->bin_start_ + ( jj * this->bin_size_ );
}

size_t Histogram::get_max_bin() const
{
  return this->max_bin_;
}

size_t Histogram::get_min_bin() const
{
  return this->min_bin_;
}

double Histogram::get_bin_size() const
{
  return this->bin_size_;
}

double Histogram::get_bin_start( size_t idx ) const
{
  return this->bin_start_ + idx * this->bin_size_;
}

double Histogram::get_bin_end( size_t idx ) const
{
  return this->bin_start_ + ( idx + 1 ) * this->bin_size_;
}

const std::vector<size_t>& Histogram::get_bins() const
{
  return this->histogram_;
}

size_t Histogram::get_size() const
{
  return this->histogram_.size();
}

bool Histogram::is_valid() const
{
  return !( IsNan( this->min_ ) || IsNan( this->max_ ) );
}
