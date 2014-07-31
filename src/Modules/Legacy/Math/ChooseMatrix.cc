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


///@file  ChooseMatrix.cc
///
///@author
///   David Weinstein
///   Department of Computer Science
///   University of Utah
///@date  November 1994

#include <Dataflow/Network/ChooseModule.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

namespace SCIRun {

/// @class ChooseMatrix
/// @brief This module takes in an arbitrary number of input matrices, and
/// passes one of them downstream. 

class ChooseMatrix : public ChooseModule< MatrixHandle > {
public:
  ChooseMatrix(GuiContext* ctx);
};

DECLARE_MAKER(ChooseMatrix)
ChooseMatrix::ChooseMatrix(GuiContext* ctx)
  : ChooseModule< MatrixHandle >("ChooseMatrix", ctx, Filter,
				 "Math", "SCIRun", "Matrix")
{
}

} // End namespace SCIRun

