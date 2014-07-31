/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#ifndef MODULES_MATH_APPENDMATRIX_H
#define MODULES_MATH_APPENDMATRIX_H

#include <Dataflow/Network/Module.h>
#include <Modules/Math/share.h>

namespace SCIRun {
namespace Modules {
namespace Math {

  /// @class AppendMatrixModule
  /// @brief Append the rows or columns of a matrix to a base matrix. 

  class SCISHARE AppendMatrixModule : public SCIRun::Dataflow::Networks::Module,
    public Has3InputPorts<MatrixPortTag, MatrixPortTag, DynamicPortTag<MatrixPortTag>>,
    public Has1OutputPort<MatrixPortTag>
  {
  public:
    AppendMatrixModule();
    virtual void execute();
    virtual void setStateDefaults();
    virtual bool hasDynamicPorts() const { return true; }

    INPUT_PORT(0, FirstMatrix, DenseMatrix);
    INPUT_PORT(1, SecondMatrix, DenseMatrix);
    INPUT_PORT_DYNAMIC(2, NextMatrix, DenseMatrix);
    OUTPUT_PORT(0, ResultMatrix, DenseMatrix);
  };
}}}

#endif
