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


#ifndef CORE_DATABLOCK_HISTOGRAM_H
#define CORE_DATABLOCK_HISTOGRAM_H

#include <vector>
#include <cstddef>
#include <Core/Math/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Math
    {

      class SCISHARE Histogram
      {
      public:

        Histogram();
        Histogram(const double* data, size_t size);

        bool compute(const double* data, size_t size);

        double get_min() const;
        double get_max() const;

        // GET_CUM_VALUE:
        /// Get the value that has a certain fraction of the data be smaller values
        double get_cum_value(double fraction) const;

        // GET_MIN_BIN:
        /// Get the minimum size of a histogram bar
        size_t get_min_bin() const;

        // GET_MAX_BIN:
        /// Get the maximum size of a histogram bar
        size_t get_max_bin() const;

        // GET_BIN_SIZE:
        /// Get the size of each bin in data values
        double get_bin_size() const;

        // GET_BIN_START:
        /// Get the value where a specific bin starts
        double get_bin_start(size_t idx = 0) const;

        // GET_BIN_END:
        /// Get the value where a specific bin ends
        double get_bin_end(size_t idx = 0) const;

        // GET_SIZE:
        // Get the number of bin in the histogram
        size_t get_size() const;

        // GET_BINS:
        /// Get the actual histogram data
        const std::vector<size_t>& get_bins() const;

        bool is_valid() const;

      private:
        double min_;
        double max_;

        size_t min_bin_;
        size_t max_bin_;

        double bin_start_;
        double bin_size_;

        std::vector<size_t> histogram_;

      };
    }
  }
}

#endif
