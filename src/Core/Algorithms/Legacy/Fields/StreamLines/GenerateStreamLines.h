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

#ifndef CORE_ALGORITHMS_FIELDS_STREAMLINES_GENERATESTREAMLINES_H
#define CORE_ALGORITHMS_FIELDS_STREAMLINES_GENERATESTREAMLINES_H 1


/// Datatypes that the algorithm uses
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>

/// Base class for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

/// for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE GenerateStreamLinesAlgo : public AlgoBase 
{

  public:
    GenerateStreamLinesAlgo()
    {
      add_scalar("step_size",0.01);
      add_scalar("tolerance",0.0001);
      add_int("max_steps",100);
      add_int("direction",1);
      add_int("value",1);
      add_bool("remove_colinear_points",true);
      add_option("method","CellWalk","AdamsBashforth|Heun|RungeKutta|RungeKuttaFehlberg|CellWalk");
      // Estimate step size and tolerance automatically based on average edge length
      add_bool("auto_parameters",false);
      
      // For output
      add_int("num_streamlines",0);
    }
    
    /// Convert data into a matrix
    bool run(FieldHandle input, FieldHandle seeds, FieldHandle& output);
};

} // end namespace SCIRunAlgo

#endif
