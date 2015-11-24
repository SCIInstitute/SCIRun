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


#ifndef CORE_ALGORITHMS_MATH_REPORTMATRIXSLICEMEASUREALGO_H
#define CORE_ALGORITHMS_MATH_REPORTMATRIXSLICEMEASUREALGO_H

//#include <Core/Algorithms/Util/AlgoLibrary.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>


#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Core/Algorithms/Math/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {
namespace Math {

//class SparseElement;
//typedef std::vector<SparseElement> SparseElementVector;

class SCISHARE ReportMatrixSliceMeasureAlgo : public AlgorithmBase
{

  public:
    ReportMatrixSliceMeasureAlgo();
    AlgorithmOutput run_generic(const AlgorithmInput& input) const;

    // Apply an operation on a row by row basis
    bool ApplyRowOperation(Datatypes::MatrixHandle input, Datatypes::MatrixHandle& output, int operation);

    // Apply an operation on a column by column basis
    bool ApplyColumnOperation(Datatypes::MatrixHandle input, Datatypes::MatrixHandle& output, int operation);

};





// helper classes
/*
class SparseElement {
public:
  index_type     row;
  index_type     col;
  double  val;
};

inline bool operator==(const SparseElement& s1,const SparseElement& s2)
{
  if ((s1.row == s2.row)&&(s1.col == s2.col)) return (true);
  return (false);
}    

inline bool operator<(const SparseElement& s1, const SparseElement& s2)
{
  if (s1.row < s2.row) return(true);
  if (s1.row == s2.row) if (s1.col < s2.col) return(true);
  return (false);
}
*/

      }}}}// end SCIRun namespace
#endif
