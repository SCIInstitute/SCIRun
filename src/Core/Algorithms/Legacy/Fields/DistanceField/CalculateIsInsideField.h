/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef CORE_ALGORITHMS_FIELDS_DISTANCEFIELD_CALCULATEISINSIDEFIELD_H
#define CORE_ALGORITHMS_FIELDS_DISTANCEFIELD_CALCULATEISINSIDEFIELD_H 1

// Datatypes that the algorithm uses
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>

// Base class for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

// for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE CalculateIsInsideFieldAlgo : public AlgoBase 
{
  public:
    CalculateIsInsideFieldAlgo() 
    {
      // How many samples inside the elements to test for being inside the
      // object field
      add_option("sampling_scheme","regular2","regular1|regular2|regular3|regular4|regular5");
      // Value for inside elements
      add_scalar("inside_value",1.0);
      // Value for outside elements
      add_scalar("outside_value",0.0);
      // Output type of the algorithm
      add_option("output_type","same as input","same as input|char|unsigned char|short|unsigned short|int|unsigned int|float|double");
      // How many nodes need to be inside to call it inside
      add_option("method","all","one|most|all");
    }
    
    /// run the algorithm
    bool run(FieldHandle input, FieldHandle object, FieldHandle& output);
};

} // end namespace SCIRunAlgo

#endif
