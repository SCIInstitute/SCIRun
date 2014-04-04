/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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


#ifndef CORE_ALGORITHMS_FIELDS_MAPFIELDDATAFROMNODETOELEM_H
#define CORE_ALGORITHMS_FIELDS_MAPFIELDDATAFROMNODETOELEM_H 1

/// STL classes to include
#include <algorithm>

/// SCIRun Datatypes to use
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>

/// Base class for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

/// for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE MapFieldDataFromNodeToElemAlgo : public AlgoBase
{
  public:

    /// Set default parameters
    MapFieldDataFromNodeToElemAlgo()
    {
      /// Add option for how to do map data from nodes to element
      add_option("method","average","interpolate|average|min|max|sum|median|mostcommon");
    }

    /// Functions
    bool run(FieldHandle& input,FieldHandle& output);
};

} // namespace SCIRunAlgo

#endif
