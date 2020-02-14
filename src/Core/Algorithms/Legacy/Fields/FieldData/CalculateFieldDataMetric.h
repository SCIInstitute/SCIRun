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


#ifndef CORE_ALGORITHMS_FIELDS_FIELDDATA_CALCULATEFIELDDATAMETRIC_H
#define CORE_ALGORITHMS_FIELDS_FIELDDATA_CALCULATEFIELDDATAMETRIC_H 1

// Datatypes that the algorithm uses
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/NrrdData.h>

// Base class for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

// for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE CalculateFieldDataMetricAlgo : public AlgoBase
{
  public:
    /// Set defaults
    CalculateFieldDataMetricAlgo()
    {
      /// keep scalar type defines whether we convert to double or not
      add_option("method","value-mean","min|max|median|value-mean|geom-mean|sum|integral|volthreshold");
      add_scalar("threshold",0.5);
    }

    bool run(std::vector<FieldHandle>& input, MatrixHandle& output);
    bool run(FieldHandle input, MatrixHandle& output);
};

} // end namespace SCIRunAlgo

#endif
