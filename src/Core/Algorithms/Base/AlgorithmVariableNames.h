/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef ALGORITHMS_BASE_ALGORITHMVARIABLENAMES_H
#define ALGORITHMS_BASE_ALGORITHMVARIABLENAMES_H

#include <Core/Algorithms/Base/Name.h>
#include <Core/Algorithms/Base/share.h>

namespace SCIRun {
namespace Core {
namespace Algorithms {

class SCISHARE Variables
{
public:
  static const AlgorithmParameterName RowsOrColumns;
  static const AlgorithmParameterName Operator;
  static const AlgorithmParameterName ScalarValue;
  static const AlgorithmParameterName TargetError;
  static const AlgorithmParameterName MaxIterations;
  static const AlgorithmParameterName Method;
  static const AlgorithmParameterName Preconditioner;
  static const AlgorithmParameterName Filename;
  static const AlgorithmParameterName BuildConvergence;
  static const AlgorithmParameterName FileTypeList;
  static const AlgorithmParameterName FileExtension;
  static const AlgorithmParameterName FileTypeName;
  static const AlgorithmParameterName FormatString;
	static const AlgorithmParameterName FunctionString;
  static const AlgorithmParameterName ObjectInfo; 

  static const AlgorithmInputName InputMatrix;
  static const AlgorithmInputName FirstMatrix;
  static const AlgorithmInputName SecondMatrix;
  static const AlgorithmInputName LHS;
  static const AlgorithmInputName RHS;
  static const AlgorithmInputName MatrixToWrite;
  static const AlgorithmInputName InputField;
  static const AlgorithmInputName ObjectField;
  static const AlgorithmInputName ListOfInputFields;
  static const AlgorithmInputName InputFields;
  static const AlgorithmInputName Source;
  static const AlgorithmInputName Destination;

  static const AlgorithmOutputName MatrixInfo;
  static const AlgorithmOutputName Result;
  static const AlgorithmOutputName ResultMatrix;
  static const AlgorithmOutputName MatrixLoaded;
  static const AlgorithmOutputName Solution;
  static const AlgorithmOutputName OutputField;
  static const AlgorithmOutputName OutputMatrix;
  static const AlgorithmOutputName ListOfOutputFields;
};

}}}

#endif
