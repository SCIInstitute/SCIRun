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

#ifndef CORE_ALGORITHMS_MATH_LINEARSYSTEM_ADDKNOWNSTOLINEARSYSTEM_H
#define CORE_ALGORITHMS_MATH_LINEARSYSTEM_ADDKNOWNSTOLINEARSYSTEM_H 1

// Datatypes that the algorithm uses
#include <Core/Datatypes/Matrix.h>

// Base class for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

// for Windows support
#include <Core/Algorithms/Math/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

// Add knowns to a linear system, set the knowns in x by giving them a value
// all unknowns should be marked with a NaN. This algorithm will take a linear
// system comprised of matrices A and b and set all the values that are specified
// in x to that value and will change the linear system so that these values
// are set.
//
// If matrix A is not sparse, set convert_matrix_types to true to convert
// to sparse row matrix type.

class SCISHARE AddKnownsToLinearSystemAlgo : public AlgoBase
{
  public:
    // Set default values
    AddKnownsToLinearSystemAlgo()
    {}
  
    bool run(MatrixHandle a_in, 
             MatrixHandle b_in,
             MatrixHandle x, 
             SparseRowMatrixHandle& a_out, 
             MatrixHandle& b_out,
             bool convert_matrix_types = false);
};


}

#endif
