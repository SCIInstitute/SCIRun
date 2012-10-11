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

#ifndef CORE_ALGORITHMS_MATH_LINEARSYSTEM_SOLVELINEARSYSTEM_H
#define CORE_ALGORITHMS_MATH_LINEARSYSTEM_SOLVELINEARSYSTEM_H 1

//! Datatypes that the algorithm uses
#include <Core/Datatypes/Matrix.h>

//! Base class for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

//! for Windows support
#include <Core/Algorithms/Math/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

// Solve a linear system in parallel using a standard iterative method
// Method solves A*x = b, with x0 being the initializer for the solution

class SCISHARE SolveLinearSystemAlgo : public AlgoBase
{
  public:
    // Set default values
    SolveLinearSystemAlgo()
    {
      // For solver
      add_option("method","cg","jacobi|cg|bicg|minres");
      add_option("pre_conditioner","jacobi","none|jacobi");
      add_scalar("target_error",1e-6);
      add_int("max_iterations",300);
      
      add_bool("build_convergence",true);
      
      // for callback
      // Read this variable to find the error at the start of the iteration
      add_scalar("original_error",0.0);
      // Read this variable to find current error
      add_scalar("current_error",0.0);
      // Read this variable to find current iteration
      add_int("iteration",0);
      // How often the algorithm should go to a callback
      add_int("callback_step",10);
      
      add_handle("solution");
      add_handle("convergence");
    }
  
    bool run(MatrixHandle A,
             MatrixHandle b,
             MatrixHandle x0, 
             MatrixHandle& x,
             MatrixHandle& convergence);

    bool run(MatrixHandle A,
             MatrixHandle b,
             MatrixHandle x0, 
             MatrixHandle& x);
};


}

#endif
